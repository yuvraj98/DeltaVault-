#pragma once

#include <string>
#include <memory>
#include "metadata_db.h"
#include "storage_manager.h"
#include "hash_engine.h"

class RestoreManager {
public:
    RestoreManager(
        std::shared_ptr<MetadataDB> db,
        std::shared_ptr<StorageManager> storage,
        std::shared_ptr<HashEngine> hasher
    );

    // Reconstruct file from version
    void restoreFile(uint64_t version_id, const std::string& output_path);

private:
    std::shared_ptr<MetadataDB> db;
    std::shared_ptr<StorageManager> storage;
    std::shared_ptr<HashEngine> hasher;
};
