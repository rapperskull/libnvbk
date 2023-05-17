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

#if defined(_MSC_VER)
  #define _CRT_NONSTDC_NO_WARNINGS  // POSIX function names
  #define _CRT_SECURE_NO_WARNINGS   // Unsafe CRT Library functions
#endif

#include <string.h>
#include <errno.h>
#include <libnvbk/nvbk_error.h>

const char* nvbk_strerror(int errnum) {
  switch (errnum) {
  case NVBK_ERR_OK:
    return "No error occurred.";
  case NVBK_SEE_ERRNO:
    return strerror(errno);
  case NVBK_INVALID_PARAM:
    return "Invalid input parameter.";
  case NVBK_READ_END:
    return "Reached EOF while reading.";
  case NVBK_INVALID_MAGIC:
    return "Invalid NVBK partition header magic.";
  case NVBK_INVALID_HEADER_SIZE:
    return "Invalid NVBK partition header size.";
  case NVBK_INVALID_ENTRIES_COUNT:
    return "Invalid NVBK partition header entries count.";
  case NVBK_PTR_NOT_NULL:
    return "Passed pointer is not NULL.";
  case NVBK_INVALID_FD:
    return "Invalid file descriptor.";
  case NVBK_ENTRY_NOT_LOADED:
    return "The specified entry has not been loaded.";
  case NVBK_SHA256_ERROR:
    return "The SHA-256 library returned an error.";
  case NVBK_WRITE_END:
    return "Reached EOF while writing.";
  default:
    return "Unknown error.";
  }
}
