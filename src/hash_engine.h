#pragma once

#include <string>
#include <vector>
#include <utility>

class HashEngine {
public:
    // Compute SHA-256 hash of block data, returning hex string
    std::string computeBlockHash(const std::vector<uint8_t>& block_data);

    // Compress block using zstd (return compressed data + original size for reference)
    // Default compression level 3 is a good balance
    std::pair<std::vector<uint8_t>, size_t> compressBlock(
        const std::vector<uint8_t>& block_data,
        int compression_level = 3
    );

    // Decompress block
    std::vector<uint8_t> decompressBlock(const std::vector<uint8_t>& compressed_data);
};
