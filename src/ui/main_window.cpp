#include "main_window.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFileDialog>
#include <QMessageBox>
#include <QDateTime>
#include <QApplication>
#include <thread>
#include <QFileInfo>

// Core Includes
// Core Includes are now in main_window.h

MainWindow::MainWindow(QWidget* parent) : QMainWindow(parent) {
    setupUi();

    // Initialize Core Components
    try {
        scanner = std::make_shared<FileScanner>();
        splitter = std::make_shared<BlockSplitter>();
        hasher = std::make_shared<HashEngine>();
        storage = std::make_shared<StorageManager>();
        db = std::make_shared<MetadataDB>();
        
        size_t threads = std::thread::hardware_concurrency();
        if (threads == 0) threads = 4;
        threadPool = std::make_shared<ThreadPool>(threads);

        // Initialize persistent storage
        storage->initialize("./.deltavault");
        db->initialize("./.deltavault/metadata.db");

        // Explicitly using new to avoid make_unique template issues if any
        pipeline.reset(new BackupPipeline(
            scanner, splitter, hasher, storage, db, threadPool
        ));
        
        logMessage("System Initialized. Storage: ./.deltavault");
    } catch (const std::exception& e) {
        QMessageBox::critical(this, "Initialization Error", e.what());
    }
}

MainWindow::~MainWindow() {
    // Destructor definition is required for unique_ptr of forward declared class
}

void MainWindow::setupUi() {
    setWindowTitle("DeltaVault - Incremental Backup Engine");
    resize(800, 600);

    QWidget* centralWidget = new QWidget(this);
    setCentralWidget(centralWidget);
    QVBoxLayout* mainLayout = new QVBoxLayout(centralWidget);

    // --- File Selection Area ---
    QHBoxLayout* fileLayout = new QHBoxLayout();
    filePathEdit = new QLineEdit();
    filePathEdit->setPlaceholderText("Select a file to backup...");
    browseButton = new QPushButton("Browse...");
    fileLayout->addWidget(filePathEdit);
    fileLayout->addWidget(browseButton);
    mainLayout->addLayout(fileLayout);

    // --- Action Buttons ---
    QHBoxLayout* actionLayout = new QHBoxLayout();
    backupButton = new QPushButton("Start Backup");
    backupButton->setStyleSheet("background-color: #4CAF50; color: white; font-weight: bold; padding: 10px;");
    
    // Restore Section
    QLabel* restoreLabel = new QLabel("Restore Version ID:");
    versionIdEdit = new QLineEdit();
    versionIdEdit->setPlaceholderText("ID");
    versionIdEdit->setFixedWidth(100);
    restoreButton = new QPushButton("Restore");
    
    actionLayout->addWidget(backupButton);
    actionLayout->addStretch();
    actionLayout->addWidget(restoreLabel);
    actionLayout->addWidget(versionIdEdit);
    actionLayout->addWidget(restoreButton);
    mainLayout->addLayout(actionLayout);

    // --- Progress & Status ---
    progressBar = new QProgressBar();
    progressBar->setValue(0);
    mainLayout->addWidget(progressBar);

    // --- Log Area ---
    logArea = new QTextEdit();
    logArea->setReadOnly(true);
    mainLayout->addWidget(logArea);

    statusLabel = new QLabel("Ready");
    statusBar()->addWidget(statusLabel);

    // Connect Signals
    connect(browseButton, &QPushButton::clicked, this, &MainWindow::onBrowseFile);
    connect(backupButton, &QPushButton::clicked, this, &MainWindow::onBackup);
    connect(restoreButton, &QPushButton::clicked, this, &MainWindow::onRestore);
}

void MainWindow::logMessage(const QString& msg) {
    QString timestamp = QDateTime::currentDateTime().toString("yyyy-MM-dd HH:mm:ss");
    logArea->append(QString("[%1] %2").arg(timestamp, msg));
}

void MainWindow::onBrowseFile() {
    QString fileName = QFileDialog::getOpenFileName(this, "Select File to Backup");
    if (!fileName.isEmpty()) {
        filePathEdit->setText(fileName);
    }
}

void MainWindow::onBackup() {
    QString path = filePathEdit->text();
    if (path.isEmpty()) {
        QMessageBox::warning(this, "Warning", "Please select a file first.");
        return;
    }

    backupButton->setEnabled(false);
    restoreButton->setEnabled(false);
    progressBar->setRange(0, 0); // Indeterminate mode
    statusLabel->setText("Backing up...");
    logMessage("Starting backup for: " + path);

    std::thread([this, path]() {
        try {
            uint64_t vid = pipeline->runBackup(path.toStdString());
            
            QMetaObject::invokeMethod(this, [this, vid]() {
                logMessage(QString("Backup Successful! Created Version ID: %1").arg(vid));
                statusLabel->setText("Backup Complete");
                progressBar->setRange(0, 100);
                progressBar->setValue(100);
                backupButton->setEnabled(true);
                restoreButton->setEnabled(true);
                versionIdEdit->setText(QString::number(vid));
            });
        } catch (const std::exception& e) {
             QString err = e.what();
             QMetaObject::invokeMethod(this, [this, err]() {
                logMessage(QString("Backup Failed: %1").arg(err));
                statusLabel->setText("Error");
                progressBar->setRange(0, 100);
                backupButton->setEnabled(true);
                restoreButton->setEnabled(true);
            });
        }
    }).detach();
}

void MainWindow::onRestore() {
    QString path = filePathEdit->text();
    QString vidStr = versionIdEdit->text();
    if (path.isEmpty() || vidStr.isEmpty()) {
        QMessageBox::warning(this, "Warning", "Select a file path (for output name) and provide a Version ID.");
        return;
    }

    QFileInfo fi(path);
    QString restorePath;
    if (!fi.suffix().isEmpty()) {
        restorePath = fi.absolutePath() + "/" + fi.completeBaseName() + ".restored." + fi.suffix();
    } else {
        restorePath = path + ".restored";
    }
    uint64_t vid = vidStr.toULongLong();

    backupButton->setEnabled(false);
    restoreButton->setEnabled(false);
    statusLabel->setText("Restoring...");
    logMessage(QString("Restoring Version %1 to %2").arg(vid).arg(restorePath));

     std::thread([this, vid, restorePath]() {
        try {
            RestoreManager restorer(db, storage, hasher);
            restorer.restoreFile(vid, restorePath.toStdString());

            QMetaObject::invokeMethod(this, [this, restorePath]() {
                logMessage("Restore Successful! File saved to: " + restorePath);
                statusLabel->setText("Restore Complete");
                backupButton->setEnabled(true);
                restoreButton->setEnabled(true);
            });
        } catch (const std::exception& e) {
             QString err = e.what();
             QMetaObject::invokeMethod(this, [this, err]() {
                logMessage(QString("Restore Failed: %1").arg(err));
                statusLabel->setText("Error");
                backupButton->setEnabled(true);
                restoreButton->setEnabled(true);
            });
        }
    }).detach();
}
