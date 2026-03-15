#include <unistd.h>
#include <errno.h>
#include <string>
#include "hyperion/connection.h"
#include "hyperion/http_parser.h"
#include "hyperion/router.h"
#include "hyperion/metrics.h"

static std::string build_response(int status_code,
                                  const std::string &status_text,
                                  const std::string &content_type,
                                  const std::string &body)
{
    std::string response;
    response.reserve(256 + body.size());

    response += "HTTP/1.1 ";
    response += std::to_string(status_code);
    response += " ";
    response += status_text;
    response += "\r\n";
    response += "Content-Type: ";
    response += content_type;
    response += "\r\n";
    response += "Content-Length: ";
    response += std::to_string(body.size());
    response += "\r\n";
    response += "Connection: keep-alive\r\n";
    response += "\r\n";
    response += body;

    return response;
}

static std::string status_text(int code)
{
    switch (code)
    {
    case 200:
        return "OK";
    case 400:
        return "Bad Request";
    case 404:
        return "Not Found";
    case 500:
        return "Internal Server Error";
    default:
        return "OK";
    }
}

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
            state = ConnectionState::CLOSED;
            return;
        }
        else
        {
            if (errno == EAGAIN || errno == EWOULDBLOCK)
                break;
            state = ConnectionState::CLOSED;
            return;
        }
    }

    if (read_buffer.find("\r\n\r\n") != std::string::npos)
    {
        state = ConnectionState::PROCESSING;
    }
}

void Connection::process_request()
{

    // ── Track request ──────────────────────────────────────────────
    hyperion::Metrics::instance().record_request();

    // ── Parse ──────────────────────────────────────────────────────
    hyperion::ParsedRequest req;
    bool ok = hyperion::HttpParser::parse(read_buffer, req);

    if (!ok || !req.valid)
    {
        write_buffer = build_response(
            400, "Bad Request", "text/plain", "Bad Request");
        write_offset = 0;
        state = ConnectionState::WRITING_RESPONSE;
        return;
    }

    // ── Route ──────────────────────────────────────────────────────
    hyperion::RouteResult result = hyperion::Router::handle(req);

    // ── Build response ─────────────────────────────────────────────
    write_buffer = build_response(
        result.status_code,
        status_text(result.status_code),
        result.content_type,
        result.body);
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
                needs_write_watch = true;
                return;
            }
            state = ConnectionState::CLOSED;
            return;
        }
    }

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