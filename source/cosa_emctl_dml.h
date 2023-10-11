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


#ifndef  _COSA_EMCTL_DML_H
#define  _COSA_EMCTL_DML_H

#include "slap_definitions.h"

/***********************************************************************

 APIs for Object:

    EasyMeshController.

    *  EasyMeshController_GetParamBoolValue
    *  EasyMeshController_GetParamStringValue
    *  EasyMeshController_GetParamIntValue
    *  EasyMeshController_SetParamBoolValue
    *  EasyMeshController_SetParamStringValue
    *  EasyMeshController_SetParamIntValue
    *  EasyMeshController_Validate
    *  EasyMeshController_Commit
    *  EasyMeshController_Rollback

***********************************************************************/
BOOL
EasyMeshController_GetParamBoolValue
    (
        ANSC_HANDLE                 hInsContext,
        char*                       pParamName,
        BOOL*                       pBool
    );

ULONG
EasyMeshController_GetParamStringValue
    (
        ANSC_HANDLE                 hInsContext,
        char*                       pParamName,
        char*                       pValue,
        ULONG*                      pulSize
    );

BOOL
EasyMeshController_GetParamIntValue
    (
        ANSC_HANDLE                 hInsContext,
        char*                       pParamName,
        int*                        pInt
    );

BOOL
EasyMeshController_SetParamBoolValue
    (
        ANSC_HANDLE                 hInsContext,
        char*                       pParamName,
        BOOL                        bValue
    );

BOOL
EasyMeshController_SetParamStringValue
    (
        ANSC_HANDLE                 hInsContext,
        char*                       pParamName,
        char*                       pString
    );

BOOL
EasyMeshController_SetParamIntValue
    (
        ANSC_HANDLE                 hInsContext,
        char*                       pParamName,
        int                         iValue
    );

BOOL
EasyMeshController_Validate
    (
        ANSC_HANDLE                 hInsContext,
        char*                       pReturnParamName,
        ULONG*                      puLength
    );

ULONG
EasyMeshController_Commit
    (
        ANSC_HANDLE                 hInsContext
    );

ULONG
EasyMeshController_Rollback
    (
        ANSC_HANDLE                 hInsContext
    );

#endif
