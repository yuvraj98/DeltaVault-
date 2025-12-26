#pragma once

#include <string>
#include <vector>
#include <unordered_map>
#include <memory>
#include <ctime>

struct VersionNode {
    uint64_t version_id;
    uint64_t parent_id; // 0 if root
    std::string file_path;
    std::vector<std::string> block_hashes; // Ordered list of blocks to reconstruct file
    std::string file_hash;
    uint64_t file_size;
    std::time_t created_at;
};

class VersionGraph {
public:
    uint64_t createVersion(
        const std::string& file_path,
        const std::vector<std::string>& block_hashes,
        const std::string& file_hash,
        uint64_t file_size,
        uint64_t parent_id = 0
    );

    VersionNode getVersion(uint64_t version_id);

    // Simple persistence for Phase 2.1 (JSON-like text)
    void saveGraph(const std::string& path);
    void loadGraph(const std::string& path);

private:
    std::unordered_map<uint64_t, VersionNode> nodes;
    uint64_t next_id = 1;
};
