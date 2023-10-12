##########################################################################
# If not stated otherwise in this file or this component's LICENSE
# file the following copyright and licenses apply:
#
# Copyright (c) 2021-2022 AirTies Wireless Networks
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
# http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
#
# Copyright (c) 2015 RDK Management
# Licensed under the Apache License, Version 2.0
#
# Copyright (c) 2014 Cisco Systems, Inc.
# Licensed under the Apache License, Version 2.0
##########################################################################

project(sspmain
    DESCRIPTION "sspmain"
    LANGUAGES C
)

set(SSP ssp)
set(SSPSTATIC ssp-static)

set(SSP_SOURCES
    cosa_emctl_apis.c
    cosa_emctl_dml.c
    plugin_main.c
    ssp_action.c
    ssp_main.c
    ssp_mbi.c
)

foreach(src_file ${SSP_SOURCES})
    list(APPEND SOURCES ${PROJECT_SOURCE_DIR}/${src_file})
endforeach()

set(SSP_INCLUDES PUBLIC
    ${CMAKE_SOURCE_DIR}/source/libplatform/include
    ${CMAKE_SOURCE_DIR}/source/controller/include
    PUBLIC ${LIBUBOX_INCLUDE_DIRS}
    PUBLIC ${BSP_SYSROOTS_DIR}/usr/include/ccsp
    PUBLIC ${BSP_SYSROOTS_DIR}/usr/include/dbus-1.0
    PUBLIC ${BSP_SYSROOTS_DIR}/usr/lib/dbus-1.0/include
)

set(SSP_COMPILE_OPTIONS
    -ffunction-sections
    -fdata-sections
    -fvisibility=hidden
    -D_ANSC_LINUX
    -D_ANSC_USER
    -D_ANSC_LITTLE_ENDIAN_
#    DBUS_CFLAGS
)

set(SSP_LIBRARIES
    -Wl,--gc-sections
    -lpthread
    -lccsp_common
    -ldbus-1
#   DBUS_LIBS
# uncomment if you want to see which symbols are GC'd
#    -Wl,--print-gc-sections
)

# Library objects
add_library(${SSP}-objs OBJECT ${SOURCES})
target_compile_definitions(${SSP}-objs PRIVATE
    _GNU_SOURCE
)
target_compile_options(${SSP}-objs PRIVATE ${SSP_COMPILE_OPTIONS})
target_include_directories(${SSP}-objs ${SSP_INCLUDES})

# Dynamic library
#add_library(${SSP} SHARED $<TARGET_OBJECTS:${SSP}-objs>)
#target_link_libraries(${SSP} ${SSP_LIBRARIES})

# Static library
add_library(${SSPSTATIC} STATIC $<TARGET_OBJECTS:${SSP}-objs>)
set_target_properties(${SSPSTATIC} PROPERTIES OUTPUT_NAME ${SSP})
target_link_libraries(${SSPSTATIC} ${SSP_LIBRARIES})

# Install header file(s)
#install(
#    DIRECTORY ${PROJECT_SOURCE_DIR}/include
#    DESTINATION include
#)

# Install library
#install(TARGETS ${LIBSTATIC}
#    RUNTIME DESTINATION bin
#    LIBRARY DESTINATION lib
#    ARCHIVE DESTINATION lib
#)
