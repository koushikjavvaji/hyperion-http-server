#pragma once
#include <string>

enum class ConnectionState
{
    READING_REQUEST,
    PROCESSING,
    WRITING_RESPONSE,
    CLOSED
};

class Connection
{
public:
    int fd;
    ConnectionState state;
    std::string read_buffer;
    std::string write_buffer;
    size_t write_offset;
    bool needs_write_watch;

    Connection(int socket_fd)
        : fd(socket_fd),
          state(ConnectionState::READING_REQUEST),
          write_offset(0),
          needs_write_watch(false) {}

    void reset(int socket_fd)
    {
        fd = socket_fd;
        state = ConnectionState::READING_REQUEST;
        read_buffer.clear();
        write_buffer.clear();
        write_offset = 0;
        needs_write_watch = false;
    }

    void handle_read();
    void process_request();
    void handle_write();
    void close_connection();
};