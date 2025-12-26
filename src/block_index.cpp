#include "block_index.h"
#include <stdexcept>

uint64_t BlockIndex::addBlock(const std::string& block_hash, const BlockMetadata& metadata) {
    std::lock_guard<std::mutex> lock(index_mutex);

    // Check if already exists
    if (hash_to_block_id.find(block_hash) != hash_to_block_id.end()) {
        uint64_t existing_id = hash_to_block_id[block_hash];
        id_to_metadata[existing_id].reference_count++;
        return existing_id;
    }

    // Add new block
    uint64_t new_id = next_block_id++;
    BlockMetadata new_meta = metadata;
    new_meta.block_id = new_id;
    new_meta.reference_count = 1;
    new_meta.created_at = std::chrono::system_clock::now();

    hash_to_block_id[block_hash] = new_id;
    id_to_metadata[new_id] = new_meta;

    return new_id;
}

bool BlockIndex::blockExists(const std::string& block_hash) {
    std::lock_guard<std::mutex> lock(index_mutex);
    return hash_to_block_id.find(block_hash) != hash_to_block_id.end();
}

uint64_t BlockIndex::getBlockId(const std::string& block_hash) {
    std::lock_guard<std::mutex> lock(index_mutex);
    auto it = hash_to_block_id.find(block_hash);
    if (it == hash_to_block_id.end()) {
        throw std::runtime_error("Block hash not found: " + block_hash);
    }
    return it->second;
}

BlockMetadata BlockIndex::getBlockMetadata(uint64_t block_id) {
    std::lock_guard<std::mutex> lock(index_mutex);
    auto it = id_to_metadata.find(block_id);
    if (it == id_to_metadata.end()) {
        throw std::runtime_error("Block ID not found");
    }
    return it->second;
}
