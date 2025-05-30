/*

 _____                           _
|  __ \                         | |
| |  \/ ___ _ __   ___ _ __ __ _| |_ ___  _ __  "Template" System for Modern C++
| | __ / _ \ '_ \ / _ \ '__/ _` | __/ _ \| '__| Version 1.0.0
| |_\ \  __/ | | |  __/ | | (_| | || (_) | |    https://github.com/xabyxd/generator
 \____/\___|_| |_|\___|_|  \__,_|\__\___/|_|    See README.md for small usage example

*/

#include "generator.hpp"
#include <filesystem>
#include <fstream>
#include <iostream>
#include <sstream>
#include "nlohmann/json.hpp"
// using only the single header file

using json = nlohmann::json;
namespace filesystem = std::filesystem;

namespace generator {

    Template::Template(const std::string& name) : name(name) {}
    
    Template& Template::add_directory(const std::string& path) {
        directories.push_back(path);
        return *this;
    }
    
    Template& Template::add_file(const std::string& path, const std::string& content) {
        files.emplace_back(path, content);
        return *this;
    }
    
    bool Template::generate(const std::string& output_root) const {
        filesystem::path base_path = filesystem::path(output_root) / name;
    
        try {
            filesystem::create_directories(base_path);
        
            for (const auto& dir : directories) {
                filesystem::create_directories(base_path / dir);
            }
        
            for (const auto& [file, content] : files) {
                filesystem::path file_path = base_path / file;
                filesystem::create_directories(file_path.parent_path());
            
                std::ofstream out(file_path);
                if (out) {
                    out << content;
                } else {
                    std::cerr << "The file could not be created: " << file_path << '\n';
                }
            }
        
            return true;
        
        } catch (const std::exception& e) {
            std::cerr << "Error generating template: " << e.what() << '\n';
            return false;
        }
    }
    
    Template Template::from_json(const std::string& json_path) {
        std::ifstream in(json_path);
        if (!in.is_open()) {
            throw std::runtime_error("Could not open JSON file: " + json_path);
        }
    
        json j;
        in >> j;
    
        Template tpl(j["name"]);
    
        for (const auto& dir : j["directories"]) {
            tpl.add_directory(dir);
        }
    
        for (const auto& file : j["files"]) {
            std::string path = file["path"];
            std::string content;
        
            if (file.contains("content")) {
                content = file["content"];
            } else if (file.contains("from_file")) {
                std::ifstream fin(file["from_file"]);
                if (!fin.is_open()) {
                    throw std::runtime_error("Could not open source file: " + file["from_file"].get<std::string>());
                }
                std::stringstream buffer;
                buffer << fin.rdbuf();
                content = buffer.str();
            }
        
            tpl.add_file(path, content);
        }
    
        return tpl;
    }

} // namespace generator
