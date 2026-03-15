#include <unistd.h>
#include <errno.h>
#include "hyperion/connection.h"

static const char RESPONSE[] =
    "HTTP/1.1 200 OK\r\n"
    "Content-Type: text/plain\r\n"
    "Content-Length: 19\r\n"
    "Connection: keep-alive\r\n"
    "\r\n"
    "Hello from Hyperion";

static const size_t RESPONSE_LEN = sizeof(RESPONSE) - 1;

void Connection::handle_read()
{
    char buffer[4096];

    while (true)
    {
        int bytes = read(fd, buffer, sizeof(buffer));

        if (bytes > 0)
        {
            read_buffer.append(buffer, bytes);
        }
        else if (bytes == 0)
        {
            // Client closed connection cleanly
            state = ConnectionState::CLOSED;
            return;
        }
        else
        {
            if (errno == EAGAIN || errno == EWOULDBLOCK)
            {
                // No more data right now
                break;
            }
            // Real error
            state = ConnectionState::CLOSED;
            return;
        }
    }

    // Complete HTTP request ends with blank line
    if (read_buffer.find("\r\n\r\n") != std::string::npos)
    {
        state = ConnectionState::PROCESSING;
    }
}

void Connection::process_request()
{
    write_buffer.assign(RESPONSE, RESPONSE_LEN);
    write_offset = 0;
    state = ConnectionState::WRITING_RESPONSE;
}

void Connection::handle_write()
{
    while (write_offset < write_buffer.size())
    {
        int bytes = write(
            fd,
            write_buffer.data() + write_offset,
            write_buffer.size() - write_offset);

        if (bytes > 0)
        {
            write_offset += bytes;
        }
        else
        {
            if (errno == EAGAIN || errno == EWOULDBLOCK)
            {
                // Kernel send buffer full — wait for EVFILT_WRITE event
                needs_write_watch = true;
                return;
            }
            state = ConnectionState::CLOSED;
            return;
        }
    }

    // Response fully sent — reuse connection for next request (keep-alive)
    read_buffer.clear();
    write_buffer.clear();
    write_offset = 0;
    state = ConnectionState::READING_REQUEST;
}

void Connection::close_connection()
{
    if (fd >= 0)
    {
        close(fd);
        fd = -1;
    }
    state = ConnectionState::CLOSED;
}