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

AM_CFLAGS = -D_ANSC_LINUX
AM_CFLAGS += -D_ANSC_USER
AM_CFLAGS += -D_ANSC_LITTLE_ENDIAN_
AM_CFLAGS += $(DBUS_CFLAGS)

AM_CFLAGS += -D_GNU_SOURCE
AM_CFLAGS += -Wall -Wextra -Werror

ACLOCAL_AMFLAGS = -I m4

noinst_LIBRARIES = libssp.a

libssp_a_CPPFLAGS = -I$(top_srcdir)/source/ssp \
                    -I=${includedir} \
                    -I=${includedir}/ccsp

libssp_a_SOURCES = $(srcdir)/collection.c \
                   $(srcdir)/cosa_emctl_dml.c \
                   $(srcdir)/cosa_emctl_apis.c \
                   $(srcdir)/plugin_main.c \
                   $(srcdir)/ssp_action.c \
                   $(srcdir)/ssp_main.c \
                   $(srcdir)/ssp_mbi.c

libssp_a_LDFLAGS = -lsyscfg -lsysevent -lpthread -lccsp_common -llog4c
libssp_a_LDFLAGS += $(DBUS_LIBS)
