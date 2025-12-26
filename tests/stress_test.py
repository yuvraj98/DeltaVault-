import os
import sys
import subprocess
import hashlib
import time
import shutil
import random

CLI_PATH = os.path.join("build", "Debug", "deltavault_cli.exe")
TEST_DIR = "test_env"
LARGE_FILE_NAME = "large_test_file.bin"
LARGE_FILE_SIZE = 100 * 1024 * 1024 # 100 MB for quick test, can be increased to 1GB
STORAGE_DIR = ".deltavault_test"

def calculate_sha256(file_path):
    sha256_hash = hashlib.sha256()
    with open(file_path, "rb") as f:
        for byte_block in iter(lambda: f.read(4096), b""):
            sha256_hash.update(byte_block)
    return sha256_hash.hexdigest()

def generate_large_file(file_path, size):
    print(f"Generating {size / (1024*1024):.2f} MB file: {file_path}")
    with open(file_path, "wb") as f:
        # Write random chunks
        chunk_size = 1024 * 1024
        bytes_written = 0
        while bytes_written < size:
            to_write = min(chunk_size, size - bytes_written)
            f.write(os.urandom(to_write))
            bytes_written += to_write

def run_backup(file_path):
    print(f"Running Backup for {file_path}...")
    start_time = time.time()
    result = subprocess.run([CLI_PATH, file_path], capture_output=True, text=True)
    end_time = time.time()
    
    if result.returncode != 0:
        print("Backup Failed!")
        print(result.stderr)
        return False
    
    print(f"Backup Completed in {end_time - start_time:.2f} seconds.")
    # Extract Version ID (Naive parsing)
    for line in result.stdout.splitlines():
        if "Version ID:" in line:
            return int(line.split(":")[-1].strip())
    return None

def verify_restore(original_path, restored_path):
    print("Verifying correctness...")
    hash_orig = calculate_sha256(original_path)
    hash_rest = calculate_sha256(restored_path)
    
    print(f"Original Hash: {hash_orig}")
    print(f"Restored Hash: {hash_rest}")
    
    if hash_orig == hash_rest:
        print("SUCCESS: Hashes Match.")
        return True
    else:
        print("FAILURE: Hash Mismatch!")
        return False

def test_large_file():
    print("--- STARTING LARGE FILE TEST ---")
    if not os.path.exists(TEST_DIR):
        os.makedirs(TEST_DIR)
        
    fpath = os.path.join(TEST_DIR, LARGE_FILE_NAME)
    if not os.path.exists(fpath):
        generate_large_file(fpath, LARGE_FILE_SIZE)
    
    version_id = run_backup(fpath)
    if version_id is None:
        print("Could not get Version ID.")
        return

    # The CLI automatically does a restore verify step to <path>.restored
    restored_path = fpath + ".restored"
    if os.path.exists(restored_path):
        verify_restore(fpath, restored_path)
    else:
        print("Restored file not found.")

def test_corruption():
    print("\n--- STARTING CORRUPTION TEST ---")
    # Clean up previous data to ensure we corrupt the right block
    if os.path.exists(STORAGE_DIR):
        shutil.rmtree(STORAGE_DIR)
        
    # 1. Create small file
    fpath = os.path.join(TEST_DIR, "corruption_test.txt")
    with open(fpath, "w") as f:
        f.write("This is a test string for corruption check." * 100)
    
    # 2. Backup
    print("Backing up file...")
    subprocess.run([CLI_PATH, fpath], capture_output=True)
    
    # 3. Find a block and corrupt it
    blocks_dir = os.path.join(STORAGE_DIR, "blocks")
    blocks = [f for f in os.listdir(blocks_dir) if os.path.isfile(os.path.join(blocks_dir, f))]
    
    if not blocks:
        print("No blocks found to corrupt.")
        return

    target_block = os.path.join(blocks_dir, blocks[0])
    print(f"Corrupting block: {target_block}")
    
    with open(target_block, "r+b") as f:
        f.seek(0)
        f.write(b"CORRUPT")
        
    # 4. Try Restore (Run CLI again)
    print("Running Restore (expecting failure or mismatch)...")
    subprocess.run([CLI_PATH, fpath], capture_output=True, text=True)
    
    restored_path = fpath + ".restored"
    if os.path.exists(restored_path):
        h1 = calculate_sha256(fpath)
        h2 = calculate_sha256(restored_path)
        if h1 != h2:
             print("SUCCESS: Corruption detected (Hashes do not match).")
        else:
             print("FAILURE: Hashes match despite corruption! (Did dedup use cached data?)")
    else:
        print("Refusal to restore? (Could be valid behavior)")

if __name__ == "__main__":
    test_large_file()
    # Note: Corruption test modifies the global storage, might affect other tests if not cleaned
    # For now running it second.
    test_corruption() 
    # For now, let's just run large file test.
