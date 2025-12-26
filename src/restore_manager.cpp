#include "restore_manager.h"
#include <fstream>
#include <iostream>

RestoreManager::RestoreManager(
    std::shared_ptr<MetadataDB> db,
    std::shared_ptr<StorageManager> storage,
    std::shared_ptr<HashEngine> hasher
) : db(db), storage(storage), hasher(hasher) {}

void RestoreManager::restoreFile(uint64_t version_id, const std::string& output_path) {
    auto block_hashes = db->getVersionBlockHashes(version_id);

    std::ofstream out_file(output_path, std::ios::binary);
    if (!out_file) {
        throw std::runtime_error("Failed to create output file: " + output_path);
    }

    for (const auto& hash : block_hashes) {
        auto compressed_data = storage->readBlock(hash);
        auto block_data = hasher->decompressBlock(compressed_data);
        out_file.write(reinterpret_cast<const char*>(block_data.data()), block_data.size());
    }
    
    out_file.close();
}
