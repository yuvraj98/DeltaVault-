cmake_minimum_required(VERSION 3.16)
project(DeltaVault VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Enable Solution Folders for Visual Studio
set_property(GLOBAL PROPERTY USE_FOLDERS ON)

# Dependencies
find_package(OpenSSL REQUIRED)
find_package(zstd CONFIG REQUIRED)
find_package(unofficial-sqlite3 CONFIG REQUIRED)

# Core library
add_library(deltavault_core STATIC
    src/file_scanner.cpp
    src/block_splitter.cpp
    src/hash_engine.cpp
    src/block_index.cpp
    src/storage_manager.cpp
    src/version_graph.cpp
    src/metadata_db.cpp
    src/restore_manager.cpp
    src/thread_pool.cpp
    src/backup_pipeline.cpp
)

target_include_directories(deltavault_core PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/src)

target_link_libraries(deltavault_core
    PUBLIC
    unofficial::sqlite3::sqlite3
    OpenSSL::Crypto
    zstd::libzstd_shared
)

# Organize Core library into a "Core" folder
set_target_properties(deltavault_core PROPERTIES FOLDER "Core")

# CLI / Test Executable for Phase 1
add_executable(deltavault_cli src/main.cpp)

target_link_libraries(deltavault_cli
    PRIVATE
    deltavault_core
)

# Organize CLI executable into an "Apps" folder
set_target_properties(deltavault_cli PROPERTIES FOLDER "Apps")


# --- UI Application (Phase 4) ---
find_package(Qt6 COMPONENTS Core Gui Widgets REQUIRED)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

add_executable(deltavault_ui 
    src/main_ui.cpp
    src/ui/main_window.cpp
)

target_link_libraries(deltavault_ui 
    PRIVATE 
    deltavault_core 
    Qt6::Core 
    Qt6::Gui 
    Qt6::Widgets
)

target_include_directories(deltavault_ui PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src/ui)
set_target_properties(deltavault_ui PROPERTIES FOLDER "Apps")

enable_testing()
