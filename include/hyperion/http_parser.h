#pragma once
#include <string>
#include <unordered_map>

namespace hyperion
{

    struct ParsedRequest
    {
        std::string method;  // GET POST etc
        std::string path;    // /  /health  /about.html
        std::string version; // HTTP/1.1
        std::unordered_map<std::string, std::string> headers;
        bool valid = false;
    };

    class HttpParser
    {
    public:
        // parse raw request string into ParsedRequest
        // returns false if request is incomplete or malformed
        static bool parse(const std::string &raw,
                          ParsedRequest &out);
    };

} // namespace hyperion