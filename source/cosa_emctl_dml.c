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

#include "ansc_platform.h"
#include "ccsp_trace.h"
#include "ccsp_syslog.h"

#include "collection.h"
#include "cosa_emctl_dml.h"
#include "cosa_emctl_apis.h"

extern ANSC_HANDLE bus_handle;//lnt
extern char g_Subsystem[32];//lnt
static BOOL bValueChanged = FALSE;
extern PCOSA_DML_EMCTL_CFG g_pEmctl_Cfg;

/**********************************************************************

 APIs for Object:

    EasyMeshController.

    *  EasyMeshController_GetParamUlongValue
    *  EasyMeshController_GetParamBoolValue
    *  EasyMeshController_GetParamStringValue
    *  EasyMeshController_GetParamIntValue
    *  EasyMeshController_SetParamUlongValue
    *  EasyMeshController_SetParamBoolValue
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
        EasyMeshController_GetParamUlongValue
            (
                ANSC_HANDLE                 hInsContext,
                char*                       pParamName,
                ULONG*                      puLong
            );

    description:

        This function is called to retrieve ULONG parameter value;

    argument:   ANSC_HANDLE                 hInsContext,
                The instance handle;

                char*                       pParamName,
                The parameter name;

                ULONG*                      puLong
                The buffer of returned ULONG value;

    return:     TRUE if succeeded.

**********************************************************************/
BOOL
EasyMeshController_GetParamUlongValue
    (
        ANSC_HANDLE                 hInsContext,
        char*                       pParamName,
        ULONG*                      puLong
    )
{
    PCOSA_DML_EMCTL_CFG cfg = g_pEmctl_Cfg;

    if (AnscEqualString(pParamName, "ConfigRenewInterval", TRUE)) {
        *puLong = cfg->ConfigRenewInterval;
        return TRUE;
    }
    if (AnscEqualString(pParamName, "ConfigRenewMaxRetry", TRUE)) {
        *puLong = cfg->ConfigRenewMaxRetry;
        return TRUE;
    }
    if (AnscEqualString(pParamName, "DeadAgentDetectionInterval", TRUE)) {
        *puLong = cfg->DeadAgentDetectionInterval;
        return TRUE;
    }
    if (AnscEqualString(pParamName, "DefaultPCP", TRUE)) {
        *puLong = cfg->DefaultPCP;
        return TRUE;
    }
    if (AnscEqualString(pParamName, "LinkMetricsQueryInterval", TRUE)) {
        *puLong = cfg->LinkMetricsQueryInterval;
        return TRUE;
    }
    if (AnscEqualString(pParamName, "SSIDProfileNumberOfEntries", TRUE)) {
        *puLong = cfg->SSIDProfileNumberOfEntries;
        return TRUE;
    }
    if (AnscEqualString(pParamName, "TopologyDiscoveryInterval", TRUE)) {
        *puLong = cfg->TopologyDiscoveryInterval;
        return TRUE;
    }
    if (AnscEqualString(pParamName, "TopologyQueryInterval", TRUE)) {
        *puLong = cfg->TopologyQueryInterval;
        return TRUE;
    }
    if (AnscEqualString(pParamName, "TopologyStableCheckInterval", TRUE)) {
        *puLong = cfg->TopologyStableCheckInterval;
        return TRUE;
    }

    return FALSE;
}

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
    PCOSA_DML_EMCTL_CFG cfg = g_pEmctl_Cfg;

    if (AnscEqualString(pParamName, "ConfigureBackhaulStation", TRUE)) {
        *pBool = cfg->ConfigureBackhaulStation;
        return TRUE;
    }
    if (AnscEqualString(pParamName, "Enable", TRUE)) {
        *pBool = cfg->Enable;
        return TRUE;
    }
    if (AnscEqualString(pParamName, "IsMaster", TRUE)) {
        *pBool = cfg->IsMaster;
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
    PCOSA_DML_EMCTL_CFG cfg = g_pEmctl_Cfg;

    if (AnscEqualString(pParamName, "InterfaceList", TRUE)) {
        if (AnscSizeOfString(cfg->InterfaceList) < *pulSize) {
            AnscCopyString(pValue, cfg->InterfaceList);
            return 0;
        } else {
            *pulSize = AnscSizeOfString(cfg->InterfaceList) + 1;
            return 1;
        }
    }
    if (AnscEqualString(pParamName, "LocalAgentMACAddress", TRUE)) {
        if (AnscSizeOfString(cfg->LocalAgentMACAddress) < *pulSize) {
            AnscCopyString(pValue, cfg->LocalAgentMACAddress);
            return 0;
        } else {
            *pulSize = AnscSizeOfString(cfg->LocalAgentMACAddress) + 1;
            return 1;
        }
    }
    if (AnscEqualString(pParamName, "MACAddress", TRUE)) {
        if (AnscSizeOfString(cfg->MACAddress) < *pulSize) {
            AnscCopyString(pValue, cfg->MACAddress);
            return 0;
        } else {
            *pulSize = AnscSizeOfString(cfg->MACAddress) + 1;
            return 1;
        }
    }
    if (AnscEqualString(pParamName, "PrimaryVLANInterfacePattern", TRUE)) {
        if (AnscSizeOfString(cfg->PrimaryVLANInterfacePattern) < *pulSize) {
            AnscCopyString(pValue, cfg->PrimaryVLANInterfacePattern);
            return 0;
        } else {
            *pulSize = AnscSizeOfString(cfg->PrimaryVLANInterfacePattern) + 1;
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
         ANSC_HANDLE                 hInsContext,
        char*                        pParamName,
        int*                      	 pInt
    )
{
    PCOSA_DML_EMCTL_CFG cfg = g_pEmctl_Cfg;

    if (AnscEqualString(pParamName, "PrimaryVLANID", TRUE)) {
        *pInt = cfg->PrimaryVLANID;
        return TRUE;
    }

    return FALSE;
}

/**********************************************************************

    caller:     owner of this object

    prototype: 

        BOOL
        EasyMeshController_SetParamUlongValue
            (
                ANSC_HANDLE                 hInsContext,
                char*                       pParamName,
                ULONG                       uValue
            );

    description:

        This function is called to set ULONG parameter value;

    argument:   ANSC_HANDLE                 hInsContext,
                The instance handle;

                char*                       pParamName,
                The parameter name;

                ULONG                       uValue
                The updated ULONG value;

    return:     TRUE if succeeded.

**********************************************************************/
BOOL
EasyMeshController_SetParamUlongValue
    (
        ANSC_HANDLE                 hInsContext,
        char*                       pParamName,
        ULONG                       uValue
    )
{
    PCOSA_DML_EMCTL_CFG cfg = g_pEmctl_Cfg;

    if (AnscEqualString(pParamName, "ConfigRenewInterval", TRUE)) {
        if (cfg->ConfigRenewInterval == uValue) {
            return TRUE;
        }
        cfg->ConfigRenewInterval = uValue;
        bValueChanged = TRUE;
        return TRUE;
    }
    if (AnscEqualString(pParamName, "ConfigRenewMaxRetry", TRUE)) {
        if (cfg->ConfigRenewMaxRetry == uValue) {
            return TRUE;
        }
        cfg->ConfigRenewMaxRetry = uValue;
        bValueChanged = TRUE;
        return TRUE;
    }
    if (AnscEqualString(pParamName, "DeadAgentDetectionInterval", TRUE)) {
        if (cfg->DeadAgentDetectionInterval == uValue) {
            return TRUE;
        }
        cfg->DeadAgentDetectionInterval = uValue;
        bValueChanged = TRUE;
        return TRUE;
    }
    if (AnscEqualString(pParamName, "DefaultPCP", TRUE)) {
        if (cfg->DefaultPCP == uValue) {
            return TRUE;
        }
        cfg->DefaultPCP = uValue;
        bValueChanged = TRUE;
        return TRUE;
    }
    if (AnscEqualString(pParamName, "LinkMetricsQueryInterval", TRUE)) {
        if (cfg->LinkMetricsQueryInterval == uValue) {
            return TRUE;
        }
        cfg->LinkMetricsQueryInterval = uValue;
        bValueChanged = TRUE;
        return TRUE;
    }
    if (AnscEqualString(pParamName, "TopologyDiscoveryInterval", TRUE)) {
        if (cfg->TopologyDiscoveryInterval == uValue) {
            return TRUE;
        }
        cfg->TopologyDiscoveryInterval = uValue;
        bValueChanged = TRUE;
        return TRUE;
    }
    if (AnscEqualString(pParamName, "TopologyQueryInterval", TRUE)) {
        if (cfg->TopologyQueryInterval == uValue) {
            return TRUE;
        }
        cfg->TopologyQueryInterval = uValue;
        bValueChanged = TRUE;
        return TRUE;
    }
    if (AnscEqualString(pParamName, "TopologyStableCheckInterval", TRUE)) {
        if (cfg->TopologyStableCheckInterval == uValue) {
            return TRUE;
        }
        cfg->TopologyStableCheckInterval = uValue;
        bValueChanged = TRUE;
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
    PCOSA_DML_EMCTL_CFG cfg = g_pEmctl_Cfg;

    if (AnscEqualString(pParamName, "InterfaceList", TRUE)) {
        if (AnscEqualString(cfg->InterfaceList, pString, TRUE)) {
            return TRUE;
        }
        AnscCopyString(cfg->InterfaceList, pString);
        bValueChanged = TRUE;
        return TRUE;
    }
    if (AnscEqualString(pParamName, "LocalAgentMACAddress", TRUE)) {
        if (AnscEqualString(cfg->LocalAgentMACAddress, pString, TRUE)) {
            return TRUE;
        }
        AnscCopyString(cfg->LocalAgentMACAddress, pString);
        bValueChanged = TRUE;
        return TRUE;
    }
    if (AnscEqualString(pParamName, "MACAddress", TRUE)) {
        if (AnscEqualString(cfg->MACAddress, pString, TRUE)) {
            return TRUE;
        }
        AnscCopyString(cfg->MACAddress, pString);
        bValueChanged = TRUE;
        return TRUE;
    }
    if (AnscEqualString(pParamName, "PrimaryVLANInterfacePattern", TRUE)) {
        if (AnscEqualString(cfg->PrimaryVLANInterfacePattern, pString, TRUE)) {
            return TRUE;
        }
        AnscCopyString(cfg->PrimaryVLANInterfacePattern, pString);
        bValueChanged = TRUE;
        return TRUE;
    }
    if (AnscEqualString(pParamName, "ProfileConfigChanged", TRUE)) {
        if (EmctlConfigChangeCB(pString) == 0) {
            return TRUE;
        }
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
    PCOSA_DML_EMCTL_CFG cfg = g_pEmctl_Cfg;

    if (AnscEqualString(pParamName, "ConfigureBackhaulStation", TRUE)) {
        if (cfg->ConfigureBackhaulStation == bValue) {
            return TRUE;
        }
        cfg->ConfigureBackhaulStation = TRUE;
        return TRUE;
    }
    if (AnscEqualString(pParamName, "Enable", TRUE)) {
        if (cfg->Enable == bValue) {
            return TRUE;
        }
        cfg->Enable = TRUE;
        return TRUE;
    }
    if (AnscEqualString(pParamName, "IsMaster", TRUE)) {
        if (cfg->IsMaster == bValue) {
            return TRUE;
        }
        cfg->IsMaster = TRUE;
        return TRUE;
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
    PCOSA_DML_EMCTL_CFG cfg = g_pEmctl_Cfg;

    if (AnscEqualString(pParamName, "PrimaryVLANID", TRUE)) {
        if (cfg->PrimaryVLANID == iValue) {
            return TRUE;
        }
        cfg->PrimaryVLANID = iValue;
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
    return 0;
}

/**********************************************************************

 APIs for Object:

    EasyMeshController.ChanSel.

    *  ChanSel_GetParamBoolValue
    *  ChanSel_GetParamStringValue

**********************************************************************/
/**********************************************************************

    caller:     owner of this object

    prototype:

        ULONG
        ChanSel_GetParamStringValue
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
ChanSel_GetParamStringValue
    (
        ANSC_HANDLE                 hInsContext,
        char*                       pParamName,
        char*                       pValue,
        ULONG*                      pulSize
    )
{
    PCOSA_DML_EMCTL_CFG cfg = g_pEmctl_Cfg;

    if (AnscEqualString(pParamName, "AllowedChannelList2G", TRUE)) {
        if (AnscSizeOfString(cfg->AllowedChannelList2G) < *pulSize) {
            AnscCopyString(pValue, cfg->AllowedChannelList2G);
            return 0;
        } else {
            *pulSize = AnscSizeOfString(cfg->AllowedChannelList2G) + 1;
            return 1;
        }
    }
    if (AnscEqualString(pParamName, "AllowedChannelList5G", TRUE)) {
        if (AnscSizeOfString(cfg->AllowedChannelList5G) < *pulSize) {
            AnscCopyString(pValue, cfg->AllowedChannelList5G);
            return 0;
        } else {
            *pulSize = AnscSizeOfString(cfg->AllowedChannelList5G) + 1;
            return 1;
        }
    }
    if (AnscEqualString(pParamName, "BandLock5G", TRUE)) {
        if (AnscSizeOfString(cfg->BandLock5G) < *pulSize) {
            AnscCopyString(pValue, cfg->BandLock5G);
            return 0;
        } else {
            *pulSize = AnscSizeOfString(cfg->BandLock5G) + 1;
            return 1;
        }
    }
    if (AnscEqualString(pParamName, "Default2GPreferredChannelList", TRUE)) {
        if (AnscSizeOfString(cfg->Default2GPreferredChannelList) < *pulSize) {
            AnscCopyString(pValue, cfg->Default2GPreferredChannelList);
            return 0;
        } else {
            *pulSize = AnscSizeOfString(cfg->Default2GPreferredChannelList) + 1;
            return 1;
        }
    }
    if (AnscEqualString(pParamName, "Default5GPreferredChannelList", TRUE)) {
        if (AnscSizeOfString(cfg->Default5GPreferredChannelList) < *pulSize) {
            AnscCopyString(pValue, cfg->Default5GPreferredChannelList);
            return 0;
        } else {
            *pulSize = AnscSizeOfString(cfg->Default5GPreferredChannelList) + 1;
            return 1;
        }
    }

    return -1;
}

/**********************************************************************

    caller:     owner of this object

    prototype:

       BOOL
       ChanSel_SetParamStringValue
            (
                ANSC_HANDLE                 hInsContext,
                char*                       pParamArray,
                char*                       pString,
            );

    description:

        This function is called to set bulk parameter values;

    argument:   ANSC_HANDLE                 hInsContext,
                The instance handle;

                char*                       pParamName,
                The parameter name array;

                char*                       pString,
                The size of the array;

    return:     TRUE if succeeded.

**********************************************************************/
BOOL
ChanSel_SetParamStringValue
    (
        ANSC_HANDLE                 hInsContext,
        char*                       pParamName,
        char*                       pString
    )
{
    PCOSA_DML_EMCTL_CFG cfg = g_pEmctl_Cfg;

    if (AnscEqualString(pParamName, "AllowedChannelList2G", TRUE)) {
        if (AnscEqualString(cfg->AllowedChannelList2G, pString, TRUE)) {
            return TRUE;
        }
        AnscCopyString(cfg->AllowedChannelList2G, pString);
        bValueChanged = TRUE;
        return TRUE;
    }
    if (AnscEqualString(pParamName, "AllowedChannelList5G", TRUE)) {
        if (AnscEqualString(cfg->AllowedChannelList5G, pString, TRUE)) {
            return TRUE;
        }
        AnscCopyString(cfg->AllowedChannelList5G, pString);
        bValueChanged = TRUE;
        return TRUE;
    }
    if (AnscEqualString(pParamName, "BandLock5G", TRUE)) {
        if (AnscEqualString(cfg->BandLock5G, pString, TRUE)) {
            return  TRUE;
        }
        AnscCopyString(cfg->BandLock5G, pString);
        bValueChanged = TRUE;
        return TRUE;
    }
    if (AnscEqualString(pParamName, "Default2GPreferredChannelList", TRUE)) {
        if (AnscEqualString(cfg->Default2GPreferredChannelList, pString, TRUE)) {
            return  TRUE;
        }
        AnscCopyString(cfg->Default2GPreferredChannelList, pString);
        bValueChanged = TRUE;
        return TRUE;
    }
    if (AnscEqualString(pParamName, "Default5GPreferredChannelList", TRUE)) {
        if (AnscEqualString(cfg->Default5GPreferredChannelList, pString, TRUE)) {
            return  TRUE;
        }
        AnscCopyString(cfg->Default5GPreferredChannelList, pString);
        bValueChanged = TRUE;
        return TRUE;
    }

    return FALSE;
}

/**********************************************************************

 APIs for Object:

    EasyMeshController.SSIDProfile.{i}

    *  SSIDProfile_GetEntryCount
    *  SSIDProfile_GetEntry
    *  SSIDProfile_AddEntry
    *  SSIDProfile_DelEntry
    *  SSIDProfile_GetParamBoolValue
    *  SSIDProfile_GetParamStringValue
    *  SSIDProfile_GetParamIntValue
    *  SSIDProfile_SetParamBoolValue
    *  SSIDProfile_SetParamStringValue
    *  SSIDProfile_SetParamIntValue
    *  SSIDProfile_Validate
    *  SSIDProfile_Commit
    *  SSIDProfile_Rollback

**********************************************************************/
/**********************************************************************

    caller:     owner of this object

    prototype:

        ULONG
        SSIDProfile_GetEntryCount
            (
                ANSC_HANDLE                 hInsContext
            );

    description:

        This function is called to retrieve the count of the table.

    argument:   ANSC_HANDLE                 hInsContext,
                The instance handle;

    return:     The count of the table

**********************************************************************/
ULONG
SSIDProfile_GetEntryCount
    (
        ANSC_HANDLE                 hInsContext
    )
{
    UNREFERENCED_PARAMETER(hInsContext);
    PCOSA_DML_EMCTL_CFG         pEmctl   = (PCOSA_DML_EMCTL_CFG      )g_pEmctl_Cfg;

    return pEmctl->SSIDProfileNumberOfEntries;
}

/**********************************************************************

    caller:     owner of this object

    prototype:

        ANSC_HANDLE
        SSIDProfile_GetEntry
            (
                ANSC_HANDLE                 hInsContext,
                ULONG                       nIndex,
                ULONG*                      pInsNumber
            );

    description:

        This function is called to retrieve the entry specified by the index.

    argument:   ANSC_HANDLE                 hInsContext,
                The instance handle;

                ULONG                       nIndex,
                The index of this entry;

                ULONG*                      pInsNumber
                The output instance number;

    return:     The handle to identify the entry

**********************************************************************/
ANSC_HANDLE
SSIDProfile_GetEntry
    (
        ANSC_HANDLE                 hInsContext,
        ULONG                       nIndex,
        ULONG*                      pInsNumber
    )
{
    UNREFERENCED_PARAMETER(hInsContext);
    PCOSA_DML_EMCTL_CFG         pEmctl   = (PCOSA_DML_EMCTL_CFG      )g_pEmctl_Cfg;
    PCOSA_DML_EMCTL_PROFILE_CFG pProfile = NULL;

    if (pEmctl && nIndex < pEmctl->SSIDProfileNumberOfEntries) {
        pProfile = &pEmctl->SSIDProfiles[nIndex];
        *pInsNumber = nIndex + 1;
    }

    return pProfile;
}

/**********************************************************************

    caller:     owner of this object

    prototype:

        ANSC_HANDLE
        SSIDProfile_AddEntry
            (
                ANSC_HANDLE                 hInsContext,
                ULONG*                      pInsNumber
            );

    description:

        This function is called to add a new entry.

    argument:   ANSC_HANDLE                 hInsContext,
                The instance handle;

                ULONG*                      pInsNumber
                The output instance number;

    return:     The handle of new added entry.

**********************************************************************/
ANSC_HANDLE
SSIDProfile_AddEntry
    (
        ANSC_HANDLE                 hInsContext,
        ULONG*                      pInsNumber
    )
{
    return NULL;
}

/**********************************************************************

    caller:     owner of this object

    prototype:

        ULONG
        SSIDProfile_DelEntry
            (
                ANSC_HANDLE                 hInsContext,
                ANSC_HANDLE                 hInstance
            );

    description:

        This function is called to delete an exist entry.

    argument:   ANSC_HANDLE                 hInsContext,
                The instance handle;

                ANSC_HANDLE                 hInstance
                The exist entry handle;

    return:     The status of the operation.

**********************************************************************/
ULONG
SSIDProfile_DelEntry
    (
        ANSC_HANDLE                 hInsContext,
        ANSC_HANDLE                 hInstance
    )
{
    ANSC_STATUS                     returnStatus         = ANSC_STATUS_SUCCESS;
    return returnStatus;
}

/**********************************************************************

    caller:     owner of this object

    prototype:

        BOOL
        SSIDProfile_GetParamBoolValue
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
SSIDProfile_GetParamBoolValue
    (
        ANSC_HANDLE                 hInsContext,
        char*                       pParamName,
        BOOL*                       pBool
    )
{
    PCOSA_DML_EMCTL_PROFILE_CFG pProfile = (PCOSA_DML_EMCTL_PROFILE_CFG)hInsContext;

    if (AnscEqualString(pParamName, "Backhaul", TRUE)) {
        *pBool = pProfile->Backhaul;
        return TRUE;
    }
    if (AnscEqualString(pParamName, "Enable", TRUE)) {
        *pBool = pProfile->Enable;
        return TRUE;
    }
    if (AnscEqualString(pParamName, "Extender", TRUE)) {
        *pBool = pProfile->Extender;
        return TRUE;
    }
    if (AnscEqualString(pParamName, "Fronthaul", TRUE)) {
        *pBool = pProfile->Fronthaul;
        return TRUE;
    }
    if (AnscEqualString(pParamName, "Gateway", TRUE)) {
        *pBool = pProfile->Gateway;
        return TRUE;
    }

    return FALSE;
}

/**********************************************************************

    caller:     owner of this object

    prototype:

        ULONG
        SSIDProfile_GetParamStringValue
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
SSIDProfile_GetParamStringValue
    (
        ANSC_HANDLE                 hInsContext,
        char*                       pParamName,
        char*                       pValue,
        ULONG*                      pulSize
    )
{
    PCOSA_DML_EMCTL_PROFILE_CFG pProfile = (PCOSA_DML_EMCTL_PROFILE_CFG)hInsContext;

    if (AnscEqualString(pParamName, "FrequencyBands", TRUE)) {
        if (AnscSizeOfString(pProfile->FrequencyBands) < *pulSize) {
            AnscCopyString(pValue, pProfile->FrequencyBands);
            return 0;
        } else {
            *pulSize = AnscSizeOfString(pProfile->FrequencyBands) + 1;
            return 1;
        }
    }
    if (AnscEqualString(pParamName, "KeyPassphrase", TRUE)) {
        if (AnscSizeOfString(pProfile->KeyPassphrase) < *pulSize) {
            AnscCopyString(pValue, pProfile->KeyPassphrase);
            return 0;
        } else {
            *pulSize = AnscSizeOfString(pProfile->KeyPassphrase) + 1;
            return 1;
        }
    }
    if (AnscEqualString(pParamName, "Label", TRUE)) {
        if (AnscSizeOfString(pProfile->Label) < *pulSize) {
            AnscCopyString(pValue, pProfile->Label);
            return 0;
        } else {
            *pulSize = AnscSizeOfString(pProfile->Label) + 1;
            return 1;
        }
    }
    if (AnscEqualString(pParamName, "SSID", TRUE)) {
        if (AnscSizeOfString(pProfile->SSID) < *pulSize) {
            AnscCopyString(pValue, pProfile->SSID);
            return 0;
        } else {
            *pulSize = AnscSizeOfString(pProfile->SSID) + 1;
            return 1;
        }
    }
    if (AnscEqualString(pParamName, "SecurityMode", TRUE)) {
        if (AnscSizeOfString(pProfile->SecurityMode) < *pulSize) {
            AnscCopyString(pValue, pProfile->SecurityMode);
            return 0;
        } else {
            *pulSize = AnscSizeOfString(pProfile->SecurityMode) + 1;
            return 1;
        }
    }
    if (AnscEqualString(pParamName, "Type", TRUE)) {
        if (AnscSizeOfString(pProfile->Type) < *pulSize) {
            AnscCopyString(pValue, pProfile->Type);
            return 0;
        } else {
            *pulSize = AnscSizeOfString(pProfile->Type) + 1;
            return 1;
        }
    }

    return -1;
}

/**********************************************************************

    caller:     owner of this object

    prototype:

        BOOL
        SSIDProfile_GetParamIntValue
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
SSIDProfile_GetParamIntValue
    (
         ANSC_HANDLE                 hInsContext,
        char*                        pParamName,
        int*                         pInt
    )
{
    PCOSA_DML_EMCTL_PROFILE_CFG pProfile = (PCOSA_DML_EMCTL_PROFILE_CFG)hInsContext;

    if (AnscEqualString(pParamName, "VLANID", TRUE)) {
        *pInt = pProfile->VLANID;
        return TRUE;
    }

    return FALSE;
}

/**********************************************************************

    caller:     owner of this object

    prototype:

        BOOL
        SSIDProfile_SetParamBoolValue
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
SSIDProfile_SetParamBoolValue
    (
        ANSC_HANDLE                 hInsContext,
        char*                       pParamName,
        BOOL                        bValue
    )
{
    PCOSA_DML_EMCTL_PROFILE_CFG pProfile = (PCOSA_DML_EMCTL_PROFILE_CFG)hInsContext;

    if (AnscEqualString(pParamName, "Backhaul", TRUE)) {
        if (pProfile->Backhaul == bValue) {
            return TRUE;
        }
        pProfile->Backhaul = TRUE;
        return TRUE;
    }
    if (AnscEqualString(pParamName, "Enable", TRUE)) {
        if (pProfile->Enable == bValue) {
            return TRUE;
        }
        pProfile->Enable = TRUE;
        return TRUE;
    }
    if (AnscEqualString(pParamName, "Extender", TRUE)) {
        if (pProfile->Extender == bValue) {
            return TRUE;
        }
        pProfile->Extender = TRUE;
        return TRUE;
    }
    if (AnscEqualString(pParamName, "Fronthaul", TRUE)) {
        if (pProfile->Fronthaul == bValue) {
            return TRUE;
        }
        pProfile->Fronthaul = TRUE;
        return TRUE;
    }
    if (AnscEqualString(pParamName, "Gateway", TRUE)) {
        if (pProfile->Gateway == bValue) {
            return TRUE;
        }
        pProfile->Gateway = TRUE;
        return TRUE;
    }

    return FALSE;
}

/**********************************************************************

    caller:     owner of this object

    prototype:

       BOOL
       SSIDProfile_SetParamStringValue
            (
                ANSC_HANDLE                 hInsContext,
                char*                       pParamArray,
                char*                       pString,
            );

    description:

        This function is called to set bulk parameter values;

    argument:   ANSC_HANDLE                 hInsContext,
                The instance handle;

                char*                       pParamName,
                The parameter name array;

                char*                       pString,
                The size of the array;

    return:     TRUE if succeeded.

**********************************************************************/
BOOL
SSIDProfile_SetParamStringValue
    (
        ANSC_HANDLE                 hInsContext,
        char*                       pParamName,
        char*                       pString
    )
{
    PCOSA_DML_EMCTL_PROFILE_CFG pProfile = (PCOSA_DML_EMCTL_PROFILE_CFG)hInsContext;

    if (AnscEqualString(pParamName, "FrequencyBands", TRUE)) {
        if (AnscEqualString(pProfile->FrequencyBands, pString, TRUE)) {
            return TRUE;
        }
        AnscCopyString(pProfile->FrequencyBands, pString);
        bValueChanged = TRUE;
        return TRUE;
    }
    if (AnscEqualString(pParamName, "KeyPassphrase", TRUE)) {
        if (AnscEqualString(pProfile->KeyPassphrase, pString, TRUE)) {
            return TRUE;
        }
        AnscCopyString(pProfile->KeyPassphrase, pString);
        bValueChanged = TRUE;
        return TRUE;
    }
    if (AnscEqualString(pParamName, "Label", TRUE)) {
        if (AnscEqualString(pProfile->Label, pString, TRUE)) {
            return TRUE;
        }
        AnscCopyString(pProfile->Label, pString);
        bValueChanged = TRUE;
        return TRUE;
    }
    if (AnscEqualString(pParamName, "SSID", TRUE)) {
        if (AnscEqualString(pProfile->SSID, pString, TRUE)) {
            return TRUE;
        }
        AnscCopyString(pProfile->SSID, pString);
        bValueChanged = TRUE;
        return TRUE;
    }
    if (AnscEqualString(pParamName, "SecurityMode", TRUE)) {
        if (AnscEqualString(pProfile->SecurityMode, pString, TRUE)) {
            return TRUE;
        }
        AnscCopyString(pProfile->SecurityMode, pString);
        bValueChanged = TRUE;
        return TRUE;
    }
    if (AnscEqualString(pParamName, "Type", TRUE)) {
        if (AnscEqualString(pProfile->Type, pString, TRUE)) {
            return TRUE;
        }
        AnscCopyString(pProfile->Type, pString);
        bValueChanged = TRUE;
        return TRUE;
    }

    return FALSE;
}

/**********************************************************************

    caller:     owner of this object

    prototype:

        BOOL
        SSIDProfile_SetParamIntValue
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
SSIDProfile_SetParamIntValue
    (
        ANSC_HANDLE                 hInsContext,
        char*                       pParamName,
        int                         iValue
    )
{
    PCOSA_DML_EMCTL_PROFILE_CFG pProfile = (PCOSA_DML_EMCTL_PROFILE_CFG)hInsContext;

    if (AnscEqualString(pParamName, "VLANID", TRUE)) {
        if (pProfile->VLANID == iValue) {
            return TRUE;
        }
        pProfile->VLANID = iValue;
        bValueChanged = TRUE;
        return TRUE;
    }

    return FALSE;
}

/**********************************************************************

    caller:     owner of this object

    prototype:

       BOOL
       SSIDProfile_Validate
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
SSIDProfile_Validate
    (
        ANSC_HANDLE                 hInsContext,
        char*                       pReturnParamName,
        ULONG*                      puLength
    )
{
    return TRUE;
}

/**********************************************************************

    caller:     owner of this object

    prototype:

       ULONG
       SSIDProfile_Commit
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
SSIDProfile_Commit
    (
        ANSC_HANDLE                 hInsContext
    )
{
    return 0;
}

/**********************************************************************

    caller:     owner of this object

    prototype:

       ULONG
       SSIDProfile_Rollback
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
SSIDProfile_Rollback
    (
        ANSC_HANDLE                 hInsContext
    )
{
    return 0;
}
