# Minecraft Server Manager (C++)

This is a **Minecraft server manager** written in C++, designed to easily control multiple local server instances. It allows you to **start**, **stop**, and **check the status** of servers defined in a configuration file.

> Currently compatible with **Windows**, with plans to port to **Linux** soon.

---

## 🚀 Current Features

- Start Minecraft servers using custom commands from `config.json`.
- Automatically detect and store the real `java.exe` process PID to stop servers reliably.
- Stop running servers safely.
- Display the current status (`running` / `stopped`) of each server.
- Event and error logging per server in `logs/`.
- Simple CLI interface for management.

---

## 📁 Project Structure

```
minecraft-server-manager/
│
├── servers/
│   ├── vanilla/
│   │   ├── server.jar
│   │   ├── config.json
│   │   └── other server files
│   └── forge/
│       ├── forge.jar
│       └── config.json
│
├── logs/
│   └── vanilla.log, forge.log, ...
├── pids/
│   └── vanilla.pid, forge.pid, ...
├── main.cpp
└── README.md
```

---

## 🛠️ How to Use

### 📌 Requirements

- Windows
- C++17 compatible compiler (MSVC / g++)
- JDK installed and in your PATH (to run `java`)

### 🧾 Server Configuration

Each server must have a `config.json` file in its folder:

```json
{
  "startCommand": "java -Xmx1024M -Xms1024M -jar server.jar nogui"
}
```

You also need to accept the EULA (`eula.txt`) as you would with a regular Minecraft server.

### ▶️ Available Commands

From the terminal:

```bash
./server_manager list               # List all available servers
./server_manager status <name>     # Check if the server is running
./server_manager start <name>      # Start the specified server
./server_manager stop <name>       # Stop the specified server
```

Example:

```bash
./server_manager start vanilla
```

---

## 🧪 Usage Example

```bash
> ./server_manager -list
- vanilla
- forge

> ./server_manager -start vanilla
🟢 Server 'vanilla' started (PID: 12345)

> ./server_manager -status vanilla
🟢 Server 'vanilla' is running.

> ./server_manager -stop vanilla
🛑 Server 'vanilla' stopped.
```

---

## 📚 Logging

Each server writes its own log in the `logs/` folder, including:

- Server start/stop events
- Errors during launch or termination
- Debug messages

Log format:

```
[ 2025-05-13 | 18:23:04 ] [ start_server ] [ vanilla ] 🟢 Server started (PID: 12345)
```

---

## 🔮 Upcoming Features

- ✅ **Linux port**: cross-platform support.
- 🔄 **Automatic server downloads**:
  - Vanilla from Mojang's official website.
  - Forge from its installer.
- ⚙️ **Initial setup automation**:
  - Auto-generation of `config.json`, `eula.txt`, etc.
- 🌐 **RCON control**:
  - Send commands to servers via RCON.
- 💻 **Web admin interface**:
  - Dashboard to monitor servers, logs, and control them via browser.

---

## 🧑‍💻 Author

Developed by xabyxd
Made with ❤️ in C++ for the Minecraft community

---

## 📄 License

This project is licensed under the Apache License 2.0. See `LICENSE` for more information.
