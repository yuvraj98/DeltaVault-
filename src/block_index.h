#pragma once

#include <string>
#include <unordered_map>
#include <mutex>
#include <cstdint>
#include <chrono>

struct BlockMetadata {
    uint64_t block_id;
    std::string block_hash;
    size_t original_size;
    size_t compressed_size;
    std::string compression_algo; // "zstd", "none"
    uint64_t storage_offset;      // Placeholder for now
    std::chrono::system_clock::time_point created_at;
    uint32_t reference_count;
};

class BlockIndex {
public:
    // Add new block to index, return BlockId. 
    // If block exists, increments ref count and returns existing ID.
    uint64_t addBlock(const std::string& block_hash, const BlockMetadata& metadata);

    // Check if block already exists (dedup)
    bool blockExists(const std::string& block_hash);

    // Get BlockId for hash
    uint64_t getBlockId(const std::string& block_hash);

    // Get metadata for block
    BlockMetadata getBlockMetadata(uint64_t block_id);

private:
    std::unordered_map<std::string, uint64_t> hash_to_block_id; // SHA256 -> BlockId
    std::unordered_map<uint64_t, BlockMetadata> id_to_metadata;
    
    std::mutex index_mutex;
    uint64_t next_block_id = 1;
};
