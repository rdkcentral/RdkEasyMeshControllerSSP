/*
 * If not stated otherwise in this file or this component's LICENSE file the
 * following copyright and licenses apply:
 *
 * Copyright (c) 2021-2022 AirTies Wireless Networks
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 * Copyright (c) 2015 RDK Management
 * Licensed under the Apache License, Version 2.0
 *
 * Copyright (c) 2014 Cisco Systems, Inc.
 * Licensed under the Apache License, Version 2.0
*/

/**************************************************************************

    module: cosa_emctl_dml.h

        For COSA Easymesh Controller Data Model Library Development

    -------------------------------------------------------------------

    description:

        This file defines the apis for objects to support Data Model Library.

    -------------------------------------------------------------------


    author:

        Engin Akca

    -------------------------------------------------------------------

    revision:

        17/01/2022    initial revision.

**************************************************************************/


#ifndef  _COSA_EMCTL_APIS_H
#define  _COSA_EMCTL_APIS_H

#include <ansc_platform.h>

#include "map_config.h"
#include "arraylist.h"

typedef struct map_emctl_ssp_s {
    map_cfg_t       *cfg;

    array_list_t    *modified_profiles;

    unsigned int     updating;
    array_list_t    *updates;
} map_emctl_ssp_t;

ANSC_HANDLE CosaEmctlCreate(void);
ANSC_STATUS CosaEmctlInitialize(ANSC_HANDLE hThisObject);
ANSC_STATUS CosaEmctlDestroy(ANSC_HANDLE hThisObject);

int map_apply_wifi_change(char *context);

#endif
