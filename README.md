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
mc-serverManager/
│
├── servers/
│   ├── vanillaWithFriends/
│   │   ├── server.jar
│   │   ├── config.json
│   │   └── other server files
│   └── AllTheMods9/
│       ├── forge_server.jar
│       └── config.json
│       └── other server files
│
├── logs/
│   └── vanillaWithFriends.log, AllTheMods9.log, ...
├── pids/
│   └── vanillaWithFriends.pid, AllTheMods9.pid, ...
├── main.cpp
└── README.md
```

---

## 🛠️ How to Use

### 📌 Requirements

- Windows
- C++17 compatible compiler (Mingw64 v15.1 / MSVC) [Mingw64 v15.1 recomended](https://gcc.gnu.org/gcc-15/).
- JDK installed and in your PATH (to run `java`).
- My custom build-script for Windows [Here :D](https://github.com/xabyxd/xabyxd_cpp-compilerScript).
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
./serverManager -status <name>     # Check if the server is running
./serverManager -start <name>      # Start the specified server
./serverManager -stop <name>       # Stop the specified server
./serverManager -restart <name>    # Restart the specified server after 10 secconds
./serverManager -version/-v        # Prints the current program version and checks for new versions
./serverManager -list/-l           # List all available servers
./serverManager -help/-h           # Prints the help (all the commands)
```

Example:

```bash
./serverManager start vanilla
```

---

## 🧪 Usage Example

```bash
> ./serverManager -list
- vanillaWithFriends
- AllTheMods9

> ./serverManager -start vanillaWithFriends
🟢 Server 'vanillaWithFriends' started (PID: 12345)

> ./serverManager -status vanillaWithFriends
🟢 Server 'vanillaWithFriends' is running.

> ./serverManager -stop vanillaWithFriends
🛑 Server 'vanillaWithFriends' stopped.
```
> PRO TIP: Add the program to the system path to use anywhere in your terminal.<br>
> example: C:\serverManager\serverManager.exe

---

## 📚 Logging

Each server writes its own log in the `logs/` folder, including:

- Server start/stop events
- Errors during launch or termination
- Debug messages

Log format:

```
[ 2025-05-13 | 18:23:04 ] [ start_server ] [ vanillaWithFriends ] 🟢 Server started (PID: 12345)
```

---

## 🔮 Upcoming Features

- 🛠 **Linux port**: cross-platform support.
- 🔄 **Automatic server downloads**:
  - Vanilla from Mojang's official website.
  - Forge from its installer.
- ⚙️ **Initial setup automation**:
  - Auto-generation of `config.json`, `eula.txt`, etc.
  - Templates system.
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
