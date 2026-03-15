#pragma once
#include <string>
#include <unordered_map>
#include <fstream>
#include <sstream>
#include <iostream>
namespace hyperion
{

    class FileCache
    {
    public:
        static FileCache &instance()
        {
            static FileCache fc;
            return fc;
        }

        void load_all()
        {
            load("static/index.html");
            load("static/404.html");
        }

        const std::string *get(const std::string &path) const
        {
            auto it = cache_.find(path);
            if (it == cache_.end())
                return nullptr;
            return &it->second;
        }

    private:
        void load(const std::string &path)
        {
            std::ifstream file(path, std::ios::binary);
            if (!file.is_open())
                return;
            std::ostringstream ss;
            ss << file.rdbuf();
            cache_[path] = ss.str();
            std::cout << "Cached: " << path << std::endl;
        }

        std::unordered_map<std::string, std::string> cache_;
    };

} // namespace hyperion