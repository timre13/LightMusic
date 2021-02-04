/*
BSD 2-Clause License

Copyright (c) 2021, timre13
All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:

1. Redistributions of source code must retain the above copyright notice, this
   list of conditions and the following disclaimer.

2. Redistributions in binary form must reproduce the above copyright notice,
   this list of conditions and the following disclaimer in the documentation
   and/or other materials provided with the distribution.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

/*
 * System specific implementation of some useful functions.
 */

#pragma once

// TODO: Test on Win32

#include <string>

#if defined(__linux__) || defined(__linux) || defined(__unix__) || defined(__unix)
#include <unistd.h>
#include <cstdio>
#elif defined(__WIN32)
#include <windows.h>
#include <shellapi.h>
#else
static_assert(0, "Unsupported platform.");
#endif

namespace SysSpecific
{

inline void wait(double seconds)
{
#if defined(__linux__) || defined(__linux) || defined(__unix__) || defined(__unix)
    usleep(seconds * 1000'000);
#elif defined(__WIN32)
    Sleep(seconds * 1000);
#endif
}

inline void openUrlInWebBrowser(const std::string &url)
{
#if defined(__linux__) || defined(__linux) || defined(__unix__) || defined(__unix)
    std::system(("xdg-open " + url).c_str());
#elif defined(__WIN32)
    ShellExecute(0, 0, url.c_str(), 0, 0, SW_SHOW);
#endif
}

} // namespace SysSpecific
