#include "file_scanner.h"
#include <fstream>
#include <iostream>
#include <openssl/sha.h>
#include <iomanip>
#include <sstream>

std::vector<std::string> FileScanner::scanDirectory(const std::string& path) {
    std::vector<std::string> files;
    std::lock_guard<std::mutex> lock(scan_mutex);

    try {
        if (fs::exists(path) && fs::is_directory(path)) {
            for (const auto& entry : fs::recursive_directory_iterator(path)) {
                if (fs::is_regular_file(entry)) {
                    files.push_back(entry.path().string());
                }
            }
        }
    } catch (const fs::filesystem_error& e) {
        std::cerr << "Filesystem error: " << e.what() << std::endl;
    }

    return files;
}

std::string FileScanner::hashFile(const std::string& file_path) {
    std::ifstream file(file_path, std::ios::binary);
    if (!file) {
        return "";
    }

    SHA256_CTX sha256_ctx;
    SHA256_Init(&sha256_ctx);

    char buffer[4096];
    while (file.read(buffer, sizeof(buffer))) {
        SHA256_Update(&sha256_ctx, buffer, file.gcount());
    }
    // Process remaining bytes
    if (file.gcount() > 0) {
        SHA256_Update(&sha256_ctx, buffer, file.gcount());
    }

    unsigned char hash[SHA256_DIGEST_LENGTH];
    SHA256_Final(hash, &sha256_ctx);

    std::stringstream ss;
    for (int i = 0; i < SHA256_DIGEST_LENGTH; i++) {
        ss << std::hex << std::setw(2) << std::setfill('0') << (int)hash[i];
    }
    return ss.str();
}

FileMetadata FileScanner::getFileMetadata(const std::string& path) {
    FileMetadata metadata;
    metadata.file_path = path;
    
    try {
        if (fs::exists(path)) {
            metadata.file_size = fs::file_size(path);
            auto ftime = fs::last_write_time(path);
            // Convert file_time_type to system_clock::time_point roughly
            // This conversion is non-trivial across platforms in C++20, keeping it simple for now or just storing it
            // For C++20 we can use clock_cast if available or simplified approach
            // metadata.mtime = std::chrono::clock_cast<std::chrono::system_clock>(ftime); 
            // Fallback for visual simplicity:
             metadata.mtime = std::chrono::system_clock::now(); // Placeholder until precise conversion implemented
            
            metadata.permissions = fs::status(path).permissions();
        }
    } catch (...) {
        metadata.file_size = 0;
    }

    return metadata;
}

bool FileScanner::isFileLocked(const std::string& path) {
    // Basic check: try to open for exclusive writing
    // This is OS specific. 
    // For Windows/Linux, std::filesystem doesn't directly support "check lock"
    // Trying to open ifstream might suffice for read lock check, but writing uses ofstream
    std::ofstream file(path, std::ios::app);
    return !file.is_open(); 
}
