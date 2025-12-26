#include <iostream>
#include <vector>
#include <memory>
#include <filesystem>
#include "file_scanner.h"
#include "block_splitter.h"
#include "hash_engine.h"
#include "storage_manager.h"
#include "metadata_db.h"
#include "restore_manager.h"
#include "thread_pool.h"
#include "backup_pipeline.h"

int main(int argc, char* argv[]) {
    if (argc < 2) {
        std::cout << "Usage: deltavault_cli <file_or_directory_path>" << std::endl;
        return 1;
    }

    std::string path = argv[1];

    if (std::filesystem::is_directory(path)) {
       std::cout << "Directory scanning not yet fully supported in pipeline. Please pass a file." << std::endl;
       return 1;
    } 

    std::cout << "Processing file: " << path << std::endl;
    
    // Initialize Components
    auto scanner = std::make_shared<FileScanner>();
    auto splitter = std::make_shared<BlockSplitter>();
    auto hasher = std::make_shared<HashEngine>();
    auto storage = std::make_shared<StorageManager>();
    auto db = std::make_shared<MetadataDB>();
    auto tp = std::make_shared<ThreadPool>(4); // Use 4 threads

    // Configure
    storage->initialize("./.deltavault_test");
    db->initialize("./.deltavault_test/metadata.db");

    // Initialize Pipeline
    BackupPipeline pipeline(scanner, splitter, hasher, storage, db, tp);

    // Run Backup
    std::cout << "Starting Parallel Backup..." << std::endl;
    uint64_t vid = pipeline.runBackup(path);
    std::cout << "Backup Pipeline Completed. Version ID: " << vid << std::endl;

    // --- Restore Verification ---
    std::cout << "\n--- Verifying Restore ---" << std::endl;
    RestoreManager restorer(db, storage, hasher);
    std::string restore_path = path + ".restored";
    
    restorer.restoreFile(vid, restore_path);
    std::cout << "Restored to: " << restore_path << std::endl;
    
    std::string restored_hash = scanner->hashFile(restore_path);
    std::string original_hash = scanner->hashFile(path);
    
    std::cout << "Original Hash: " << original_hash << std::endl;
    std::cout << "Restored Hash: " << restored_hash << std::endl;
    
    if (original_hash == restored_hash) {
        std::cout << "SUCCESS: Integration Test Passed!" << std::endl;
    } else {
        std::cout << "FAILURE: Hashes do not match!" << std::endl;
    }

    return 0;
}
