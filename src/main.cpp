#include <filesystem>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

#include "arena.hpp"
#include "codegen.hpp"
#include "parser.hpp"
#include "tokenizer.hpp"

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
    std::cerr << "  vel <file.vel>            compile and run\n";
    std::cerr << "  vel build <file.vel>      compile to native binary\n";
    std::cerr << "  vel asm   <file.vel>      emit assembly only\n";
    std::cerr << "  vel tokens <file.vel>     print token stream (debug)\n";
    std::cerr << "  vel version               print version\n";
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

static std::string run_codegen(Program prog)
{
    CodeGen gen(std::move(prog));
    return gen.generate();
}

static std::string compile(const std::string& vel_path, bool verbose = false)
{
    fs::path in(vel_path);
    if (!in.has_extension() || in.extension() != ".vel") {
        std::cerr << "[Vel] Input file must have .vel extension\n";
        exit(EXIT_FAILURE);
    }

    std::string stem    = in.stem().string();
    std::string asm_out = stem + ".asm";
    std::string obj_out = stem + ".o";
    std::string bin_out = stem;

    std::string src = read_file(vel_path);

    Arena arena(1024 * 1024 * 8);

    if (verbose) std::cerr << "[Vel] Tokenizing...\n";
    auto tokens = run_tokenizer(src);

    if (verbose) std::cerr << "[Vel] Parsing...\n";
    auto prog = run_parser(std::move(tokens), arena);

    if (verbose) std::cerr << "[Vel] Generating assembly...\n";
    std::string asm_code = run_codegen(std::move(prog));

    {
        std::ofstream f(asm_out);
        f << asm_code;
    }

    if (verbose) std::cerr << "[Vel] Assembling with NASM...\n";
    std::string nasm_cmd = "nasm -f elf64 " + asm_out + " -o " + obj_out;
    if (system(nasm_cmd.c_str()) != 0) {
        std::cerr << "[Vel] Assembly failed\n";
        exit(EXIT_FAILURE);
    }

    if (verbose) std::cerr << "[Vel] Linking...\n";
    std::string ld_cmd = "ld -o " + bin_out + " " + obj_out;
    if (system(ld_cmd.c_str()) != 0) {
        std::cerr << "[Vel] Linking failed\n";
        exit(EXIT_FAILURE);
    }

    std::remove(asm_out.c_str());
    std::remove(obj_out.c_str());

    return bin_out;
}

int main(int argc, char* argv[])
{
    if (argc < 2) {
        usage();
        return EXIT_FAILURE;
    }

    std::string cmd = argv[1];

    if (cmd == "version") {
        std::cout << "Vel 0.1.0 — Early Build\n";
        std::cout << "Target: x86-64 Linux (Windows/macOS cross-compile coming)\n";
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

    if (cmd == "asm" && argc >= 3) {
        Arena arena(1024 * 1024 * 8);
        auto src  = read_file(argv[2]);
        auto toks = run_tokenizer(src);
        auto prog = run_parser(std::move(toks), arena);
        auto asm_ = run_codegen(std::move(prog));
        std::cout << asm_;
        return EXIT_SUCCESS;
    }

    if (cmd == "build" && argc >= 3) {
        auto bin = compile(argv[2], /*verbose=*/true);
        std::cout << "[Vel] Built: ./" << bin << "\n";
        return EXIT_SUCCESS;
    }

    if (argc == 2 && cmd.size() >= 4 && cmd.compare(cmd.size() - 4, 4, ".vel") == 0) {
        auto bin = compile(cmd);
        int ret  = system(("./" + bin).c_str());
        std::remove(bin.c_str());
        return ret;
    }

    usage();
    return EXIT_FAILURE;
}
