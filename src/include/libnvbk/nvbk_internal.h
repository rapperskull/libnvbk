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

#ifndef NVBK_INTERNAL_H
#define NVBK_INTERNAL_H

#include <stdint.h>
#include <libnvbk/nvbk_common.h>

#pragma pack(push, 1)
typedef struct _nvbk_int_header_t { // All fields are Little Endian
  char      magic[8];               // 'OEMNVBK\0'
  uint16_t  version_minor;          // Not sure yet
  uint8_t   version_major;          // Always 0x01
  uint8_t   type;                   // 0x00 = dynamic NVBK, 0x01 = static NVBK
  uint8_t   num_entries;            // Number of entries following this header, can't be 0.
  uint32_t  header_length;          // Always 0x1C (28)
  uint8_t   rf_id_type;             // 0x00 = rfid, 0x01 = unique_rfid, 0x02 = unknown
  uint8_t   year;                   // 2-digit year
  uint8_t   month;
  uint8_t   day;
  uint8_t   revision;               // Used in dynamic NVBK. Probably a check against downgrade, but not sure.
  char      nv_image_version[NVBK_VERSION_SIZE];  // ASCII sequence (NULL terminated only if shorter than NVBK_VERSION_SIZE), can be empty.
} nvbk_int_header_t;
#pragma pack(pop)

#pragma pack(push, 1)
typedef struct _nvbk_int_header_entry_t { // All fields are Little Endian
  uint32_t  revision;     // Changes when the entry changes
  uint16_t  sector_num;   // Sector at which the entry is located, can't be 0.
  uint16_t  sectors_size; // Size of the entry in sectors
  uint8_t   sha256[32];   // Hash calculated over the entire entry
  uint8_t   rf_id;        // This ID is used by software to identify which entries it's interested in.
} nvbk_int_header_entry_t;
#pragma pack(pop)

/*

// Here I was trying to document the format of the TLV entries

struct nvbk_entry {             // All fields are Little Endian
    uint32_t    entry_size;     // Total entry size including this field
    uint8_t     flags;          // See below
    uint8_t     type;           // See below
    uint8_t     unk;            // Always 0xFF or 0x6D
    uint8_t     unk2;           // 0x18 for static NVBK, 0x00 for virgin dynamic NVBK, 0x10 for written dynamic NVBK, also 0x08 seen on dynamic NVBK
    uint8_t     NVItemID[2];    // Only meaningful in case of NVItem
};

If the entry has a name, it is followed by the following struct :
struct nvbk_filename_part {
    uint16_t	filename_size;				// Filename size including terminating \0
    uint8_t		filename[filename_size];	// The filename itself
    uint16_t	unk;						// Always 0x02 0x00
};

In any case, it's followed by type-specific struct
struct nvbk_file_entry {
    uint16_t	file_size;
    uint8_t		file_specific_data[file_size];	// Depends on flags
};

struct nvbk_big_file_entry {
    uint32_t	file_size;
    uint32_t	file_size_copy;					// Do not trust
    uint8_t		file_specific_data[file_size];	// Depends on flags (maybe)
};

union file_specific_data {
    uint8_t		file_data[file_size];		// Actual file data
    struct {
        uint8_t	flags;						// Always 0x07
        uint8_t	file_data[file_size - 1];	// Actual file data
    }
}

// Flags can be:
// 0x01 = no name
// 0x02 = has name
// 0xF1 = no name, same as normal file (maybe bigger than 1 sector)
// 0xF2 = has name, same as normal file (maybe bigger than 1 sector)
// 0xF3 = no name, file size is repeated and is uint32_t
// 0xF4 = has name, file size is repeated and is uint32_t

// Type can be:
// 0x09 (0000 1001) = File
// 0x0D (0000 1101) = ItemFile
// 0x19 (0001 1001) = ItemFile with flags
// 0x39 (0011 1001) = NVItem
// 0x40 (0100 0000) = Hidden/Deleted (?) without flag

*/

#endif	//NVBK_INTERNAL_H
