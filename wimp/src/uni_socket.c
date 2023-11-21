#include <uni_socket.h>

bool check_buffer_command(const char* buffer, const char* command, size_t buffer_len, size_t command_len)
{
    //Check the buffer is at least the length of the command
    if (buffer_len < command_len)
    {
        return false;
    }

    //Check the buffer up to the last character, ignoring the command null terminator
    if (strncmp(buffer, command, command_len - 1) != 0)
    {
        return false;
    }

    return true;
}