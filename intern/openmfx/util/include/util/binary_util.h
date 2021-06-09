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

/** \file
 * \ingroup openmesheffect
 *
 * This section provides a unified access to dynamic library loading.
 */

#ifndef __MFX_BINARY_UTIL_H__
#define __MFX_BINARY_UTIL_H__

#ifdef _WIN32
#include <windows.h>
#endif// _WIN32

/**
 * Blind handle to a library binary
 */
#ifdef _WIN32
typedef HINSTANCE BinaryHandle;
#else // _WIN32
typedef void* BinaryHandle;
#endif // _WIN32

/**
 * Blind handle to a procedure
 */
#ifdef _WIN32
typedef FARPROC ProcHandle;
#else // _WIN32
typedef void* ProcHandle;
#endif // _WIN32

/**
 * Open a binary library.
 * Return a null handle if the library could not be loaded.
 * A null handle must be closed with binary_close;
 */
BinaryHandle binary_open(const char *filepath);

/**
 * Close a binary handle that has been previously openned with binary_open
 */
void binary_close(BinaryHandle handle);

/**
 * Get a procedure by name from a loaded binary library. The library must have
 * been loaded first with binary_open.
 */
ProcHandle binary_get_proc(BinaryHandle handle, const char *name);

#endif // __MFX_BINARY_UTIL_H__
