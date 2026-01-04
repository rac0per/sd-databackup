#include <gtest/gtest.h>
#include <filesystem>
#include <fstream>
#include <random>
#include <string>
#include <vector>
#include <thread>
#include <chrono>

#include "encryption/Encryption.h"
#include "backup/BackupManager.h"

using namespace backup::core;
using namespace backup::core::encryption;
namespace fs = std::filesystem;

namespace {
    void writeFile(const fs::path &p, const std::string &content)
    {
        fs::create_directories(p.parent_path());
        std::ofstream(p, std::ios::binary) << content;
    }

    std::string readFile(const fs::path &p)
    {
        std::ifstream in(p, std::ios::binary);
        return std::string((std::istreambuf_iterator<char>(in)), std::istreambuf_iterator<char>());
    }
}

TEST(ExtraEdgeCases, AesDecryptionFailsOnCorruption)
{
    auto tmp = fs::temp_directory_path() / "aes_corrupt_test";
    fs::create_directories(tmp);
    auto in = tmp / "plain.txt";
    auto encfile = tmp / "enc.bin";
    auto out = tmp / "out.txt";

    writeFile(in, "sensitive payload for corruption test");

    auto enc = createEncryptor(EncryptionType::AES);
    enc->setKey("strongpass");
    enc->encrypt(in, encfile);

    // Corrupt the ciphertext by flipping the first byte
    std::fstream f(encfile, std::ios::in | std::ios::out | std::ios::binary);
    ASSERT_TRUE(f.is_open());
    char c;
    f.read(&c, 1);
    f.seekp(0);
    c = static_cast<char>(c ^ 0xFF);
    f.write(&c, 1);
    f.close();

    auto dec = createEncryptor(EncryptionType::AES);
    dec->setKey("strongpass");

    EXPECT_THROW(dec->decrypt(encfile, out), std::runtime_error);

    fs::remove_all(tmp);
}

TEST(ExtraEdgeCases, BackupManagerIncrementalUpdate)
{
    auto src = fs::temp_directory_path() / "bm_inc_src";
    auto dst = fs::temp_directory_path() / "bm_inc_dst";
    auto restore = fs::temp_directory_path() / "bm_inc_restore";
    fs::create_directories(src);
    fs::create_directories(dst);

    auto f = src / "file.txt";
    writeFile(f, "first_version");

    BackupManager::BackupConfig cfg{};
    cfg.sourceRoot = src;
    cfg.backupRoot = dst;
    cfg.deleteRemoved = true;

    BackupManager mgr(cfg);
    mgr.scan();
    auto plan = mgr.buildPlan();
    mgr.executePlan(plan);

    EXPECT_EQ(readFile(dst / "file.txt"), "first_version");

    // Ensure timestamp difference on many filesystems
    std::this_thread::sleep_for(std::chrono::milliseconds(1100));

    writeFile(f, "second_version");
    mgr.scan();
    plan = mgr.buildPlan();
    mgr.executePlan(plan);

    EXPECT_EQ(readFile(dst / "file.txt"), "second_version");

    fs::remove_all(src);
    fs::remove_all(dst);
    fs::remove_all(restore);
}

TEST(ExtraEdgeCases, LargeFileCompressionEncryptionRoundTrip)
{
    auto src = fs::temp_directory_path() / "bm_large_src";
    auto dst = fs::temp_directory_path() / "bm_large_dst";
    auto restore = fs::temp_directory_path() / "bm_large_restore";
    fs::create_directories(src);
    fs::create_directories(dst);

    auto large = src / "large.bin";

    const size_t SZ = 3 * 1024 * 1024; // 3 MiB
    std::vector<uint8_t> data(SZ);
    std::mt19937 rng(42);
    std::uniform_int_distribution<int> dist(0, 255);
    for (auto &b : data) b = static_cast<uint8_t>(dist(rng));

    std::ofstream(large, std::ios::binary).write(reinterpret_cast<const char*>(data.data()), data.size());

    BackupManager::BackupConfig cfg{};
    cfg.sourceRoot = src;
    cfg.backupRoot = dst;
    cfg.enableCompression = true;
    cfg.compressionType = BackupManager::CompressionType::Lz77;
    cfg.enableEncryption = true;
    cfg.encryptionType = BackupManager::EncryptionType::AES;
    cfg.encryptionKey = "bigfilekey";

    BackupManager mgr(cfg);
    mgr.scan();
    auto plan = mgr.buildPlan();
    mgr.executePlan(plan);

    BackupManager::BackupConfig restoreCfg{};
    restoreCfg.backupRoot = dst;
    restoreCfg.encryptionKey = "bigfilekey";
    BackupManager restoreMgr(restoreCfg);
    restoreMgr.restore(restore);

    std::ifstream in(restore / "large.bin", std::ios::binary);
    ASSERT_TRUE(in.is_open());
    std::vector<uint8_t> restored(data.size());
    in.read(reinterpret_cast<char*>(restored.data()), restored.size());
    EXPECT_EQ(in.gcount(), static_cast<std::streamsize>(restored.size()));
    EXPECT_EQ(restored, data);

    fs::remove_all(src);
    fs::remove_all(dst);
    fs::remove_all(restore);
}
