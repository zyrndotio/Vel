#include "vel_std.h"

#include <cstdlib>
#include <cstring>
#include <filesystem>
#include <fstream>
#include <iterator>
#include <string>
#include <system_error>
#include <vector>

namespace fs = std::filesystem;

namespace {

int copy_string(const std::string& value, VelBuffer* out)
{
    if (!out) return VEL_STD_INVALID_ARGUMENT;
    out->data = nullptr;
    out->length = 0;
    auto* data = static_cast<unsigned char*>(std::malloc(value.size() + 1));
    if (!data) return VEL_STD_IO_ERROR;
    std::memcpy(data, value.data(), value.size());
    data[value.size()] = 0;
    out->data = data;
    out->length = value.size();
    return VEL_STD_OK;
}

int map_error(const std::error_code& error)
{
    if (!error) return VEL_STD_OK;
    if (error == std::errc::no_such_file_or_directory) return VEL_STD_NOT_FOUND;
    if (error == std::errc::permission_denied) return VEL_STD_PERMISSION_DENIED;
    return VEL_STD_IO_ERROR;
}

}

extern "C" int vel_env_get(const char* name, VelBuffer* out)
{
    if (!name || !out || *name == '\0') return VEL_STD_INVALID_ARGUMENT;
    const char* value = std::getenv(name);
    if (!value) return VEL_STD_NOT_FOUND;
    return copy_string(value, out);
}

extern "C" int vel_env_current_dir(VelBuffer* out)
{
    if (!out) return VEL_STD_INVALID_ARGUMENT;
    std::error_code error;
    auto path = fs::current_path(error);
    if (error) return map_error(error);
    return copy_string(path.string(), out);
}

extern "C" int vel_fs_exists(const char* path, int* out_exists)
{
    if (!path || !out_exists || *path == '\0') return VEL_STD_INVALID_ARGUMENT;
    std::error_code error;
    *out_exists = fs::exists(fs::path(path), error) ? 1 : 0;
    return map_error(error);
}

extern "C" int vel_fs_read_text(const char* path, VelBuffer* out)
{
    if (!path || !out || *path == '\0') return VEL_STD_INVALID_ARGUMENT;
    std::ifstream file(path, std::ios::binary);
    if (!file.is_open()) {
        std::error_code error;
        if (!fs::exists(fs::path(path), error)) return error ? map_error(error) : VEL_STD_NOT_FOUND;
        return VEL_STD_PERMISSION_DENIED;
    }
    std::string content((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
    if (!file.good() && !file.eof()) return VEL_STD_IO_ERROR;
    return copy_string(content, out);
}

extern "C" int vel_fs_write_text(const char* path, const unsigned char* data, size_t length)
{
    if (!path || (!data && length != 0) || *path == '\0') return VEL_STD_INVALID_ARGUMENT;
    std::ofstream file(path, std::ios::binary | std::ios::trunc);
    if (!file.is_open()) return VEL_STD_PERMISSION_DENIED;
    file.write(reinterpret_cast<const char*>(data), static_cast<std::streamsize>(length));
    return file.good() ? VEL_STD_OK : VEL_STD_IO_ERROR;
}

extern "C" int vel_process_run(const char* command, int* out_exit_code)
{
    if (!command || !out_exit_code || *command == '\0') return VEL_STD_INVALID_ARGUMENT;
    const int result = std::system(command);
    if (result == -1) return VEL_STD_PROCESS_ERROR;
    *out_exit_code = result;
    return VEL_STD_OK;
}

extern "C" void vel_buffer_free(VelBuffer* buffer)
{
    if (!buffer) return;
    std::free(buffer->data);
    buffer->data = nullptr;
    buffer->length = 0;
}
