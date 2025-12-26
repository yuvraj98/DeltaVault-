#include "metadata_db.h"
#include <stdexcept>
#include <iostream>

MetadataDB::~MetadataDB() {
    if (db) {
        sqlite3_close(db);
    }
}

void MetadataDB::executeSQL(const std::string& sql) {
    char* errMsg = nullptr;
    if (sqlite3_exec(db, sql.c_str(), nullptr, nullptr, &errMsg) != SQLITE_OK) {
        std::string err = errMsg;
        sqlite3_free(errMsg);
        throw std::runtime_error("SQL Error: " + err);
    }
}

int64_t MetadataDB::getLastInsertId() {
    return sqlite3_last_insert_rowid(db);
}

void MetadataDB::initialize(const std::string& db_path) {
    if (sqlite3_open(db_path.c_str(), &db) != SQLITE_OK) {
        throw std::runtime_error("Failed to open DB: " + db_path);
    }

    // Create Tables
    const char* schema = R"(
        CREATE TABLE IF NOT EXISTS files (
            file_id INTEGER PRIMARY KEY,
            file_path TEXT UNIQUE,
            created_at INTEGER
        );
        CREATE TABLE IF NOT EXISTS blocks (
            block_id INTEGER PRIMARY KEY,
            block_hash TEXT UNIQUE,
            size INTEGER,
            compressed_size INTEGER
        );
        CREATE TABLE IF NOT EXISTS versions (
            version_id INTEGER PRIMARY KEY,
            file_id INTEGER,
            parent_id INTEGER,
            file_hash TEXT,
            created_at INTEGER,
            FOREIGN KEY(file_id) REFERENCES files(file_id)
        );
        CREATE TABLE IF NOT EXISTS file_blocks (
            version_id INTEGER,
            block_sequence INTEGER,
            block_id INTEGER,
            PRIMARY KEY(version_id, block_sequence),
            FOREIGN KEY(version_id) REFERENCES versions(version_id),
            FOREIGN KEY(block_id) REFERENCES blocks(block_id)
        );
    )";
    executeSQL(schema);
}

uint64_t MetadataDB::getOrCreateFile(const std::string& path) {
    sqlite3_stmt* stmt;
    std::string sql = "SELECT file_id FROM files WHERE file_path = ?";
    
    if (sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, nullptr) != SQLITE_OK) {
        throw std::runtime_error("Prepare failed");
    }
    
    sqlite3_bind_text(stmt, 1, path.c_str(), -1, SQLITE_STATIC);
    
    uint64_t id = 0;
    if (sqlite3_step(stmt) == SQLITE_ROW) {
        id = sqlite3_column_int64(stmt, 0);
    }
    sqlite3_finalize(stmt);

    if (id != 0) return id;

    // Insert
    sql = "INSERT INTO files (file_path, created_at) VALUES (?, ?)";
    if (sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, nullptr) != SQLITE_OK) throw std::runtime_error("Prepare failed");
    
    sqlite3_bind_text(stmt, 1, path.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_int64(stmt, 2, std::time(nullptr));
    
    if (sqlite3_step(stmt) != SQLITE_DONE) throw std::runtime_error("Insert file failed");
    sqlite3_finalize(stmt);
    
    return getLastInsertId();
}

uint64_t MetadataDB::storeBlock(const std::string& hash, int size, int compressed_size) {
    std::lock_guard<std::mutex> lock(db_mutex);
    // Check existence first
    sqlite3_stmt* stmt;
    std::string sql = "SELECT block_id FROM blocks WHERE block_hash = ?";
    if (sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, nullptr) != SQLITE_OK) throw std::runtime_error("Prepare failed");
    sqlite3_bind_text(stmt, 1, hash.c_str(), -1, SQLITE_STATIC);
    
    uint64_t id = 0;
    if (sqlite3_step(stmt) == SQLITE_ROW) {
        id = sqlite3_column_int64(stmt, 0);
    }
    sqlite3_finalize(stmt);

    if (id != 0) return id;

    // Insert
    sql = "INSERT INTO blocks (block_hash, size, compressed_size) VALUES (?, ?, ?)";
    if (sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, nullptr) != SQLITE_OK) throw std::runtime_error("Prepare failed");
    sqlite3_bind_text(stmt, 1, hash.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_int(stmt, 2, size);
    sqlite3_bind_int(stmt, 3, compressed_size);

    if (sqlite3_step(stmt) != SQLITE_DONE) throw std::runtime_error("Insert block failed");
    sqlite3_finalize(stmt);

    return getLastInsertId();
}

uint64_t MetadataDB::createVersion(
    uint64_t file_id, 
    const std::string& file_hash, 
    const std::vector<uint64_t>& block_ids,
    uint64_t parent_id
) {
    executeSQL("BEGIN TRANSACTION");
    try {
        sqlite3_stmt* stmt;
        std::string sql = "INSERT INTO versions (file_id, parent_id, file_hash, created_at) VALUES (?, ?, ?, ?)";
        if (sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, nullptr) != SQLITE_OK) throw std::runtime_error("Prepare version failed");
        
        sqlite3_bind_int64(stmt, 1, file_id);
        sqlite3_bind_int64(stmt, 2, parent_id);
        sqlite3_bind_text(stmt, 3, file_hash.c_str(), -1, SQLITE_STATIC);
        sqlite3_bind_int64(stmt, 4, std::time(nullptr));

        if (sqlite3_step(stmt) != SQLITE_DONE) throw std::runtime_error("Step version failed");
        sqlite3_finalize(stmt);

        uint64_t version_id = getLastInsertId();

        // Insert mappings
        sql = "INSERT INTO file_blocks (version_id, block_sequence, block_id) VALUES (?, ?, ?)";
        if (sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, nullptr) != SQLITE_OK) throw std::runtime_error("Prepare mapping failed");

        int seq = 0;
        for (uint64_t bid : block_ids) {
            sqlite3_reset(stmt);
            sqlite3_bind_int64(stmt, 1, version_id);
            sqlite3_bind_int(stmt, 2, seq++);
            sqlite3_bind_int64(stmt, 3, bid);
            if (sqlite3_step(stmt) != SQLITE_DONE) throw std::runtime_error("Step mapping failed");
        }
        sqlite3_finalize(stmt);

        executeSQL("COMMIT");
        return version_id;
    } catch (...) {
        executeSQL("ROLLBACK");
        throw;
    }
}

std::vector<std::string> MetadataDB::getVersionBlockHashes(uint64_t version_id) {
    std::vector<std::string> hashes;
    sqlite3_stmt* stmt;
    std::string sql = R"(
        SELECT b.block_hash 
        FROM file_blocks fb
        JOIN blocks b ON fb.block_id = b.block_id
        WHERE fb.version_id = ?
        ORDER BY fb.block_sequence ASC
    )";

    if (sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, nullptr) != SQLITE_OK) throw std::runtime_error("Prepare query failed");
    sqlite3_bind_int64(stmt, 1, version_id);

    while (sqlite3_step(stmt) == SQLITE_ROW) {
        const char* h = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 0));
        hashes.push_back(h ? std::string(h) : "");
    }
    sqlite3_finalize(stmt);
    return hashes;
}
