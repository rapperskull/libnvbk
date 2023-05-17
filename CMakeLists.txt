cmake_minimum_required(VERSION 3.12)
project(libnvbk
  VERSION "0.0.1"
  DESCRIPTION "NVBK library"
  LANGUAGES C
)

# https://stackoverflow.com/posts/74664306/revisions
include(CMakeDependentOption)
cmake_dependent_option(
  LIBNVBK_STATIC            # option variable
  "Build static library"    # description
  ON                        # default value if exposed; user can override
  "NOT BUILD_SHARED_LIBS"   # condition to expose option
  OFF                       # value if not exposed; user can't override
)

add_subdirectory(mbedtls EXCLUDE_FROM_ALL)

set(sources_dir ${CMAKE_CURRENT_SOURCE_DIR}/src)
set(LIBNVBK_SRCS
    ${sources_dir}/libnvbk.c
    ${sources_dir}/nvbk_error.c
)
set(headers_dir ${CMAKE_CURRENT_SOURCE_DIR}/include)
set(LIBNVBK_PUBLIC_HDRS
    ${headers_dir}/libnvbk/libnvbk.h
    ${headers_dir}/libnvbk/nvbk_error.h
    ${headers_dir}/panzi/portable_endian.h
)
set(LIBNVBK_PRIVATE_HDRS
    ${sources_dir}/include/libnvbk/nvbk_internal.h
)

if (LIBNVBK_STATIC)
  add_library(nvbk STATIC ${LIBNVBK_SRCS})
else ()
  add_library(nvbk SHARED ${LIBNVBK_SRCS})
  set_target_properties(nvbk PROPERTIES POSITION_INDEPENDENT_CODE 1)
  set_target_properties(nvbk PROPERTIES SOVERSION 1.0)
endif ()

target_sources(nvbk PUBLIC ${LIBNVBK_PUBLIC_HDRS})
target_sources(nvbk PRIVATE ${LIBNVBK_PRIVATE_HDRS})
target_include_directories(nvbk PUBLIC ${headers_dir})
target_include_directories(nvbk PRIVATE ${sources_dir}/include)
set_property(TARGET nvbk PROPERTY C_STANDARD 99)
set_property(TARGET nvbk PROPERTY C_EXTENSIONS ON)
if(MSVC)
    target_compile_options(nvbk PUBLIC /W4)
else()
    target_compile_options(nvbk PUBLIC -Wall PUBLIC -Wextra PUBLIC -Wpedantic)
endif()
target_link_libraries(nvbk PUBLIC MbedTLS::mbedcrypto)
