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

#ifndef LIBNVBK_H
#define LIBNVBK_H

#include <stdint.h>
#include <stdbool.h>
#include "nvbk_common.h"

typedef struct _nvbk_header_entry_t {
  unsigned long revision;
  unsigned long pos;
  unsigned long size;
  unsigned char rf_id;
  uint8_t sha256[32];
  unsigned char* entry_data;
} nvbk_header_entry_t;

typedef struct _nvbk_header_t {
  // First converted values
  unsigned char num_entries;
  unsigned char type;
  unsigned char rf_id_type;
  unsigned int version_minor;
  struct {
    unsigned int year;
    unsigned char month;
    unsigned char day;
  } date;
  unsigned char revision;
  char nv_image_version[NVBK_VERSION_SIZE + 1];
  // Then default values that we might want to change
  uint8_t version_major;
  char magic[8];
  uint32_t header_length;
  // Internal fields
  unsigned long size;
  int fd;
  nvbk_header_entry_t* entries;
} nvbk_header_t;

nvbk_header_t* nvbk_open(const char* filepath, int *error);
int nvbk_close_fd(nvbk_header_t* header);
void nvbk_free_header(nvbk_header_t* header);
int nvbk_read_entry(nvbk_header_t* header, int entry);
int nvbk_read_all_entries(nvbk_header_t* header);
int nvbk_update_hash(nvbk_header_t* header, int entry);
int nvbk_write_to_file(nvbk_header_t* header, const char* filepath, bool overwrite);

#endif	//LIBNVBK_H
