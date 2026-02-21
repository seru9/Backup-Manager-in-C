# Interactive Backup Manager 📁🔄
A Linux-based system designed for managing interactive backups with real-time synchronization. The program automates the backup process, replacing time-consuming manual copying with a responsive, multi-processed background system.

## 🚀 Key Features

* **Real-Time Monitoring**: Automatically tracks changes to files, subdirectories, and symbolic links in the source folder.
* **Multi-Target Support**: Allows a single source directory to be backed up to multiple destination paths simultaneously.
* **Intelligent Restoration**: The `restore` command optimizes performance by only copying files that have changed since the backup was created.
* **Symbolic Link Handling**: Smartly manages absolute symbolic links to ensure they point to the correct files within the backup structure.
* **High Responsiveness**: Features a multi-process architecture where each target directory is handled by an independent subprocess, ensuring the UI remains active for new commands.

## 💻 Interface & Commands

Upon startup, the program displays available commands and waits for user input. Invalid commands trigger an error message but do not terminate the session.

| Command | Description |
| :--- | :--- |
| `add <src> <tgt1>...` | Starts a backup and begins real-time monitoring of the source. |
| `end <src> <tgt1>...` | Stops synchronization for the specified target paths. |
| `restore <src> <tgt>` | Restores the source from a backup (blocking operation). |
| `list` | Displays a summary of all active backup tasks. |
| `exit` | Safely terminates the program and all child processes. |

## ⚠️ Rules and Constraints

* **Directory Validation**: 
    * Prevents creating a backup of a directory inside itself to avoid infinite recursion.
    * Prevents duplicate backup tasks for the same source/target pairs.
    * Target directories must be empty if they already exist; otherwise, an error is reported.
* **Path Handling**: Supports folder names containing spaces using Bash-style quoting.
* **Process Safety**: Properly handles `SIGINT` and `SIGTERM` signals to ensure resources are released before shutdown.
* **Implementation Note**: Developed strictly using system calls; use of the `system()` function is prohibited.

## 🛠️ Technical Requirements

* **OS**: Linux (required for `inotify` API).
* **Core APIs**: `inotify` (init, add_watch, rm_watch) for change tracking and `realpath` for path comparison.
* **Language**: C/C++ (Low-level resource and process management).

---
<img width="497" height="436" alt="image" src="https://github.com/user-attachments/assets/9d4c675c-9c64-402f-97b0-8cb7cc3b907e" />

*This project provides a high-performance, responsive solution for local data redundancy.*
