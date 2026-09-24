/*
 * -------------------------------------------------------------------------
 * This file is part of the MindStudio project.
 * Copyright (c) 2025 Huawei Technologies Co.,Ltd.
 *
 * MindStudio is licensed under Mulan PSL v2.
 * You can use this software according to the terms and conditions of the Mulan PSL v2.
 * You may obtain a copy of Mulan PSL v2 at:
 *
 *          http://license.coscl.org.cn/MulanPSL2
 *
 * THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND,
 * EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT,
 * MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.
 * See the Mulan PSL v2 for more details.
 * -------------------------------------------------------------------------
 */
#include <linux/limits.h>
#include <sys/syscall.h>

#include <cstdint>
#include <fstream>
#include <map>
#include <unordered_map>
#include <vector>

#include "csrc/common/runtime_utils.h"
#include "csrc/common/utils.h"
#include "gtest/gtest.h"
#include "mockcpp/mockcpp.hpp"
#include "securec.h"

namespace
{
class UtilsUtest : public testing::Test
{
   protected:
    virtual void SetUp() {}
    virtual void TearDown() {}
};

TEST_F(UtilsUtest, RealPathShouldReturnRealPathWhenInputRelativePath)
{
    std::string path = "./test";
    auto realPath = Mspti::Common::Utils::RealPath(path);
    char buf[PATH_MAX];
    getcwd(buf, PATH_MAX);
    std::string targetPath = std::string(buf) + "/test";
    EXPECT_STREQ(targetPath.c_str(), realPath.c_str());
}

TEST_F(UtilsUtest, RealPathShouldReturnEmptyPathWhenInputInvalidPath)
{
    auto realPath = Mspti::Common::Utils::RealPath("");
    EXPECT_TRUE(realPath.empty());
    realPath = Mspti::Common::Utils::RealPath(std::string(PATH_MAX + 1, 'x'));
    EXPECT_TRUE(realPath.empty());
}

TEST_F(UtilsUtest, RealPathShouldReturnRealPathWhenInputAbsolutePath)
{
    char buf[PATH_MAX];
    getcwd(buf, PATH_MAX);
    std::string path = std::string(buf) + "/test";
    auto realPath = Mspti::Common::Utils::RealPath(path);
    EXPECT_STREQ(path.c_str(), realPath.c_str());
}

TEST_F(UtilsUtest, GetClockMonotonicRawNsTest)
{
    auto monoTime = Mspti::Common::Utils::GetClockMonotonicRawNs();
    EXPECT_GT(monoTime, 0UL);
}

TEST_F(UtilsUtest, GetClockRealTimeNsTest)
{
    auto realTime = Mspti::Common::Utils::GetClockRealTimeNs();
    EXPECT_GT(realTime, 0UL);
}

TEST_F(UtilsUtest, GetClockRealTimeNsMemsetFail)
{
    MOCKER_CPP(&memset_s).stubs().will(returnValue(1));
    uint64_t result = Mspti::Common::Utils::GetClockRealTimeNs();
    EXPECT_EQ(result, 0);
}

TEST_F(UtilsUtest, GetHostSysCntTest)
{
    auto sysCnt = Mspti::Common::Utils::GetHostSysCnt();
    EXPECT_GT(sysCnt, 0UL);
}

TEST_F(UtilsUtest, GetHostSysCntTestMemsetFail)
{
    MOCKER_CPP(&memset_s).stubs().will(returnValue(1));
    uint64_t result = Mspti::Common::Utils::GetClockRealTimeNs();
    EXPECT_EQ(result, 0);
}

TEST_F(UtilsUtest, GetPidTest)
{
    auto pid = Mspti::Common::Utils::GetPid();
    EXPECT_EQ(pid, static_cast<uint32_t>(getpid()));
}

TEST_F(UtilsUtest, GetTidTest)
{
    auto tid = Mspti::Common::Utils::GetTid();
    EXPECT_EQ(tid, static_cast<uint32_t>(syscall(SYS_gettid)));
}

TEST_F(UtilsUtest, GetDeviceIdTest)
{
    const uint32_t expectDeviceId = 0U;
    EXPECT_EQ(expectDeviceId, Mspti::Common::GetDeviceId());
}

TEST_F(UtilsUtest, GetStreamIdTest)
{
    const uint32_t expectStreamId = 0U;
    AclrtStream stm = nullptr;
    EXPECT_EQ(expectStreamId, Mspti::Common::GetStreamId(stm));
}

TEST_F(UtilsUtest, RelativeToAbsPathTest)
{
    std::string stubPath = "test.txt";
    char pwdPath[PATH_MAX] = {0};
    if (getcwd(pwdPath, PATH_MAX) != nullptr)
    {
        std::string targetPath = std::string(pwdPath) + "/" + stubPath;
        std::string path = Mspti::Common::Utils::RelativeToAbsPath(stubPath);
        EXPECT_STREQ(targetPath.c_str(), path.c_str());
    }
    else
    {
        std::string targetPath = "";
        std::string path = Mspti::Common::Utils::RelativeToAbsPath(stubPath);
        EXPECT_STREQ(targetPath.c_str(), path.c_str());
    }
}

TEST_F(UtilsUtest, EmptyPath)
{
    std::string result = Mspti::Common::Utils::RelativeToAbsPath("");
    EXPECT_EQ(result, "");
}

TEST_F(UtilsUtest, PathTooLong)
{
    std::string longPath(PATH_MAX + 1, 'a');
    std::string result = Mspti::Common::Utils::RelativeToAbsPath(longPath);
    EXPECT_EQ(result, "");
}

TEST_F(UtilsUtest, ShouldGetTrueWhenFileExist)
{
    std::string stubPath = "test.txt";
    std::ofstream f(stubPath);
    if (f.is_open())
    {
        f.close();
    }
    EXPECT_EQ(true, Mspti::Common::Utils::FileExist(stubPath));
    std::remove(stubPath.c_str());
}

TEST_F(UtilsUtest, ShouldGetFalseWhenFileNotExist)
{
    std::string stubPath = "test.txt";
    EXPECT_EQ(false, Mspti::Common::Utils::FileExist(stubPath));
}

TEST_F(UtilsUtest, FileReadableShouldGetFalseWhenFileEmpty)
{
    std::string path = "";
    EXPECT_EQ(false, Mspti::Common::Utils::FileReadable(path));
}

TEST_F(UtilsUtest, CheckCharValidShouldReturnFalseWhenMsgContainsSpecialCharacter)
{
    const char* msg = "record&";
    EXPECT_FALSE(Mspti::Common::Utils::CheckCharValid(msg));
}

TEST_F(UtilsUtest, CheckCharValidShouldReturnTrueWhenMsgNotContainSpecialCharacter)
{
    const char* msg = "xxxx";
    EXPECT_TRUE(Mspti::Common::Utils::CheckCharValid(msg));
}

TEST_F(UtilsUtest, StartsWithShouldReturnFalseWhenInputStrNotStartWithPrefix)
{
    EXPECT_FALSE(Mspti::Common::Utils::StartsWith("xx", "xxxxxx"));
    EXPECT_FALSE(Mspti::Common::Utils::StartsWith("xx", "y"));
}

TEST_F(UtilsUtest, StartsWithShouldReturnTrueWhenInputStrStartWithPrefix)
{
    EXPECT_TRUE(Mspti::Common::Utils::StartsWith("xx", "x"));
}

TEST_F(UtilsUtest, StrToI32ShouldReturnTrueWhenConvertValidNumber)
{
    int32_t dest = 0;
    EXPECT_TRUE(Mspti::Common::Utils::StrToI32(dest, "123"));
    EXPECT_EQ(dest, 123);
    EXPECT_TRUE(Mspti::Common::Utils::StrToI32(dest, "-456"));
    EXPECT_EQ(dest, -456);
    EXPECT_TRUE(Mspti::Common::Utils::StrToI32(dest, "0"));
    EXPECT_EQ(dest, 0);
}

TEST_F(UtilsUtest, StrToI32ShouldReturnTrueWhenConvertMaxMin)
{
    int32_t dest = 0;
    EXPECT_TRUE(Mspti::Common::Utils::StrToI32(dest, "2147483647"));
    EXPECT_EQ(dest, 2147483647);
    EXPECT_TRUE(Mspti::Common::Utils::StrToI32(dest, "-2147483648"));
    EXPECT_EQ(dest, -2147483648);
}

TEST_F(UtilsUtest, StrToI32ShouldReturnFalseWhenConvertEmptyString)
{
    int32_t dest = 0;
    EXPECT_FALSE(Mspti::Common::Utils::StrToI32(dest, ""));
}

TEST_F(UtilsUtest, StrToI32ShouldReturnFalseWhenConvertNonNumericString)
{
    int32_t dest = 0;
    EXPECT_FALSE(Mspti::Common::Utils::StrToI32(dest, "abc"));
    EXPECT_FALSE(Mspti::Common::Utils::StrToI32(dest, "12a34"));
}

TEST_F(UtilsUtest, StrToI32ShouldReturnFalseWhenConvertOutOfRange)
{
    int32_t dest = 0;
    EXPECT_FALSE(Mspti::Common::Utils::StrToI32(dest, "999999999999"));
    EXPECT_FALSE(Mspti::Common::Utils::StrToI32(dest, "-999999999999"));
}

TEST_F(UtilsUtest, StrToU32ShouldReturnTrueWhenConvertValidNumber)
{
    uint32_t dest = 0;
    EXPECT_TRUE(Mspti::Common::Utils::StrToU32(dest, "123"));
    EXPECT_EQ(dest, 123);
    EXPECT_TRUE(Mspti::Common::Utils::StrToU32(dest, "0"));
    EXPECT_EQ(dest, 0);
}

TEST_F(UtilsUtest, StrToU32ShouldReturnTrueWhenConvertUint32Max)
{
    uint32_t dest = 0;
    EXPECT_TRUE(Mspti::Common::Utils::StrToU32(dest, "4294967295"));
    EXPECT_EQ(dest, UINT32_MAX);
}

TEST_F(UtilsUtest, StrToU32ShouldReturnFalseWhenConvertEmptyString)
{
    uint32_t dest = 0;
    EXPECT_FALSE(Mspti::Common::Utils::StrToU32(dest, ""));
}

TEST_F(UtilsUtest, StrToU32ShouldReturnFalseWhenConvertNonNumericString)
{
    uint32_t dest = 0;
    EXPECT_FALSE(Mspti::Common::Utils::StrToU32(dest, "abc"));
    EXPECT_FALSE(Mspti::Common::Utils::StrToU32(dest, "12a34"));
}

TEST_F(UtilsUtest, StrToU32ShouldReturnFalseWhenConvertOutOfLongRange)
{
    uint32_t dest = 0;
    EXPECT_FALSE(Mspti::Common::Utils::StrToU32(dest, std::string(30, '9')));
}

TEST_F(UtilsUtest, StrToU32ShouldReturnFalseWhenConvertExceedsUint32Max)
{
    uint32_t dest = 0;
    EXPECT_FALSE(Mspti::Common::Utils::StrToU32(dest, "4294967296"));
    EXPECT_FALSE(Mspti::Common::Utils::StrToU32(dest, "18446744073709551615"));
}

TEST_F(UtilsUtest, StrToU32ShouldReturnFalseWhenConvertNegativeNumber)
{
    uint32_t dest = 0;
    EXPECT_FALSE(Mspti::Common::Utils::StrToU32(dest, "-1"));
}

TEST_F(UtilsUtest, GetCANNModuleVersionShouldReturnEmptyWhenVersionNotFound)
{
    auto version = Mspti::Common::GetCANNModuleVersion("unknown_module");
    EXPECT_TRUE(version.empty());
}

TEST_F(UtilsUtest, GetCANNModuleVersionShouldReturnVersionWhenFound)
{
    auto version = Mspti::Common::GetCANNModuleVersion("runtime");
    EXPECT_EQ(version, "9.2.0");
}

TEST_F(UtilsUtest, IsRuntimeSupportMemoryReportShouldReturnTrueWhenVersionValid)
{
    EXPECT_TRUE(Mspti::Common::IsRuntimeSupportMemoryReport());
}

TEST_F(UtilsUtest, ParseMsptiVersionReturnsVersionWhenModuleVersionAvailable)
{
    constexpr uint32_t EXPECTED_VERSION = 9 * 10000 + 2 * 100 + 0;
    EXPECT_EQ(EXPECTED_VERSION, Mspti::Common::ParseMsptiVersion("9.2.0"));
}

TEST_F(UtilsUtest, ParseMsptiVersionReturnsVersionWhenModuleVersionHasSuffix)
{
    constexpr uint32_t EXPECTED_VERSION = 26 * 10000 + 2 * 100 + 0;
    EXPECT_EQ(EXPECTED_VERSION, Mspti::Common::ParseMsptiVersion("26.2.0.dev"));
}

TEST_F(UtilsUtest, ParseMsptiVersionReturnsVersionWhenModuleVersionHasSuffixAndBuildMeta)
{
    constexpr uint32_t EXPECTED_VERSION = 26 * 10000 + 2 * 100 + 0;
    EXPECT_EQ(EXPECTED_VERSION, Mspti::Common::ParseMsptiVersion("26.2.0-rc1"));
}

TEST_F(UtilsUtest, ParseMsptiVersionReturnsZeroWhenModuleVersionInvalid)
{
    EXPECT_EQ(0U, Mspti::Common::ParseMsptiVersion("invalid"));
}

TEST_F(UtilsUtest, ParseMsptiVersionReturnsZeroWhenModuleVersionEmpty)
{
    EXPECT_EQ(0U, Mspti::Common::ParseMsptiVersion(""));
}

TEST_F(UtilsUtest, ParseMsptiVersionReturnsZeroWhenSegmentsMissing)
{
    EXPECT_EQ(0U, Mspti::Common::ParseMsptiVersion("9.2"));
    EXPECT_EQ(0U, Mspti::Common::ParseMsptiVersion("9.2."));
    EXPECT_EQ(0U, Mspti::Common::ParseMsptiVersion("9..0"));
    EXPECT_EQ(0U, Mspti::Common::ParseMsptiVersion("9"));
}

TEST_F(UtilsUtest, ParseMsptiVersionReturnsZeroWhenPrefixNotNumeric)
{
    EXPECT_EQ(0U, Mspti::Common::ParseMsptiVersion(".9.2.0"));
    EXPECT_EQ(0U, Mspti::Common::ParseMsptiVersion(" 9.2.0"));
    EXPECT_EQ(0U, Mspti::Common::ParseMsptiVersion("v9.2.0"));
    EXPECT_EQ(0U, Mspti::Common::ParseMsptiVersion("9.2a.0"));
}

TEST_F(UtilsUtest, ParseMsptiVersionIgnoresTrailingContentAfterPatch)
{
    constexpr uint32_t EXPECTED_VERSION = 9 * 10000 + 2 * 100 + 0;
    EXPECT_EQ(EXPECTED_VERSION, Mspti::Common::ParseMsptiVersion("9.2.0.1"));
    EXPECT_EQ(EXPECTED_VERSION, Mspti::Common::ParseMsptiVersion("9.2.0a"));
    EXPECT_EQ(EXPECTED_VERSION, Mspti::Common::ParseMsptiVersion("9.2.0 "));
}

TEST_F(UtilsUtest, ParseMsptiVersionSupportsLeadingZeros)
{
    constexpr uint32_t EXPECTED_VERSION = 9 * 10000 + 2 * 100 + 3;
    EXPECT_EQ(EXPECTED_VERSION, Mspti::Common::ParseMsptiVersion("09.02.003"));
}

TEST_F(UtilsUtest, ParseMsptiVersionReturnsZeroWhenNumericOverflow)
{
    EXPECT_EQ(4294967295U, Mspti::Common::ParseMsptiVersion("429496.72.95"));
    EXPECT_EQ(0U, Mspti::Common::ParseMsptiVersion("999999999999.0.0"));
    EXPECT_EQ(0U, Mspti::Common::ParseMsptiVersion("4294967296.0.0"));
    EXPECT_EQ(0U, Mspti::Common::ParseMsptiVersion("4294967295.0.0"));
    EXPECT_EQ(0U, Mspti::Common::ParseMsptiVersion("429497.0.0"));
    EXPECT_EQ(0U, Mspti::Common::ParseMsptiVersion("429496.73.0"));
}

TEST_F(UtilsUtest, EraseIfShouldRemoveMatchingElementsFromUnorderedMap)
{
    std::unordered_map<int, std::string> container{{1, "a"}, {2, "b"}, {3, "c"}, {4, "d"}};
    Mspti::Common::EraseIf(container, [](const auto& kv) { return kv.first % 2 == 0; });
    EXPECT_EQ(2U, container.size());
    EXPECT_TRUE(container.find(1) != container.end());
    EXPECT_TRUE(container.find(3) != container.end());
}

TEST_F(UtilsUtest, EraseIfShouldKeepUnmatchedElementsInOrderedMap)
{
    std::map<uint64_t, bool> container{{10, true}, {20, false}, {30, true}};
    Mspti::Common::EraseIf(container, [](const auto& kv) { return kv.second; });
    EXPECT_EQ(1U, container.size());
    EXPECT_TRUE(container.find(20) != container.end());
}

TEST_F(UtilsUtest, EraseIfShouldWorkOnSequenceContainer)
{
    std::vector<int> container{1, 2, 3, 4, 5};
    Mspti::Common::EraseIf(container, [](int value) { return value > 3; });
    EXPECT_EQ(3U, container.size());
    EXPECT_EQ(1, container[0]);
    EXPECT_EQ(3, container[2]);
}

TEST_F(UtilsUtest, EraseIfShouldHandleEmptyContainerAndFullMatch)
{
    std::unordered_map<int, int> empty;
    Mspti::Common::EraseIf(empty, [](const auto& kv) { return true; });
    EXPECT_TRUE(empty.empty());

    std::unordered_map<int, int> allMatch{{1, 1}, {2, 2}};
    Mspti::Common::EraseIf(allMatch, [](const auto& kv) { return true; });
    EXPECT_TRUE(allMatch.empty());

    std::unordered_map<int, int> noneMatch{{1, 1}, {2, 2}};
    Mspti::Common::EraseIf(noneMatch, [](const auto& kv) { return false; });
    EXPECT_EQ(2U, noneMatch.size());
}
}  // namespace
