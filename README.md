# Interactive Backup Manager 📁🔄

[cite_start]Linux-based system program designed for managing interactive backups with real-time synchronization[cite: 1, 2]. [cite_start]The program automates the backup process, ensuring data integrity across multiple destinations simultaneously[cite: 7, 10].

## 🚀 Key Features

* [cite_start]**Real-Time Monitoring**: Utilizes the `inotify` API to detect changes in files, directories, and symbolic links, reflecting them immediately in the backup[cite: 14, 37].
* [cite_start]**Multi-Target Support**: Allows a single source directory to be backed up to multiple target paths at once[cite: 7].
* [cite_start]**Intelligent Restoration**: The `restore` command optimizes performance by only copying files that have changed since the backup was created[cite: 28].
* [cite_start]**Symbolic Link Handling**: Smartly manages absolute symbolic links to ensure they point to the correct files within the backup structure[cite: 12].
* [cite_start]**High Responsiveness**: Features a multi-process architecture where each target directory is handled by an independent subprocess, allowing the user to issue new commands without waiting for background tasks[cite: 16, 17].

## 💻 Interface & Commands

[cite_start]The program provides an interactive command-line interface that remains active even after incorrect inputs[cite: 3, 4].

| Command | Description |
| :--- | :--- |
| `add <src> <tgt1>...` | [cite_start]Starts a backup and begins real-time monitoring of the source[cite: 6]. |
| `end <src> <tgt1>...` | [cite_start]Stops the synchronization for the specified target paths[cite: 22]. |
| `restore <src> <tgt>` | [cite_start]Restores the source from a backup (blocking operation)[cite: 26, 30]. |
| `list` | [cite_start]Displays a summary of all active backup tasks[cite: 24]. |
| `exit` | [cite_start]Safely terminates the program and all child processes[cite: 32]. |



## ⚠️ Rules and Constraints

* **Directory Validation**: 
    * [cite_start]Prevents creating a backup of a directory inside itself to avoid infinite recursion[cite: 19].
    * [cite_start]Target directories must be empty if they already exist[cite: 9].
    * [cite_start]Non-existent target directories are created automatically[cite: 8].
* [cite_start]**Path Handling**: Supports folder names containing spaces (requires Bash-style quoting)[cite: 36].
* [cite_start]**Process Safety**: Properly handles `SIGINT` and `SIGTERM` signals to ensure resources are released before shutdown[cite: 33].
* [cite_start]**Implementation Note**: Developed strictly using system calls; use of the `system()` function is prohibited[cite: 35].

## 🛠️ Technical Requirements

* [cite_start]**OS**: Linux (required for `inotify` and POSIX signal handling)[cite: 33, 37].
* [cite_start]**Core APIs**: `inotify` (init, add_watch, rm_watch) and `realpath` for path comparison[cite: 39, 40, 41, 42].
* [cite_start]**Language**: C/C++ (Low-level resource management)[cite: 46].

---
*This project was developed to provide a high-performance, responsive solution for local data redundancy.*
