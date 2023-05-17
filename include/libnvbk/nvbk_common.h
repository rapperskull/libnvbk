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

#ifndef NVBK_COMMON_H
#define NVBK_COMMON_H

#define NVBK_SECTOR_SIZE    0x200	// 512 bytes
#define NVBK_HEADER_SIZE    0x1C
#define NVBK_VERSION_SIZE   6
#define NVBK_MAGIC          "OEMNVBK"

#define NVBK_TYPE_DYNAMIC   0x00
#define NVBK_TYPE_STATIC    0x01

#define NVBK_ENTRY_SIZE     0x29

#endif	//NVBK_COMMON_H
