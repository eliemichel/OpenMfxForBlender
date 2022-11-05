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

#include "Logger.h"

#include <chrono>
#include <ctime>
#include <cstring>
#include <iomanip>

#ifdef _WIN32
#include <windows.h>
#endif

using namespace OpenMfx;

size_t Logger::align_width = 0;

static bool EnableVTMode()
{
#ifdef _WIN32
	// Set output mode to handle virtual terminal sequences
	HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);
	if (hOut == INVALID_HANDLE_VALUE)
	{
		return false;
	}

	DWORD dwMode = 0;
	if (!GetConsoleMode(hOut, &dwMode))
	{
		return false;
	}

	dwMode |= ENABLE_VIRTUAL_TERMINAL_PROCESSING;
	if (!SetConsoleMode(hOut, dwMode))
	{
		return false;
	}
#endif
	return true;
}

namespace Color
{
#ifdef _WIN32
	static const std::string CSI = "\x1b[";
#else
	static const std::string CSI = "\033[";
#endif
    const std::string bold_grey      = CSI + "1;30m";
    const std::string bold_red       = CSI + "1;31m";
    const std::string bold_green     = CSI + "1;32m";
    const std::string bold_yellow    = CSI + "1;33m";
    const std::string bold_blue      = CSI + "1;34m";
    const std::string bold_purple    = CSI + "1;35m";
    const std::string bold_lightblue = CSI + "1;36m";
    const std::string bold_white     = CSI + "1;37m";

    const std::string black     = CSI + "0;30m";
    const std::string red       = CSI + "0;31m";
    const std::string green     = CSI + "0;32m";
    const std::string yellow    = CSI + "0;33m";
    const std::string blue      = CSI + "0;34m";
    const std::string purple    = CSI + "0;35m";
    const std::string lightblue = CSI + "0;36m";
    const std::string white     = CSI + "0;37m";

    const std::string nocolor   = CSI + "0m";
} // namespace Color

OpenMfx::Logger::Logger(const char *func, const char *file, int line, Logger::Level level)
{
    (void)file;
	if (Logger::align_width == 0) {
		Logger::init();
		Logger::align_width = 1;
	}

    {
        using namespace std::chrono;
        std::time_t now_c = system_clock::to_time_t(system_clock::now());
#ifdef _WIN32
		struct tm timeinfoData;
		struct tm * timeinfo = &timeinfoData;
		localtime_s(timeinfo, &now_c);
#else // _WIN32
		struct tm * timeinfo = std::localtime(&now_c);
#endif // _WIN32
		char buffer[26];
		strftime(buffer, 26, "%Y-%m-%d %H:%M:%S", timeinfo);
		m_ss << buffer << ": ";
    }

    switch(level)
    {
    case Level::Debug:
        m_ss << Color::bold_grey   << "[Debug]  " << Color::nocolor;
        break;
    case Level::Verbose:
        m_ss << Color::bold_white  << "[Details]" << Color::nocolor;
        break;
    case Level::Info:
        m_ss << Color::bold_white  << "[Info]   " << Color::nocolor;
        break;
    case Level::Warning:
        m_ss << Color::bold_yellow << "[Warning]" << Color::nocolor;
        break;
    case Level::Error:
        m_ss << Color::bold_red    << "[Error]  " << Color::nocolor;
        break;
    }

    if( level != Level::Info )
    {
        std::stringstream pos;
        pos << " (" << func << "():" << ":" << line << ")";
        m_ss << Color::blue << pos.str() << Color::nocolor;
    }

    m_ss << " ";

    switch( level) {
    case Level::Debug:   m_ss << Color::bold_grey;    break;
    case Level::Verbose: m_ss << Color::nocolor; break;
    case Level::Info:    m_ss << Color::nocolor; break;
    case Level::Warning: m_ss << Color::yellow;  break;
    case Level::Error:   m_ss << Color::red;     break;
    }
}

OpenMfx::Logger::~Logger()
{
    m_ss << Color::nocolor;
    std::string str = m_ss.str();
    // trim extra newlines
    while ( str.empty() == false && str[str.length() - 1] == '\n')
        str.resize(str.length() - 1);
    std::cerr << str << std::endl;
}

void OpenMfx::Logger::init() {
	if (!EnableVTMode()) {
		std::cerr << "Warning: Logger could not enable Virtual Terminal mode." << std::endl;
	}
}
