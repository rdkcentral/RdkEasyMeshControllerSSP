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

#define EMCTL_CONTROLLER_ENABLE     "Device.DeviceInfo.X_RDKCENTRAL-COM_RFC.Feature.WiFi-PSM-DB.Enable"
#define EMCTL_CONTROLLER_ISMASTER   "Device.EasyMeshController.IsMaster"
//static char *Enable       = "eRT.com.cisco.spvtg.ccsp.tr181pa.Device.EasyMeshController.Enable";

/*
 * meta-cmf-raspberrypi/recipes-ccsp/ccsp/ccsp-psm.bbappend
 * rdkb/components/opensource/ccsp/CcspPsm/config/bbhm_def_cfg_qemu.xml
 * 1. system init
 * 2. init globals
 * 3. link with cosa api
 */

#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>

#include <user_base.h>
#include <user_socket.h>
#include <ansc_platform.h>
#include <ccsp_message_bus.h>
#include <ccsp_base_api.h>
#include <ccsp_memory.h>
#include <ccsp_custom.h>
#include "ccsp_trace.h"
#include "ccsp_syslog.h"
#include <dslh_definitions_database.h>

#include "collection.h"
#include "cosa_emctl_apis.h"

typedef struct {
    uint32_t mac_b;
    uint16_t mac_a;
} mac_s;

typedef union {
    uint8_t mac_u8[6];
    mac_s mac_ul;
} mac_u;

typedef unsigned char mac_addr[6];
typedef char mac_addr_str[18];

extern ANSC_HANDLE bus_handle;
extern char g_Subsystem[32];
extern PCOSA_DML_EMCTL_CFG g_pEmctl_Cfg;

static pthread_cond_t g_emctl_cond = PTHREAD_COND_INITIALIZER;
static pthread_mutex_t g_emctl_mutex = PTHREAD_MUTEX_INITIALIZER;

static int DmlEmctlGetParamValues(char *pathname, char *value, size_t valuesize)
{
    int ret = 0;
    int i;
    int retval = -1; // default failure
    const char *subsystem_prefix = "eRT.";
    int size = 0;
    int size2 = 0;
    const char *dst_pathname_cr =  "eRT.com.cisco.spvtg.ccsp.CR";
    componentStruct_t **ppComponents = NULL;
    parameterValStruct_t **val = NULL;

    ret = CcspBaseIf_discComponentSupportingNamespace(bus_handle, dst_pathname_cr, pathname,
            subsystem_prefix, &ppComponents, &size2);
    // check known erros
    if ((ret == CCSP_MESSAGE_BUS_NOT_EXIST) || (ret == CCSP_CR_ERR_UNSUPPORTED_NAMESPACE)) {
        printf("Can't find destination component, err: (%d) [%s=%s]\n", ret, pathname, value);
        goto bail;
    }
    // check unknown errors
    if (ret != CCSP_SUCCESS) {
        printf("Ccsp msg bus internal error: (%d) [%s=%s]\n", ret, pathname, value);
        goto bail;
    }
    // check component
    if (size2 == 0) {
        printf("Can't find destination component. [%s=%s]\n", pathname, value);
        goto bail;
    }

    for (i = 0; i < size2; i++) {
        char *dst_componentid = NULL;
        char *dst_pathname = NULL;
        dst_componentid = ppComponents[i]->componentName;
        dst_pathname    = ppComponents[i]->dbusPath;
        ret = CcspBaseIf_getParameterValues(bus_handle, dst_componentid, dst_pathname,
                &pathname, 1, &size, &val);
        if (ret != CCSP_SUCCESS) {
            printf("CcspBaseIf_getParameterValues failed with: (%d) [%s=%s] dst_componentid: %s\n", ret, pathname, value, dst_componentid);
            goto bail;
        }
        if (size > 0) {
            if (val[0]->parameterValue && strlen(val[0]->parameterValue) != 0) {
                snprintf(value, valuesize, "%s", val[0]->parameterValue);
                retval = 0; // set success
            }
        }
        free_parameterValStruct_t (bus_handle, size, val);
        val = NULL;
    }

bail:
    if (val != NULL) {
        free_parameterValStruct_t (bus_handle, size, val);
        val = NULL;
    }
    if (ppComponents == NULL) {
        goto next;
    }
    // free ppComponents
    for (i = 0; i < size2; i++) {
        if (ppComponents[i]->remoteCR_dbus_path) {
            AnscFreeMemory(ppComponents[i]->remoteCR_dbus_path);
        }
        if (ppComponents[i]->remoteCR_name) {
            AnscFreeMemory(ppComponents[i]->remoteCR_name);
        }
        if (ppComponents[i]->componentName) {
            AnscFreeMemory(ppComponents[i]->componentName);
        }
        if (ppComponents[i]->dbusPath) {
            AnscFreeMemory(ppComponents[i]->dbusPath);
        }
        AnscFreeMemory(ppComponents[i]);
    }
    AnscFreeMemory(ppComponents);

next:
    return retval;
}

static int DmlEmctlSetParamValues(const char *pathname, enum dataType_e type, const char *value, int commit)
{
    int ret = 0;
    int i;
    int retval = -1; // default failure
    const char *subsystem_prefix = "eRT.";
    int size2 = 0;
    const char *dst_pathname_cr =  "eRT.com.cisco.spvtg.ccsp.CR";
    componentStruct_t **ppComponents = NULL;
    parameterValStruct_t val = { 0 };
    char *pFaultParameter = NULL;

    ret = CcspBaseIf_discComponentSupportingNamespace(bus_handle, dst_pathname_cr, pathname,
            subsystem_prefix, &ppComponents, &size2);
    // check known erros
    if ((ret == CCSP_MESSAGE_BUS_NOT_EXIST) || (ret == CCSP_CR_ERR_UNSUPPORTED_NAMESPACE)) {
        printf("Can't find destination component, err: (%d) [%s=%s]\n", ret, pathname, value);
        goto bail;
    }
    // check unknown errors
    if (ret != CCSP_SUCCESS) {
        printf("Ccsp msg bus internal error: (%d) [%s=%s]\n", ret, pathname, value);
        goto bail;
    }
    // check component
    if (size2 == 0) {
        printf("Can't find destination component. [%s=%s]\n", pathname, value);
        goto bail;
    }

    // set value struct
    val.type = type;
    val.parameterValue = AnscCloneString((char *)value);
    val.parameterName = AnscCloneString((char *)pathname);
    for (i = 0; i < size2; i++) {
        char *dst_componentid = NULL;
        char *dst_pathname = NULL;
        dst_componentid = ppComponents[i]->componentName;
        dst_pathname    = ppComponents[i]->dbusPath;
        ret = CcspBaseIf_setParameterValues(bus_handle, dst_componentid, dst_pathname, 0,
                DSLH_MPA_ACCESS_CONTROL_CLIENTTOOL, &val, 1,
                commit, &pFaultParameter);
        if (ret != CCSP_SUCCESS) {
            printf("CcspBaseIf_setParameterValues failed with: (%d) [%s=%s] dst_componentid: %s\n", ret, pathname, value, dst_componentid);
            goto bail;
        }
    }
    retval = ANSC_STATUS_SUCCESS; // set success

bail:
    if (val.parameterValue != NULL) {
        AnscFreeMemory(val.parameterValue);
    }
    if (val.parameterName != NULL) {
        AnscFreeMemory(val.parameterName);
    }
    if (pFaultParameter != NULL) {
        AnscFreeMemory(pFaultParameter);
    }
    if (ppComponents == NULL) {
        goto next;
    }
    // free ppComponents
    for (i = 0; i < size2; i++) {
        if (ppComponents[i]->remoteCR_dbus_path) {
            AnscFreeMemory(ppComponents[i]->remoteCR_dbus_path);
        }
        if (ppComponents[i]->remoteCR_name) {
            AnscFreeMemory(ppComponents[i]->remoteCR_name);
        }
        if (ppComponents[i]->componentName) {
            AnscFreeMemory(ppComponents[i]->componentName);
        }
        if (ppComponents[i]->dbusPath) {
            AnscFreeMemory(ppComponents[i]->dbusPath);
        }
        AnscFreeMemory(ppComponents[i]);
    }
    AnscFreeMemory(ppComponents);

next:
    return retval;
}

static int string_to_mac(const char *macstr, mac_addr mac)
{
    if ((NULL != macstr) && (NULL != mac)) {
        if (sscanf(macstr, "%hhx:%hhx:%hhx:%hhx:%hhx:%hhx",
                   &mac[5], &mac[4], &mac[3], &mac[2], &mac[1], &mac[0]) == 6) {
            return 0;
        }
    }
    return -1;
}

static int mac_to_string(const mac_addr mac, mac_addr_str macstr)
{
    if ((NULL != macstr) && (NULL != mac)) {
        snprintf(macstr, sizeof(mac_addr_str), "%02hhx:%02hhx:%02hhx:%02hhx:%02hhx:%02hhx",
                 mac[5], mac[4], mac[3], mac[2], mac[1], mac[0]);
        return 0;
    }
    return -1;
}

static void get_mac_addresses(PCOSA_DML_EMCTL_CFG cfg)
{
    char acTmpReturnValue[256] = {0};
    mac_u mac;
    mac_addr_str macstr;
    
    if (ANSC_STATUS_FAILURE == DmlEmctlGetParamValues("Device.Ethernet.Interface.1.MACAddress", acTmpReturnValue, sizeof(acTmpReturnValue))) {
        CcspTraceError(("%s %d Failed to get param value\n", __FUNCTION__, __LINE__));
        return;
    }

    string_to_mac(acTmpReturnValue, mac.mac_u8);
    mac.mac_u8[5] |= 1 << 1;
    mac_to_string(mac.mac_u8, macstr);
    AnscCopyString(cfg->LocalAgentMACAddress, macstr);
    mac.mac_ul.mac_b += 1;
    mac_to_string(mac.mac_u8, macstr);
    AnscCopyString(cfg->MACAddress, macstr);
}

int CosaEmctlGetSSIDbyIndex(char *value, int index)
{
    char acTmpReturnValue[256] = {0};
    char path[64] = {0};

    sprintf(path, "Device.WiFi.SSID.%d.SSID", index);
    if (ANSC_STATUS_FAILURE == DmlEmctlGetParamValues(path, acTmpReturnValue, sizeof(acTmpReturnValue))) {
        printf("%s %d Failed to get param value\n", __FUNCTION__, __LINE__);
        return ANSC_STATUS_FAILURE;
    }
    AnscCopyString(value, acTmpReturnValue);
    printf("%s %d param value = %s\n", __FUNCTION__, __LINE__, value);
    return 0;
}

int CosaEmctlGetPassPhrasebyIndex(char *value, int index)
{
    char acTmpReturnValue[256] = {0};
    char path[64] = {0};

    sprintf(path, "Device.WiFi.AccessPoint.%d.Security.KeyPassphrase", index);
    if (ANSC_STATUS_FAILURE == DmlEmctlGetParamValues(path, acTmpReturnValue, sizeof(acTmpReturnValue))) {
        printf("%s %d Failed to get param value\n", __FUNCTION__, __LINE__);
        return ANSC_STATUS_FAILURE;
    }
    AnscCopyString(value, acTmpReturnValue);
    printf("%s %d param value = %s\n", __FUNCTION__, __LINE__, value);
    return 0;
}

static void load_ssid_profile(PCOSA_DML_EMCTL_PROFILE_CFG profile, unsigned int idx)
{
    /* Let's use this basic static mapping for now
     * SSIDProfile.1 (Home,     2.4GHz, Fronthaul) => Device.WiFi.SSID.1
     * SSIDProfile.1 (Home,     5GHz,   Fronthaul) => Device.WiFi.SSID.5
     * SSIDProfile.2 (Guest,    2.4GHz, Fronthaul) => Device.WiFi.SSID.2
     * SSIDProfile.2 (Guest,    5GHz,   Fronthaul) => Device.WiFi.SSID.6
     * SSIDProfile.2 (Backhaul, 5GHz,   Backhaul)  => Device.WiFi.SSID.8
     */
    switch (idx) {
        case 0:
            strncpy(profile->Type, "backhaul", sizeof(profile->Type) - 1);
            strncpy(profile->Label, "Backhaul", sizeof(profile->Label) - 1);
            strncpy(profile->SSID, "rpi-bh", sizeof(profile->SSID) - 1);
            strncpy(profile->FrequencyBands, "5", sizeof(profile->FrequencyBands) - 1);
            profile->Fronthaul = FALSE;
            profile->Backhaul = TRUE;
            break;
        case 1:
            strncpy(profile->Type, "home", sizeof(profile->Type) - 1);
            strncpy(profile->Label, "Family", sizeof(profile->Label) - 1);
            strncpy(profile->SSID, "rpi-fh", sizeof(profile->SSID) - 1);
            strncpy(profile->FrequencyBands, "2,5", sizeof(profile->FrequencyBands) - 1);
            profile->Fronthaul = TRUE;
            profile->Backhaul = FALSE;
            break;
        case 2:
            strncpy(profile->Type, "guest", sizeof(profile->Type) - 1);
            strncpy(profile->Label, "Guest", sizeof(profile->Label) - 1);
            strncpy(profile->SSID, "rpi-guest", sizeof(profile->SSID) - 1);
            strncpy(profile->FrequencyBands, "2,5", sizeof(profile->FrequencyBands) - 1);
            profile->Fronthaul = TRUE;
            profile->Backhaul = FALSE;
            break;
        default:
            printf("Invalid profile index\n");
            return;
    }
    strncpy(profile->SecurityMode, "wpa2-psk", sizeof(profile->SecurityMode) - 1);
    strncpy(profile->KeyPassphrase, "12345678", sizeof(profile->KeyPassphrase) - 1);
    profile->Extender = TRUE;
    profile->Gateway = TRUE;
    profile->VLANID = -1;
}

ANSC_HANDLE CosaEmctlCreate(void)
{
    PCOSA_DML_EMCTL_CFG pMyObject = (PCOSA_DML_EMCTL_CFG)NULL;

    pMyObject = (PCOSA_DML_EMCTL_CFG)AnscAllocateMemory(sizeof(COSA_DML_EMCTL_CFG));
    AnscZeroMemory(pMyObject, sizeof(COSA_DML_EMCTL_CFG));

    return pMyObject;
}

ANSC_STATUS CosaEmctlInitialize(ANSC_HANDLE hThisObject)
{
    PCOSA_DML_EMCTL_CFG pEmctl_Cfg = (PCOSA_DML_EMCTL_CFG)hThisObject;
    unsigned int i;

    pEmctl_Cfg->ConfigRenewInterval = 5;
    pEmctl_Cfg->ConfigRenewMaxRetry = 3;
    pEmctl_Cfg->ConfigureBackhaulStation = 1;
    pEmctl_Cfg->DeadAgentDetectionInterval = 30;
    pEmctl_Cfg->Enable = 1;
    AnscCopyString(pEmctl_Cfg->InterfaceList, "^lo$|^eth.*|^wl.*");
    pEmctl_Cfg->IsMaster = 1;
    pEmctl_Cfg->LinkMetricsQueryInterval = 20;
    pEmctl_Cfg->PrimaryVLANID = -1;
    AnscCopyString(pEmctl_Cfg->PrimaryVLANInterfacePattern, "^lo$|^eth.*|^wl.*");
    pEmctl_Cfg->SSIDProfileNumberOfEntries = 3;
    for (i = 0; i < pEmctl_Cfg->SSIDProfileNumberOfEntries; i++) {
        load_ssid_profile(&pEmctl_Cfg->SSIDProfiles[i], i);
    }
    pEmctl_Cfg->TopologyDiscoveryInterval = 60;
    pEmctl_Cfg->TopologyQueryInterval = 60;
    pEmctl_Cfg->TopologyStableCheckInterval = 120;
    get_mac_addresses(pEmctl_Cfg);

    return ANSC_STATUS_SUCCESS;
}

int CosaEmctlGetAllowedChannelList2G(char *value)
{
    AnscCopyString(value, g_pEmctl_Cfg->AllowedChannelList2G);
    return 0;
}

int CosaEmctlGetAllowedChannelList5G(char *value)
{
    AnscCopyString(value, g_pEmctl_Cfg->AllowedChannelList5G);
    return 0;
}

int CosaEmctlGetBandLock5G(char *value)
{
    AnscCopyString(value, g_pEmctl_Cfg->BandLock5G);
    return 0;
}

int CosaEmctlGetConfigRenewInterval(unsigned int *value)
{
    *value = g_pEmctl_Cfg->ConfigRenewInterval;
    return 0;
}

int CosaEmctlGetConfigRenewMaxRetry(unsigned int *value)
{
    *value = g_pEmctl_Cfg->ConfigRenewMaxRetry;
    return 0;
}

int CosaEmctlGetConfigureBackhaulStation(uint8_t *value)
{
    *value = g_pEmctl_Cfg->ConfigureBackhaulStation;
    return 0;
}

int CosaEmctlGetDeadAgentDetectionInterval(unsigned int *value)
{
    *value = g_pEmctl_Cfg->DeadAgentDetectionInterval;
    return 0;
}

int CosaEmctlGetDefault2GPreferredChannelList(char *value)
{
    AnscCopyString(value, g_pEmctl_Cfg->Default2GPreferredChannelList);
    return 0;
}

int CosaEmctlGetDefault5GPreferredChannelList(char *value)
{
    AnscCopyString(value, g_pEmctl_Cfg->Default5GPreferredChannelList);
    return 0;
}

int CosaEmctlGetDefaultPCP(uint8_t *value)
{
    *value = g_pEmctl_Cfg->DefaultPCP;
    return 0;
}

int CosaEmctlGetEnable(unsigned int *value)
{
    *value = g_pEmctl_Cfg->Enable;
    return 0;
}

int CosaEmctlGetInterfaceList(char *value)
{
    AnscCopyString(value, g_pEmctl_Cfg->InterfaceList);
    return 0;
}

int CosaEmctlGetIsMaster(unsigned int *value)
{
    *value = g_pEmctl_Cfg->IsMaster;
    return 0;
}

int CosaEmctlGetLinkMetricsQueryInterval(unsigned int *value)
{
    *value = g_pEmctl_Cfg->LinkMetricsQueryInterval;
    return 0;
}

int CosaEmctlGetLocalAgentMACAddress(char *value)
{
    AnscCopyString(value, g_pEmctl_Cfg->LocalAgentMACAddress);
    return 0;
}

int CosaEmctlGetMACAddress(char *value)
{
    AnscCopyString(value, g_pEmctl_Cfg->MACAddress);
    return 0;
}

int CosaEmctlGetPrimaryVLANID(int *value)
{
    *value = g_pEmctl_Cfg->PrimaryVLANID;
    return 0;
}

int CosaEmctlGetPrimaryVLANInterfacePattern(char *value)
{
    AnscCopyString(value, g_pEmctl_Cfg->PrimaryVLANInterfacePattern);
    return 0;
}

int CosaEmctlGetTopologyDiscoveryInterval(unsigned int *value)
{
    *value = g_pEmctl_Cfg->TopologyDiscoveryInterval;
    return 0;
}

int CosaEmctlGetTopologyQueryInterval(unsigned int *value)
{
    *value = g_pEmctl_Cfg->TopologyQueryInterval;
    return 0;
}

int CosaEmctlTopologyStableCheckInterval(unsigned int *value)
{
    *value = g_pEmctl_Cfg->TopologyStableCheckInterval;
    return 0;
}

int CosaEmctlProfileConfigChangeNotification(update_params_t *update)
{
    char value[256];

    snprintf(value, sizeof(value) - 1, "%s,%d,%s", update->type, update->index, update->value);
    if (DmlEmctlSetParamValues("Device.WiFi.X_RDK_EasymeshControllerNotification", ccsp_string, value, TRUE) != 0) {
        fprintf(stderr, "failed sending notification to wifi agent\n");
        return -1;
    }

    return 0;
}

int CosaEmctlProfileGetBackhaul(uint8_t index, bool *backhaul)
{
    PCOSA_DML_EMCTL_PROFILE_CFG profile;

    if (index > g_pEmctl_Cfg->SSIDProfileNumberOfEntries) {
        return -1;
    }
    profile = &g_pEmctl_Cfg->SSIDProfiles[index];
    *backhaul = profile->Backhaul;

    return 0;
}

int CosaEmctlProfileGetExtender(uint8_t index, uint8_t *extender)
{
    PCOSA_DML_EMCTL_PROFILE_CFG profile;

    if (index > g_pEmctl_Cfg->SSIDProfileNumberOfEntries) {
        return -1;
    }
    profile = &g_pEmctl_Cfg->SSIDProfiles[index];
    *extender = profile->Extender;

    return 0;
}

int CosaEmctlProfileGetFrequencyBands(uint8_t index, char **freq_bands)
{
    PCOSA_DML_EMCTL_PROFILE_CFG profile;

    if (freq_bands == NULL) {
        return -1;
    }
    if (index > g_pEmctl_Cfg->SSIDProfileNumberOfEntries) {
        return -1;
    }
    profile = &g_pEmctl_Cfg->SSIDProfiles[index];
    *freq_bands = strdup(profile->FrequencyBands);

    return 0;
}

int CosaEmctlProfileGetFronthaul(uint8_t index, bool *fronthaul)
{
    PCOSA_DML_EMCTL_PROFILE_CFG profile;

    if (index > g_pEmctl_Cfg->SSIDProfileNumberOfEntries) {
        return -1;
    }
    profile = &g_pEmctl_Cfg->SSIDProfiles[index];
    *fronthaul = profile->Fronthaul;

    return 0;
}

int CosaEmctlProfileGetGateway(uint8_t index, uint8_t *gateway)
{
    PCOSA_DML_EMCTL_PROFILE_CFG profile;

    if (index > g_pEmctl_Cfg->SSIDProfileNumberOfEntries) {
        return -1;
    }
    profile = &g_pEmctl_Cfg->SSIDProfiles[index];
    *gateway = profile->Gateway;

    return 0;
}

int CosaEmctlProfileGetKeypassphrase(uint8_t index, char **key_passphrase)
{
    PCOSA_DML_EMCTL_PROFILE_CFG profile;

    if (key_passphrase == NULL) {
        return -1;
    }
    if (index > g_pEmctl_Cfg->SSIDProfileNumberOfEntries) {
        return -1;
    }
    profile = &g_pEmctl_Cfg->SSIDProfiles[index];
    *key_passphrase = strdup(profile->KeyPassphrase);

    return 0;
}

int CosaEmctlProfileGetLabel(uint8_t index, char **label)
{
    PCOSA_DML_EMCTL_PROFILE_CFG profile;

    if (label == NULL) {
        return -1;
    }
    if (index > g_pEmctl_Cfg->SSIDProfileNumberOfEntries) {
        return -1;
    }
    profile = &g_pEmctl_Cfg->SSIDProfiles[index];
    *label = strdup(profile->Label);

    return 0;
}

int CosaEmctlProfileGetSecurityMode(uint8_t index, char **security_mode)
{
    PCOSA_DML_EMCTL_PROFILE_CFG profile;

    if (security_mode == NULL) {
        return -1;
    }
    if (index > g_pEmctl_Cfg->SSIDProfileNumberOfEntries) {
        return -1;
    }
    profile = &g_pEmctl_Cfg->SSIDProfiles[index];
    *security_mode = strdup(profile->SecurityMode);

    return 0;
}

int CosaEmctlProfileGetSSID(uint8_t index, char **ssid)
{
    PCOSA_DML_EMCTL_PROFILE_CFG profile;

    if (ssid == NULL) {
        return -1;
    }
    if (index > g_pEmctl_Cfg->SSIDProfileNumberOfEntries) {
        return -1;
    }
    profile = &g_pEmctl_Cfg->SSIDProfiles[index];
    *ssid = strdup(profile->SSID);

    return 0;
}

int CosaEmctlProfileGetType(uint8_t index, char **type)
{
    PCOSA_DML_EMCTL_PROFILE_CFG profile;

    if (type == NULL) {
        return -1;
    }
    if (index > g_pEmctl_Cfg->SSIDProfileNumberOfEntries) {
        return -1;
    }
    profile = &g_pEmctl_Cfg->SSIDProfiles[index];
    *type = strdup(profile->Type);

    return 0;
}

int CosaEmctlProfileGetVLANID(uint8_t index, int *vlan_id)
{
    PCOSA_DML_EMCTL_PROFILE_CFG profile;

    if (index > g_pEmctl_Cfg->SSIDProfileNumberOfEntries) {
        return -1;
    }
    profile = &g_pEmctl_Cfg->SSIDProfiles[index];
    *vlan_id = profile->VLANID;

    return 0;
}

static void *update_handler(void *farg)
{
    struct timeval tval;
    struct timespec tspec;
    update_params_t *update;
    int rc;

    while (1) {
        pthread_mutex_lock(&g_emctl_mutex);
        g_pEmctl_Cfg->updating = 1;
        gettimeofday(&tval, NULL);
        tspec.tv_sec = tval.tv_sec + 5;
        rc = pthread_cond_timedwait(&g_emctl_cond, &g_emctl_mutex, &tspec);
        if (rc == 0) {
            pthread_mutex_unlock(&g_emctl_mutex);
            continue;
        }
        if (g_pEmctl_Cfg->profile_update_cb != NULL) {
            g_pEmctl_Cfg->profile_update_cb(g_pEmctl_Cfg->updates);
        } else {
            update = queue_pop(g_pEmctl_Cfg->updates);
            while (update != NULL) {
                free(update->type);
                free(update->value);
                free(update);
                update = queue_pop(g_pEmctl_Cfg->updates);
            }
        }
        queue_destroy(g_pEmctl_Cfg->updates);
        g_pEmctl_Cfg->updates = NULL;
        g_pEmctl_Cfg->updating = 0;
        pthread_mutex_unlock(&g_emctl_mutex);
        break;
    }

    return NULL;
}

int EmctlConfigChangeCB(char *context)
{
    PCOSA_DML_EMCTL_PROFILE_CFG profile;
    update_params_t *update;
    char *p_tok, *st;
    pthread_attr_t attr;
    pthread_attr_t *attrp = NULL;
    pthread_t thread;
    size_t len = 0;
    int i = 0;

    if (g_pEmctl_Cfg == NULL) {
        return -1;
    }

    update = malloc(sizeof(update_params_t));
    memset(update, 0, sizeof(update_params_t));
    for (p_tok = strtok_r(context, ",", &st); p_tok; p_tok = strtok_r(NULL, ",", &st)) {
        switch (i) {
            case 0:
                update->type = strdup(p_tok);
                break;
            case 1:
                update->index = atoi(p_tok);
                break;
            case 2:
                update->value = strdup(p_tok);
                break;
        }
        i++;
        if (i == 3) {
            break;
        }
    }

    pthread_mutex_lock(&g_emctl_mutex);
    if (g_pEmctl_Cfg->updating) {
        queue_push(g_pEmctl_Cfg->updates, update);
        pthread_cond_signal(&g_emctl_cond);
        pthread_mutex_unlock(&g_emctl_mutex);
        return 0;
    }
    if (g_pEmctl_Cfg->updates != NULL) {
        queue_destroy(g_pEmctl_Cfg->updates);
    }
    g_pEmctl_Cfg->updates = queue_create();
    queue_push(g_pEmctl_Cfg->updates, update);
    pthread_mutex_unlock(&g_emctl_mutex);

    pthread_attr_init(&attr);
    pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);
    attrp = &attr;
    pthread_create(&thread, attrp, &update_handler, NULL);
    if (attrp != NULL) {
        pthread_attr_destroy(attrp);
    }
	
    return 0;
}

int EmctlRegisterConfigChangeCB(cb_func_t cb_func)
{
    if (g_pEmctl_Cfg == NULL) {
        return -1;
    }
    g_pEmctl_Cfg->profile_update_cb = cb_func;
    
    return 0;
}

