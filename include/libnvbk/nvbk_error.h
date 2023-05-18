/*
 * Copyright 2023 Rapper_skull
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *    http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef NVBK_ERROR_H
#define NVBK_ERROR_H

#ifdef __cplusplus
extern "C" {
#endif

#define NVBK_ERR_OK                 0
#define NVBK_SEE_ERRNO              -1
#define NVBK_INVALID_PARAM          -2
#define NVBK_READ_END               -3
#define NVBK_INVALID_MAGIC          -4
#define NVBK_INVALID_HEADER_SIZE    -5
#define NVBK_INVALID_ENTRIES_COUNT  -6
#define NVBK_PTR_NOT_NULL           -7
#define NVBK_INVALID_FD             -8
#define NVBK_ENTRY_NOT_LOADED       -9
#define NVBK_SHA256_ERROR           -10
#define NVBK_WRITE_END              -11

const char* nvbk_strerror(int errnum);

#ifdef __cplusplus
}
#endif

#endif	//NVBK_ERROR_H
