#pragma once

#include <string>
#include <vector>
#include <mutex>

class StorageManager {
public:
    // Initialize storage directory, e.g., ".deltavault"
    void initialize(const std::string& root_path);

    // Write block to persistent storage, return true on success
    // filename derived from block_id or hash
    bool writeBlock(const std::string& block_hash, const std::vector<uint8_t>& block_data);

    // Read block from storage
    std::vector<uint8_t> readBlock(const std::string& block_hash);

private:
    std::string root_path;
    std::string blocks_path;
    std::mutex storage_mutex;

    std::string getBlockPath(const std::string& block_hash);
};
