#pragma once

#include <string>
#include <vector>
#include <cstdint>

class BlockSplitter {
public:
    static constexpr size_t BLOCK_SIZE = 256 * 1024; // 256KB blocks

    // Split file into blocks, return block data vectors
    // Note: For very large files, returning vector<vector<uint8_t>> is memory intensive
    // In later phases this should utilize a callback or iterator. 
    // Implementing basic version for Phase 1 as requested.
    std::vector<std::vector<uint8_t>> splitFile(const std::string& file_path);

    // Get block count for a file
    size_t getBlockCount(size_t file_size);
};
