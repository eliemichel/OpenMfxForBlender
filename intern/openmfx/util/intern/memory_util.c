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

#include <stdio.h>
#include <stdlib.h>
#include "memory_util.h"

void * malloc_array(size_t size, size_t count, const char *reason) {
	void * p = NULL;
	p = malloc(size * count);
	if (NULL == p) {
		fprintf(stderr, "Could not allocate memory for '%s' (requires %zu * %zu = %zu bytes)\n", reason, size, count, size * count);
	}
	return p;
}

void free_array(void *p) {
	free(p);
}
