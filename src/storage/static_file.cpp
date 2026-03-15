#include "hyperion/static_file.h"
#include <fstream>
#include <sstream>

namespace hyperion
{

    std::string StaticFile::read(const std::string &path)
    {
        std::ifstream file(path, std::ios::binary);

        if (!file.is_open())
        {
            return ""; // file not found
        }

        std::ostringstream ss;
        ss << file.rdbuf();
        return ss.str();
    }

    std::string StaticFile::content_type(const std::string &path)
    {
        size_t dot = path.rfind('.');

        if (dot == std::string::npos)
        {
            return "application/octet-stream";
        }

        std::string ext = path.substr(dot);

        if (ext == ".html")
            return "text/html";
        if (ext == ".css")
            return "text/css";
        if (ext == ".js")
            return "application/javascript";
        if (ext == ".json")
            return "application/json";
        if (ext == ".png")
            return "image/png";
        if (ext == ".jpg")
            return "image/jpeg";
        if (ext == ".jpeg")
            return "image/jpeg";
        if (ext == ".ico")
            return "image/x-icon";
        if (ext == ".svg")
            return "image/svg+xml";
        if (ext == ".txt")
            return "text/plain";

        return "application/octet-stream";
    }

} // namespace hyperion