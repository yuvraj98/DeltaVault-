#pragma once

#include <string>
#include <vector>
#include <memory>
#include <mutex>
#include <sqlite3.h>

struct DBFile {
    uint64_t file_id;
    std::string file_path;
    uint64_t file_size;
    std::string original_hash;
};

struct DBBlock {
    uint64_t block_id;
    std::string block_hash;
    int size;
    int compressed_size;
};

struct DBVersion {
    uint64_t version_id;
    uint64_t file_id;
    uint64_t parent_id;
    std::string file_hash;
    uint64_t created_at;
};

class MetadataDB {
public:
    ~MetadataDB();
    
    // Open DB connection and create tables if needed
    void initialize(const std::string& db_path);

    // File Operations
    uint64_t getOrCreateFile(const std::string& path);
    
    // Block Operations
    // Returns block_id. If block exists, returns existing ID
    uint64_t storeBlock(const std::string& hash, int size, int compressed_size);
    
    // Version Operations
    uint64_t createVersion(
        uint64_t file_id, 
        const std::string& file_hash, 
        const std::vector<uint64_t>& block_ids,
        uint64_t parent_id = 0
    );

    // Queries
    std::vector<std::string> getVersionBlockHashes(uint64_t version_id);

private:
    sqlite3* db = nullptr;
    std::mutex db_mutex;
    
    void executeSQL(const std::string& sql);
    int64_t getLastInsertId();
};
