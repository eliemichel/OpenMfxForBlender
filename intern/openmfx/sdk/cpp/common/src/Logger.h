/*
 * Copyright 2019-2022 Elie Michel
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#pragma once

#include <iostream>
#include <sstream>
#include <string>

namespace OpenMfx {

class Logger {
public:
    enum class Level
    {
        Debug,
        Verbose,
        Info,
        Warning,
        Error
    };

public:
    Logger(const char *func, const char *file, int line, Level level = Level::Info);
    ~Logger();

    inline std::ostream &stream() { return m_ss; }

private:
	static void init();

private:
    std::ostringstream m_ss;

private:
    static size_t align_width;
};

} // namespace OpenMfx

#define DEBUG_LOG OpenMfx::Logger(__func__, __FILE__, __LINE__, OpenMfx::Logger::Level::Debug).stream()
#define LOG OpenMfx::Logger(__func__, __FILE__, __LINE__, OpenMfx::Logger::Level::Info).stream()
#define WARN_LOG OpenMfx::Logger(__func__, __FILE__, __LINE__, OpenMfx::Logger::Level::Warning).stream()
#define ERR_LOG OpenMfx::Logger(__func__, __FILE__, __LINE__, OpenMfx::Logger::Level::Error).stream()

