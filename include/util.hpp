#pragma once

#define C_API extern "C"

[[nodiscard]] char *read_file(const char *path, unsigned int *buff_len);