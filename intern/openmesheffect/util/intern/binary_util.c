/*
 * Copyright 2019 Elie Michel
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

#include <stddef.h>
#include <stdio.h>

#ifdef _WIN32
#include <windows.h>
#else // _WIN32
#include <dlfcn.h>
#endif // _WIN32

#include "binary_util.h"

#ifdef _WIN32
static LPVOID getLastErrorMessage() {
  LPVOID msg;
  FormatMessage(
    FORMAT_MESSAGE_ALLOCATE_BUFFER |
    FORMAT_MESSAGE_FROM_SYSTEM |
    FORMAT_MESSAGE_IGNORE_INSERTS,
    NULL,
    GetLastError(),
    MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
    (LPTSTR)&msg,
    0, NULL);
  return msg;
}
#endif // _WIN32


BinaryHandle binary_open(const char *filepath) {
	BinaryHandle handle;
#ifdef _WIN32
	handle = LoadLibrary(TEXT(filepath));
	if (NULL == handle) {
		LPVOID msg = getLastErrorMessage();
		printf("mfxHost: Unable to load plugin binary at path %s. LoadLibrary returned: %s\n", filepath, (char*)msg);
		LocalFree(msg);
	}
#else
	handle = dlopen(filepath, RTLD_LAZY | RTLD_LOCAL);
	if (NULL == handle) {
		printf("mfxHost: Unable to load plugin binary at path %s. dlopen returned: %s\n", filepath, dlerror());
	}
#endif
	return handle;
}

void binary_close(BinaryHandle handle) {
#ifdef _WIN32
	FreeLibrary(handle);
#else // _WIN32
	dlclose(handle);
#endif // _WIN32
}

ProcHandle binary_get_proc(BinaryHandle handle, const char *name) {
	ProcHandle proc;

#ifdef _WIN32
	proc = GetProcAddress(handle, name);
#else // _WIN32
	proc = dlsym(handle, name);
#endif // _WIN32
	
	if (NULL == proc) {
		printf("mfxHost: Unable to load symbol '%s'. ", name);
#ifdef _WIN32
		LPVOID msg = getLastErrorMessage();
		printf("GetProcAddress returned: %s\n", (char*)msg);
		LocalFree(msg);
#else // _WIN32
		printf("dlsym returned: %s\n", dlerror());
#endif // _WIN32
	}

	return proc;
}

