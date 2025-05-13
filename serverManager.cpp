/*
Author: xabyxd (https://github.com/xabyxd)
Date: 12-05-2025
License: Apache 2.0
Version: 0.4.0
C++ 17 implementation of a server manager for Minecraft servers.
Compile for windows using mingw-w64 v15.1.0
*/


#include <iostream>
#include <filesystem>
#include <map>
#include <string>
#include <windows.h>
#include <tlhelp32.h>
#include <fstream>
#include <sstream>
#include <ctime>
#include <nlohmann/json.hpp>

namespace filesystem = std::filesystem;
using json = nlohmann::json;

const bool DEBUG = true;
const std::string BASE_DIR = "./InstanceServers/";
const std::string PID_DIR = BASE_DIR + ".pids/";
const std::string LOG_FILE = BASE_DIR + ".logs/serverManager.log";

// ---------------- LOGGING ----------------

void write_log(const std::string& entry) {
    if (!filesystem::exists(BASE_DIR + ".logs/")) filesystem::create_directory(BASE_DIR + ".logs/");
    
    std::ofstream log(LOG_FILE, std::ios::app);
    
    std::time_t now = std::time(nullptr);
    char time_buf[64];
    std::strftime(time_buf, sizeof(time_buf), "%Y-%m-%d %H:%M:%S", std::localtime(&now));
    
    log << "[" << time_buf << "] " << entry << "\n";
}

void debug_log(const std::string& msg) {
    if (DEBUG) {
        std::cout << "[DEBUG] " << msg << "\n";
        write_log("[DEBUG] " + msg);
    }
}

// ---------------- PROCESSES ----------------

bool is_process_running(DWORD pid) {
    HANDLE snap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    PROCESSENTRY32 pe;
    pe.dwSize = sizeof(PROCESSENTRY32);
    if (!Process32First(snap, &pe)) return false;
    do {
        if (pe.th32ProcessID == pid) {
            CloseHandle(snap);
            return true;
        }
    } while (Process32Next(snap, &pe));
    CloseHandle(snap);
    return false;
}

DWORD find_child_java_pid(DWORD parent_pid) {
    HANDLE snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if (snapshot == INVALID_HANDLE_VALUE) return 0;

    PROCESSENTRY32 pe;
    pe.dwSize = sizeof(PROCESSENTRY32);
    DWORD java_pid = 0;

    if (Process32First(snapshot, &pe)) {
        do {
            if (pe.th32ParentProcessID == parent_pid && std::string(pe.szExeFile) == "java.exe") {
                java_pid = pe.th32ProcessID;
                break;
            }
        } while (Process32Next(snapshot, &pe));
    }

    CloseHandle(snapshot);
    return java_pid;
}

// ---------------- CONFIG ----------------

bool load_server_config(const std::string& name, json& config) {
    std::string config_path = BASE_DIR + name + "/config.json";
    debug_log("Trying to load config: " + config_path);
    if (!filesystem::exists(config_path)) {
        debug_log("config.json file not found.");
        return false;
    }
    std::ifstream in(config_path);
    try {
        in >> config;
        debug_log("Configuration loaded successfully for '" + name + "'.");
        return true;
    } catch (const std::exception& e) {
        debug_log("Error parsing config.json: " + std::string(e.what()));
        return false;
    }
}

// ---------------- FUNCTIONS ----------------

void sleep(unsigned milliseconds)
    {
        Sleep(milliseconds);
    }

void list_servers() {
    std::cout << "Server list:\n";
    for (auto& entry : filesystem::directory_iterator(BASE_DIR)) {
        if (entry.is_directory() && entry.path().filename().string().rfind(".", 0) != 0) {
            std::string name = entry.path().filename().string();
            std::string pid_file = PID_DIR + name + ".pid";
            std::ifstream pid_in(pid_file);
            DWORD pid;
            if (pid_in >> pid && is_process_running(pid)) {
                std::cout << "🟢 " << name << " (PID: " << pid << ")\n";
            } else {
                std::cout << "🔴 " << name << " (shutdown)\n";
            }
        }
    }
}

bool start_server(const std::string& name) {
    json config;
    if (!load_server_config(name, config)) {
        std::string msg = "[ERROR] Missing or invalid config.json for server '" + name + "'.";
        std::cerr << msg << "\n";
        write_log(msg);
        return false;
    }

    std::string workingDir = BASE_DIR + name;
    std::string command = config.value("startCommand", "");
    if (command.empty()) {
        std::string msg = "[ERROR] 'startCommand' is missing in config file for '" + name + "'.";
        std::cerr << msg << "\n";
        write_log(msg);
        return false;
    }

    std::string fullCommand = "cmd.exe /C \"cd /d " + workingDir + " && " + command + "\"";

    debug_log("Full command to execute: " + fullCommand);

    STARTUPINFOA si = { sizeof(si) };
    PROCESS_INFORMATION pi;

    BOOL success = CreateProcessA(
        nullptr,
        (LPSTR)fullCommand.c_str(),
        nullptr, nullptr,
        FALSE,
        CREATE_NO_WINDOW,
        nullptr,
        nullptr,
        &si, &pi
    );

    if (!success) {
        std::string msg = "[ERROR] Failed to start server process for '" + name + "'. Error code: " + std::to_string(GetLastError());
        std::cerr << msg << "\n";
        write_log(msg);
        return false;
    }

    Sleep(200);

    DWORD java_pid = find_child_java_pid(pi.dwProcessId);
    CloseHandle(pi.hProcess);
    CloseHandle(pi.hThread);

    if (java_pid == 0) {
        std::string msg = "[ERROR] Could not find java.exe child process for '" + name + "'.";
        std::cerr << msg << "\n";
        write_log(msg);
        return false;
    }

    if (!filesystem::exists(PID_DIR)) filesystem::create_directories(PID_DIR);
    std::ofstream pid_out(PID_DIR + name + ".pid");
    pid_out << java_pid;
    pid_out.close();

    std::string successMsg = "🟢 Server '" + name + "' started (PID: " + std::to_string(java_pid) + ")";
    debug_log(successMsg);
    write_log(successMsg);

    std::cout << "Server '" << name << "' is now running.\n";
    return true;
}

void stop_server(const std::string& name) {
    std::string pid_file = PID_DIR + name + ".pid";
    std::ifstream pid_in(pid_file);
    DWORD pid;
    if (!(pid_in >> pid) || !is_process_running(pid)) {
        std::cerr << "Server '" << name << "' is not running.\n";
        pid_in.close();
        return;
    }

    pid_in.close();

    HANDLE hProcess = OpenProcess(PROCESS_TERMINATE, FALSE, pid);
    if (!hProcess) {
        std::cerr << "Could not open the process of the server.\n";
        return;
    }

    TerminateProcess(hProcess, 0);
    CloseHandle(hProcess);

    filesystem::remove(pid_file);

    write_log("🔴 Server '" + name + "' stopped (PID: " + std::to_string(pid) + ")");
    std::cout << "Server '" << name << "' stopped.\n";
}

void restart_server(const std::string& name) {
    stop_server(name);
    sleep(1000);
    start_server(name);
    std::cout << "🟢 Server '" + name + "' restarted.\n";
    write_log("Server '" + name + "' restarted.");
}

void status_server(const std::string& name) {
    std::string pid_file = PID_DIR + name + ".pid";
    std::ifstream pid_in(pid_file);
    DWORD pid;
    if (pid_in >> pid && is_process_running(pid)) {
        std::cout << "🟢 '" << name << "' is running (PID: " << pid << ")\n";
        // print_process_usage(pid); NEXT TO IMPLEMENT
    } else {
        std::cout << "🔴 '" << name << "' is stoped\n";
    }
}

// ---------------- CLI ----------------

void show_help() {
    std::cout << "Usage:\n";
    std::cout << "  serverManager -help -h                Show this help\n";
    std::cout << "  serverManager -list -l                List all servers\n";
    std::cout << "  serverManager -start <name>           Start a server example: serverManager -start server1\n";
    std::cout << "  serverManager -stop <name>            Stop a server example: serverManager -stop server1\n";
    std::cout << "  serverManager -status <name>          Show status of a server example: serverManager -status server1\n";
    std::cout << "  serverManager -restart <name>         Restart a server example: serverManager -restart server1\n";
}

// ---------------- MAIN PROGRAM ----------------

int main(int argc, char* argv[]) {
    // Ensure required directories exist
    try {
        if (!filesystem::exists(BASE_DIR)) filesystem::create_directories(BASE_DIR);
        if (!filesystem::exists(PID_DIR)) filesystem::create_directories(PID_DIR);
        if (!filesystem::exists(BASE_DIR + ".logs/")) filesystem::create_directories(BASE_DIR + ".logs/");
    } catch (const filesystem::filesystem_error& e) {
        std::cerr << "Error creating the required directories: " << e.what() << "\n";
        return 1;
    }

    if (argc < 2) {
        show_help();
        return 0;
    }

    std::string command = argv[1];
    if (command == "-list" || command == "-l") {
        list_servers();
    } else if (command == "-start" && argc == 3) {
        start_server(argv[2]);
    } else if (command == "-stop" && argc == 3) {
        stop_server(argv[2]);
    } else if (command == "-status" && argc == 3) {
        status_server(argv[2]);
    } else if (command == "-restart" && argc == 3) {
        restart_server(argv[2]);
    } else if (command == "-help" || command == "-h" && argc == 3) {
        show_help();
    } else {
        show_help();
    }

    return 0;
}