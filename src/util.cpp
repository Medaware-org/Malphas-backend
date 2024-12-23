#include <util.hpp>
#include <cstdio>
#include <crow_all.h>

char *read_file(const char *path, unsigned int *buff_len)
{
        FILE *fd;

#if defined(_WIN32)
        if (fopen_s(&fd, path, "r") != 0) {
#else
        fd = fopen(path, "r");
        if (!fd) {
#endif
                CROW_LOG_CRITICAL << "Could not open file for reading: " << path;
                return nullptr;
        }

        unsigned int size;
        char *buffer;

        fseek(fd, 0, SEEK_END);
        size = ftell(fd);
        rewind(fd);

        buffer = (char *) calloc(size + 1, sizeof(char));
        *buff_len = size;

        fread((void *) buffer, size, 1, fd);

        buffer[size] = '\0';

        fclose(fd);

        return buffer;
}

