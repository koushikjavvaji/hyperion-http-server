#pragma once
#include <string>

namespace hyperion
{

    class StaticFile
    {
    public:
        // read file from disk
        // returns empty string if file not found
        static std::string read(const std::string &path);

        // get content type from file extension
        static std::string content_type(const std::string &path);
    };

} // namespace hyperion