#ifndef VEL_STD_H
#define VEL_STD_H

#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct VelBuffer {
    unsigned char* data;
    size_t length;
} VelBuffer;

typedef enum VelStdStatus {
    VEL_STD_OK = 0,
    VEL_STD_INVALID_ARGUMENT = 1,
    VEL_STD_NOT_FOUND = 2,
    VEL_STD_PERMISSION_DENIED = 3,
    VEL_STD_IO_ERROR = 4,
    VEL_STD_PROCESS_ERROR = 5
} VelStdStatus;

int vel_env_get(const char* name, VelBuffer* out);
int vel_env_current_dir(VelBuffer* out);
int vel_fs_exists(const char* path, int* out_exists);
int vel_fs_read_text(const char* path, VelBuffer* out);
int vel_fs_write_text(const char* path, const unsigned char* data, size_t length);
/* Provisional v0.3.2 API: command is passed to the host shell; do not use with untrusted input. */
int vel_process_run(const char* command, int* out_exit_code);
void vel_buffer_free(VelBuffer* buffer);

#ifdef __cplusplus
}
#endif

#endif
