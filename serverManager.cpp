/*
Author: xabyxd (https://github.com/xabyxd)
Date: 12-05-2025
License: Apache 2.0
Version: 0.3.0
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

namespace fs = std::filesystem;
using json = nlohmann::json;

const bool DEBUG = true;
const std::string BASE_DIR = "./InstanceServers/";
const std::string PID_DIR = BASE_DIR + ".pids/";
const std::string LOG_FILE = BASE_DIR + ".logs/serverManager.log";

// -------- LOGGING --------

void write_log(const std::string& entry) {
    if (!fs::exists(BASE_DIR + ".logs/")) fs::create_directory(BASE_DIR + ".logs/");
    
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

// -------- PROCESSES --------

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

// -------- CONFIG --------

bool load_server_config(const std::string& name, json& config) {
    std::string config_path = BASE_DIR + name + "/config.json";
    debug_log("Trying to load config: " + config_path);
    if (!fs::exists(config_path)) {
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

// -------- FUNCTIONS --------

void list_servers() {
    std::cout << "Server list:\n";
    for (auto& entry : fs::directory_iterator(BASE_DIR)) {
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
    // Load config by server name (not by full path)
    json config;
    if (!load_server_config(name, config)) {
        std::cerr << "[ERROR] Missing or invalid config.json for server '" << name << "'.\n";
        return false;
    }

    // Working directory and command from JSON
    std::string workingDir = BASE_DIR + name;
    std::string command = config.value("startCommand", "");
    if (command.empty()) {
        std::cerr << "[ERROR] 'startCommand' is missing in config file for '" << name << "'.\n";
        return false;
    }

    // Build full cmd line: change to server dir then execute
    std::string fullCommand =
        "cmd.exe /C \"cd /d " + workingDir + " && " + command + "\"";

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
        std::cerr << "[ERROR] Failed to start server process for '"
                  << name << "'. Error code: " << GetLastError() << "\n";
        return false;
    }

    // Wait briefly to catch immediate exit
    DWORD waitResult = WaitForSingleObject(pi.hProcess, 100);
    if (waitResult == WAIT_OBJECT_0) {
        DWORD exitCode;
        GetExitCodeProcess(pi.hProcess, &exitCode);
        std::cerr << "[WARNING] Server '" << name
                  << "' exited immediately with code: " << exitCode << "\n";
        CloseHandle(pi.hProcess);
        CloseHandle(pi.hThread);
        return false;
    }

    // Persist PID
    if (!fs::exists(PID_DIR)) fs::create_directories(PID_DIR);
    std::ofstream pid_out(PID_DIR + name + ".pid");
    pid_out << pi.dwProcessId;
    pid_out.close();

    debug_log("Server '" + name + "' started with PID: " + std::to_string(pi.dwProcessId));

    CloseHandle(pi.hProcess);
    CloseHandle(pi.hThread);

    std::cout << "Server '" << name << "' is now running.\n";
    return true;
}

void stop_server(const std::string& name) {
    std::string pid_file = PID_DIR + name + ".pid";
    std::ifstream pid_in(pid_file);
    DWORD pid;
    if (!(pid_in >> pid) || !is_process_running(pid)) {
        std::cerr << "Server '" << name << "' is not running.\n";
        return;
    }

    HANDLE hProcess = OpenProcess(PROCESS_TERMINATE, FALSE, pid);
    if (!hProcess) {
        std::cerr << "Could not open the process of the server.\n";
        return;
    }

    TerminateProcess(hProcess, 0);
    CloseHandle(hProcess);
    fs::remove(pid_file);

    write_log("🔴 Server '" + name + "' stoped (PID: " + std::to_string(pid) + ")");
    std::cout << "Server '" << name << "' stoped.\n";
}

void status_server(const std::string& name) {
    std::string pid_file = PID_DIR + name + ".pid";
    std::ifstream pid_in(pid_file);
    DWORD pid;
    if (pid_in >> pid && is_process_running(pid)) {
        std::cout << "🟢 '" << name << "' is running (PID: " << pid << ")\n";
    } else {
        std::cout << "🔴 '" << name << "' is stoped\n";
    }
}

// -------- CLI --------

void show_help() {
    std::cout << "Usage:\n";
    std::cout << "  serverManager -list\n";
    std::cout << "  serverManager -start <name>\n";
    std::cout << "  serverManager -stop <name>\n";
    std::cout << "  serverManager -status <name>\n";
}

// -------- MAIN PROGRAM --------

int main(int argc, char* argv[]) {
    // Ensure required directories exist
    try {
        if (!fs::exists(BASE_DIR)) fs::create_directories(BASE_DIR);
        if (!fs::exists(PID_DIR)) fs::create_directories(PID_DIR);
        if (!fs::exists(BASE_DIR + ".logs/")) fs::create_directories(BASE_DIR + ".logs/");
    } catch (const fs::filesystem_error& e) {
        std::cerr << "Error creating the required directories: " << e.what() << "\n";
        return 1;
    }

    if (argc < 2) {
        show_help();
        return 0;
    }

    std::string command = argv[1];
    if (command == "-list") {
        list_servers();
    } else if (command == "-start" && argc == 3) {
        start_server(argv[2]);
    } else if (command == "-stop" && argc == 3) {
        stop_server(argv[2]);
    } else if (command == "-status" && argc == 3) {
        status_server(argv[2]);
    } else {
        show_help();
    }

    return 0;
}

