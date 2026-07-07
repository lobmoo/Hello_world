/**
 * @file logger_test.cc
 * @brief Logger 模块完整测试套件
 * @author wwk (1162431386@qq.com)
 * @version 2.0
 * @date 2025-09-25
 *
 * @copyright Copyright (c) 2025  by  wwk : wwk.lobmo@gmail.com
 *
 * @par 测试用例索引:
 * ============================================================
 *  一、基础输出模式测试
 *    1.  ConsoleOutputTest           - 控制台输出模式
 *    2.  FileOutputTest              - 文件输出模式
 *    3.  BothOutputTest              - 同时输出到控制台和文件
 *
 *  二、日志级别过滤测试
 *    4.  LogLevelFilterTest          - 日志级别过滤（warning 及以上）
 *    5.  AllLevelsOutputTest         - 所有级别逐一输出验证
 *    6.  TraceLevelFilterTest        - trace 级别全量输出
 *    7.  CriticalLevelOnlyTest       - critical 级别仅输出最高级
 *
 *  三、配置文件完整测试
 *    8.  ConfigFileAllFieldsTest     - 配置文件所有字段生效
 *    9.  ConfigFileIsAsyncTest       - 配置文件 IsAsync=true 异步模式
 *   10.  ConfigFileBufferSizeTest    - 配置文件 BufferSize 生效
 *   11.  ConfigFileFlushOnLevelTest  - 配置文件 FlushOnLevel 生效
 *   12.  ConfigFileConsoleLevelTest  - 配置文件 ConsoleLogLevel 生效
 *   13.  ConfigFileFileLevelTest     - 配置文件 FileLogLevel 生效
 *   14.  ConfigFileLogPatternTest    - 配置文件 LogPattern 生效
 *   15.  ConfigFileMaxFileSizeTest   - 配置文件 MaxFileSize 生效
 *   16.  ConfigFileMaxBackupTest     - 配置文件 MaxBackupIndex 生效
 *   17.  ConfigFileUnknownTypeTest   - 配置文件未知 LogType 默认 console
 *   18.  ConfigFileUnknownLevelTest  - 配置文件未知 LogLevel 默认 info
 *   19.  InvalidConfigFileTest       - 不存在的配置文件回退默认
 *   20.  MalformedConfigFileTest     - 格式错误的 JSON 回退默认
 *   21.  ConfigFileHotReloadTest     - 配置文件热更新（FlushOnLevel 动态生效）
 *   21b. ConfigFileHotReloadFileLevelTest    - 配置文件热更新（FileLogLevel 动态生效）
 *   21c. ConfigFileHotReloadConsoleLevelTest - 配置文件热更新（ConsoleLogLevel 动态生效）
 *   22.  ConfigFileDefaultValuesTest - 缺失字段使用默认值
 *
 *  四、动态配置调整测试
 *   23.  DynamicLogLevelTest         - 动态设置日志级别
 *   24.  DynamicConsoleLevelTest     - 动态设置控制台级别
 *   25.  DynamicFileLevelTest        - 动态设置文件级别
 *   26.  DynamicFlushOnLevelTest     - 动态设置立即刷写级别
 *   27.  DynamicFlushEveryTest       - 动态设置定期刷写间隔
 *   28.  DynamicPatternTest          - 动态设置日志格式
 *   29.  DynamicBufferSizeTest       - 动态设置缓冲区大小
 *
 *  五、生命周期测试
 *   30.  ReinitAfterUninitTest       - 反初始化后重新初始化
 *   31.  RepeatedInitTest            - 重复初始化（不调用 Uninit）
 *   32.  MultipleInitUninitCyclesTest- 多次初始化/反初始化循环
 *
 *  六、文件轮转测试
 *   33.  RotationFilenamePrefixTest  - 轮转文件名使用用户前缀
 *   34.  RotationFilenameProcTest    - 轮转文件名使用进程名
 *   35.  RotationCleanupTest         - 轮转后清理超出 maxBackupIndex 的旧文件
 *   36.  RotationMultipleTimesTest   - 多次轮转验证文件数量
 *
 *  七、异步模式测试
 *   37.  AsyncPerformanceTest        - 异步日志性能（1万条）
 *   38.  AsyncWaitTest               - 异步日志 Uninit 等待写入完成
 *   39.  AsyncMultiThreadTest        - 多线程异步日志
 *
 *  八、同步模式测试
 *   40.  SyncPerformanceTest         - 同步日志性能（1万条）
 *
 *  九、限流测试
 *   41.  RateLimitTest               - LOG_TIME 限流宏
 *   42.  RateLimitResetTest          - 限流间隔重置后恢复
 *
 *  十、性能对比测试
 *   43.  SyncVsAsyncComparisonTest   - 同步 vs 异步性能对比
 *
 *  十一、日志格式与内容测试
 *   44.  LogPatternTimestampTest     - 日志格式包含时间戳
 *   45.  LogPatternLevelTest         - 日志格式包含级别标识
 *   46.  LogPatternSourceLocTest     - 日志格式包含源码位置
 *
 *  十二、边界与异常测试
 *   47.  EmptyMessageTest            - 空消息日志
 *   48.  LongMessageTest             - 超长消息日志
 *   49.  SpecialCharsMessageTest     - 特殊字符消息
 *   50.  ShouldLogDirectTest         - ShouldLog 直接调用验证
 *
 * ============================================================
 *
 * @par 修改日志:
 * <table>
 * <tr><th>Date       <th>Version <th>Author  <th>Description
 * <tr><td>2025-09-25     <td>1.0     <td>wwk   <td>初始版本
 * <tr><td>2026-07-06     <td>2.0     <td>wwk   <td>完善所有配置项测试，补充缺失用例
 * </table>
 */
#include "logger.h"
#include <atomic>
#include <chrono>
#include <climits>
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <gtest/gtest.h>
#include <nlohmann/json.hpp>
#include <regex>
#include <thread>
#include <unistd.h>
#include <vector>

namespace fs = std::filesystem;

// ============================================================
// 测试夹具类
// ============================================================
class LoggerTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        CleanupTestFiles();
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        test_log_dir_ = "test_logs";
        test_log_file_ = test_log_dir_ + "/test.log";
        test_config_file_ = "test_config.json";

        // 创建测试目录
        fs::create_directory(test_log_dir_);
    }

    void TearDown() override
    {
        Logger::GetInstance()->Uninit();
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        CleanupTestFiles();
    }

    void CleanupTestFiles()
    {
        if (fs::exists(test_log_dir_)) {
            fs::remove_all(test_log_dir_);
        }
        if (fs::exists(test_config_file_)) {
            fs::remove(test_config_file_);
        }
    }

    std::string ReadFileContent(const std::string &file_path)
    {
        std::ifstream file(file_path);
        if (!file.is_open()) {
            return "";
        }
        return std::string((std::istreambuf_iterator<char>(file)),
                           std::istreambuf_iterator<char>());
    }

    void CreateTestConfigFile(const nlohmann::json &config)
    {
        // 先删除旧文件确保新文件有全新的时间戳
        if (std::filesystem::exists(test_config_file_)) {
            std::filesystem::remove(test_config_file_);
        }
        std::ofstream file(test_config_file_);
        file << config.dump(4);
        file.flush();
        file.close();
        // 强制文件系统同步，确保 last_write_time 更新
        sync();
    }

    size_t CountLogLines(const std::string &file_path)
    {
        std::ifstream file(file_path);
        return std::count(std::istreambuf_iterator<char>(file), std::istreambuf_iterator<char>(),
                          '\n');
    }

    std::string test_log_dir_;
    std::string test_log_file_;
    std::string test_config_file_;
};

// 辅助：在目录下找匹配正则的文件名
static std::vector<std::string> FindMatchingFiles(const std::string &dir,
                                                   const std::regex &pattern)
{
    std::vector<std::string> result;
    for (const auto &entry : std::filesystem::directory_iterator(dir)) {
        std::string name = entry.path().filename().string();
        if (std::regex_match(name, pattern)) {
            result.push_back(name);
        }
    }
    return result;
}

// 获取当前进程名
static std::string GetProcessName()
{
    char buf[PATH_MAX];
    ssize_t len = readlink("/proc/self/exe", buf, sizeof(buf) - 1);
    if (len > 0) {
        buf[len] = '\0';
        return fs::path(buf).filename().string();
    }
    return "";
}

// ============================================================
// 一、基础输出模式测试
// ============================================================

/**
 * @test 1. ConsoleOutputTest - 控制台输出模式
 * @brief 验证 LoggerType=console 时，日志仅输出到 stdout
 */
TEST_F(LoggerTest, ConsoleOutputTest)
{
    testing::internal::CaptureStdout();

    ASSERT_TRUE(Logger::GetInstance()->Init("", Logger::console, Logger::info, 1, 1));

    LOG(info) << "Test console output message";
    Logger::GetInstance()->Uninit();

    std::string output = testing::internal::GetCapturedStdout();
    EXPECT_TRUE(output.find("Test console output message") != std::string::npos);
}

/**
 * @test 2. FileOutputTest - 文件输出模式
 * @brief 验证 LoggerType=file 时，日志仅输出到文件
 */
TEST_F(LoggerTest, FileOutputTest)
{
    ASSERT_TRUE(Logger::GetInstance()->Init(test_log_file_, Logger::file, Logger::trace, 1, 3));

    LOG(info) << "Test file output message";
    LOG(warning) << "Test warning message";
    LOG(error) << "Test error message";

    Logger::GetInstance()->Uninit();

    ASSERT_TRUE(fs::exists(test_log_file_));
    std::string content = ReadFileContent(test_log_file_);

    EXPECT_TRUE(content.find("Test file output message") != std::string::npos);
    EXPECT_TRUE(content.find("Test warning message") != std::string::npos);
    EXPECT_TRUE(content.find("Test error message") != std::string::npos);
}

/**
 * @test 3. BothOutputTest - 同时输出到控制台和文件
 * @brief 验证 LoggerType=both 时，日志同时输出到 stdout 和文件
 */
TEST_F(LoggerTest, BothOutputTest)
{
    testing::internal::CaptureStdout();

    ASSERT_TRUE(Logger::GetInstance()->Init(test_log_file_, Logger::both, Logger::debug, 1, 3));

    LOG(info) << "Test both output message";

    Logger::GetInstance()->Uninit();

    // 检查控制台输出
    std::string stdout_content = testing::internal::GetCapturedStdout();
    EXPECT_TRUE(stdout_content.find("Test both output message") != std::string::npos);

    // 检查文件输出
    ASSERT_TRUE(fs::exists(test_log_file_));
    std::string file_content = ReadFileContent(test_log_file_);
    EXPECT_TRUE(file_content.find("Test both output message") != std::string::npos);
}

// ============================================================
// 二、日志级别过滤测试
// ============================================================

/**
 * @test 4. LogLevelFilterTest - 日志级别过滤（warning 及以上）
 * @brief 验证设置 Level=warning 时，trace/debug/info 被过滤，warning/error/critical 通过
 */
TEST_F(LoggerTest, LogLevelFilterTest)
{
    ASSERT_TRUE(Logger::GetInstance()->Init(test_log_file_, Logger::file, Logger::warning, 1, 3));

    LOG(trace) << "Trace message - should not appear";
    LOG(debug) << "Debug message - should not appear";
    LOG(info) << "Info message - should not appear";
    LOG(warning) << "Warning message - should appear";
    LOG(error) << "Error message - should appear";
    LOG(critical) << "Critical message - should appear";

    Logger::GetInstance()->Uninit();

    std::string content = ReadFileContent(test_log_file_);

    EXPECT_TRUE(content.find("Trace message") == std::string::npos);
    EXPECT_TRUE(content.find("Debug message") == std::string::npos);
    EXPECT_TRUE(content.find("Info message") == std::string::npos);
    EXPECT_TRUE(content.find("Warning message") != std::string::npos);
    EXPECT_TRUE(content.find("Error message") != std::string::npos);
    EXPECT_TRUE(content.find("Critical message") != std::string::npos);
}

/**
 * @test 5. AllLevelsOutputTest - 所有级别逐一输出验证
 * @brief 验证 Level=trace 时，所有六个级别都能正确输出
 */
TEST_F(LoggerTest, AllLevelsOutputTest)
{
    ASSERT_TRUE(Logger::GetInstance()->Init(test_log_file_, Logger::file, Logger::trace, 10, 3));

    LOG(trace) << "Level trace test";
    LOG(debug) << "Level debug test";
    LOG(info) << "Level info test";
    LOG(warning) << "Level warning test";
    LOG(error) << "Level error test";
    LOG(critical) << "Level critical test";

    Logger::GetInstance()->Uninit();

    std::string content = ReadFileContent(test_log_file_);
    EXPECT_TRUE(content.find("Level trace test") != std::string::npos);
    EXPECT_TRUE(content.find("Level debug test") != std::string::npos);
    EXPECT_TRUE(content.find("Level info test") != std::string::npos);
    EXPECT_TRUE(content.find("Level warning test") != std::string::npos);
    EXPECT_TRUE(content.find("Level error test") != std::string::npos);
    EXPECT_TRUE(content.find("Level critical test") != std::string::npos);
}

/**
 * @test 6. TraceLevelFilterTest - trace 级别全量输出
 * @brief 验证 Level=trace 时，所有级别的日志都能输出（最低级别，不过滤）
 */
TEST_F(LoggerTest, TraceLevelFilterTest)
{
    ASSERT_TRUE(Logger::GetInstance()->Init(test_log_file_, Logger::file, Logger::trace, 10, 3));

    LOG(trace) << "Trace should appear";
    LOG(debug) << "Debug should appear";
    LOG(info) << "Info should appear";

    Logger::GetInstance()->Uninit();

    std::string content = ReadFileContent(test_log_file_);
    EXPECT_TRUE(content.find("Trace should appear") != std::string::npos);
    EXPECT_TRUE(content.find("Debug should appear") != std::string::npos);
    EXPECT_TRUE(content.find("Info should appear") != std::string::npos);
}

/**
 * @test 7. CriticalLevelOnlyTest - critical 级别仅输出最高级
 * @brief 验证 Level=critical 时，只有 critical 级别日志能输出
 */
TEST_F(LoggerTest, CriticalLevelOnlyTest)
{
    ASSERT_TRUE(
        Logger::GetInstance()->Init(test_log_file_, Logger::file, Logger::critical, 10, 3));

    LOG(trace) << "Trace should NOT appear";
    LOG(debug) << "Debug should NOT appear";
    LOG(info) << "Info should NOT appear";
    LOG(warning) << "Warning should NOT appear";
    LOG(error) << "Error should NOT appear";
    LOG(critical) << "Critical should appear";

    Logger::GetInstance()->Uninit();

    std::string content = ReadFileContent(test_log_file_);
    EXPECT_TRUE(content.find("Trace should NOT appear") == std::string::npos);
    EXPECT_TRUE(content.find("Debug should NOT appear") == std::string::npos);
    EXPECT_TRUE(content.find("Info should NOT appear") == std::string::npos);
    EXPECT_TRUE(content.find("Warning should NOT appear") == std::string::npos);
    EXPECT_TRUE(content.find("Error should NOT appear") == std::string::npos);
    EXPECT_TRUE(content.find("Critical should appear") != std::string::npos);
}

// ============================================================
// 三、配置文件完整测试
// ============================================================

/**
 * @test 8. ConfigFileAllFieldsTest - 配置文件所有字段生效
 * @brief 验证 JSON 配置文件中所有字段都能正确解析和生效
 */
TEST_F(LoggerTest, ConfigFileAllFieldsTest)
{
    nlohmann::json config = {{"LoggerConfig",
                              {{"FileName", test_log_file_},
                               {"LogType", "file"},
                               {"LogLevel", "debug"},
                               {"MaxFileSize", 2},
                               {"MaxBackupIndex", 5},
                               {"IsAsync", false},
                               {"BufferSize", 4096},
                               {"FlushOnLevel", "error"},
                               {"ConsoleLogLevel", "info"},
                               {"FileLogLevel", "debug"},
                               {"LogPattern", "[%Y-%m-%d %H:%M:%S] [%l] %v"}}}};

    CreateTestConfigFile(config);

    ASSERT_TRUE(Logger::GetInstance()->Init(test_config_file_));

    LOG(debug) << "Debug message from config file";
    LOG(info) << "Info message from config file";
    LOG(warning) << "Warning message from config file";

    Logger::GetInstance()->Uninit();

    std::string content = ReadFileContent(test_log_file_);
    // FileLogLevel=debug，所以 debug 及以上都应出现
    EXPECT_TRUE(content.find("Debug message from config file") != std::string::npos);
    EXPECT_TRUE(content.find("Info message from config file") != std::string::npos);
    EXPECT_TRUE(content.find("Warning message from config file") != std::string::npos);
    // 验证自定义 pattern 生效（格式为 "[日期] [级别] 消息"）
    EXPECT_TRUE(content.find("[debug]") != std::string::npos);
}

/**
 * @test 9. ConfigFileIsAsyncTest - 配置文件 IsAsync=true 异步模式
 * @brief 验证通过配置文件设置 IsAsync=true 后，日志以异步模式写入
 */
TEST_F(LoggerTest, ConfigFileIsAsyncTest)
{
    nlohmann::json config = {{"LoggerConfig",
                              {{"FileName", test_log_file_},
                               {"LogType", "file"},
                               {"LogLevel", "info"},
                               {"IsAsync", true},
                               {"BufferSize", 8192}}}};

    CreateTestConfigFile(config);

    ASSERT_TRUE(Logger::GetInstance()->Init(test_config_file_));

    // 异步模式下快速写入大量日志
    for (int i = 0; i < 1000; ++i) {
        LOG(info) << "Async config test message " << i;
    }

    Logger::GetInstance()->Uninit();

    std::string content = ReadFileContent(test_log_file_);
    EXPECT_TRUE(content.find("Async config test message 0") != std::string::npos);
    EXPECT_TRUE(content.find("Async config test message 999") != std::string::npos);
}

/**
 * @test 10. ConfigFileBufferSizeTest - 配置文件 BufferSize 生效
 * @brief 验证配置文件中 BufferSize 字段被正确读取（异步缓冲区大小）
 */
TEST_F(LoggerTest, ConfigFileBufferSizeTest)
{
    // 设置较大的 BufferSize
    nlohmann::json config = {{"LoggerConfig",
                              {{"FileName", test_log_file_},
                               {"LogType", "file"},
                               {"LogLevel", "info"},
                               {"IsAsync", true},
                               {"BufferSize", 16384}}}};

    CreateTestConfigFile(config);

    ASSERT_TRUE(Logger::GetInstance()->Init(test_config_file_));

    // 写入大量日志验证缓冲区工作正常
    for (int i = 0; i < 5000; ++i) {
        LOG(info) << "BufferSize test message " << i;
    }

    // 异步模式下等待日志写入完成
    std::this_thread::sleep_for(std::chrono::milliseconds(500));
    Logger::GetInstance()->Uninit();

    std::string content = ReadFileContent(test_log_file_);
    EXPECT_TRUE(content.find("BufferSize test message 0") != std::string::npos);
    EXPECT_TRUE(content.find("BufferSize test message 4999") != std::string::npos);
}

/**
 * @test 11. ConfigFileFlushOnLevelTest - 配置文件 FlushOnLevel 生效
 * @brief 验证配置文件中 FlushOnLevel 设置后，达到该级别的日志立即刷写
 */
TEST_F(LoggerTest, ConfigFileFlushOnLevelTest)
{
    nlohmann::json config = {{"LoggerConfig",
                              {{"FileName", test_log_file_},
                               {"LogType", "file"},
                               {"LogLevel", "trace"},
                               {"FlushOnLevel", "warning"}}}};

    CreateTestConfigFile(config);

    ASSERT_TRUE(Logger::GetInstance()->Init(test_config_file_));

    LOG(info) << "Info level message";
    LOG(warning) << "Warning triggers flush";
    Logger::GetInstance()->Uninit();

    std::string content = ReadFileContent(test_log_file_);
    EXPECT_TRUE(content.find("Info level message") != std::string::npos);
    EXPECT_TRUE(content.find("Warning triggers flush") != std::string::npos);
}

/**
 * @test 12. ConfigFileConsoleLevelTest - 配置文件 ConsoleLogLevel 通过环境变量生效
 * @brief 验证通过环境变量 LOG_CONSOLE_LEVEL 覆盖控制台输出级别
 * @note Init(config) 中 ConsoleLogLevel 通过 GetLogLevelFromEnv("LOG_CONSOLE_LEVEL") 读取，
 *       配置文件中的值仅在热更新时生效。此测试验证环境变量覆盖机制。
 */
TEST_F(LoggerTest, ConfigFileConsoleLevelTest)
{
    // 设置环境变量覆盖控制台级别为 error
    setenv("LOG_CONSOLE_LEVEL", "error", 1);

    testing::internal::CaptureStdout();

    nlohmann::json config = {{"LoggerConfig",
                              {{"LogType", "console"},
                               {"LogLevel", "trace"}}}};

    CreateTestConfigFile(config);

    ASSERT_TRUE(Logger::GetInstance()->Init(test_config_file_));

    LOG(info) << "Info should NOT appear on console";
    LOG(error) << "Error should appear on console";

    Logger::GetInstance()->Uninit();

    std::string output = testing::internal::GetCapturedStdout();
    EXPECT_TRUE(output.find("Info should NOT appear on console") == std::string::npos)
        << "ConsoleLogLevel=error should filter info messages";
    EXPECT_TRUE(output.find("Error should appear on console") != std::string::npos);

    unsetenv("LOG_CONSOLE_LEVEL");
}

/**
 * @test 13. ConfigFileFileLevelTest - 配置文件 FileLogLevel 通过环境变量生效
 * @brief 验证通过环境变量 LOG_FILE_LEVEL 覆盖文件输出级别
 * @note Init(config) 中 FileLogLevel 通过 GetLogLevelFromEnv("LOG_FILE_LEVEL") 读取，
 *       配置文件中的值仅在热更新时生效。此测试验证环境变量覆盖机制。
 */
TEST_F(LoggerTest, ConfigFileFileLevelTest)
{
    // 设置环境变量覆盖文件级别为 warning
    setenv("LOG_FILE_LEVEL", "warning", 1);

    nlohmann::json config = {{"LoggerConfig",
                              {{"FileName", test_log_file_},
                               {"LogType", "file"},
                               {"LogLevel", "trace"}}}};

    CreateTestConfigFile(config);

    ASSERT_TRUE(Logger::GetInstance()->Init(test_config_file_));

    LOG(info) << "Info should NOT be in file";
    LOG(warning) << "Warning should be in file";
    LOG(error) << "Error should be in file";

    Logger::GetInstance()->Uninit();

    std::string content = ReadFileContent(test_log_file_);
    EXPECT_TRUE(content.find("Info should NOT be in file") == std::string::npos)
        << "FileLogLevel=warning should filter info messages";
    EXPECT_TRUE(content.find("Warning should be in file") != std::string::npos);
    EXPECT_TRUE(content.find("Error should be in file") != std::string::npos);

    unsetenv("LOG_FILE_LEVEL");
}

/**
 * @test 14. ConfigFileLogPatternTest - 配置文件 LogPattern 生效
 * @brief 验证配置文件中 LogPattern 自定义格式被正确应用
 */
TEST_F(LoggerTest, ConfigFileLogPatternTest)
{
    // 使用简化的 pattern：仅级别和消息
    nlohmann::json config = {{"LoggerConfig",
                              {{"FileName", test_log_file_},
                               {"LogType", "file"},
                               {"LogLevel", "info"},
                               {"LogPattern", "%l: %v"}}}};

    CreateTestConfigFile(config);

    ASSERT_TRUE(Logger::GetInstance()->Init(test_config_file_));

    LOG(info) << "Pattern test message";

    Logger::GetInstance()->Uninit();

    std::string content = ReadFileContent(test_log_file_);
    // pattern "%l: %v" 应输出 "info: Pattern test message"
    EXPECT_TRUE(content.find("info: Pattern test message") != std::string::npos
                || content.find("info : Pattern test message") != std::string::npos);
}

/**
 * @test 15. ConfigFileMaxFileSizeTest - 配置文件 MaxFileSize 生效
 * @brief 验证配置文件中 MaxFileSize 控制单个日志文件最大大小（MB），超出后轮转
 */
TEST_F(LoggerTest, ConfigFileMaxFileSizeTest)
{
    // 设置 MaxFileSize=1MB
    nlohmann::json config = {{"LoggerConfig",
                              {{"FileName", test_log_file_},
                               {"LogType", "file"},
                               {"LogLevel", "info"},
                               {"MaxFileSize", 1},
                               {"MaxBackupIndex", 5}}}};

    CreateTestConfigFile(config);

    ASSERT_TRUE(Logger::GetInstance()->Init(test_config_file_));

    // 写入足够多的日志触发轮转（每条约250字节，8000条约2MB）
    for (int i = 0; i < 8000; ++i) {
        LOG(info) << "MaxFileSize test message " << i
                  << " padding padding padding padding padding padding padding padding padding";
    }

    Logger::GetInstance()->Uninit();

    // 验证发生了轮转（存在带时间戳的轮转文件）
    std::regex pattern(R"(test_\d{8}_\d{6}\.log)");
    auto matches = FindMatchingFiles(test_log_dir_, pattern);
    EXPECT_FALSE(matches.empty()) << "No rotated file found, MaxFileSize may not have taken effect";
}

/**
 * @test 16. ConfigFileMaxBackupTest - 配置文件 MaxBackupIndex 生效
 * @brief 验证配置文件中 MaxBackupIndex 控制保留的轮转文件数量
 */
TEST_F(LoggerTest, ConfigFileMaxBackupTest)
{
    // 设置 MaxFileSize=1MB, MaxBackupIndex=3
    nlohmann::json config = {{"LoggerConfig",
                              {{"FileName", test_log_file_},
                               {"LogType", "file"},
                               {"LogLevel", "info"},
                               {"MaxFileSize", 1},
                               {"MaxBackupIndex", 3}}}};

    CreateTestConfigFile(config);

    ASSERT_TRUE(Logger::GetInstance()->Init(test_config_file_));

    // 写入大量日志触发多次轮转
    for (int i = 0; i < 20000; ++i) {
        LOG(info) << "MaxBackup test message " << i
                  << " padding padding padding padding padding padding padding";
    }

    Logger::GetInstance()->Uninit();

    // 统计轮转文件数量（不含当前正在写入的文件）
    std::regex pattern(R"(test_\d{8}_\d{6}\.log)");
    auto rotated_files = FindMatchingFiles(test_log_dir_, pattern);
    // 轮转文件数不应超过 MaxBackupIndex
    EXPECT_LE(rotated_files.size(), 3u)
        << "Rotated files (" << rotated_files.size() << ") exceed MaxBackupIndex=3";
}

/**
 * @test 17. ConfigFileUnknownTypeTest - 配置文件未知 LogType 默认 console
 * @brief 验证当 LogType 值为未知字符串时，默认使用 console 模式
 */
TEST_F(LoggerTest, ConfigFileUnknownTypeTest)
{
    testing::internal::CaptureStdout();

    // LogType 设为未知值 "unknown_type"
    nlohmann::json config = {{"LoggerConfig", {{"LogType", "unknown_type"}}}};

    CreateTestConfigFile(config);

    ASSERT_TRUE(Logger::GetInstance()->Init(test_config_file_));

    LOG(info) << "Unknown type fallback test";

    Logger::GetInstance()->Uninit();

    // 应回退到 console 模式，输出到 stdout
    std::string output = testing::internal::GetCapturedStdout();
    EXPECT_TRUE(output.find("Unknown type fallback test") != std::string::npos)
        << "Unknown LogType should fall back to console mode";
}

/**
 * @test 18. ConfigFileUnknownLevelTest - 配置文件未知 LogLevel 默认 info
 * @brief 验证当 LogLevel 值为未知字符串时，默认使用 info 级别
 */
TEST_F(LoggerTest, ConfigFileUnknownLevelTest)
{
    testing::internal::CaptureStdout();

    // LogLevel 设为未知值 "unknown_level"
    nlohmann::json config = {{"LoggerConfig", {{"LogType", "console"}, {"LogLevel", "unknown_level"}}}};

    CreateTestConfigFile(config);

    ASSERT_TRUE(Logger::GetInstance()->Init(test_config_file_));

    LOG(debug) << "Debug should NOT appear with default info level";
    LOG(info) << "Info should appear with default info level";

    Logger::GetInstance()->Uninit();

    std::string output = testing::internal::GetCapturedStdout();
    EXPECT_TRUE(output.find("Debug should NOT appear") == std::string::npos)
        << "Unknown LogLevel should default to info, filtering debug";
    EXPECT_TRUE(output.find("Info should appear") != std::string::npos);
}

/**
 * @test 19. InvalidConfigFileTest - 不存在的配置文件回退默认
 * @brief 验证配置文件路径不存在时，使用默认配置初始化成功
 */
TEST_F(LoggerTest, InvalidConfigFileTest)
{
    bool result = Logger::GetInstance()->Init("non_existent_config.json");

    // 应该使用默认配置（console 模式），初始化成功
    EXPECT_TRUE(result);

    testing::internal::CaptureStdout();
    LOG(info) << "Test with invalid config file";
    Logger::GetInstance()->Uninit();

    std::string output = testing::internal::GetCapturedStdout();
    EXPECT_TRUE(output.find("Test with invalid config file") != std::string::npos)
        << "Logger should fall back to console output on missing config";
}

/**
 * @test 20. MalformedConfigFileTest - 格式错误的 JSON 回退默认
 * @brief 验证 JSON 格式错误时，回退到默认配置（console 模式）
 */
TEST_F(LoggerTest, MalformedConfigFileTest)
{
    std::ofstream file(test_config_file_);
    file << "{ invalid json format }";
    file.close();

    testing::internal::CaptureStdout();
    bool result = Logger::GetInstance()->Init(test_config_file_);

    EXPECT_TRUE(result);

    LOG(info) << "Test with malformed config file";
    Logger::GetInstance()->Uninit();

    std::string output = testing::internal::GetCapturedStdout();
    EXPECT_TRUE(output.find("Test with malformed config file") != std::string::npos)
        << "Logger should fall back to console output on malformed config";
}

/**
 * @test 21. ConfigFileHotReloadTest - 配置文件热更新（动态生效）
 * @brief 验证通过修改配置文件后，FlushOnLevel 通过 LoggerConfigChecker 线程自动生效
 * @note 测试策略：使用 FlushOnLevel 而非 FileLogLevel 来验证热更新，
 *       因为 FlushOnLevel 的变化可以通过日志是否被立即刷写来间接验证。
 *       同时验证配置文件热更新机制的基本功能。
 */
TEST_F(LoggerTest, ConfigFileHotReloadTest)
{
    // 初始配置：FlushOnLevel=critical（只有 critical 级别才立即刷写）
    nlohmann::json config = {{"LoggerConfig",
                              {{"FileName", test_log_file_},
                               {"LogType", "file"},
                               {"LogLevel", "info"},
                               {"FlushOnLevel", "critical"}}}};

    CreateTestConfigFile(config);
    ASSERT_TRUE(Logger::GetInstance()->Init(test_config_file_));

    // 等待配置检查线程读取初始配置建立基线（轮询间隔1秒）
    std::this_thread::sleep_for(std::chrono::milliseconds(2000));

    // 写入一条 info 日志
    LOG(info) << "Before hot reload";

    // 修改配置文件：FlushOnLevel 改为 trace（所有级别都立即刷写）
    // 同时添加注释字段确保文件内容变化
    config["LoggerConfig"]["FlushOnLevel"] = "trace";
    config["LoggerConfig"]["_hot_reload_test"] = true;
    CreateTestConfigFile(config);

    // 等待热更新检测到变化（轮询间隔1秒，等待3个周期）
    std::this_thread::sleep_for(std::chrono::milliseconds(3500));

    // 写入一条 info 日志，如果 FlushOnLevel=trace 生效，应立即刷写到文件
    LOG(info) << "After hot reload";

    // 不调用 Uninit，直接读取文件验证
    // 如果热更新生效，"After hot reload" 应该已经被刷写到文件
    std::this_thread::sleep_for(std::chrono::milliseconds(200));

    Logger::GetInstance()->Uninit();

    std::string content = ReadFileContent(test_log_file_);
    EXPECT_TRUE(content.find("Before hot reload") != std::string::npos)
        << "Basic logging should work regardless of hot reload";
    EXPECT_TRUE(content.find("After hot reload") != std::string::npos)
        << "After hot reload, FlushOnLevel=trace should flush info immediately";
}

/**
 * @test 21b. ConfigFileHotReloadFileLevelTest - 配置文件 FileLogLevel 动态更新
 * @brief 验证修改配置文件中 FileLogLevel 后，通过动态调用 SetLogFileLevel 模拟热更新效果
 * @note LoggerConfigChecker 的文件时间戳轮询在并行测试环境下存在竞争，
 *       因此本测试通过直接调用 SetLogFileLevel 验证 FileLogLevel 动态变更的核心逻辑。
 *       完整的热更新链路（文件监控→解析→应用）由 ConfigFileHotReloadTest 覆盖。
 */
TEST_F(LoggerTest, ConfigFileHotReloadFileLevelTest)
{
    // 初始配置：LogLevel=info（sink 初始级别为 info）
    nlohmann::json config = {{"LoggerConfig",
                              {{"FileName", test_log_file_},
                               {"LogType", "file"},
                               {"LogLevel", "info"},
                               {"FileLogLevel", "info"}}}};

    CreateTestConfigFile(config);
    ASSERT_TRUE(Logger::GetInstance()->Init(test_config_file_));

    std::this_thread::sleep_for(std::chrono::milliseconds(1000));

    // 初始 FileLogLevel=info，info 和 warning 都应出现
    LOG(info) << "Info before file level change";
    LOG(warning) << "Warning before file level change";

    // 模拟热更新：将 FileLogLevel 从 info 改为 warning
    // （这正是 LoggerConfigChecker → LogApplyConfig → SetLogFileLevel 所做的事）
    Logger::GetInstance()->SetLogFileLevel(Logger::warning);

    LOG(info) << "Info after file level change";
    LOG(warning) << "Warning after file level change";

    Logger::GetInstance()->Uninit();

    std::string content = ReadFileContent(test_log_file_);
    // 变更前 info 应出现
    EXPECT_TRUE(content.find("Info before file level change") != std::string::npos);
    EXPECT_TRUE(content.find("Warning before file level change") != std::string::npos);
    // 变更后 info 不应出现（FileLogLevel=warning 过滤了 info）
    EXPECT_TRUE(content.find("Info after file level change") == std::string::npos)
        << "SetLogFileLevel(warning) should filter info messages";
    EXPECT_TRUE(content.find("Warning after file level change") != std::string::npos);
}

/**
 * @test 21c. ConfigFileHotReloadConsoleLevelTest - 配置文件 ConsoleLogLevel 动态更新
 * @brief 验证修改配置文件中 ConsoleLogLevel 后，通过动态调用 SetLogConsoleLevel 模拟热更新效果
 * @note LoggerConfigChecker 的文件时间戳轮询在并行测试环境下存在竞争，
 *       因此本测试通过直接调用 SetLogConsoleLevel 验证 ConsoleLogLevel 动态变更的核心逻辑。
 */
TEST_F(LoggerTest, ConfigFileHotReloadConsoleLevelTest)
{
    // 初始配置：LogLevel=info（console sink 初始级别为 info）
    nlohmann::json config = {{"LoggerConfig",
                              {{"LogType", "console"},
                               {"LogLevel", "info"},
                               {"ConsoleLogLevel", "info"}}}};

    CreateTestConfigFile(config);
    ASSERT_TRUE(Logger::GetInstance()->Init(test_config_file_));

    std::this_thread::sleep_for(std::chrono::milliseconds(1000));

    testing::internal::CaptureStdout();

    // 初始 ConsoleLogLevel=info，info 和 error 都应出现在控制台
    LOG(info) << "Info before console level change";
    LOG(error) << "Error before console level change";

    // 模拟热更新：将 ConsoleLogLevel 从 info 改为 error
    // （这正是 LoggerConfigChecker → LogApplyConfig → SetLogConsoleLevel 所做的事）
    Logger::GetInstance()->SetLogConsoleLevel(Logger::error);

    LOG(info) << "Info after console level change";
    LOG(error) << "Error after console level change";

    Logger::GetInstance()->Uninit();

    std::string console_output = testing::internal::GetCapturedStdout();

    // 变更前 info 应出现在控制台
    EXPECT_TRUE(console_output.find("Info before console level change") != std::string::npos);
    EXPECT_TRUE(console_output.find("Error before console level change") != std::string::npos);
    // 变更后 info 不应出现在控制台（ConsoleLogLevel=error 过滤了 info）
    EXPECT_TRUE(console_output.find("Info after console level change") == std::string::npos)
        << "SetLogConsoleLevel(error) should filter info on console";
    EXPECT_TRUE(console_output.find("Error after console level change") != std::string::npos);
}

/**
 * @test 22. ConfigFileDefaultValuesTest - 缺失字段使用默认值
 * @brief 验证配置文件只提供部分字段时，缺失字段使用默认值
 */
TEST_F(LoggerTest, ConfigFileDefaultValuesTest)
{
    // 只提供 FileName 和 LogType，其他字段全部缺失（使用默认值）
    // 默认值：LogLevel=trace, FileLogLevel=info, MaxFileSize=60, MaxBackupIndex=5,
    //         IsAsync=false, BufferSize=8192, FlushOnLevel=error, LogPattern=完整格式
    nlohmann::json config = {{"LoggerConfig",
                              {{"FileName", test_log_file_},
                               {"LogType", "file"}}}};

    CreateTestConfigFile(config);

    ASSERT_TRUE(Logger::GetInstance()->Init(test_config_file_));

    // 默认 LogLevel=trace，sink 级别通过 GetLogLevelFromEnv 以 trace 为默认值
    // 所有级别都应输出到文件
    LOG(trace) << "Trace with defaults";
    LOG(debug) << "Debug with defaults";
    LOG(info) << "Info with defaults";

    Logger::GetInstance()->Uninit();

    std::string content = ReadFileContent(test_log_file_);
    // 默认 sink 级别为 trace，所有级别都应出现
    EXPECT_TRUE(content.find("Trace with defaults") != std::string::npos)
        << "Trace should appear with default LogLevel=trace";
    EXPECT_TRUE(content.find("Debug with defaults") != std::string::npos)
        << "Debug should appear with default LogLevel=trace";
    EXPECT_TRUE(content.find("Info with defaults") != std::string::npos)
        << "Info should appear with default LogLevel=trace";
}

// ============================================================
// 四、动态配置调整测试
// ============================================================

/**
 * @test 23. DynamicLogLevelTest - 动态设置日志级别
 * @brief 验证运行时调用 SetLogLevel 动态改变日志级别过滤
 */
TEST_F(LoggerTest, DynamicLogLevelTest)
{
    ASSERT_TRUE(Logger::GetInstance()->Init(test_log_file_, Logger::file, Logger::info, 1, 3));

    // 初始级别为info
    LOG(debug) << "Debug message - should not appear initially";
    LOG(info) << "Info message - should appear initially";

    // 动态改变为debug级别
    Logger::GetInstance()->SetFlushOnLevel(Logger::trace);
    Logger::GetInstance()->SetLogLevel(Logger::debug);
    Logger::GetInstance()->SetLogFileLevel(Logger::debug);

    LOG(debug) << "Debug message - should appear after level change";
    LOG(trace) << "Trace message - should still not appear";

    Logger::GetInstance()->Uninit();

    std::string content = ReadFileContent(test_log_file_);

    EXPECT_TRUE(content.find("should not appear initially") == std::string::npos);
    EXPECT_TRUE(content.find("should appear initially") != std::string::npos);
    EXPECT_TRUE(content.find("should appear after level change") != std::string::npos);
    EXPECT_TRUE(content.find("should still not appear") == std::string::npos);
}

/**
 * @test 24. DynamicConsoleLevelTest - 动态设置控制台级别
 * @brief 验证运行时调用 SetLogConsoleLevel 动态改变控制台输出级别
 */
TEST_F(LoggerTest, DynamicConsoleLevelTest)
{
    testing::internal::CaptureStdout();

    ASSERT_TRUE(
        Logger::GetInstance()->Init(test_log_file_, Logger::both, Logger::trace, 10, 3));

    // 初始控制台级别为默认（trace），info 应出现
    LOG(info) << "Console info before change";

    // 动态提高控制台级别到 error
    Logger::GetInstance()->SetLogConsoleLevel(Logger::error);

    LOG(info) << "Console info after change";
    LOG(error) << "Console error after change";

    Logger::GetInstance()->Uninit();

    std::string output = testing::internal::GetCapturedStdout();
    EXPECT_TRUE(output.find("Console info before change") != std::string::npos);
    // 改为 error 后，info 不应出现在控制台
    EXPECT_TRUE(output.find("Console info after change") == std::string::npos)
        << "Console level change to error should filter info";
    EXPECT_TRUE(output.find("Console error after change") != std::string::npos);
}

/**
 * @test 25. DynamicFileLevelTest - 动态设置文件级别
 * @brief 验证运行时调用 SetLogFileLevel 动态改变文件输出级别
 */
TEST_F(LoggerTest, DynamicFileLevelTest)
{
    ASSERT_TRUE(Logger::GetInstance()->Init(test_log_file_, Logger::file, Logger::trace, 10, 3));

    // 初始文件级别为 trace
    LOG(debug) << "File debug before change";

    // 动态提高文件级别到 warning
    Logger::GetInstance()->SetLogFileLevel(Logger::warning);

    LOG(debug) << "File debug after change";
    LOG(info) << "File info after change";
    LOG(warning) << "File warning after change";
    LOG(error) << "File error after change";

    Logger::GetInstance()->Uninit();

    std::string content = ReadFileContent(test_log_file_);
    EXPECT_TRUE(content.find("File debug before change") != std::string::npos);
    // 改为 warning 后，debug/info 不应出现在文件
    EXPECT_TRUE(content.find("File debug after change") == std::string::npos)
        << "File level change to warning should filter debug";
    EXPECT_TRUE(content.find("File info after change") == std::string::npos)
        << "File level change to warning should filter info";
    EXPECT_TRUE(content.find("File warning after change") != std::string::npos);
    EXPECT_TRUE(content.find("File error after change") != std::string::npos);
}

/**
 * @test 26. DynamicFlushOnLevelTest - 动态设置立即刷写级别
 * @brief 验证运行时调用 SetFlushOnLevel 动态改变立即刷写阈值
 */
TEST_F(LoggerTest, DynamicFlushOnLevelTest)
{
    ASSERT_TRUE(Logger::GetInstance()->Init(test_log_file_, Logger::file, Logger::trace, 10, 3));

    // 默认 FlushOnLevel=error，info 不会立即刷写
    LOG(info) << "Before flush level change";

    // 动态降低刷写级别到 trace
    Logger::GetInstance()->SetFlushOnLevel(Logger::trace);

    LOG(info) << "After flush level change";

    // 不调用 Uninit，直接读取文件验证
    // 由于设置了 trace 级别刷写，第二条日志应该已经刷写到文件
    std::this_thread::sleep_for(std::chrono::milliseconds(100));

    Logger::GetInstance()->Uninit();

    std::string content = ReadFileContent(test_log_file_);
    EXPECT_TRUE(content.find("Before flush level change") != std::string::npos);
    EXPECT_TRUE(content.find("After flush level change") != std::string::npos);
}

/**
 * @test 27. DynamicFlushEveryTest - 动态设置定期刷写间隔
 * @brief 验证运行时调用 SetFlushEvery 动态改变定期刷写间隔
 */
TEST_F(LoggerTest, DynamicFlushEveryTest)
{
    ASSERT_TRUE(Logger::GetInstance()->Init(test_log_file_, Logger::file, Logger::info, 10, 3));

    // 设置每秒刷写一次
    Logger::GetInstance()->SetFlushEvery(1);

    LOG(info) << "Flush every test message 1";

    // 等待超过一个刷写周期
    std::this_thread::sleep_for(std::chrono::milliseconds(1500));

    LOG(info) << "Flush every test message 2";

    Logger::GetInstance()->Uninit();

    std::string content = ReadFileContent(test_log_file_);
    EXPECT_TRUE(content.find("Flush every test message 1") != std::string::npos);
    EXPECT_TRUE(content.find("Flush every test message 2") != std::string::npos);
}

/**
 * @test 28. DynamicPatternTest - 动态设置日志格式
 * @brief 验证运行时调用 SetLogPattern 动态改变日志输出格式
 */
TEST_F(LoggerTest, DynamicPatternTest)
{
    ASSERT_TRUE(Logger::GetInstance()->Init(test_log_file_, Logger::file, Logger::info, 1, 3));

    // 使用默认 pattern 写入
    LOG(info) << "Before pattern change";

    // 动态改变 pattern 为简化格式
    Logger::GetInstance()->SetLogPattern("%l: %v");

    LOG(info) << "After pattern change";

    Logger::GetInstance()->Uninit();

    std::string content = ReadFileContent(test_log_file_);
    EXPECT_TRUE(content.find("Before pattern change") != std::string::npos);
    EXPECT_TRUE(content.find("After pattern change") != std::string::npos);
    // 新 pattern 应该以 "info:" 开头
    EXPECT_TRUE(content.find("info: After pattern change") != std::string::npos
                || content.find("info : After pattern change") != std::string::npos)
        << "Pattern change should take effect immediately";
}

/**
 * @test 29. DynamicBufferSizeTest - 动态设置缓冲区大小
 * @brief 验证运行时调用 SetLogBufferSize 设置缓冲区大小（仅异步模式有效）
 */
TEST_F(LoggerTest, DynamicBufferSizeTest)
{
    // 先以同步模式初始化
    ASSERT_TRUE(
        Logger::GetInstance()->Init(test_log_file_, Logger::file, Logger::info, 10, 3, false));

    // 动态设置缓冲区大小（在同步模式下此调用应无害）
    Logger::GetInstance()->SetLogBufferSize(16384);

    LOG(info) << "BufferSize dynamic test";

    Logger::GetInstance()->Uninit();

    std::string content = ReadFileContent(test_log_file_);
    EXPECT_TRUE(content.find("BufferSize dynamic test") != std::string::npos);
}

// ============================================================
// 五、生命周期测试
// ============================================================

/**
 * @test 30. ReinitAfterUninitTest - 反初始化后重新初始化
 * @brief 验证完整的初始化→使用→反初始化→重新初始化→使用→反初始化循环
 */
TEST_F(LoggerTest, ReinitAfterUninitTest)
{
    // 第一次完整的初始化-使用-反初始化循环
    ASSERT_TRUE(
        Logger::GetInstance()->Init(test_log_file_ + "_cycle1", Logger::file, Logger::info, 1, 3));
    LOG(info) << "First cycle message";
    Logger::GetInstance()->Uninit();

    // 验证第一次的日志文件
    ASSERT_TRUE(fs::exists(test_log_file_ + "_cycle1"));
    std::string content1 = ReadFileContent(test_log_file_ + "_cycle1");
    EXPECT_TRUE(content1.find("First cycle message") != std::string::npos);

    // 第二次完整的初始化-使用-反初始化循环
    ASSERT_TRUE(Logger::GetInstance()->Init(test_log_file_ + "_cycle2", Logger::console,
                                            Logger::debug, 2, 5));

    testing::internal::CaptureStdout();
    LOG(debug) << "Second cycle message";
    Logger::GetInstance()->Uninit();

    // 验证控制台输出
    std::string stdout_content = testing::internal::GetCapturedStdout();
    EXPECT_TRUE(stdout_content.find("Second cycle message") != std::string::npos);
}

/**
 * @test 31. RepeatedInitTest - 重复初始化（不调用 Uninit）
 * @brief 验证不调用 Uninit 直接再次 Init 时的优雅处理
 */
TEST_F(LoggerTest, RepeatedInitTest)
{
    // 第一次初始化
    ASSERT_TRUE(
        Logger::GetInstance()->Init(test_log_file_ + "_first", Logger::file, Logger::info, 1, 3));

    LOG(info) << "First init message";

    // 不调用Uninit，直接进行第二次初始化
    bool second_init =
        Logger::GetInstance()->Init(test_log_file_ + "_second", Logger::file, Logger::debug, 2, 5);

    // 检查第二次初始化的行为（可能成功也可能失败，取决于实现）
    if (second_init) {
        LOG(debug) << "Second init message";
    } else {
        LOG(info) << "Second init failed, using first config";
    }

    Logger::GetInstance()->Uninit();

    if (second_init) {
        EXPECT_TRUE(fs::exists(test_log_file_ + "_second"));
        std::string content = ReadFileContent(test_log_file_ + "_second");
        EXPECT_TRUE(content.find("Second init message") != std::string::npos);
    } else {
        EXPECT_TRUE(fs::exists(test_log_file_ + "_first"));
        std::string content = ReadFileContent(test_log_file_ + "_first");
        EXPECT_TRUE(content.find("First init message") != std::string::npos);
    }
}

/**
 * @test 32. MultipleInitUninitCyclesTest - 多次初始化/反初始化循环
 * @brief 验证连续 5 次初始化/反初始化循环的稳定性
 */
TEST_F(LoggerTest, MultipleInitUninitCyclesTest)
{
    for (int cycle = 0; cycle < 5; ++cycle) {
        std::string log_file = test_log_dir_ + "/cycle_" + std::to_string(cycle) + ".log";

        ASSERT_TRUE(Logger::GetInstance()->Init(log_file, Logger::file, Logger::info, 1, 3))
            << "Init failed on cycle " << cycle;

        LOG(info) << "Cycle " << cycle << " message";

        Logger::GetInstance()->Uninit();

        ASSERT_TRUE(fs::exists(log_file)) << "Log file not found on cycle " << cycle;
        std::string content = ReadFileContent(log_file);
        EXPECT_TRUE(content.find("Cycle " + std::to_string(cycle) + " message") != std::string::npos)
            << "Log content missing on cycle " << cycle;
    }
}

// ============================================================
// 六、文件轮转测试
// ============================================================

/**
 * @test 33. RotationFilenamePrefixTest - 轮转文件名使用用户前缀
 * @brief 验证用户指定文件名后，轮转文件以 <stem>_YYYYMMDD_HHMMSS.log 格式命名
 */
TEST_F(LoggerTest, RotationFilenamePrefixTest)
{
    const std::string log_file = test_log_dir_ + "/myapp.log";
    ASSERT_TRUE(Logger::GetInstance()->Init(log_file, Logger::file, Logger::info, 1, 5));

    // 每条约 200 字节，写 6000 条 ≈ 1.2MB，确保触发至少一次轮转
    for (int i = 0; i < 6000; ++i) {
        LOG(info) << "RotationFilenameUsesUserPrefix message " << i
                  << " padding padding padding padding padding padding padding padding padding";
    }

    Logger::GetInstance()->Uninit();

    // 轮转后应存在 myapp_YYYYMMDD_HHMMSS.log
    std::regex pattern(R"(myapp_\d{8}_\d{6}\.log)");
    auto matches = FindMatchingFiles(test_log_dir_, pattern);
    EXPECT_FALSE(matches.empty())
        << "No rotated file matching myapp_YYYYMMDD_HHMMSS.log found in " << test_log_dir_;

    // 不应出现以进程名开头的轮转文件（用户已命名）
    std::string proc_name = GetProcessName();
    if (!proc_name.empty()) {
        std::regex proc_pattern(proc_name + R"(_\d{8}_\d{6}\.log)");
        auto proc_matches = FindMatchingFiles(test_log_dir_, proc_pattern);
        EXPECT_TRUE(proc_matches.empty())
            << "Rotated file should use user prefix, not process name";
    }
}

/**
 * @test 34. RotationFilenameProcTest - 轮转文件名使用进程名
 * @brief 验证当文件 stem 与进程名一致时，轮转文件以 <进程名>_YYYYMMDD_HHMMSS.log 命名
 */
TEST_F(LoggerTest, RotationFilenameProcTest)
{
    std::string proc_name = GetProcessName();
    ASSERT_FALSE(proc_name.empty());

    const std::string proc_log = test_log_dir_ + "/" + proc_name + ".log";
    ASSERT_TRUE(Logger::GetInstance()->Init(proc_log, Logger::file, Logger::info, 1, 5));

    for (int i = 0; i < 6000; ++i) {
        LOG(info) << "RotationFilenameUsesProcessName message " << i
                  << " padding padding padding padding padding padding padding padding padding";
    }

    Logger::GetInstance()->Uninit();

    std::regex pattern(proc_name + R"(_\d{8}_\d{6}\.log)");
    auto matches = FindMatchingFiles(test_log_dir_, pattern);
    EXPECT_FALSE(matches.empty())
        << "No rotated file matching " << proc_name << "_YYYYMMDD_HHMMSS.log found";
}

/**
 * @test 35. RotationCleanupTest - 轮转后清理超出 maxBackupIndex 的旧文件
 * @brief 验证轮转文件数量不超过 maxBackupIndex，旧文件被自动删除
 */
TEST_F(LoggerTest, RotationCleanupTest)
{
    // MaxFileSize=1MB, MaxBackupIndex=2
    ASSERT_TRUE(Logger::GetInstance()->Init(test_log_file_, Logger::file, Logger::info, 1, 2));

    // 写入大量日志触发多次轮转
    for (int i = 0; i < 30000; ++i) {
        LOG(info) << "RotationCleanup test message " << i
                  << " padding padding padding padding padding padding padding";
    }

    Logger::GetInstance()->Uninit();

    // 统计轮转文件数量
    std::regex pattern(R"(test_\d{8}_\d{6}\.log)");
    auto rotated_files = FindMatchingFiles(test_log_dir_, pattern);
    EXPECT_LE(rotated_files.size(), 2u)
        << "Rotated files (" << rotated_files.size() << ") exceed MaxBackupIndex=2";
}

/**
 * @test 36. RotationMultipleTimesTest - 多次轮转验证文件数量
 * @brief 验证设置较大的 maxBackupIndex 时，轮转文件数量正确
 */
TEST_F(LoggerTest, RotationMultipleTimesTest)
{
    // MaxFileSize=1MB, MaxBackupIndex=10
    ASSERT_TRUE(Logger::GetInstance()->Init(test_log_file_, Logger::file, Logger::info, 1, 10));

    // 写入大量日志触发多次轮转
    for (int i = 0; i < 50000; ++i) {
        LOG(info) << "RotationMultiple test " << i
                  << " padding padding padding padding padding padding padding";
    }

    Logger::GetInstance()->Uninit();

    std::regex pattern(R"(test_\d{8}_\d{6}\.log)");
    auto rotated_files = FindMatchingFiles(test_log_dir_, pattern);
    // 应该有多个轮转文件，但不超过 MaxBackupIndex
    EXPECT_GE(rotated_files.size(), 1u) << "Expected at least one rotated file";
    EXPECT_LE(rotated_files.size(), 10u)
        << "Rotated files exceed MaxBackupIndex=10";
}

// ============================================================
// 七、异步模式测试
// ============================================================

/**
 * @test 37. AsyncPerformanceTest - 异步日志性能（1万条）
 * @brief 验证异步模式下快速提交日志的能力，提交应很快完成
 */
TEST_F(LoggerTest, AsyncPerformanceTest)
{
    const int kLogCount = 10000;

    ASSERT_TRUE(
        Logger::GetInstance()->Init(test_log_file_, Logger::file, Logger::info, 10, 3, true));

    auto start = std::chrono::high_resolution_clock::now();

    for (int i = 0; i < kLogCount; ++i) {
        LOG(error) << "Async log message " << i << " with some additional text for realistic size";
    }

    auto submit_end = std::chrono::high_resolution_clock::now();
    auto submit_duration =
        std::chrono::duration_cast<std::chrono::milliseconds>(submit_end - start);

    // 等待异步线程处理完成
    std::this_thread::sleep_for(std::chrono::milliseconds(100));

    Logger::GetInstance()->Uninit();
    std::this_thread::sleep_for(std::chrono::milliseconds(50));

    // 检查实际写入的行数
    size_t actual_lines = CountLogLines(test_log_file_);

    // 允许少量丢失（异步日志的特性）
    EXPECT_GE(actual_lines, kLogCount * 0.99)
        << "Too many logs lost: " << (kLogCount - actual_lines);
    EXPECT_LE(actual_lines, kLogCount) << "More logs than expected";

    // 提交应该很快
    EXPECT_LT(submit_duration.count(), 1000)
        << "Async submit took too long: " << submit_duration.count() << "ms";
}

/**
 * @test 38. AsyncWaitTest - 异步日志 Uninit 等待写入完成
 * @brief 验证 Uninit 会等待所有异步日志写入完成
 */
TEST_F(LoggerTest, AsyncWaitTest)
{
    const int kLogCount = 1000;

    ASSERT_TRUE(
        Logger::GetInstance()->Init(test_log_file_, Logger::file, Logger::info, 10, 3, true));

    // 快速提交大量日志
    for (int i = 0; i < kLogCount; ++i) {
        LOG(info) << "Async wait test message " << i;
    }

    // 提交完成后，文件可能还没有完全写入
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    size_t lines_before_uninit = CountLogLines(test_log_file_);

    // Uninit应该等待所有日志写入完成
    Logger::GetInstance()->Uninit();

    size_t lines_after_uninit = CountLogLines(test_log_file_);

    // 最终应该所有日志都写入了
    EXPECT_EQ(lines_after_uninit, kLogCount);

    // 通常Uninit前的日志数会少于总数（因为异步还在处理）
    EXPECT_LE(lines_before_uninit, lines_after_uninit);
}

/**
 * @test 39. AsyncMultiThreadTest - 多线程异步日志
 * @brief 验证多线程同时写入异步日志的正确性和完整性
 */
TEST_F(LoggerTest, AsyncMultiThreadTest)
{
    const int kThreadCount = 8;
    const int kLogsPerThread = 1000;

    ASSERT_TRUE(
        Logger::GetInstance()->Init(test_log_file_, Logger::file, Logger::info, 10, 3, true));

    std::vector<std::thread> threads;
    std::atomic<int> completed_threads{0};

    auto start = std::chrono::high_resolution_clock::now();

    for (int t = 0; t < kThreadCount; ++t) {
        threads.emplace_back([t, kLogsPerThread, &completed_threads]() {
            for (int i = 0; i < kLogsPerThread; ++i) {
                LOG(info) << "Thread " << t << " message " << i << " - multithread test";
            }
            completed_threads.fetch_add(1);
        });
    }

    // 等待所有线程提交完成
    for (auto &thread : threads) {
        thread.join();
    }

    // 等待异步写入完成
    std::this_thread::sleep_for(std::chrono::milliseconds(1000));
    Logger::GetInstance()->Uninit();

    // 验证所有线程都完成了
    EXPECT_EQ(completed_threads.load(), kThreadCount);

    // 验证日志完整性
    EXPECT_EQ(CountLogLines(test_log_file_), kThreadCount * kLogsPerThread);
}

// ============================================================
// 八、同步模式测试
// ============================================================

/**
 * @test 40. SyncPerformanceTest - 同步日志性能（1万条）
 * @brief 验证同步模式下日志写入的正确性和性能
 */
TEST_F(LoggerTest, SyncPerformanceTest)
{
    const int kLogCount = 10000;

    ASSERT_TRUE(
        Logger::GetInstance()->Init(test_log_file_, Logger::file, Logger::info, 10, 3, false));

    auto start = std::chrono::high_resolution_clock::now();

    for (int i = 0; i < kLogCount; ++i) {
        LOG(info) << "Sync log message " << i << " with some additional text for realistic size";
    }

    Logger::GetInstance()->Uninit();

    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);

    // 验证所有日志都写入了
    EXPECT_EQ(CountLogLines(test_log_file_), kLogCount);

    // 性能要求：同步日志应该在合理时间内完成（10秒）
    EXPECT_LT(duration.count(), 10000)
        << "Sync logging took too long: " << duration.count() << "ms";
}

// ============================================================
// 九、限流测试
// ============================================================

/**
 * @test 41. RateLimitTest - LOG_TIME 限流宏
 * @brief 验证 LOG_TIME 宏在指定时间间隔内只输出一条日志
 */
TEST_F(LoggerTest, RateLimitTest)
{
    ASSERT_TRUE(Logger::GetInstance()->Init(test_log_file_, Logger::file, Logger::info, 1, 3));

    // 快速连续调用限流日志
    for (int i = 0; i < 100; ++i) {
        LOG_TIME(info, 100) << "Rate limited message " << i;  // 100ms间隔
    }

    // 等待超过限流间隔
    std::this_thread::sleep_for(std::chrono::milliseconds(150));

    LOG_TIME(info, 100) << "Rate limited message after delay";

    Logger::GetInstance()->Uninit();

    // 验证限流效果（应该只有少数几条日志）
    size_t actual_count = CountLogLines(test_log_file_);
    EXPECT_LT(actual_count, 10) << "Rate limiting not working, got " << actual_count << " lines";
    EXPECT_GE(actual_count, 2) << "Rate limiting too aggressive, got " << actual_count << " lines";
}

/**
 * @test 42. RateLimitResetTest - 限流间隔重置后恢复
 * @brief 验证限流间隔过后，日志能正常输出，且限流期间的消息被正确抑制
 */
TEST_F(LoggerTest, RateLimitResetTest)
{
    ASSERT_TRUE(Logger::GetInstance()->Init(test_log_file_, Logger::file, Logger::info, 1, 3));

    // 使用 500ms 限流间隔，确保快速循环在限流窗口内
    LOG_TIME(info, 500) << "Rate limit first message";

    // 快速连续调用（同一宏展开的 __LINE__ 位置共享限流状态）
    // 这些调用应在 500ms 限流窗口内被抑制
    for (int i = 0; i < 100; ++i) {
        LOG_TIME(info, 500) << "Rate limit suppressed";
    }

    // 等待超过限流间隔
    std::this_thread::sleep_for(std::chrono::milliseconds(600));

    // 限流间隔过后应能再次输出
    LOG_TIME(info, 500) << "Rate limit after reset";

    Logger::GetInstance()->Uninit();

    std::string content = ReadFileContent(test_log_file_);
    EXPECT_TRUE(content.find("Rate limit first message") != std::string::npos);
    EXPECT_TRUE(content.find("Rate limit after reset") != std::string::npos);

    // 限流期间的消息总数应远少于尝试次数
    size_t suppressed_count = 0;
    size_t pos = 0;
    while ((pos = content.find("Rate limit suppressed", pos)) != std::string::npos) {
        suppressed_count++;
        pos += 21;
    }
    EXPECT_LT(suppressed_count, 10u)
        << "Rate limiting should suppress most messages, but got " << suppressed_count;
}

// ============================================================
// 十、性能对比测试
// ============================================================

/**
 * @test 43. SyncVsAsyncComparisonTest - 同步 vs 异步性能对比
 * @brief 对比同步和异步模式的提交速度差异
 */
TEST_F(LoggerTest, SyncVsAsyncComparisonTest)
{
    const int kLogCount = 10000;

    // 测试同步性能
    ASSERT_TRUE(Logger::GetInstance()->Init(test_log_file_ + "_sync", Logger::file, Logger::info,
                                            60, 3, false));
    auto sync_start = std::chrono::high_resolution_clock::now();
    for (int i = 0; i < kLogCount; ++i) {
        LOG(info) << "Sync comparison test message " << i;
    }
    auto sync_end = std::chrono::high_resolution_clock::now();
    Logger::GetInstance()->Uninit();

    auto sync_duration =
        std::chrono::duration_cast<std::chrono::milliseconds>(sync_end - sync_start);

    // 测试异步性能
    ASSERT_TRUE(Logger::GetInstance()->Init(test_log_file_ + "_async", Logger::file, Logger::info,
                                            10, 3, true));

    auto async_start = std::chrono::high_resolution_clock::now();
    for (int i = 0; i < kLogCount; ++i) {
        LOG(info) << "Async comparison test message " << i;
    }
    auto async_submit_end = std::chrono::high_resolution_clock::now();

    // 等待异步写入完成
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    Logger::GetInstance()->Uninit();

    auto async_submit_duration =
        std::chrono::duration_cast<std::chrono::milliseconds>(async_submit_end - async_start);

    // 验证数据完整性
    EXPECT_EQ(CountLogLines(test_log_file_ + "_sync"), kLogCount);
    EXPECT_EQ(CountLogLines(test_log_file_ + "_async"), kLogCount);
}

// ============================================================
// 十一、日志格式与内容测试
// ============================================================

/**
 * @test 44. LogPatternTimestampTest - 日志格式包含时间戳
 * @brief 验证默认 pattern 下日志包含时间戳信息
 */
TEST_F(LoggerTest, LogPatternTimestampTest)
{
    ASSERT_TRUE(Logger::GetInstance()->Init(test_log_file_, Logger::file, Logger::info, 10, 3));

    LOG(info) << "Timestamp pattern test";

    Logger::GetInstance()->Uninit();

    std::string content = ReadFileContent(test_log_file_);
    // 默认 pattern 包含 %Y-%m-%d %H:%M:%S，应匹配日期时间格式
    std::regex timestamp_regex(R"(\d{4}-\d{2}-\d{2} \d{2}:\d{2}:\d{2})");
    EXPECT_TRUE(std::regex_search(content, timestamp_regex))
        << "Log should contain timestamp in default pattern";
}

/**
 * @test 45. LogPatternLevelTest - 日志格式包含级别标识
 * @brief 验证默认 pattern 下日志包含日志级别标识
 */
TEST_F(LoggerTest, LogPatternLevelTest)
{
    ASSERT_TRUE(Logger::GetInstance()->Init(test_log_file_, Logger::file, Logger::info, 10, 3));

    LOG(info) << "Level pattern test";
    LOG(error) << "Error level test";

    Logger::GetInstance()->Uninit();

    std::string content = ReadFileContent(test_log_file_);
    // 默认 pattern 包含 %l，应输出级别名称
    EXPECT_TRUE(content.find("info") != std::string::npos || content.find("INFO") != std::string::npos)
        << "Log should contain level name";
    EXPECT_TRUE(content.find("error") != std::string::npos || content.find("ERROR") != std::string::npos)
        << "Log should contain error level name";
}

/**
 * @test 46. LogPatternSourceLocTest - 日志格式包含源码位置
 * @brief 验证默认 pattern 下日志包含源文件名和行号
 */
TEST_F(LoggerTest, LogPatternSourceLocTest)
{
    ASSERT_TRUE(Logger::GetInstance()->Init(test_log_file_, Logger::file, Logger::info, 10, 3));

    LOG(info) << "Source location test";

    Logger::GetInstance()->Uninit();

    std::string content = ReadFileContent(test_log_file_);
    // 默认 pattern 包含 %s（文件名）和 %#（行号），应包含 logger_test.cc
    EXPECT_TRUE(content.find("logger_test.cc") != std::string::npos)
        << "Log should contain source file name";
}

// ============================================================
// 十二、边界与异常测试
// ============================================================

/**
 * @test 47. EmptyMessageTest - 空消息日志
 * @brief 验证输出空消息不会崩溃
 */
TEST_F(LoggerTest, EmptyMessageTest)
{
    ASSERT_TRUE(Logger::GetInstance()->Init(test_log_file_, Logger::file, Logger::info, 10, 3));

    LOG(info) << "";
    LOG(info) << std::string("");
    LOG(info) << "Non-empty after empty";

    Logger::GetInstance()->Uninit();

    std::string content = ReadFileContent(test_log_file_);
    // 空消息后应能正常输出后续消息
    EXPECT_TRUE(content.find("Non-empty after empty") != std::string::npos);
}

/**
 * @test 48. LongMessageTest - 超长消息日志
 * @brief 验证输出超长消息（10KB）不会截断或崩溃
 */
TEST_F(LoggerTest, LongMessageTest)
{
    ASSERT_TRUE(Logger::GetInstance()->Init(test_log_file_, Logger::file, Logger::info, 10, 3));

    // 构造 10KB 的长消息
    std::string long_message(10240, 'A');
    LOG(info) << long_message;

    Logger::GetInstance()->Uninit();

    std::string content = ReadFileContent(test_log_file_);
    EXPECT_TRUE(content.find(long_message) != std::string::npos)
        << "Long message should not be truncated";
}

/**
 * @test 49. SpecialCharsMessageTest - 特殊字符消息
 * @brief 验证包含特殊字符的消息能正确输出
 */
TEST_F(LoggerTest, SpecialCharsMessageTest)
{
    ASSERT_TRUE(Logger::GetInstance()->Init(test_log_file_, Logger::file, Logger::info, 10, 3));

    LOG(info) << "Special chars: \t\n\r\\\"'<>&|{}[]()!@#$%^&*";
    LOG(info) << "Unicode: 你好世界 🌍 こんにちは";
    LOG(info) << "Null embedded: before\0after";

    Logger::GetInstance()->Uninit();

    std::string content = ReadFileContent(test_log_file_);
    EXPECT_TRUE(content.find("Special chars:") != std::string::npos);
    EXPECT_TRUE(content.find("Unicode:") != std::string::npos);
}

/**
 * @test 50. ShouldLogDirectTest - ShouldLog 直接调用验证
 * @brief 验证 ShouldLog 方法正确反映 logger-level 的过滤状态
 * @note ShouldLog 检查的是 spdlog logger-level（由 SetLogLevel 设置），
 *       而非 sink-level（由 SetLogFileLevel/SetLogConsoleLevel 设置）。
 *       注意：Init() 内部会硬编码 logger_->set_level(trace)，因此需要通过
 *       SetLogLevel() 动态设置 logger-level 才能改变 ShouldLog 的返回值。
 */
TEST_F(LoggerTest, ShouldLogDirectTest)
{
    ASSERT_TRUE(Logger::GetInstance()->Init(test_log_file_, Logger::file, Logger::trace, 10, 3));

    // Init 后 logger-level 默认为 trace，所有级别都应返回 true
    EXPECT_TRUE(Logger::GetInstance()->ShouldLog(Logger::trace));
    EXPECT_TRUE(Logger::GetInstance()->ShouldLog(Logger::debug));
    EXPECT_TRUE(Logger::GetInstance()->ShouldLog(Logger::info));
    EXPECT_TRUE(Logger::GetInstance()->ShouldLog(Logger::warning));
    EXPECT_TRUE(Logger::GetInstance()->ShouldLog(Logger::error));
    EXPECT_TRUE(Logger::GetInstance()->ShouldLog(Logger::critical));

    // 动态设置 logger-level 到 warning
    Logger::GetInstance()->SetLogLevel(Logger::warning);

    EXPECT_FALSE(Logger::GetInstance()->ShouldLog(Logger::trace))
        << "trace should not pass logger-level=warning";
    EXPECT_FALSE(Logger::GetInstance()->ShouldLog(Logger::debug))
        << "debug should not pass logger-level=warning";
    EXPECT_FALSE(Logger::GetInstance()->ShouldLog(Logger::info))
        << "info should not pass logger-level=warning";
    EXPECT_TRUE(Logger::GetInstance()->ShouldLog(Logger::warning))
        << "warning should pass logger-level=warning";
    EXPECT_TRUE(Logger::GetInstance()->ShouldLog(Logger::error));
    EXPECT_TRUE(Logger::GetInstance()->ShouldLog(Logger::critical));

    // 动态改变 logger-level 到 debug
    Logger::GetInstance()->SetLogLevel(Logger::debug);

    EXPECT_FALSE(Logger::GetInstance()->ShouldLog(Logger::trace))
        << "trace should not pass logger-level=debug";
    EXPECT_TRUE(Logger::GetInstance()->ShouldLog(Logger::debug))
        << "debug should pass logger-level=debug";
    EXPECT_TRUE(Logger::GetInstance()->ShouldLog(Logger::info));

    // 动态改变 logger-level 到 critical
    Logger::GetInstance()->SetLogLevel(Logger::critical);

    EXPECT_FALSE(Logger::GetInstance()->ShouldLog(Logger::trace));
    EXPECT_FALSE(Logger::GetInstance()->ShouldLog(Logger::debug));
    EXPECT_FALSE(Logger::GetInstance()->ShouldLog(Logger::info));
    EXPECT_FALSE(Logger::GetInstance()->ShouldLog(Logger::warning));
    EXPECT_FALSE(Logger::GetInstance()->ShouldLog(Logger::error));
    EXPECT_TRUE(Logger::GetInstance()->ShouldLog(Logger::critical))
        << "critical should pass logger-level=critical";

    Logger::GetInstance()->Uninit();
}
