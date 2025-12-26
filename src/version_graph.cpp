#include "version_graph.h"
#include <fstream>
#include <sstream>
#include <iostream>
#include <stdexcept>

uint64_t VersionGraph::createVersion(
    const std::string& file_path,
    const std::vector<std::string>& block_hashes,
    const std::string& file_hash,
    uint64_t file_size,
    uint64_t parent_id
) {
    VersionNode node;
    node.version_id = next_id++;
    node.parent_id = parent_id;
    node.file_path = file_path;
    node.block_hashes = block_hashes;
    node.file_hash = file_hash;
    node.file_size = file_size;
    node.created_at = std::time(nullptr);

    nodes[node.version_id] = node;
    return node.version_id;
}

VersionNode VersionGraph::getVersion(uint64_t version_id) {
    if (nodes.find(version_id) == nodes.end()) {
        throw std::runtime_error("Version not found");
    }
    return nodes[version_id];
}

void VersionGraph::saveGraph(const std::string& path) {
    std::ofstream file(path);
    if (!file) return;

    file << nodes.size() << "\n";
    for (const auto& [id, node] : nodes) {
        file << node.version_id << "|"
             << node.parent_id << "|"
             << node.file_path << "|"
             << node.file_hash << "|"
             << node.file_size << "|"
             << node.created_at << "\n";
        
        // Write block hashes space-separated
        for (const auto& b : node.block_hashes) {
            file << b << " ";
        }
        file << "\n";
    }
}

void VersionGraph::loadGraph(const std::string& path) {
    std::ifstream file(path);
    if (!file) return;

    size_t count;
    file >> count;
    // Consume newline
    std::string dummy;
    std::getline(file, dummy); 

    for (size_t i = 0; i < count; ++i) {
        VersionNode node;
        std::string line;
        
        // Read Metadata
        if (!std::getline(file, line)) break;
        std::stringstream ss(line);
        std::string segment;
        
        std::vector<std::string> parts;
        while(std::getline(ss, segment, '|')) {
            parts.push_back(segment);
        }

        if (parts.size() < 6) continue;

        node.version_id = std::stoull(parts[0]);
        node.parent_id = std::stoull(parts[1]);
        node.file_path = parts[2];
        node.file_hash = parts[3];
        node.file_size = std::stoull(parts[4]);
        node.created_at = std::stoll(parts[5]);

        // Read Blocks
        if (!std::getline(file, line)) break;
        std::stringstream ss_blocks(line);
        std::string block;
        while (ss_blocks >> block) {
            node.block_hashes.push_back(block);
        }

        nodes[node.version_id] = node;
        if (node.version_id >= next_id) {
            next_id = node.version_id + 1;
        }
    }
}
