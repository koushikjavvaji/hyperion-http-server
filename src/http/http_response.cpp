#include <hyperion/http_response.h>

namespace hyperion
{
    std::string HttpResponse::to_string() const
    {
        std::string result;
        result += "HTTP/1.1 ";
        result += std::to_string(status_code);
        result += " ";
        result += status_text;
        result += "\r\n";
        for (auto &h : headers)
        {
            result += h.first + ": " + h.second + "\r\n";
        }
        result += "\r\n";
        result += body;
        return result;
    }
}