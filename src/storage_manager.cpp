#include "storage_manager.h"
#include <filesystem>
#include <fstream>
#include <stdexcept>

namespace fs = std::filesystem;

void StorageManager::initialize(const std::string& root) {
    std::lock_guard<std::mutex> lock(storage_mutex);
    root_path = root;
    blocks_path = root + "/blocks";

    if (!fs::exists(blocks_path)) {
        fs::create_directories(blocks_path);
    }
}

std::string StorageManager::getBlockPath(const std::string& block_hash) {
    // Simple flat structure for now. 
    // In production, might want subfolders like /blocks/a/b/abcdef... to avoid too many files in one dir
    return blocks_path + "/" + block_hash + ".bin";
}

bool StorageManager::writeBlock(const std::string& block_hash, const std::vector<uint8_t>& block_data) {
    std::lock_guard<std::mutex> lock(storage_mutex);
    std::string path = getBlockPath(block_hash);

    // If already exists, we can skip writing (immutability assumption), 
    // but explicit write is safer for now.
    if (fs::exists(path)) {
        return true; 
    }

    std::ofstream file(path, std::ios::binary);
    if (!file) {
        return false;
    }

    file.write(reinterpret_cast<const char*>(block_data.data()), block_data.size());
    return true;
}

std::vector<uint8_t> StorageManager::readBlock(const std::string& block_hash) {
    std::lock_guard<std::mutex> lock(storage_mutex);
    std::string path = getBlockPath(block_hash);

    std::ifstream file(path, std::ios::binary);
    if (!file) {
        throw std::runtime_error("Block not found: " + block_hash);
    }

    // Get size
    file.seekg(0, std::ios::end);
    size_t fileSize = file.tellg();
    file.seekg(0, std::ios::beg);

    std::vector<uint8_t> buffer(fileSize);
    file.read(reinterpret_cast<char*>(buffer.data()), fileSize);
    return buffer;
}
