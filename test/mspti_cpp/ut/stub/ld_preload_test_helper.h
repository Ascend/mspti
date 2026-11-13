/*
 * -------------------------------------------------------------------------
 * This file is part of the MindStudio project.
 * Copyright (c) 2026 Huawei Technologies Co.,Ltd.
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
#ifndef MSPTI_UT_LD_PRELOAD_TEST_HELPER_H
#define MSPTI_UT_LD_PRELOAD_TEST_HELPER_H

#include <linux/limits.h>
#include <unistd.h>

#include <cstdio>
#include <cstdlib>
#include <fstream>
#include <string>

namespace MsptiTest
{
// Creates a temporary directory holding a real libmspti.so file. The file is kept
// alive for the whole test process so that LD_PRELOAD checks which require the
// library file to exist can be exercised.
class LdPreloadLibFile
{
   public:
    LdPreloadLibFile()
    {
        std::string dirTemplate = "/tmp/mspti_ut_ld_preload_XXXXXX";
        char* dir = mkdtemp(&dirTemplate[0]);
        if (dir == nullptr)
        {
            return;
        }
        dir_ = dir;
        path_ = dir_ + "/libmspti.so";
        std::ofstream file(path_);
    }

    ~LdPreloadLibFile()
    {
        if (!path_.empty())
        {
            std::remove(path_.c_str());
        }
        if (!dir_.empty())
        {
            rmdir(dir_.c_str());
        }
    }

    const std::string& Path() const { return path_; }
    const std::string& Dir() const { return dir_; }

   private:
    std::string dir_;
    std::string path_;
};

inline const LdPreloadLibFile& GetLdPreloadLibFile()
{
    static const LdPreloadLibFile libFile;
    return libFile;
}

inline const std::string& GetLdPreloadLibPath() { return GetLdPreloadLibFile().Path(); }

inline const std::string& GetLdPreloadLibDir() { return GetLdPreloadLibFile().Dir(); }

// Switches the process working directory to @p dir for the lifetime of the object
// and restores the previous directory in the destructor, so that scenarios which
// resolve a bare library name against the current directory never touch the real
// test working directory. Check IsValid() before relying on it.
class ScopedWorkingDirectory
{
   public:
    explicit ScopedWorkingDirectory(const std::string& dir)
    {
        char buffer[PATH_MAX] = {0};
        if (dir.empty() || getcwd(buffer, sizeof(buffer)) == nullptr)
        {
            return;
        }
        oldDir_ = buffer;
        if (chdir(dir.c_str()) != 0)
        {
            oldDir_.clear();
            return;
        }
        valid_ = true;
    }

    ~ScopedWorkingDirectory()
    {
        if (valid_ && !oldDir_.empty())
        {
            chdir(oldDir_.c_str());
        }
    }

    ScopedWorkingDirectory(const ScopedWorkingDirectory&) = delete;
    ScopedWorkingDirectory& operator=(const ScopedWorkingDirectory&) = delete;

    bool IsValid() const { return valid_; }

   private:
    std::string oldDir_;
    bool valid_ = false;
};
}  // namespace MsptiTest

#endif  // MSPTI_UT_LD_PRELOAD_TEST_HELPER_H
