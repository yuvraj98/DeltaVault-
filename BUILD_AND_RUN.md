# Build and Run Instructions

## Prerequisites

1.  **C++ Compiler**: Microsoft Visual Studio with C++ Desktop Development workload.
2.  **CMake**: 3.20 or newer.
3.  **vcpkg**: Microsoft C++ Library Manager (installed at `c:/Users/rajch/OneDrive/Desktop/vcpkg`).

## 1. Install Dependencies (One-time setup)

Open PowerShell in the project directory (`c:\Users\rajch\OneDrive\Desktop\Project-1`) and run:

```powershell
# Bootstrap vcpkg
c:/Users/rajch/OneDrive/Desktop/vcpkg/bootstrap-vcpkg.bat

# Install required libraries
c:/Users/rajch/OneDrive/Desktop/vcpkg/vcpkg.exe install sqlite3 openssl zstd
```

## 2. Clean Build

If you have previous build artifacts, clean them first:

```powershell
Remove-Item -Recurse -Force build -ErrorAction SilentlyContinue
mkdir build
```

## 3. Configure

Configure CMake using the vcpkg toolchain. This ensures CMake finds the installed libraries.

```powershell
cmake -S . -B build -DCMAKE_TOOLCHAIN_FILE=c:/Users/rajch/OneDrive/Desktop/vcpkg/scripts/buildsystems/vcpkg.cmake
```

## 4. Build

Compile the project:

```powershell
cmake --build build
```

## 5. Run

Execute the test CLI tool. You must provide a file path to test the scanner and splitter.

```powershell
.\build\Debug\deltavault_cli.exe src/main.cpp
```

### Expected Output
You should see output similar to checks for file size, SHA-256 hash, and block count verification.

