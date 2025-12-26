#pragma once

#include <string>
#include <vector>
#include <unordered_map>
#include <chrono>
#include <filesystem>
#include <mutex>

namespace fs = std::filesystem;

struct FileMetadata {
    std::string file_path;
    uint64_t file_size;
    std::string file_hash; // SHA256
    std::chrono::system_clock::time_point mtime;
    // Simple permission representation for now
    std::filesystem::perms permissions;
};

class FileScanner {
public:
    // Scan directory recursively, return files that changed since last backup (simplified for now to just return all files)
    std::vector<std::string> scanDirectory(const std::string& path);

    // Compute SHA-256 hash of entire file
    std::string hashFile(const std::string& file_path);

    // Get file metadata (size, mtime, permissions)
    FileMetadata getFileMetadata(const std::string& path);

    // Check if file is locked/in-use
    bool isFileLocked(const std::string& path);

private:
    std::unordered_map<std::string, FileMetadata> file_manifest;
    std::mutex scan_mutex;
};
