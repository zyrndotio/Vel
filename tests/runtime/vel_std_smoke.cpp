#include "vel_std.h"

#include <cassert>
#include <cstdlib>
#include <filesystem>
#include <string>

int main()
{
    VelBuffer buffer{};
    assert(vel_env_current_dir(&buffer) == VEL_STD_OK);
    assert(buffer.length > 0);
    vel_buffer_free(&buffer);

#if defined(_WIN32)
    _putenv_s("VEL_STD_TEST", "ready");
#else
    setenv("VEL_STD_TEST", "ready", 1);
#endif
    assert(vel_env_get("VEL_STD_TEST", &buffer) == VEL_STD_OK);
    assert(std::string(reinterpret_cast<char*>(buffer.data), buffer.length) == "ready");
    vel_buffer_free(&buffer);

    const auto path = (std::filesystem::temp_directory_path() / "vel-std-smoke.txt").string();
    const unsigned char text[] = "Vel host shim";
    assert(vel_fs_write_text(path.c_str(), text, sizeof(text) - 1) == VEL_STD_OK);
    int exists = 0;
    assert(vel_fs_exists(path.c_str(), &exists) == VEL_STD_OK && exists == 1);
    assert(vel_fs_read_text(path.c_str(), &buffer) == VEL_STD_OK);
    assert(std::string(reinterpret_cast<char*>(buffer.data), buffer.length) == "Vel host shim");
    vel_buffer_free(&buffer);
    std::filesystem::remove(path);

    int exit_code = -1;
#if defined(_WIN32)
    assert(vel_process_run("exit /b 0", &exit_code) == VEL_STD_OK);
#else
    assert(vel_process_run("true", &exit_code) == VEL_STD_OK);
#endif
    assert(exit_code == 0);
    return 0;
}
