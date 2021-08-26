/*
 * Copyright 2019 - 2021 Elie Michel
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
 */

#ifndef __MFX_ALLOCATOR_H__
#define __MFX_ALLOCATOR_H__

/**
 * Class responsible for memory allocation, to ease future drop in replacement
 */
class Allocator {
public:
	template <typename T>
	static T* malloc(size_t count, const char *description) {
		(void*)description; // ignore description in this basic implementation
		return new T[count];
	}

	template <typename T>
	static void free(T *pointer) {
		delete[] pointer;
	}
};

#endif // __MFX_ALLOCATOR_H__
