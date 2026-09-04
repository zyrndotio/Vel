#include <cstdlib>
#include <algorithm>
#include <filesystem>
#include <cstdio>
#include <fstream>
#include <iostream>
#include <regex>
#include <sstream>
#include <string>
#include <vector>

#include "arena.hpp"
#include "codegen.hpp"
#include "parser.hpp"
#include "tokenizer.hpp"
#include "type_checker.hpp"

namespace fs = std::filesystem;

static std::string read_file(const std::string& path)
{
    std::ifstream f(path);
    if (!f.is_open()) {
        std::cerr << "[Vel] Cannot open file: " << path << "\n";
        exit(EXIT_FAILURE);
    }
    std::ostringstream ss;
    ss << f.rdbuf();
    return ss.str();
}

static void usage()
{
    std::cerr << "Vel Programming Language Compiler\n";
    std::cerr << "Usage:\n";
    std::cerr << "  vel <file.vel>            compile and run for the host target\n";
    std::cerr << "  vel build <file.vel>      compile to native binary for the host target\n";
    std::cerr << "  vel new <project>         create a starter Vel project\n";
    std::cerr << "  vel run <file.vel>       compile and run for the host target\n";
    std::cerr << "  vel test [path]          check all .vel files in a path\n";
    std::cerr << "  vel check <file.vel>      tokenize and parse without native tools\n";
    std::cerr << "  vel asm   <file.vel> [target] emit target assembly\n";
    std::cerr << "                             targets: linux-x86_64, macos-x86_64, windows-x86_64\n";
    std::cerr << "  vel tokens <file.vel>     print token stream (debug)\n";
    std::cerr << "  vel doctor               inspect native tool availability\n";
    std::cerr << "  vel clean <file.vel>     remove generated artifacts\n";
    std::cerr << "  vel update [--download]  check or download a verified release\n";
    std::cerr << "  vel version               print version\n";
    std::cerr << "  vel help                  show this help\n";
}

static CodeGenTarget host_target()
{
#if defined(_WIN32) && defined(_M_X64)
    return CodeGenTarget::WindowsX86_64;
#elif defined(__APPLE__) && defined(__x86_64__)
    return CodeGenTarget::MacOSX86_64;
#else
    return CodeGenTarget::LinuxX86_64;
#endif
}

static bool native_backend_available(CodeGenTarget target)
{
#if defined(__linux__) && defined(__x86_64__)
    return target == CodeGenTarget::LinuxX86_64;
#elif defined(__APPLE__) && defined(__x86_64__)
    return target == CodeGenTarget::MacOSX86_64;
#elif defined(_WIN32) && defined(_M_X64)
    return target == CodeGenTarget::WindowsX86_64;
#else
    (void)target;
    return false;
#endif
}

static std::optional<CodeGenTarget> parse_target(const std::string& name)
{
    if (name == "linux-x86_64") return CodeGenTarget::LinuxX86_64;
    if (name == "macos-x86_64") return CodeGenTarget::MacOSX86_64;
    if (name == "windows-x86_64") return CodeGenTarget::WindowsX86_64;
    return {};
}

static std::vector<Token> run_tokenizer(const std::string& src)
{
    Tokenizer tok(src);
    return tok.tokenize();
}

static Program run_parser(std::vector<Token> tokens, Arena& arena)
{
    Parser p(std::move(tokens), arena);
    return p.parse();
}

static void run_type_checker(const Program& prog)
{
    TypeChecker checker(prog);
    checker.check();
}

static std::string run_codegen(Program prog, CodeGenTarget target = host_target())
{
    CodeGen gen(std::move(prog), target);
    return gen.generate();
}

static std::string shell_quote(const std::string& value)
{
#if defined(_WIN32)
    std::string quoted = "\\\"";
    for (char c : value) {
        if (c == '\"') quoted += "\\\\\"";
        else quoted += c;
    }
    return quoted + "\\\"";
#else
    std::string quoted = "'";
    for (char c : value) {
        if (c == '\'') quoted += "'\\''";
        else quoted += c;
    }
    return quoted + "'";
#endif
}

static std::string compile(const std::string& vel_path, bool verbose = false)
{
    CodeGenTarget target = host_target();
    if (!native_backend_available(target)) {
        std::cerr << "[Vel] No native backend is available for this host.\n"
                  << "[Vel] Use 'vel check' or 'vel asm' instead.\n";
        exit(EXIT_FAILURE);
    }

    fs::path in(vel_path);
    if (!in.has_extension() || in.extension() != ".vel") {
        std::cerr << "[Vel] Input file must have .vel extension\n";
        exit(EXIT_FAILURE);
    }

    fs::path out_dir = in.parent_path().empty() ? fs::current_path() : in.parent_path();
    std::string stem = in.stem().string();
    fs::path asm_path = out_dir / (stem + ".asm");
    fs::path obj_path = out_dir / (stem + ".o");
    fs::path bin_path = out_dir / stem;
    std::string asm_out = asm_path.string();
    const char* object_format = target == CodeGenTarget::MacOSX86_64 ? "macho64"
        : target == CodeGenTarget::WindowsX86_64 ? "win64" : "elf64";
    std::string obj_out = obj_path.string();
    std::string bin_out = bin_path.string();

    std::string src = read_file(vel_path);

    Arena arena(1024 * 1024 * 8);

    if (verbose) std::cerr << "[Vel] Tokenizing...\n";
    auto tokens = run_tokenizer(src);

    if (verbose) std::cerr << "[Vel] Parsing...\n";
    auto prog = run_parser(std::move(tokens), arena);

    if (verbose) std::cerr << "[Vel] Type checking...\n";
    run_type_checker(prog);
    if (verbose) std::cerr << "[Vel] Generating assembly...\n";
    std::string asm_code = run_codegen(std::move(prog), target);

    {
        std::ofstream f(asm_out);
        f << asm_code;
    }

    if (verbose) std::cerr << "[Vel] Assembling with NASM...\n";
    std::string nasm_cmd = "nasm -f " + std::string(object_format) + " " + shell_quote(asm_out) + " -o " + shell_quote(obj_out);
    if (system(nasm_cmd.c_str()) != 0) {
        std::cerr << "[Vel] Assembly failed\n";
        exit(EXIT_FAILURE);
    }

    if (verbose) std::cerr << "[Vel] Linking...\n";
    std::string ld_cmd;
    if (target == CodeGenTarget::MacOSX86_64) {
        ld_cmd = "ld -arch x86_64 -macosx_version_min 10.15 -e _start -o " + shell_quote(bin_out) + " " + shell_quote(obj_out);
    } else if (target == CodeGenTarget::WindowsX86_64) {
#if defined(_WIN32)
        ld_cmd = "gcc -o " + shell_quote(bin_out + ".exe") + " " + shell_quote(obj_out) + " -lkernel32";
#else
        ld_cmd = "x86_64-w64-mingw32-gcc -o " + shell_quote(bin_out + ".exe") + " " + shell_quote(obj_out) + " -lkernel32";
#endif
    } else {
        ld_cmd = "ld -o " + shell_quote(bin_out) + " " + shell_quote(obj_out);
    }
    if (system(ld_cmd.c_str()) != 0) {
        std::cerr << "[Vel] Linking failed\n";
        exit(EXIT_FAILURE);
    }

    std::remove(asm_out.c_str());
    std::remove(obj_out.c_str());

    return bin_out;
}

static std::string capture_command(const std::string& command)
{
    std::string output;
#if defined(_WIN32)
    FILE* pipe = _popen(command.c_str(), "r");
#else
    FILE* pipe = popen(command.c_str(), "r");
#endif
    if (!pipe) return {};
    char buffer[4096];
    while (fgets(buffer, sizeof(buffer), pipe)) output += buffer;
#if defined(_WIN32)
    _pclose(pipe);
#else
    pclose(pipe);
#endif
    return output;
}

static std::string release_asset_name(CodeGenTarget target, const std::string& tag)
{
    if (target == CodeGenTarget::WindowsX86_64) return "vel-" + tag + "-windows-x86_64.zip";
    if (target == CodeGenTarget::MacOSX86_64) return "vel-" + tag + "-macos-x86_64.tar.gz";
    return "vel-" + tag + "-linux-x86_64.tar.gz";
}

static int update_release(bool download)
{
    const std::string api = "https://api.github.com/repos/zyrndotio/Vel/releases/latest";
    const std::string json = capture_command("curl -fsSL -H " + shell_quote("Accept: application/vnd.github+json") + " " + shell_quote(api));
    if (json.empty()) {
        std::cerr << "[Vel] Could not query the latest GitHub release. Install curl or check your network.\n";
        return EXIT_FAILURE;
    }
    std::smatch tag_match;
    if (!std::regex_search(json, tag_match, std::regex("\\\"tag_name\\\"\\s*:\\s*\\\"([^\\\"]+)\\\""))) {
        std::cerr << "[Vel] GitHub returned no published release.\n";
        return EXIT_FAILURE;
    }
    const auto tag = tag_match[1].str();
    const auto asset = release_asset_name(host_target(), tag);
    std::regex asset_re("\\\"name\\\"\\s*:\\s*\\\"" + asset + "\\\"[\\s\\S]*?\\\"digest\\\"\\s*:\\s*\\\"sha256:([0-9a-fA-F]+)\\\"[\\s\\S]*?\\\"browser_download_url\\\"\\s*:\\s*\\\"([^\\\"]+)\\\"");
    std::smatch asset_match;
    if (!std::regex_search(json, asset_match, asset_re)) {
        std::cerr << "[Vel] No release asset is available for this host: " << asset << "\n";
        return EXIT_FAILURE;
    }
    std::cout << "Latest release: " << tag << "\n";
    if (tag == "v0.3.0" && !download) {
        std::cout << "[Vel] Already up to date.\n";
        return EXIT_SUCCESS;
    }
    if (!download) {
        std::cout << "[Vel] Update available for " << asset << ". Use `vel update --download` to download it.\n";
        return EXIT_SUCCESS;
    }
    const auto destination = fs::current_path() / asset;
    const auto url = asset_match[2].str();
    if (std::system(("curl -fL --retry 3 -o " + shell_quote(destination.string()) + " " + shell_quote(url)).c_str()) != 0) {
        std::cerr << "[Vel] Release download failed.\n";
        return EXIT_FAILURE;
    }
#if defined(_WIN32)
    const auto digest = capture_command("certutil -hashfile " + shell_quote(destination.string()) + " SHA256");
#else
    const auto digest = capture_command("sha256sum " + shell_quote(destination.string()) + " 2>/dev/null || shasum -a 256 " + shell_quote(destination.string()));
#endif
    std::regex hex_re(R"(([0-9a-fA-F]{64}))");
    std::smatch digest_match;
    if (!std::regex_search(digest, digest_match, hex_re) || digest_match[1].str() != asset_match[1].str()) {
        fs::remove(destination);
        std::cerr << "[Vel] Checksum verification failed; downloaded file was removed.\n";
        return EXIT_FAILURE;
    }
    std::cout << "[Vel] Verified update downloaded to: " << destination << "\n";
    std::cout << "[Vel] The archive was not installed automatically. Review it, then replace Vel using the platform installer/package.\n";
    return EXIT_SUCCESS;
}

static int tool_available(const char* tool)
{
#if defined(_WIN32)
    std::string command = "where " + std::string(tool) + " >NUL 2>NUL";
#else
    std::string command = "command -v " + std::string(tool) + " >/dev/null 2>&1";
#endif
    return std::system(command.c_str()) == 0 ? 0 : 1;
}

static bool create_project(const fs::path& root)
{
    if (fs::exists(root)) {
        std::cerr << "[Vel] Project path already exists: " << root << "\n";
        return false;
    }
    fs::create_directories(root / "src");
    std::ofstream(root / "vel.toml") << "name = \"" << root.filename().string() << "\"\nversion = \"0.3.0\"\nentry = \"src/main.vel\"\n";
    std::ofstream(root / "src/main.vel") << "fn main() {\n    print \"Hello from Vel\";\n}\n\nmain();\n";
    std::ofstream(root / "README.md") << "# " << root.filename().string() << "\n\nA Vel project.\n\nRun with `vel run src/main.vel`.\n";
    std::cout << "[Vel] Created project: " << root << "\n";
    return true;
}

static void clean_artifacts(const fs::path& input)
{
    fs::path dir = input.parent_path().empty() ? fs::current_path() : input.parent_path();
    auto stem = input.stem().string();
    for (const auto& suffix : {".asm", ".o", ""}) {
        auto path = dir / (stem + suffix);
        if (fs::exists(path) && fs::is_regular_file(path)) fs::remove(path);
#if defined(_WIN32)
        auto exe = dir / (stem + ".exe");
        if (fs::exists(exe)) fs::remove(exe);
#endif
    }
}

static int test_sources(const fs::path& root)
{
    std::vector<fs::path> sources;
    if (fs::is_regular_file(root)) {
        if (root.extension() == ".vel") sources.push_back(root);
    } else if (fs::is_directory(root)) {
        for (const auto& entry : fs::recursive_directory_iterator(root)) {
            if (entry.is_regular_file() && entry.path().extension() == ".vel")
                sources.push_back(entry.path());
        }
    }
    std::sort(sources.begin(), sources.end());
    if (sources.empty()) {
        std::cerr << "[Vel] No .vel test sources found in: " << root << "\n";
        return EXIT_FAILURE;
    }

    for (const auto& source : sources) {
        Arena arena(1024 * 1024 * 8);
        auto prog = run_parser(run_tokenizer(read_file(source.string())), arena);
        run_type_checker(prog);
        std::cout << "[Vel] PASS: " << source.string() << "\n";
    }
    std::cout << "[Vel] Tested " << sources.size() << " source file(s).\n";
    return EXIT_SUCCESS;
}

int main(int argc, char* argv[])
{
    if (argc < 2) {
        usage();
        return EXIT_FAILURE;
    }

    std::string cmd = argv[1];

    if (cmd == "help" || cmd == "--help" || cmd == "-h") {
        usage();
        return EXIT_SUCCESS;
    }

    if (cmd == "new" && argc >= 3) {
        return create_project(fs::path(argv[2])) ? EXIT_SUCCESS : EXIT_FAILURE;
    }

    if (cmd == "update") {
        bool download = argc >= 3 && std::string(argv[2]) == "--download";
        return update_release(download);
    }

    if (cmd == "doctor") {
        std::cout << "Vel doctor\n";
        for (const char* tool : {"cmake", "nasm"})
            std::cout << "  " << tool << ": " << (tool_available(tool) == 0 ? "available" : "missing") << "\n";
        std::cout << "  host native backend: " << (native_backend_available(host_target()) ? "available" : "unavailable") << "\n";
        return EXIT_SUCCESS;
    }

    if (cmd == "version") {
        std::cout << "Vel 0.3.0\n";
        std::cout << "Frontend: portable C++23\n";
        std::cout << "Native backends: Linux x86-64, macOS x86-64, Windows x86-64\n";
        std::cout << "Host backend: " << (native_backend_available(host_target()) ? "available" : "unavailable") << "\n";
        return EXIT_SUCCESS;
    }

    if (cmd == "tokens" && argc >= 3) {
        auto src    = read_file(argv[2]);
        auto tokens = run_tokenizer(src);
        for (auto& t : tokens) {
            std::cout << "["  << t.line << ":" << t.col << "] "
                      << tt_str(t.type);
            if (t.value) std::cout << " = " << *t.value;
            std::cout << "\n";
        }
        return EXIT_SUCCESS;
    }

    if (cmd == "check" && argc >= 3) {
        auto src = read_file(argv[2]);
        Arena arena(1024 * 1024 * 8);
        auto prog = run_parser(run_tokenizer(src), arena);
        run_type_checker(prog);
        std::cout << "[Vel] OK: " << argv[2] << "\n";
        return EXIT_SUCCESS;
    }

    if (cmd == "test") {
        fs::path root = argc >= 3 ? fs::path(argv[2]) : fs::current_path() / "tests";
        return test_sources(root);
    }

    if (cmd == "asm" && argc >= 3) {
        Arena arena(1024 * 1024 * 8);
        auto src  = read_file(argv[2]);
        auto toks = run_tokenizer(src);
        auto prog = run_parser(std::move(toks), arena);
        run_type_checker(prog);
        CodeGenTarget target = host_target();
        if (argc >= 4) {
            auto parsed = parse_target(argv[3]);
            if (!parsed) {
                std::cerr << "[Vel] Unknown target: " << argv[3] << "\n";
                return EXIT_FAILURE;
            }
            target = *parsed;
        }
        auto asm_ = run_codegen(std::move(prog), target);
        std::cout << asm_;
        return EXIT_SUCCESS;
    }

    if ((cmd == "build" || cmd == "run") && argc >= 3) {
        auto bin = compile(argv[2], /*verbose=*/true);
        if (cmd == "run") {
            int ret = system(shell_quote(bin).c_str());
            std::remove(bin.c_str());
            return ret;
        }
        std::cout << "[Vel] Built: " << bin << "\n";
        return EXIT_SUCCESS;
    }

    if (cmd == "clean" && argc >= 3) {
        clean_artifacts(fs::path(argv[2]));
        std::cout << "[Vel] Cleaned generated artifacts for " << argv[2] << "\n";
        return EXIT_SUCCESS;
    }

    if (argc == 2 && cmd.size() >= 4 && cmd.compare(cmd.size() - 4, 4, ".vel") == 0) {
        auto bin = compile(cmd);
        int ret = system(shell_quote(bin).c_str());
        std::remove(bin.c_str());
        return ret;
    }

    usage();
    return EXIT_FAILURE;
}
