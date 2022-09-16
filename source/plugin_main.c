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

/*********************************************************************** 
  
    module: plugin_main.c

        Implement COSA Data Model Library Init and Unload apis.
 
    ---------------------------------------------------------------

    author:

        COSA XML TOOL CODE GENERATOR 1.0

    ---------------------------------------------------------------

    revision:

        09/28/2011    initial revision.

**********************************************************************/

#include "ansc_platform.h"
#include "ansc_load_library.h"
#include "cosa_plugin_api.h"

#include "plugin_main.h"

#include "cosa_emctl_dml.h"
#include "cosa_emctl_apis.h"

#define THIS_PLUGIN_VERSION                         1

PCOSA_DML_EMCTL_CFG g_pEmctl_Cfg;

int ANSC_EXPORT_API
COSA_Init
    (
        ULONG                       uMaxVersionSupported, 
        void*                       hCosaPlugInfo         /* PCOSA_PLUGIN_INFO passed in by the caller */
    )
{
    PCOSA_PLUGIN_INFO               pPlugInfo  = (PCOSA_PLUGIN_INFO)hCosaPlugInfo;

    if ( uMaxVersionSupported < THIS_PLUGIN_VERSION )
    {
      /* this version is not supported */
        return -1;
    }   
    
    pPlugInfo->uPluginVersion       = THIS_PLUGIN_VERSION;
    /* register the back-end apis for the data model */
    pPlugInfo->RegisterFunction(pPlugInfo->hContext, "EasyMeshController_GetParamUlongValue", EasyMeshController_GetParamUlongValue);
    pPlugInfo->RegisterFunction(pPlugInfo->hContext, "EasyMeshController_SetParamUlongValue", EasyMeshController_SetParamUlongValue);
    pPlugInfo->RegisterFunction(pPlugInfo->hContext, "EasyMeshController_GetParamStringValue", EasyMeshController_GetParamStringValue);
    pPlugInfo->RegisterFunction(pPlugInfo->hContext, "EasyMeshController_SetParamStringValue", EasyMeshController_SetParamStringValue);
	pPlugInfo->RegisterFunction(pPlugInfo->hContext, "EasyMeshController_GetParamBoolValue", EasyMeshController_GetParamBoolValue);
    pPlugInfo->RegisterFunction(pPlugInfo->hContext, "EasyMeshController_SetParamBoolValue", EasyMeshController_SetParamBoolValue);
	pPlugInfo->RegisterFunction(pPlugInfo->hContext, "EasyMeshController_GetParamIntValue", EasyMeshController_GetParamIntValue);
    pPlugInfo->RegisterFunction(pPlugInfo->hContext, "EasyMeshController_SetParamIntValue", EasyMeshController_SetParamIntValue);
    pPlugInfo->RegisterFunction(pPlugInfo->hContext, "EasyMeshController_Validate", EasyMeshController_Validate);
    pPlugInfo->RegisterFunction(pPlugInfo->hContext, "EasyMeshController_Commit", EasyMeshController_Commit);
    pPlugInfo->RegisterFunction(pPlugInfo->hContext, "EasyMeshController_Rollback", EasyMeshController_Rollback);

    pPlugInfo->RegisterFunction(pPlugInfo->hContext, "ChanSel_GetParamStringValue", ChanSel_GetParamStringValue);
    pPlugInfo->RegisterFunction(pPlugInfo->hContext, "ChanSel_SetParamStringValue", ChanSel_SetParamStringValue);

    pPlugInfo->RegisterFunction(pPlugInfo->hContext, "SSIDProfile_GetEntryCount", SSIDProfile_GetEntryCount);
    pPlugInfo->RegisterFunction(pPlugInfo->hContext, "SSIDProfile_GetEntry", SSIDProfile_GetEntry);
    pPlugInfo->RegisterFunction(pPlugInfo->hContext, "SSIDProfile_IsUpdated", SSIDProfile_IsUpdated);
    pPlugInfo->RegisterFunction(pPlugInfo->hContext, "SSIDProfile_Synchronize", SSIDProfile_Synchronize);
    pPlugInfo->RegisterFunction(pPlugInfo->hContext, "SSIDProfile_GetParamBoolValue", SSIDProfile_GetParamBoolValue);
    pPlugInfo->RegisterFunction(pPlugInfo->hContext, "SSIDProfile_SetParamBoolValue", SSIDProfile_SetParamBoolValue);
    pPlugInfo->RegisterFunction(pPlugInfo->hContext, "SSIDProfile_GetParamStringValue", SSIDProfile_GetParamStringValue);
    pPlugInfo->RegisterFunction(pPlugInfo->hContext, "SSIDProfile_SetParamStringValue", SSIDProfile_SetParamStringValue);
    pPlugInfo->RegisterFunction(pPlugInfo->hContext, "SSIDProfile_GetParamIntValue", SSIDProfile_GetParamIntValue);
    pPlugInfo->RegisterFunction(pPlugInfo->hContext, "SSIDProfile_SetParamIntValue", SSIDProfile_SetParamIntValue);
    pPlugInfo->RegisterFunction(pPlugInfo->hContext, "SSIDProfile_Validate", SSIDProfile_Validate);
    pPlugInfo->RegisterFunction(pPlugInfo->hContext, "SSIDProfile_Commit", SSIDProfile_Commit);
    pPlugInfo->RegisterFunction(pPlugInfo->hContext, "SSIDProfile_Rollback", SSIDProfile_Rollback);

    g_pEmctl_Cfg = (PCOSA_DML_EMCTL_CFG)CosaEmctlCreate();
    CosaEmctlInitialize(g_pEmctl_Cfg);

    return  0;
}

BOOL ANSC_EXPORT_API
COSA_IsObjectSupported
    (
        char*                        pObjName
    )
{
    UNREFERENCED_PARAMETER(pObjName);

    return TRUE;
}

void ANSC_EXPORT_API
COSA_Unload
    (
        void
    )
{
    /* unload the memory here */
}
