#pragma once
#include <string>
#include "hyperion/http_parser.h"

namespace hyperion
{

    struct RouteResult
    {
        int status_code;
        std::string content_type;
        std::string body;
    };

    class Router
    {
    public:
        static RouteResult handle(const ParsedRequest &req);
    };

} // namespace hyperion