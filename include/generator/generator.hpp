/*

 _____                           _
|  __ \                         | |
| |  \/ ___ _ __   ___ _ __ __ _| |_ ___  _ __  "Template" System for Modern C++
| | __ / _ \ '_ \ / _ \ '__/ _` | __/ _ \| '__| Version 1.0.0
| |_\ \  __/ | | |  __/ | | (_| | || (_) | |    https://github.com/xabyxd/generator
 \____/\___|_| |_|\___|_|  \__,_|\__\___/|_|    See README.md for small usage example

*/

#pragma once
#include <string>
#include <vector>
#include <utility>

namespace generator {

    class Template {
    public:
        Template(const std::string& name);
    
        Template& add_directory(const std::string& path);
        Template& add_file(const std::string& path, const std::string& content = "");
    
        bool generate(const std::string& output_root) const;
    
        static Template from_json(const std::string& json_path);
    
    private:
        std::string name;
        std::vector<std::string> directories;
        std::vector<std::pair<std::string, std::string>> files;
    };

} // namespace generator
