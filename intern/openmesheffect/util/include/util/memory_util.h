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

#ifndef __MFX_MEMORY_UTIL_H__
#define __MFX_MEMORY_UTIL_H__

#include "stddef.h" // for size_t

#ifdef __cplusplus
extern "C" {
#endif

void *malloc_array(size_t size, size_t count, const char *reason);
void free_array(void *p);

#ifdef __cplusplus
}
#endif

#endif // __MFX_MEMORY_UTIL_H__
