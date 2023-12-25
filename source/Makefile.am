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
AM_CFLAGS += $(SYSTEMD_CFLAGS)

AM_CFLAGS += -D_GNU_SOURCE
AM_CFLAGS += -Wall -Wextra -Werror

bin_PROGRAMS = em_ctl

em_ctl_CPPFLAGS = -I$(top_srcdir)/source/ssp \
                  -I$(top_srcdir)/source/libplatform/include \
                  -I$(top_srcdir)/source/controller/include \
                  -I=${includedir} \
                  -I=${includedir}/ccsp

em_ctl_SOURCES = cosa_emctl_dml.c \
                 cosa_emctl_apis.c \
                 plugin_main.c \
                 ssp_action.c \
                 ssp_main.c \
                 ssp_mbi.c

em_ctl_LDFLAGS = $(SYSTEMD_LDFLAGS) \
                 $(DBUS_LIBS) \
                 -rdynamic

em_ctl_LDADD = ${top_builddir}/source/controller/src/libcontroller.a \
               ${top_builddir}/source/ieee1905/src/al/libal.a \
               ${top_builddir}/source/ieee1905/src/factory/libfactory.a \
               ${top_builddir}/source/ieee1905/src/common/libcommon.a \
               ${top_builddir}/source/libplatform/src/libutils.a \
               -lccsp_common \
               -lrdkloggers \
               -llog4c \
               -ldbus-1 \
               -lrbus \
               -lubox \
               -ljson-c \
               -lcrypto \
               -lpthread \
               -ldl

