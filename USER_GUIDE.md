# DeltaVault User Guide

DeltaVault is a high-performance, incremental file backup system designed to efficiently store and retrieve multiple versions of your files locally.

## What Has Been Achieved

The current release of DeltaVault includes the following core capabilities:

*   **Incremental Backup Engine**:
    *   Files are split into fixed-size blocks.
    *   Only unique, new blocks are stored. If you change 1MB of a 10GB file, only that 1MB is effectively backed up again, saving massive amounts of space.
    *   **Deduplication**: Identical content across different files or versions shares the same storage space.
*   **Robust Local Storage**:
    *   All data is securely stored in a local repository (`.deltavault` directory).
    *   Uses **SQLite** for reliable metadata management (tracking file versions and block lists).
*   **Performance**:
    *   **Multi-threaded Processing**: Utilizes your CPU's available threads for faster hashing and processing.
    *   **Low Memory Footprint**: Streamed processing ensures large files can be handled without consuming excessive RAM.
*   **Desktop UI**:
    *   A clean, responsive **Qt 6** interface.
    *   Real-time logs and status updates.
    *   One-click Backup and Restore operations.

## Operating the User Interface

### 1. Launching the Application
Run the `deltavault_ui.exe` executable. Upon launch, the system automatically initializes the storage repository in the local directory.

### 2. Backing Up a File
1.  **Select File**: Click the **Browse...** button to open the file chooser, or manually paste the absolute path of the file you wish to backup into the input field.
2.  **Start Backup**: Click the green **Start Backup** button.
3.  **Monitor Progress**:
    *   The status bar will change to "Backing up...".
    *   The Log Area will show real-time progress.
4.  **Completion**:
    *   Once finished, the log will display: `Backup Successful! Created Version ID: <number>`.
    *   **Note**: The System automatically populates the "Restore Version ID" field with this new ID for convenience.

### 3. Restoring a File
You can restore any previous version of a file if you have its **Version ID**.

1.  **Set Target Path**: Ensure the main input field contains the path of the file you want to restore (or at least the filename).
    *   *Note: The restored file will be saved as `<OriginalFilename>.restored.<Extension>` (e.g., `document.restored.txt`) to preserve the file type association.*
2.  **Enter Version ID**: Input the specific `Version ID` (integer) you wish to recover into the "Restore Version ID" box.
3.  **Restore**: Click the **Restore** button.
4.  **Completion**:
    *   The log will confirm: `Restore Successful! File saved to: ...path.restored`.

### 4. Application Logs
The large text area at the bottom displays a history of operations performed during the session, including timestamps, errors, and successful validation messages.

## Data Storage Location

DeltaVault stores all your backup data in a hidden directory within the project folder.

*   **Repository Path**: `<ProjectRoot>/.deltavault/`
*   **Data Chunks**: All file blocks are stored individually in `<ProjectRoot>/.deltavault/blocks/`. Each file is named after its SHA-256 hash (e.g., `a1b2c3...bin`).
*   **Metadata**: The database mapping files to these blocks is located at `<ProjectRoot>/.deltavault/metadata.db`.

