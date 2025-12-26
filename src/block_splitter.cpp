#include "block_splitter.h"
#include <fstream>
#include <iostream>
#include <cmath>

std::vector<std::vector<uint8_t>> BlockSplitter::splitFile(const std::string& file_path) {
    std::vector<std::vector<uint8_t>> blocks;
    std::ifstream file(file_path, std::ios::binary);

    if (!file) {
        std::cerr << "Failed to open file: " << file_path << std::endl;
        return blocks;
    }

    // Pre-allocate buffer
    std::vector<uint8_t> buffer(BLOCK_SIZE);
    
    while (file.read(reinterpret_cast<char*>(buffer.data()), BLOCK_SIZE)) {
        blocks.push_back(buffer); 
    }

    // Handle last partial block
    if (file.gcount() > 0) {
        std::vector<uint8_t> last_block(buffer.begin(), buffer.begin() + file.gcount());
        blocks.push_back(last_block);
    }

    return blocks;
}

size_t BlockSplitter::getBlockCount(size_t file_size) {
    if (file_size == 0) return 0;
    return (file_size + BLOCK_SIZE - 1) / BLOCK_SIZE;
}
