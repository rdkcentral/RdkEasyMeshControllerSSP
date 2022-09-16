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

/* Fill this array, if static mapping is used */
static COSA_DML_EMCTL_PROFILE_CFG g_default_profiles[] = {
    {
        .Type = "home",
        .Label = "Family",
        .Fronthaul = TRUE,
        .Backhaul = FALSE,
        .Indices = {0,1},
    },
    {
        .Type = "guest",
        .Label = "Guest",
        .Fronthaul = TRUE,
        .Backhaul = FALSE,
        .Indices = {2,3},
    },
    {
        .Type = "backhaul",
        .Label = "Backhaul",
        .Fronthaul = FALSE,
        .Backhaul = TRUE,
        .Indices = {5,-1},
    }
};

static ANSC_STATUS DmlEmctlGetParamValues(char *pathname, char *value, size_t valuesize)
{
    int ret;
    int i;
    ANSC_STATUS retval = ANSC_STATUS_FAILURE; // default failure
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
                retval = ANSC_STATUS_SUCCESS; // set success
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

static ANSC_STATUS DmlEmctlSetParamValues(const char *pathname, enum dataType_e type, const char *value, int commit)
{
    int ret;
    int i;
    ANSC_STATUS retval = ANSC_STATUS_FAILURE; // default failure
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

static ANSC_STATUS wifi_get_radio_freqband(char *value, unsigned int index)
{
    char path[512] = {0};
    char acTmpReturnValue[256] = {0};

    sprintf(path, "Device.WiFi.SSID.%d.LowerLayers", index);
    if (ANSC_STATUS_FAILURE == DmlEmctlGetParamValues(path, acTmpReturnValue, sizeof(acTmpReturnValue))) {
        fprintf(stderr, "Failed to get param values(%s-%d)\n", __FUNCTION__, __LINE__);
        return ANSC_STATUS_FAILURE;
    }
    /* Lets assume if SSID is enabled, so is its Radio */
    sprintf(path, "%sOperatingFrequencyBand", acTmpReturnValue);
    if (ANSC_STATUS_FAILURE == DmlEmctlGetParamValues(path, acTmpReturnValue, sizeof(acTmpReturnValue))) {
        fprintf(stderr, "Failed to get param values(%s-%d)\n", __FUNCTION__, __LINE__);
        return ANSC_STATUS_FAILURE;
    }
    if (strcmp("5GHz", acTmpReturnValue) == 0) {
        strcpy(value, "5");
    } else {
        strcpy(value, "2");
    }

    return ANSC_STATUS_SUCCESS;
}

static ANSC_STATUS wifi_get_ssid_count(unsigned int *value)
{
    char path[64] = {0};
    char acTmpReturnValue[256] = {0};

    sprintf(path, "Device.WiFi.SSIDNumberOfEntries");
    if (ANSC_STATUS_FAILURE == DmlEmctlGetParamValues(path, acTmpReturnValue, sizeof(acTmpReturnValue))) {
        fprintf(stderr, "Failed to get param values(%s-%d)\n", __FUNCTION__, __LINE__);
        return ANSC_STATUS_FAILURE;
    }
    *value = (unsigned int)atoi(acTmpReturnValue);

    return ANSC_STATUS_SUCCESS;
}

static ANSC_STATUS wifi_get_ssid_enable(bool *value, unsigned int index)
{
    char path[64] = {0};
    char acTmpReturnValue[256] = {0};

    sprintf(path, "Device.WiFi.SSID.%d.Enable", index);
    if (ANSC_STATUS_FAILURE == DmlEmctlGetParamValues(path, acTmpReturnValue, sizeof(acTmpReturnValue))) {
        fprintf(stderr, "Failed to get param values(%s-%d)\n", __FUNCTION__, __LINE__);
        return ANSC_STATUS_FAILURE;
    }
    if (strcmp("true", acTmpReturnValue) == 0) {
        *value = true;
    } else {
        *value = false;
    }

    return ANSC_STATUS_SUCCESS;
}

static ANSC_STATUS wifi_get_ssid_ssid(char *value, unsigned int index)
{
    char path[64] = {0};
    char acTmpReturnValue[256] = {0};

    sprintf(path, "Device.WiFi.SSID.%d.SSID", index);
    if (ANSC_STATUS_FAILURE == DmlEmctlGetParamValues(path, acTmpReturnValue, sizeof(acTmpReturnValue))) {
        fprintf(stderr, "Failed to get param values(%s-%d)\n", __FUNCTION__, __LINE__);
        return ANSC_STATUS_FAILURE;
    }
    AnscCopyString(value, acTmpReturnValue);

    return ANSC_STATUS_SUCCESS;
}

static ANSC_STATUS wifi_get_security_index(unsigned int *value, unsigned int index)
{
    char path[64] = {0};
    char acTmpReturnValue[256] = {0};
    char pattern[64] = {0};
    unsigned int ap_count;
    unsigned int i;

    sprintf(pattern, "Device.WiFi.SSID.%u.", index);
    sprintf(path, "Device.WiFi.AccessPointNumberOfEntries");
    if (ANSC_STATUS_FAILURE == DmlEmctlGetParamValues(path, acTmpReturnValue, sizeof(acTmpReturnValue))) {
        fprintf(stderr, "Failed to get param values(%s-%d)\n", __FUNCTION__, __LINE__);
        return ANSC_STATUS_FAILURE;
    }
    ap_count = atoi(acTmpReturnValue);
    for (i = 0; i < ap_count; i++) {
        sprintf(path, "Device.WiFi.AccessPoint.%d.SSIDReference", i + 1);
        if (ANSC_STATUS_FAILURE == DmlEmctlGetParamValues(path, acTmpReturnValue, sizeof(acTmpReturnValue))) {
            fprintf(stderr, "Failed to get param values(%s-%d)\n", __FUNCTION__, __LINE__);
            return ANSC_STATUS_FAILURE;
        }
        if (strcmp(pattern, acTmpReturnValue) == 0) {
            *value = i + 1;
            return ANSC_STATUS_SUCCESS;
        }
    }
    fprintf(stderr, "Failed to find SSID reference: %d\n", index);

    return ANSC_STATUS_FAILURE;
}

static ANSC_STATUS wifi_get_security_keypassphrase(char *value, unsigned int index)
{
    char path[64] = {0};
    char acTmpReturnValue[256] = {0};

    sprintf(path, "Device.WiFi.AccessPoint.%u.Security.KeyPassphrase", index);
    if (ANSC_STATUS_FAILURE == DmlEmctlGetParamValues(path, acTmpReturnValue, sizeof(acTmpReturnValue))) {
        fprintf(stderr, "Failed to get param values(%s-%d)\n", __FUNCTION__, __LINE__);
        return ANSC_STATUS_FAILURE;
    }
    AnscCopyString(value, acTmpReturnValue);

    return ANSC_STATUS_SUCCESS;
}

static ANSC_STATUS wifi_get_security_mode(char *value, unsigned int index)
{
    char path[64] = {0};
    char acTmpReturnValue[256] = {0};

    sprintf(path, "Device.WiFi.AccessPoint.%d.Security.ModeEnabled", index);
    if (ANSC_STATUS_FAILURE == DmlEmctlGetParamValues(path, acTmpReturnValue, sizeof(acTmpReturnValue))) {
        fprintf(stderr, "Failed to get param values(%s-%d)\n", __FUNCTION__, __LINE__);
        return ANSC_STATUS_FAILURE;
    }
    if (strcmp(acTmpReturnValue, "WPA2-Personal") == 0) {
        strcpy(value, "wpa2-psk");
    }

    return ANSC_STATUS_SUCCESS;
}

static inline void profile_set_backhaul(PCOSA_DML_EMCTL_PROFILE_CFG profile)
{
    strncpy(profile->Type, "backhaul", sizeof(profile->Type) - 1);
    strncpy(profile->Label, "Backhaul", sizeof(profile->Label) - 1);
    profile->Backhaul = TRUE;
    profile->Fronthaul = FALSE;
}

static void profile_create_backhaul(PCOSA_DML_EMCTL_CFG emctl)
{
    unsigned int index;
    PCOSA_DML_EMCTL_PROFILE_CFG base;
    PCOSA_DML_EMCTL_PROFILE_CFG profile;

    index = emctl->SSIDProfileNumberOfEntries;
    profile = &emctl->SSIDProfiles[index];
    base = &emctl->SSIDProfiles[0];
    profile->Enable = TRUE;
    strncpy(profile->SSID, "Backhaul", sizeof(profile->SSID) - 1);
    strncpy(profile->SecurityMode, base->SecurityMode, sizeof(profile->SecurityMode) - 1);
    strncpy(profile->KeyPassphrase, base->KeyPassphrase, sizeof(profile->KeyPassphrase) - 1);
    profile->FrequencyBands[0] = '5';
    profile_set_backhaul(profile);
    profile->Indices[0] = -1;
    profile->Indices[1] = -1;
    profile->Extender = TRUE;
    profile->Gateway = TRUE;
    profile->VLANID = -1;
    emctl->SSIDProfileNumberOfEntries++;

    return;
}

static void device_wifi_dynamic_mapper(PCOSA_DML_EMCTL_CFG emctl)
{
    bool found;
    bool ssid_enable = false;
    bool dedicated_backhaul = true;
    unsigned int ap_index = 0;
    unsigned int ssid_count = 0;
    unsigned int ssid_index;
    unsigned int profile_count;
    unsigned int profile_2GHz_count = 0;
    unsigned int profile_5GHz_count = 0;
    char key[64];
    char ssid[32];
    char mode[32];
    char freqband[8];
    PCOSA_DML_EMCTL_PROFILE_CFG profile;
    unsigned int i, j;

    profile_count = emctl->SSIDProfileNumberOfEntries;
    if (wifi_get_ssid_count(&ssid_count) == ANSC_STATUS_FAILURE) {
        return;
    }
    for (i = 0; i < ssid_count; i++) {
        ssid_index = i + 1;
        wifi_get_ssid_enable(&ssid_enable, ssid_index);
        if (!ssid_enable) {
            continue;
        }
        found = false;
        wifi_get_radio_freqband(freqband, ssid_index);
        wifi_get_ssid_ssid(ssid, ssid_index);
        for (j = 0; j < profile_count; j++) {
            profile = &emctl->SSIDProfiles[j];
            if (strcmp(profile->SSID, ssid) == 0) {
                strcat(profile->FrequencyBands, ",");
                strcat(profile->FrequencyBands, freqband);
                profile->Indices[1] = ssid_index;
                if (freqband[0] == '2') {
                    profile_5GHz_count--;
                } else {
                    profile_2GHz_count--;
                }
                found = true;
                break;
            }
        }
        if (!found) {
            profile = &emctl->SSIDProfiles[profile_count++];
            memset(profile, 0, sizeof(emctl->SSIDProfiles[0]));
            profile->Enable = TRUE;
            if (wifi_get_security_index(&ap_index, ssid_index) == ANSC_STATUS_FAILURE) {
                continue;
            }
            if (wifi_get_security_mode(mode, ap_index) == ANSC_STATUS_FAILURE) {
                continue;
            }
            if (wifi_get_security_keypassphrase(key, ap_index) == ANSC_STATUS_FAILURE) {
                continue;
            }
            strcpy(profile->SSID, ssid);
            strcpy(profile->FrequencyBands, freqband);
            if (freqband[0] == '2') {
                profile_2GHz_count++;
            } else {
                profile_5GHz_count++;
            }
            strcpy(profile->SecurityMode, mode);
            strcpy(profile->KeyPassphrase, key);
            profile->Indices[0] = ssid_index;
            profile->Indices[1] = -1;
        }
        profile->Fronthaul = TRUE;
        profile->Extender = TRUE;
        profile->Gateway = TRUE;
        profile->VLANID = -1;
    }
    emctl->SSIDProfileNumberOfEntries = profile_count;
    if (profile_count == 1) {
        profile = &emctl->SSIDProfiles[0];
        strncpy(profile->Type, "home", sizeof(profile->Type) - 1);
        strncpy(profile->Label, "Home", sizeof(profile->Label) - 1);
        if (dedicated_backhaul == true || profile_2GHz_count == 1) {
            profile_create_backhaul(emctl);
        } else {
            profile->Backhaul = TRUE;
        }
    } else if (profile_count > 1) {
        if (profile_2GHz_count == 0 && profile_5GHz_count == 0) {
            /* multi frequency bands only */
            if (dedicated_backhaul == true && profile_count < COSA_EMCTL_MAX_PROFILES_COUNT - 1) {
                profile_create_backhaul(emctl);
            } else {
                /* use first multi-band profile (home) */
                profile = &emctl->SSIDProfiles[0];
                profile->Backhaul = TRUE;
            }
        } else if (profile_count - (profile_2GHz_count + profile_5GHz_count) == 0) {
            /* single frequncy bands only */
            if (profile_5GHz_count > 2) {
                /* use last 5GHz profile */
                for (i = 0; i < profile_count; i++) {
                    profile = &emctl->SSIDProfiles[profile_count - i - 1];
                    if (strcmp(profile->FrequencyBands, "5") == 0) {
                        profile_set_backhaul(profile);
                        break;
                    }
                }
            } else if (dedicated_backhaul == true ||  profile_5GHz_count == 0) {
                if (profile_count < COSA_EMCTL_MAX_PROFILES_COUNT - 1) {
                    profile_create_backhaul(emctl);
                } else {
                    fprintf(stderr, "All profiles filled with 2.4GHz\n");
                    return;
                }
            } else {
                /* use first 5GHz profile (home) */
                for (i = 0; i < profile_count; i++) {
                    profile = &emctl->SSIDProfiles[i];
                    if (strcmp(profile->FrequencyBands, "5") == 0) {
                        profile->Backhaul = TRUE;
                        break;
                    }
                }
            }
        } else if (profile_count - (profile_2GHz_count + profile_5GHz_count) > 1) {
            /* mixed with 2 or more multi-bands */
            if (profile_5GHz_count > 0) {
                /* use last 5GHz profile */
                for (i = 0; i < profile_count; i++) {
                    profile = &emctl->SSIDProfiles[profile_count - i - 1];
                    if (strcmp(profile->FrequencyBands, "5") == 0) {
                        profile_set_backhaul(profile);
                        break;
                    }
                }
            } else if (dedicated_backhaul == true && profile_count < COSA_EMCTL_MAX_PROFILES_COUNT - 1) {
                profile_create_backhaul(emctl);
            } else {
                /* use first multi-band profile (home) */
                for (i = 0; i < profile_count; i++) {
                    profile = &emctl->SSIDProfiles[i];
                    if (strcmp(profile->FrequencyBands, "2") != 0 && strcmp(profile->FrequencyBands, "5") != 0) {
                        profile->Backhaul = TRUE;
                        break;
                    }
                }
            }
        } else {
            /* mixed with 1 multi-band */
            if (profile_5GHz_count > 1) {
                /* use last 5GHz profile */
                for (i = 0; i < profile_count; i++) {
                    profile = &emctl->SSIDProfiles[profile_count - i - 1];
                    if (strcmp(profile->FrequencyBands, "5") == 0) {
                        profile_set_backhaul(profile);
                        break;
                    }
                }
            } else if (dedicated_backhaul == true && profile_count < COSA_EMCTL_MAX_PROFILES_COUNT - 1) {
                profile_create_backhaul(emctl);
            } else {
                /* use first multi-band profile (home) */
                for (i = 0; i < profile_count; i++) {
                    profile = &emctl->SSIDProfiles[i];
                    if (strcmp(profile->FrequencyBands, "2") != 0 && strcmp(profile->FrequencyBands, "5") != 0) {
                        profile->Backhaul = TRUE;
                        break;
                    }
                }
            }
        }
        if (profile_count - (profile_2GHz_count + profile_5GHz_count) > 1) {
            /* use last multi-band profile for guest */
            for (i = 0; i < profile_count; i++) {
                profile = &emctl->SSIDProfiles[profile_count - i - 1];
                if (strcmp(profile->FrequencyBands, "2") != 0 && strcmp(profile->FrequencyBands, "5") != 0) {
                    strncpy(profile->Type, "guest", sizeof(profile->Type) - 1);
                    strncpy(profile->Label, "Guest", sizeof(profile->Label) - 1);
                    break;
                }
            }
        } else if (profile_2GHz_count > 1 || profile_5GHz_count > 1) {
            bool found_2GHz = false;
            bool found_5GHz = false;
            unsigned int guest_index = 0;
            /* use last of each bands as guest */
            for (i = 0; i < profile_count; i++) {
                profile = &emctl->SSIDProfiles[profile_count - i - 1];
                if ((profile_2GHz_count > 1) && !found_2GHz && (strcmp(profile->FrequencyBands, "2") != 0)) {
                    strncpy(profile->Type, "guest", sizeof(profile->Type) - 1);
                    if (profile_5GHz_count > 1) {
                        snprintf(profile->Label, sizeof(profile->Label) - 1, "Guest %d", guest_index++);
                        if (found_5GHz) {
                            break;
                        }
                    } else {
                        strncpy(profile->Label, "Guest", sizeof(profile->Label) - 1);
                        break;
                    }
                    found_2GHz = true;
                }
                if ((profile_5GHz_count > 1) && !found_5GHz && (profile->Backhaul == false) && (strcmp(profile->FrequencyBands, "5") != 0)) {
                    strncpy(profile->Type, "guest", sizeof(profile->Type) - 1);
                    if (profile_2GHz_count > 1) {
                        snprintf(profile->Label, sizeof(profile->Label) - 1, "Guest %d", guest_index++);
                        if (found_2GHz) {
                            break;
                        }
                    } else {
                        strncpy(profile->Label, "Guest", sizeof(profile->Label) - 1);
                        break;
                    }
                    found_5GHz = true;
                }
            }
        }
        unsigned int home_index = 0;
        for (i = 0; i < profile_count; i++) {
            profile = &emctl->SSIDProfiles[i];
            if (profile->Type[0] != 0) {
                continue;
            }
            strncpy(profile->Type, "home", sizeof(profile->Type) - 1);
            snprintf(profile->Label, sizeof(profile->Label) - 1, "Home %d", home_index++);
        }
    }

    return;
}

static void device_wifi_static_mapper(PCOSA_DML_EMCTL_CFG emctl)
{
    bool ssid_enable;
    bool backhaul_found = false;
    unsigned int ap_index;
    unsigned int ssid_index;
    unsigned int profile_count;
    char key[64];
    char ssid[32];
    char mode[32];
    char freqband[8];
    PCOSA_DML_EMCTL_PROFILE_CFG defp, profile;
    unsigned int i, j;

    profile_count = sizeof(g_default_profiles) / sizeof(g_default_profiles[0]);
    if (profile_count >= COSA_EMCTL_MAX_PROFILES_COUNT) {
        fprintf(stderr, "Too many profiles defined\n");
        return;
    }
    emctl->SSIDProfileNumberOfEntries = profile_count;
    for (i = 0; i < profile_count; i++) {
        defp = &g_default_profiles[i];
        profile = &emctl->SSIDProfiles[i];
        memset(profile, 0, sizeof(emctl->SSIDProfiles[0]));
        for (j = 0; j < 2; j++) {
            if (defp->Indices[0] < 0 && defp->Indices[1] < 0) {
                /* No SSID reference, all values must be provided at definition */
                memcpy(profile, defp, sizeof(g_default_profiles[0]));
                /* TODO: Add random key generation, RDK doesn't like exposed ones */
                if (defp->Backhaul == TRUE) {
                    backhaul_found = true;
                }
                break;
            }
            if (defp->Indices[j] < 0) {
                continue;
            }
            ssid_index = defp->Indices[j];
            if (wifi_get_ssid_enable(&ssid_enable, ssid_index) == ANSC_STATUS_FAILURE) {
                continue;
            }
            if (wifi_get_radio_freqband(freqband, ssid_index) == ANSC_STATUS_FAILURE) {
                continue;
            }
            if (wifi_get_ssid_ssid(ssid, ssid_index) == ANSC_STATUS_FAILURE) {
                continue;
            }
            if (wifi_get_security_index(&ap_index, ssid_index) == ANSC_STATUS_FAILURE) {
                continue;
            }
            if (wifi_get_security_mode(mode, ap_index) == ANSC_STATUS_FAILURE) {
                continue;
            }
            if (wifi_get_security_keypassphrase(key, ap_index) == ANSC_STATUS_FAILURE) {
                continue;
            }
            if (j == 0) {
                memcpy(profile, defp, sizeof(g_default_profiles[0]));
                profile->Enable = ssid_enable ? TRUE : FALSE;
                strncpy(profile->SSID, ssid, sizeof(profile->SSID) - 1);
                strncpy(profile->FrequencyBands, freqband, sizeof(profile->FrequencyBands) - 1);
                strncpy(profile->SecurityMode, mode, sizeof(profile->SecurityMode) - 1);
                strncpy(profile->KeyPassphrase, key, sizeof(profile->KeyPassphrase) - 1);
                profile->Extender = TRUE;
                profile->Gateway = TRUE;
                profile->VLANID = -1;
            } else {
                strcat(profile->FrequencyBands, ",");
                strcat(profile->FrequencyBands, freqband);
                if (strcmp(profile->SSID, ssid) != 0 ||
                    strcmp(profile->SecurityMode, mode) != 0 ||
                    strcmp(profile->KeyPassphrase, key) != 0 ||
                    strcmp(profile->Type, defp->Type) != 0 ||
                    strcmp(profile->Label, defp->Label) != 0 ||
                    profile->Fronthaul != defp->Fronthaul ||
                    profile->Backhaul != defp->Backhaul) {
                    fprintf(stderr, "Some parameters don't match\n");
                }
            }
            if (profile->Backhaul == TRUE) {
                backhaul_found = true;
            }
        }
    }
    if (!backhaul_found) {
        if (profile_count < COSA_EMCTL_MAX_PROFILES_COUNT - 1) {
            profile_create_backhaul(emctl);
        } else {
            fprintf(stderr, "No profile reserved for backhaul\n");
            return;
        }
    }

    return;
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

    pEmctl_Cfg->ConfigRenewInterval = 5;
    pEmctl_Cfg->ConfigRenewMaxRetry = 3;
    pEmctl_Cfg->ConfigureBackhaulStation = 1;
    pEmctl_Cfg->DeadAgentDetectionInterval = 30;
    pEmctl_Cfg->Enable = 1;
    AnscCopyString(pEmctl_Cfg->InterfaceList, "^lo$|^eth.*|^wl.*|^sw_.*|^n[rs]gmii.*");
    pEmctl_Cfg->IsMaster = 1;
    pEmctl_Cfg->LinkMetricsQueryInterval = 20;
    pEmctl_Cfg->PrimaryVLANID = -1;
    AnscCopyString(pEmctl_Cfg->PrimaryVLANInterfacePattern, "^lo$|^eth.*|^wl.*|^sw_.*|^n[rs]gmii.*");
    if (1) {
        device_wifi_dynamic_mapper(pEmctl_Cfg);
    } else {
        device_wifi_static_mapper(pEmctl_Cfg);
    }
    pEmctl_Cfg->TopologyDiscoveryInterval = 60;
    pEmctl_Cfg->TopologyQueryInterval = 60;
    pEmctl_Cfg->TopologyStableCheckInterval = 120;
    get_mac_addresses(pEmctl_Cfg);

    return ANSC_STATUS_SUCCESS;
}

int CosaEmctlGetAllowedBandwidth2G(char *value)
{
    AnscCopyString(value, g_pEmctl_Cfg->AllowedBandwidth2G);
    return 0;
}

int CosaEmctlGetAllowedBandwidth5G(char *value)
{
    AnscCopyString(value, g_pEmctl_Cfg->AllowedBandwidth5G);
    return 0;
}

int CosaEmctlGetAllowedBandwidth6G(char *value)
{
    AnscCopyString(value, g_pEmctl_Cfg->AllowedBandwidth6G);
    return 0;
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

int CosaEmctlGetAllowedChannelList6G(char *value)
{
    AnscCopyString(value, g_pEmctl_Cfg->AllowedChannelList6G);
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

int CosaEmctlGetDefault6GPreferredChannelList(char *value)
{
    AnscCopyString(value, g_pEmctl_Cfg->Default6GPreferredChannelList);
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

int CosaEmctlProfileGetEnable(uint8_t index, bool *enable)
{
    PCOSA_DML_EMCTL_PROFILE_CFG profile;

    if (index > g_pEmctl_Cfg->SSIDProfileNumberOfEntries) {
        return -1;
    }
    profile = &g_pEmctl_Cfg->SSIDProfiles[index];
    *enable = profile->Enable;

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

static void profile_merge(PCOSA_DML_EMCTL_CFG emctl, unsigned int index1, unsigned int index2)
{
    unsigned int len;
    unsigned int index;
    PCOSA_DML_EMCTL_PROFILE_CFG dst;
    PCOSA_DML_EMCTL_PROFILE_CFG profile;

    if (index1 > index2) {
        index = index1;
        profile = &emctl->SSIDProfiles[index1];
        dst = &emctl->SSIDProfiles[index2];
    } else {
        index = index2;
        profile = &emctl->SSIDProfiles[index2];
        dst = &emctl->SSIDProfiles[index1];
    }
    strcat(dst->FrequencyBands, ",");
    strcat(dst->FrequencyBands, profile->FrequencyBands);
    dst->Indices[1] = profile->Indices[0];

    len = (emctl->SSIDProfileNumberOfEntries - index + 1) * sizeof(emctl->SSIDProfiles[0]);
    memcpy(&emctl->SSIDProfiles[index], &emctl->SSIDProfiles[index + 1], len);
    profile = &emctl->SSIDProfiles[emctl->SSIDProfileNumberOfEntries];
    memset(profile, 0, sizeof(emctl->SSIDProfiles[0]));
    profile->Indices[0] = profile->Indices[1] = -1;
    emctl->SSIDProfileNumberOfEntries--;

    return;
}

static void profile_split(PCOSA_DML_EMCTL_CFG emctl, unsigned int profile_index, unsigned int ssid_index)
{
    char *sp;
    char *band;
    char bands[17];
    PCOSA_DML_EMCTL_PROFILE_CFG dst;
    PCOSA_DML_EMCTL_PROFILE_CFG profile;

    profile = &emctl->SSIDProfiles[profile_index];
    if (profile->Indices[0] < 0 || profile->Indices[1] < 0) {
        return;
    }

    dst = &emctl->SSIDProfiles[emctl->SSIDProfileNumberOfEntries];
    dst->Backhaul = profile->Backhaul;
    dst->Enable = profile->Enable;
    dst->Extender = profile->Extender;
    dst->Fronthaul = profile->Fronthaul;
    dst->Gateway = profile->Gateway;
    strcpy(dst->KeyPassphrase, profile->KeyPassphrase);
    strcpy(dst->Label, profile->Label);
    strcpy(dst->SecurityMode, profile->SecurityMode);
    strcpy(dst->SSID, profile->SSID);
    strcpy(dst->Type, profile->Type);
    dst->VLANID = profile->VLANID;

    strcpy(bands, profile->FrequencyBands);
    band = strtok_r(bands, ",", &sp);
    if (ssid_index == (unsigned int)profile->Indices[0]) {
        dst->Indices[0] = profile->Indices[1];
        strcpy(profile->FrequencyBands, band);
        band = strtok_r(NULL, ",", &sp);
        strcpy(dst->FrequencyBands, band);
    } else {
        dst->Indices[0] = profile->Indices[0];
        profile->Indices[0] = profile->Indices[1];
        strcpy(dst->FrequencyBands, band);
        band = strtok_r(NULL, ",", &sp);
        strcpy(profile->FrequencyBands, band);
    }
    dst->Indices[1] = profile->Indices[1] = -1;

    emctl->SSIDProfileNumberOfEntries++;

    return;
}

static int profile_update(PCOSA_DML_EMCTL_CFG emctl, update_params_t *update)
{
    char *value;
    char *sec_type;
    char *auth_mode;
    unsigned int ssid_index;
    PCOSA_DML_EMCTL_PROFILE_CFG check;
    PCOSA_DML_EMCTL_PROFILE_CFG profile;
    unsigned int i, j;

    for (i = 0; i < emctl->SSIDProfileNumberOfEntries; i++) {
        profile = &emctl->SSIDProfiles[i];
        ssid_index = update->index + 1;
        if (ssid_index != (unsigned int)profile->Indices[0] &&
            ssid_index != (unsigned int)profile->Indices[1]) {
            continue;
        }
        if (strcmp(update->type, "SSIDEnable") == 0) {
            profile_split(emctl, i, ssid_index);
            if (strcmp(update->value, "0") == 0) {
                profile->Enable = FALSE;
            } else {
                profile->Enable = TRUE;
            }
        } else if (strcmp(update->type, "SSID") == 0) {
            profile_split(emctl, i, ssid_index);
            strncpy(profile->SSID, update->value, sizeof(profile->SSID) - 1);
        } else if (strcmp(update->type, "SecMode") == 0) {
            profile_split(emctl, i, ssid_index);
            /* Do not modify update */
            value = strdup(update->value);
            sec_type = value;
            auth_mode = strchr(value, ';');
            if (auth_mode) {
                *(auth_mode++) = 0;
            }
            if (strcmp(sec_type, "None") == 0) {
                strcpy(profile->SecurityMode, "none");
            } else if (strcmp(sec_type, "WPAand11i") == 0) {
                if (strcmp(auth_mode, "PSKAuthentication") == 0) {
                    strcpy(profile->SecurityMode, "wpa-wpa2-psk");
                } else {
                    fprintf(stderr, "Unknown auth mode arrived\n");
                    break;
                }
            } else if (strcmp(sec_type, "11i") == 0) {
                if (strcmp(auth_mode, "PSKAuthentication") == 0) {
                    strcpy(profile->SecurityMode, "wpa2-psk");
                } else {
                    fprintf(stderr, "Unknown auth mode arrived\n");
                    break;
                }
            } else {
                fprintf(stderr, "Unknown sec type arrived\n");
                break;
            }
            free(value);
        } else if (strcmp(update->type, "KeyPassphrase") == 0) {
            profile_split(emctl, i, ssid_index);
            strncpy(profile->KeyPassphrase, update->value, sizeof(profile->KeyPassphrase) - 1);
        } else if (strcmp(update->type, "RadioEnable") == 0) {
            return 0;
        } else if (strcmp(update->type, "AutoChannelEnable") == 0) {
            return 0;
        } else if (strcmp(update->type, "Channel") == 0) {
            return 0;
        } else if (strcmp(update->type, "ChannelMode") == 0) {
            return 0;
        } else {
            fprintf(stderr, "Unknown update arrived\n");
            break;
        }
        if (profile->Indices[0] < 0 || profile->Indices[1] < 0) {
            /* check for profile merge possibilities */
            for (j = 0; j < emctl->SSIDProfileNumberOfEntries; j++) {
                if (i == j) {
                    continue;
                }
                check = &emctl->SSIDProfiles[j];
                if (strcmp(profile->SSID, check->SSID) == 0 &&
                    strcmp(profile->SecurityMode, check->SecurityMode) == 0 &&
                    strcmp(profile->KeyPassphrase, check->KeyPassphrase) == 0) {
                    profile_merge(emctl, i, j);
                }
            }
        }
        break;
    }

    return 1;
}

static void *update_handler(void *farg)
{
    UNREFERENCED_PARAMETER(farg);
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
    update_params_t *update;
    bool notify_needed;
    char *p_tok, *st;
    pthread_attr_t attr;
    pthread_attr_t *attrp = NULL;
    pthread_t thread;
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
    notify_needed = profile_update(g_pEmctl_Cfg, update);
    if (notify_needed == false) {
        return 0;
    }
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

