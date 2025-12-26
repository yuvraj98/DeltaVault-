# DeltaVault 🛡️

**DeltaVault** is a high-performance, local incremental file backup engine designed to efficiently store and retrieve multiple versions of your files. Built with modern C++ and Qt, it ensures data integrity while minimizing storage usage through block-level deduplication.

## 🚀 Key Features

*   **Incremental Backups**: Only new or modified data blocks are stored. If you change 1MB of a 10GB file, only that 1MB is backed up.
*   **Smart Deduplication**: Identical content across different files or versions shares the same storage space.
*   **Version Control**: precise restoration of any historical version of your files.
*   **Local & Secure**: All data is stored locally in a self-contained `.deltavault` repository using SQLite for robust metadata management.
*   **High Performance**: Multi-threaded processing and optimized hashing mechanisms.

## 🛠️ Technology Stack

*   **Core**: C++17
*   **UI**: Qt 6.8
*   **Database**: SQLite3
*   **Build System**: CMake

## 📚 Documentation

*   **[User Guide](USER_GUIDE.md)**: detailed instructions on how to use the Desktop UI to backup and restore files.
*   **[Build & Run Instructions](BUILD_AND_RUN.md)**: Setup guide for developers to build the project from source.

## 📦 Quick Start

1.  **Build the Project**: Follow the steps in `BUILD_AND_RUN.md`.
2.  **Run the UI**: Execute `deltavault_ui.exe`.
3.  **Backup**: Select a file and click "Start Backup".
4.  **Restore**: Enter a Version ID to restore a file to a specific point in time.

## 📂 Project Structure

*   `src/`: Core C++ source code.
*   `src/ui/`: Qt-based user interface components.
*   `tests/`: Unit and integration tests.
*   `.deltavault/`: (Hidden) Local repository where your data and metadata are stored.

---
*Created as part of the Advanced Systems Programming module.*
