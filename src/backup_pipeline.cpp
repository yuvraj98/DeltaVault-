#include "backup_pipeline.h"
#include "file_scanner.h"
#include "block_splitter.h"
#include "hash_engine.h"
#include "storage_manager.h"
#include "metadata_db.h"
#include "thread_pool.h"
#include <iostream>
#include <future>

BackupPipeline::BackupPipeline(
    std::shared_ptr<FileScanner> scanner,
    std::shared_ptr<BlockSplitter> splitter,
    std::shared_ptr<HashEngine> hasher,
    std::shared_ptr<StorageManager> storage,
    std::shared_ptr<MetadataDB> db,
    std::shared_ptr<ThreadPool> thread_pool
) : scanner(scanner), splitter(splitter), hasher(hasher), storage(storage), db(db), thread_pool(thread_pool) {}

uint64_t BackupPipeline::runBackup(const std::string& file_path) {
    // 1. Scan and register file
    auto metadata = scanner->getFileMetadata(file_path);
    uint64_t file_id = db->getOrCreateFile(file_path);
    std::string full_file_hash = scanner->hashFile(file_path);

    // 2. Split file
    auto blocks = splitter->splitFile(file_path);
    
    // 3. Process blocks in parallel
    std::vector<std::future<uint64_t>> futures;
    futures.reserve(blocks.size());

    // We need to maintain order of block ids for the version creation
    // But threads finish out of order. We can store futures in order and retrieve results in order.
    
    for (const auto& block_data : blocks) {
        // Enqueue job
        futures.push_back(thread_pool->enqueue([this, block_data]() -> uint64_t {
            std::string hash = this->hasher->computeBlockHash(block_data);
            
            // Optimization: Could check DB here if block exists before compressing
            // For now, continuing with previous logic for robustness
            auto [compressed, _] = this->hasher->compressBlock(block_data);
            
            this->storage->writeBlock(hash, compressed);
            return this->db->storeBlock(hash, block_data.size(), compressed.size());
        }));
    }

    // 4. Collect results (waits for all threads)
    std::vector<uint64_t> block_ids;
    block_ids.reserve(futures.size());
    for (auto& f : futures) {
        block_ids.push_back(f.get());
    }

    // 5. Create Version
    return db->createVersion(file_id, full_file_hash, block_ids);
}
