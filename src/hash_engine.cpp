#include "hash_engine.h"
#include <openssl/sha.h>
#include <zstd.h>
#include <sstream>
#include <iomanip>
#include <stdexcept>
#include <vector>

std::string HashEngine::computeBlockHash(const std::vector<uint8_t>& block_data) {
    unsigned char hash[SHA256_DIGEST_LENGTH];
    SHA256_CTX sha256_ctx;
    SHA256_Init(&sha256_ctx);
    SHA256_Update(&sha256_ctx, block_data.data(), block_data.size());
    SHA256_Final(hash, &sha256_ctx);

    // Convert to hex string
    std::stringstream ss;
    for (int i = 0; i < SHA256_DIGEST_LENGTH; i++) {
        ss << std::hex << std::setw(2) << std::setfill('0') << (int)hash[i];
    }
    return ss.str();
}

std::pair<std::vector<uint8_t>, size_t> HashEngine::compressBlock(
    const std::vector<uint8_t>& block_data,
    int compression_level
) {
    size_t max_compressed_size = ZSTD_compressBound(block_data.size());
    std::vector<uint8_t> compressed(max_compressed_size);

    size_t compressed_size = ZSTD_compress(
        compressed.data(),
        max_compressed_size,
        block_data.data(),
        block_data.size(),
        compression_level
    );

    if (ZSTD_isError(compressed_size)) {
        throw std::runtime_error(std::string("Compression failed: ") + ZSTD_getErrorName(compressed_size));
    }

    compressed.resize(compressed_size);
    return {compressed, block_data.size()};
}

std::vector<uint8_t> HashEngine::decompressBlock(const std::vector<uint8_t>& compressed_data) {
    unsigned long long decompressed_size = ZSTD_getFrameContentSize(
        compressed_data.data(),
        compressed_data.size()
    );

    if (decompressed_size == ZSTD_CONTENTSIZE_ERROR) {
        throw std::runtime_error("Not compressed by zstd");
    }
    if (decompressed_size == ZSTD_CONTENTSIZE_UNKNOWN) {
        throw std::runtime_error("Original size unknown");
    }

    std::vector<uint8_t> decompressed(decompressed_size);
    size_t result = ZSTD_decompress(
        decompressed.data(),
        decompressed_size,
        compressed_data.data(),
        compressed_data.size()
    );

    if (ZSTD_isError(result)) {
        throw std::runtime_error(std::string("Decompression failed: ") + ZSTD_getErrorName(result));
    }

    return decompressed;
}
