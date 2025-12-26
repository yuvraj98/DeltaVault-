#pragma once

#include <QMainWindow>
#include <QPushButton>
#include <QLineEdit>
#include <QTextEdit>
#include <QLabel>
#include <QProgressBar>
#include <QStatusBar>
#include <memory>

#include "file_scanner.h"
#include "block_splitter.h"
#include "hash_engine.h"
#include "storage_manager.h"
#include "metadata_db.h"
#include "thread_pool.h"
#include "backup_pipeline.h"
#include "restore_manager.h"

class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    explicit MainWindow(QWidget* parent = nullptr);
    ~MainWindow();

private slots:
    void onBrowseFile();
    void onBackup();
    void onRestore();

private:
    void setupUi();
    void logMessage(const QString& msg);

    // UI Elements
    QLineEdit* filePathEdit;
    QPushButton* browseButton;
    QPushButton* backupButton;
    QPushButton* restoreButton;
    QTextEdit* logArea;
    QProgressBar* progressBar;
    QLabel* statusLabel;
    QLineEdit* versionIdEdit; // Simple input for restoration for now

    // Core Components
    std::shared_ptr<FileScanner> scanner;
    std::shared_ptr<BlockSplitter> splitter;
    std::shared_ptr<HashEngine> hasher;
    std::shared_ptr<StorageManager> storage;
    std::shared_ptr<MetadataDB> db;
    std::shared_ptr<ThreadPool> threadPool;
    std::unique_ptr<BackupPipeline> pipeline;
};
