#include "hyperion/http_parser.h"
#include <sstream>

namespace hyperion
{

    bool HttpParser::parse(const std::string &raw,
                           ParsedRequest &out)
    {

        // find end of headers
        size_t header_end = raw.find("\r\n\r\n");
        if (header_end == std::string::npos)
        {
            return false; // incomplete request
        }

        std::istringstream stream(raw.substr(0, header_end));
        std::string line;

        // ── Parse request line ─────────────────────────────────────────
        // example: "GET / HTTP/1.1"
        if (!std::getline(stream, line))
            return false;

        // remove \r if present
        if (!line.empty() && line.back() == '\r')
        {
            line.pop_back();
        }

        std::istringstream request_line(line);
        if (!(request_line >> out.method >> out.path >> out.version))
        {
            return false; // malformed request line
        }

        // ── Parse headers ──────────────────────────────────────────────
        // example: "Host: localhost:8080"
        while (std::getline(stream, line))
        {
            if (!line.empty() && line.back() == '\r')
            {
                line.pop_back();
            }
            if (line.empty())
                break;

            size_t colon = line.find(':');
            if (colon == std::string::npos)
                continue;

            std::string key = line.substr(0, colon);
            std::string value = line.substr(colon + 2); // skip ": "

            out.headers[key] = value;
        }

        out.valid = true;
        return true;
    }

} // namespace hyperion