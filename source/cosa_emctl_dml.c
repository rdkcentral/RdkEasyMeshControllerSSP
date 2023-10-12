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

#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>

#include <ansc_platform.h>

#include "map_utils.h"
#include "map_config.h"
#include "cosa_emctl_dml.h"
#include "cosa_emctl_apis.h"

extern map_emctl_ssp_t *g_emctl_ssp;
static BOOL bValueChanged = FALSE;

/**********************************************************************

 APIs for Object:

    EasyMeshController.

    *  EasyMeshController_GetParamStringValue
    *  EasyMeshController_GetParamIntValue
    *  EasyMeshController_SetParamStringValue
    *  EasyMeshController_SetParamIntValue
    *  EasyMeshController_Validate
    *  EasyMeshController_Commit
    *  EasyMeshController_Rollback

**********************************************************************/
/**********************************************************************

    caller:     owner of this object

    prototype:

        BOOL
        EasyMeshController_GetParamBoolValue
            (
                ANSC_HANDLE                 hInsContext,
                char*                       pParamName,
                BOOL*                       pBool
            );

    description:

        This function is called to retrieve Boolean parameter value;

    argument:   ANSC_HANDLE                 hInsContext,
                The instance handle;

                char*                       pParamName,
                The parameter name;

                BOOL*                       pBool
                The buffer of returned boolean value;

    return:     TRUE if succeeded.

**********************************************************************/
BOOL
EasyMeshController_GetParamBoolValue
    (
        ANSC_HANDLE                 hInsContext,
        char*                       pParamName,
        BOOL*                       pBool
    )
{
    UNREFERENCED_PARAMETER(hInsContext);
    map_cfg_t *cfg = g_emctl_ssp->cfg;

    if (AnscEqualString(pParamName, "Enable", TRUE)) {
        *pBool = cfg->enabled;
        return TRUE;
    }

    return FALSE;
}

/**********************************************************************

    caller:     owner of this object

    prototype: 

        ULONG
        EasyMeshController_GetParamStringValue
            (
                ANSC_HANDLE                 hInsContext,
                char*                       pParamName,
                char*                       pValue,
                ULONG*                      pulSize
            );

    description:

        This function is called to retrieve string parameter value;

    argument:   ANSC_HANDLE                 hInsContext,
                The instance handle;

                char*                       pParamName,
                The parameter name;

                char*                       pValue,
                The string value buffer;

                ULONG*                      pulSize
                The buffer of length of string value;
                Usually size of 1023 will be used.
                If it's not big enough, put required size here and return 1;

    return:     0 if succeeded;
                1 if short of buffer size; (*pulSize = required size)
                -1 if not supported.

**********************************************************************/
ULONG
EasyMeshController_GetParamStringValue
    (
        ANSC_HANDLE                 hInsContext,
        char*                       pParamName,
        char*                       pValue,
        ULONG*                      pulSize
    )
{
    UNREFERENCED_PARAMETER(hInsContext);
    map_cfg_t *cfg = g_emctl_ssp->cfg;
    map_controller_cfg_t *ctrl_cfg = &cfg->controller_cfg;
    mac_addr_str mac_str;

    if (AnscEqualString(pParamName, "InterfaceList", TRUE)) {
        if (AnscSizeOfString(cfg->interfaces) < *pulSize) {
            AnscCopyString(pValue, cfg->interfaces);
            return 0;
        } else {
            *pulSize = AnscSizeOfString(cfg->interfaces) + 1;
            return 1;
        }
    }
    if (AnscEqualString(pParamName, "PrimaryVLANInterfacePattern", TRUE)) {
        if (AnscSizeOfString(cfg->primary_vlan_pattern) < *pulSize) {
            AnscCopyString(pValue, cfg->primary_vlan_pattern);
            return 0;
        } else {
            *pulSize = AnscSizeOfString(cfg->primary_vlan_pattern) + 1;
            return 1;
        }
    }
    if (AnscEqualString(pParamName, "MACAddress", TRUE)) {
        mac_to_string(ctrl_cfg->al_mac, mac_str);
        if (AnscSizeOfString(mac_str) < *pulSize) {
            AnscCopyString(pValue, mac_str);
            return 0;
        } else {
            *pulSize = AnscSizeOfString(mac_str) + 1;
            return 1;
        }
    }
    if (AnscEqualString(pParamName, "LocalAgentMACAddress", TRUE)) {
        mac_to_string(ctrl_cfg->local_agent_al_mac, mac_str);
        if (AnscSizeOfString(mac_str) < *pulSize) {
            AnscCopyString(pValue, mac_str);
            return 0;
        } else {
            *pulSize = AnscSizeOfString(mac_str) + 1;
            return 1;
        }
    }
    if (AnscEqualString(pParamName, "ProfileConfigChanged", TRUE)) {
        AnscCopyString(pValue, "None");
        return 0;
    }

    return -1;
}

/**********************************************************************

    caller:     owner of this object

    prototype: 

        BOOL
        EasyMeshController_GetParamIntValue
            (
                ANSC_HANDLE                 hInsContext,
                char*                       pParamName,
                int*                        pInt
            );

    description:

        This function is called to retrieve integer parameter value; 

    argument:   ANSC_HANDLE                 hInsContext,
                The instance handle;

                char*                       pParamName,
                The parameter name;

                int*                        pInt
                The buffer of returned integer value;

    return:     TRUE if succeeded.

**********************************************************************/
BOOL
EasyMeshController_GetParamIntValue
    (
        ANSC_HANDLE                  hInsContext,
        char*                        pParamName,
        int*                         pInt
    )
{
    UNREFERENCED_PARAMETER(hInsContext);
    map_cfg_t *cfg = g_emctl_ssp->cfg;

    if (AnscEqualString(pParamName, "PrimaryVLANID", TRUE)) {
        *pInt = cfg->primary_vlan_id;
        return TRUE;
    }

    return FALSE;
}

/**********************************************************************

    caller:     owner of this object

    prototype:

        BOOL
        EasyMeshController_SetParamBoolValue
            (
                ANSC_HANDLE                 hInsContext,
                char*                       pParamName,
                BOOL                        bValue
            );

    description:

        This function is called to set BOOL parameter value;

    argument:   ANSC_HANDLE                 hInsContext,
                The instance handle;

                char*                       pParamName,
                The parameter name;

                BOOL                        bValue
                The updated BOOL value;

    return:     TRUE if succeeded.

**********************************************************************/
BOOL
EasyMeshController_SetParamBoolValue
    (
        ANSC_HANDLE                 hInsContext,
        char*                       pParamName,
        BOOL                        bValue
    )
{
    UNREFERENCED_PARAMETER(hInsContext);
    map_cfg_t *cfg = g_emctl_ssp->cfg;

    if (AnscEqualString(pParamName, "Enable", TRUE)) {
        if (cfg->enabled == bValue) {
            return TRUE;
        }
        cfg->enabled = TRUE;
        return TRUE;
    }

    return FALSE;
}

/**********************************************************************

    caller:     owner of this object

    prototype: 

       BOOL
       EasyMeshController_SetParamStringValue
            (
                ANSC_HANDLE                 hInsContext,
                char*                       pParamArray,
                char*                       pString,
            );

    description:

        This function is called to set string parameter values;

    argument:   ANSC_HANDLE                 hInsContext,
                The instance handle;

                char*                       pParamName,
                The parameter name array;

                char*                       pString,
                The size of the array;

    return:     TRUE if succeeded.

**********************************************************************/
BOOL
EasyMeshController_SetParamStringValue
    (
        ANSC_HANDLE                 hInsContext,
        char*                       pParamName,
        char*                       pString
    )
{
    UNREFERENCED_PARAMETER(hInsContext);

    if (AnscEqualString(pParamName, "ProfileConfigChanged", TRUE)) {
        if (map_apply_wifi_change(pString) == 0) {
            return TRUE;
        }
    }

    return FALSE;
}

/**********************************************************************

    caller:     owner of this object

    prototype: 

        BOOL
        EasyMeshController_SetParamIntValue
            (
                ANSC_HANDLE                 hInsContext,
                char*                       pParamName,
                int                         iValue
            );

    description:

        This function is called to set integer parameter value; 

    argument:   ANSC_HANDLE                 hInsContext,
                The instance handle;

                char*                       pParamName,
                The parameter name;

                int                         iValue
                The updated integer value;

    return:     TRUE if succeeded.

**********************************************************************/
BOOL
EasyMeshController_SetParamIntValue
    (
        ANSC_HANDLE                 hInsContext,
        char*                       pParamName,
        int                         iValue
    )
{
    UNREFERENCED_PARAMETER(hInsContext);
    map_cfg_t *cfg = g_emctl_ssp->cfg;

    if (AnscEqualString(pParamName, "PrimaryVLANID", TRUE)) {
        if (cfg->primary_vlan_id == iValue) {
            return TRUE;
        }
        cfg->primary_vlan_id = iValue;
        bValueChanged = TRUE;
        return TRUE;
    }

    return FALSE;
}

/**********************************************************************

    caller:     owner of this object

    prototype: 

       BOOL
       EasyMeshController_Validate
            (
                ANSC_HANDLE                 hInsContext,
                char*                       pReturnParamName,
                ULONG*                      puLength
            );

    description:

        This function is called to finally commit all the update.

    argument:   ANSC_HANDLE                 hInsContext,
                The instance handle;

                char*                       pReturnParamName,
                The buffer (128 bytes) of parameter name if there's a validation.

                ULONG*                      puLength
                The output length of the param name.

    return:     TRUE if there's no validation.

**********************************************************************/
BOOL
EasyMeshController_Validate
    (
        ANSC_HANDLE                 hInsContext,
        char*                       pReturnParamName,
        ULONG*                      puLength
    )
{
    UNREFERENCED_PARAMETER(hInsContext);
    UNREFERENCED_PARAMETER(pReturnParamName);
    UNREFERENCED_PARAMETER(puLength);

    return TRUE;
}

/**********************************************************************

    caller:     owner of this object

    prototype:

       ULONG
       EasyMeshController_Commit
            (
                ANSC_HANDLE                 hInsContext
            );

    description:

        This function is called to finally commit all the update.

    argument:   ANSC_HANDLE                 hInsContext,
                The instance handle;

    return:     The status of the operation.

**********************************************************************/
ULONG
EasyMeshController_Commit
    (
        ANSC_HANDLE                 hInsContext
    )
{
    UNREFERENCED_PARAMETER(hInsContext);

    return 0;
}

/**********************************************************************

    caller:     owner of this object

    prototype:

       ULONG
       EasyMeshController_Rollback
            (
                ANSC_HANDLE                 hInsContext
            );

    description:

        This function is called to roll back the update whenever there's a
        validation found.

    argument:   ANSC_HANDLE                 hInsContext,
                The instance handle;

    return:     The status of the operation.

**********************************************************************/
ULONG
EasyMeshController_Rollback
    (
        ANSC_HANDLE                 hInsContext
    )
{
    UNREFERENCED_PARAMETER(hInsContext);

    return 0;
}
