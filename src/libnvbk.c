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

#if defined(__linux__)
  #define _LARGEFILE_SOURCE
  #define	_FILE_OFFSET_BITS 64
  #define	_TIME_BITS        64
#endif

#if defined(_MSC_VER)
  #define _CRT_NONSTDC_NO_WARNINGS  // POSIX function names
  #define _CRT_SECURE_NO_WARNINGS   // Unsafe CRT Library functions
  #include <io.h>
  #include <BaseTsd.h>
  typedef SSIZE_T ssize_t;
#else
  #include <unistd.h>
#endif

#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <fcntl.h>
#include <string.h>
#include <errno.h>
#include <panzi/portable_endian.h>
#include <libnvbk/nvbk_internal.h>
#include <libnvbk/nvbk_error.h>
#include <libnvbk/libnvbk.h>
#include <mbedtls/sha256.h>

#if defined(_WIN32) || defined(__CYGWIN__)
    #define READFLAGS       O_RDONLY | O_BINARY
    #define WRITEFLAGS      O_WRONLY | O_CREAT | O_BINARY
    #define READWRITEFLAGS  O_RDWR   | O_BINARY
#else
    #define READFLAGS       O_RDONLY
    #define WRITEFLAGS      O_WRONLY | O_CREAT
    #define READWRITEFLAGS  O_RDWR
#endif

#define round_to_sector(n)  ( (n) / NVBK_SECTOR_SIZE + ( (n) % NVBK_SECTOR_SIZE ? 1 : 0 ) )

#define abort_open(code) do {                   \
  int saved_errno = errno;                      \
  if(fd != -1) close(fd);                       \
  if(int_header) free(int_header);              \
  if(header) {                                  \
    if(header->entries) free(header->entries);  \
    free(header);                               \
  }                                             \
  *error = code;                                \
  errno = saved_errno;                          \
  return NULL;                                  \
} while(0)

#define abort_write_to_file(code) do {          \
  int saved_errno = errno;                      \
  if(int_header) free(int_header);              \
  if(fd != -1) close(fd);                       \
  if(filepath) unlink(filepath);                \
  errno = saved_errno;                          \
  return code;                                  \
} while(0)

ssize_t read_all(int fd, unsigned char* buf, size_t count) {
  size_t left = count;
  ssize_t readBytes;
  do {
    readBytes = read(fd, buf, left);
    if (readBytes < 0) return readBytes;
    if (readBytes == 0) return count - left;
    left -= readBytes;
    buf += readBytes;
  } while (left > 0);
  return count;
}

ssize_t write_all(int fd, const unsigned char* buf, size_t count){
  size_t left = count;
  ssize_t writeBytes;
  do {
    writeBytes = write(fd, buf, left);
    if (writeBytes < 0) return writeBytes;
    if (writeBytes == 0) return count - left;
    left -= writeBytes;
    buf += writeBytes;
  } while(left > 0);
  return count;
}

void nvbk_free_header(nvbk_header_t* header) {
  if(header) {
    if(header->entries) {
      for (int i = 0; i < header->num_entries; i++) {
        if(header->entries[i].entry_data) free(header->entries[i].entry_data);
      }
      free(header->entries);
    }
    if(header->fd != -1) close(header->fd);
    free(header);
  }
}

nvbk_header_t* nvbk_open(const char* filepath, int *error) {
  ssize_t readBytes;
  int fd = -1;
  nvbk_int_header_t* int_header = NULL;
  nvbk_header_t* header = NULL;
  uint32_t header_length;
  
  if (!error) return NULL;
  
  *error = NVBK_ERR_OK;

  if (!filepath) abort_open(NVBK_INVALID_PARAM);
  if ((fd = open(filepath, READFLAGS)) == -1) abort_open(NVBK_SEE_ERRNO);
  
  if((int_header = (nvbk_int_header_t*)malloc(NVBK_SECTOR_SIZE)) == NULL) abort_open(NVBK_SEE_ERRNO);

  readBytes = read_all(fd, (unsigned char*)int_header, NVBK_SECTOR_SIZE);
  if (readBytes < 0) abort_open(NVBK_SEE_ERRNO);
  if (readBytes != NVBK_SECTOR_SIZE) abort_open(NVBK_READ_END);

  if (strcmp(int_header->magic, NVBK_MAGIC)) abort_open(NVBK_INVALID_MAGIC);
  if (int_header->num_entries == 0) abort_open(NVBK_INVALID_ENTRIES_COUNT);
  
  header_length = le32toh(int_header->header_length);
  if (header_length != NVBK_HEADER_SIZE) abort_open(NVBK_INVALID_HEADER_SIZE);
  
  if((header = (nvbk_header_t*)malloc(sizeof(nvbk_header_t))) == NULL) abort_open(NVBK_SEE_ERRNO);
  
  header->num_entries = int_header->num_entries;
  header->type = int_header->type;
  header->rf_id_type = int_header->rf_id_type;
  header->version_minor = le16toh(int_header->version_minor);
  header->date.year = 2000 + int_header->year;
  header->date.month = int_header->month;
  header->date.day = int_header->day;
  header->revision = int_header->revision;
  memcpy(&header->nv_image_version, &int_header->nv_image_version, NVBK_VERSION_SIZE);
  header->nv_image_version[NVBK_VERSION_SIZE] = '\0';
  header->version_major = int_header->version_major;
  memcpy(&header->magic, &int_header->magic, 8);
  header->header_length = header_length;
  header->size = NVBK_SECTOR_SIZE * round_to_sector(header->header_length + header->num_entries * NVBK_ENTRY_SIZE);
  
  if (header->size > NVBK_SECTOR_SIZE) { // Header spans more than one sector
    void* temp = realloc(int_header, header->size);  // Reallocate to accomodate entire header
    if (temp == NULL) abort_open(NVBK_SEE_ERRNO);
    int_header = (nvbk_int_header_t*)temp;
    readBytes = read_all(fd, (unsigned char *)int_header + NVBK_SECTOR_SIZE, header->size - NVBK_SECTOR_SIZE);  // Read the rest of the header
    if (readBytes < 0) abort_open(NVBK_SEE_ERRNO);
    if ((size_t)readBytes != header->size - NVBK_SECTOR_SIZE) abort_open(NVBK_READ_END);
  }
  
  if((header->entries = (nvbk_header_entry_t*)malloc(header->num_entries * sizeof(nvbk_header_entry_t))) == NULL) abort_open(NVBK_SEE_ERRNO);
  
  nvbk_int_header_entry_t* entries = (nvbk_int_header_entry_t*)((char *)int_header + header->header_length);
  for(int i = 0; i < header->num_entries; i++) {
    /*
    * We copy all entries, even if an entry with sector_num = 0 is invalid.
    * This allows us to keep track of them and produce an identical file.
    */
    header->entries[i].revision = le32toh(entries[i].revision);
    header->entries[i].pos = le16toh(entries[i].sector_num) * NVBK_SECTOR_SIZE;
    header->entries[i].size = le16toh(entries[i].sectors_size) * NVBK_SECTOR_SIZE;
    memcpy(&header->entries[i].sha256, &entries[i].sha256, 32);
    header->entries[i].rf_id = entries[i].rf_id;
    header->entries[i].entry_data = NULL; // Initialized on demand
  }
  
  header->fd = fd;
  
  free(int_header);
  
  return header;
  
}

int nvbk_close_fd(nvbk_header_t* header) {
  int ret;
  if (!header) return NVBK_INVALID_PARAM;
  if (header->fd == -1) return NVBK_INVALID_FD;

  ret = close(header->fd);
  header->fd = -1;
  return ret ? NVBK_SEE_ERRNO : NVBK_ERR_OK;
}

int nvbk_read_entry(nvbk_header_t* header, int entry) {
  ssize_t readBytes;
  
  if (!header) return NVBK_INVALID_PARAM;
  if (entry >= header->num_entries) return NVBK_INVALID_PARAM;
  if (header->entries[entry].entry_data != NULL) return NVBK_PTR_NOT_NULL;
  if (header->fd == -1) return NVBK_INVALID_FD;
  
  if ((header->entries[entry].entry_data = (unsigned char*)malloc(header->entries[entry].size)) == NULL) return NVBK_SEE_ERRNO;
  if (lseek(header->fd, header->entries[entry].pos, SEEK_SET) == -1) {
    free(header->entries[entry].entry_data);
    header->entries[entry].entry_data = NULL;
    return NVBK_SEE_ERRNO;
  }
  
  readBytes = read_all(header->fd, header->entries[entry].entry_data, header->entries[entry].size);
  if (readBytes < 0 || (size_t)readBytes != header->entries[entry].size) {
    free(header->entries[entry].entry_data);
    header->entries[entry].entry_data = NULL;
    return readBytes < 0 ? NVBK_SEE_ERRNO : NVBK_READ_END;
  }
  
  return NVBK_ERR_OK;
}

int nvbk_read_all_entries(nvbk_header_t* header) {
  int i, err;
  if (!header) return NVBK_INVALID_PARAM;
  for (i = 0; i < header->num_entries; i++) {
    if (header->entries[i].entry_data == NULL) {
      err = nvbk_read_entry(header, i);
      if (err != NVBK_ERR_OK) return err;
    }
  }
  return NVBK_ERR_OK;
}

int nvbk_update_hash(nvbk_header_t* header, int entry) {
  int ret;
  if (!header) return NVBK_INVALID_PARAM;
  if (entry >= header->num_entries) return NVBK_INVALID_PARAM;
  if (header->entries[entry].entry_data == NULL) return NVBK_ENTRY_NOT_LOADED;
  
  ret = mbedtls_sha256(header->entries[entry].entry_data, header->entries[entry].size, header->entries[entry].sha256, 0);
  if (ret) return NVBK_SHA256_ERROR;
  
  return NVBK_ERR_OK;
}

int nvbk_write_to_file(nvbk_header_t* header, const char* filepath, bool overwrite) {
  int fd = -1, ret;
  ssize_t writeBytes;
  nvbk_int_header_t* int_header = NULL;

  if (!header || !filepath) return NVBK_INVALID_PARAM;

  for (int i = 0; i < header->num_entries; i++) {
    /* If we have a fd, we can still process the missing entries. */
    if (header->entries[i].entry_data == NULL && header->fd == -1) return NVBK_INVALID_FD;
  }

  if ((fd = open(filepath, WRITEFLAGS | (overwrite ? O_TRUNC : O_EXCL), 0664)) == -1) return NVBK_SEE_ERRNO;

  if ((int_header = (nvbk_int_header_t*)calloc(1, header->size)) == NULL) abort_write_to_file(NVBK_SEE_ERRNO);

  memcpy(&int_header->magic, &header->magic, 8);
  int_header->version_minor = htole16((uint16_t)header->version_minor);
  int_header->version_major = header->version_major;
  int_header->type = header->type;
  int_header->num_entries = header->num_entries;
  int_header->header_length = htole32((uint32_t)header->header_length);
  int_header->rf_id_type = header->rf_id_type;
  int_header->year = (uint8_t)(header->date.year - 2000);
  int_header->month = header->date.month;
  int_header->day = header->date.day;
  int_header->revision = header->revision;
  memcpy(&int_header->nv_image_version, &header->nv_image_version, NVBK_VERSION_SIZE);

  nvbk_int_header_entry_t* entries = (nvbk_int_header_entry_t*)((char*)int_header + header->header_length);
  for (int i = 0; i < header->num_entries; i++) {
    entries[i].revision = htole32((uint32_t)header->entries[i].revision);
    entries[i].sector_num = htole16((uint16_t)round_to_sector(header->entries[i].pos));
    entries[i].sectors_size = htole16((uint16_t)round_to_sector(header->entries[i].size));
    memcpy(&entries[i].sha256, &header->entries[i].sha256, 32);
    entries[i].rf_id = header->entries[i].rf_id;
  }

  writeBytes = write_all(fd, (unsigned char*)int_header, header->size);
  if (writeBytes < 0) abort_write_to_file(NVBK_SEE_ERRNO);
  if ((size_t)writeBytes != header->size) abort_write_to_file(NVBK_WRITE_END);

  for (int i = 0; i < header->num_entries; i++) {

    bool new_load = header->entries[i].entry_data == NULL;
    if (new_load) {
      ret = nvbk_read_entry(header, i);
      if (ret) abort_write_to_file(ret);
    }

    if (lseek(fd, header->entries[i].pos, SEEK_SET) == -1) {
      if (new_load) {
        free(header->entries[i].entry_data);
        header->entries[i].entry_data = NULL;
      }
      abort_write_to_file(NVBK_SEE_ERRNO);
    }
    writeBytes = write_all(fd, header->entries[i].entry_data, header->entries[i].size);
    if (writeBytes < 0 || (size_t)writeBytes != header->entries[i].size) {
      if (new_load) {
        free(header->entries[i].entry_data);
        header->entries[i].entry_data = NULL;
      }
      abort_write_to_file(writeBytes < 0 ? NVBK_SEE_ERRNO : NVBK_WRITE_END);
    }

    if (header->entries[i].size % NVBK_SECTOR_SIZE) {
      /* Fill the sector with zeroes. */
      if (lseek(fd, header->entries[i].pos + header->entries[i].size - 1, SEEK_SET) == -1) {
        if (new_load) {
          free(header->entries[i].entry_data);
          header->entries[i].entry_data = NULL;
        }
        abort_write_to_file(NVBK_SEE_ERRNO);
      }
      char b = 0;
      writeBytes = write(fd, &b, 1);
      if (writeBytes < 0 || writeBytes != 1) {
        if (new_load) {
          free(header->entries[i].entry_data);
          header->entries[i].entry_data = NULL;
        }
        abort_write_to_file(writeBytes < 0 ? NVBK_SEE_ERRNO : NVBK_WRITE_END);
      }
    }

    if (new_load) {
      free(header->entries[i].entry_data);
      header->entries[i].entry_data = NULL;
    }

  }

  ret = close(fd);
  free(int_header);

  return ret ? NVBK_SEE_ERRNO : NVBK_ERR_OK;
}
