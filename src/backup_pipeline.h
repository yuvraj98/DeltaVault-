#pragma once

#include <string>
#include <memory>
#include <mutex>

class FileScanner;
class BlockSplitter;
class HashEngine;
class StorageManager;
class MetadataDB;
class ThreadPool;

class BackupPipeline {
public:
    BackupPipeline(
        std::shared_ptr<FileScanner> scanner,
        std::shared_ptr<BlockSplitter> splitter,
        std::shared_ptr<HashEngine> hasher,
        std::shared_ptr<StorageManager> storage,
        std::shared_ptr<MetadataDB> db,
        std::shared_ptr<ThreadPool> thread_pool
    );

    // Run the backup for a given file path
    // Returns the Version ID created
    uint64_t runBackup(const std::string& file_path);

private:
    std::shared_ptr<FileScanner> scanner;
    std::shared_ptr<BlockSplitter> splitter;
    std::shared_ptr<HashEngine> hasher;
    std::shared_ptr<StorageManager> storage;
    std::shared_ptr<MetadataDB> db;
    std::shared_ptr<ThreadPool> thread_pool;
};
