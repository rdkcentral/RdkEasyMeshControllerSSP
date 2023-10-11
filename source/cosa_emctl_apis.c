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
#include <stdbool.h>
#include <pthread.h>

#include <ansc_status.h>
#include <ccsp_base_api.h>

#define LOG_TAG "api"

#include "1905_platform.h"
#include "map_utils.h"
#include "map_config.h"
#include "cosa_emctl_apis.h"

typedef struct update_params_s {
    char            *type;
    unsigned int     index;
    char            *value;
} update_params_t;

extern ANSC_HANDLE bus_handle;
extern map_emctl_ssp_t *g_emctl_ssp;

static pthread_cond_t g_emctl_cond = PTHREAD_COND_INITIALIZER;
static pthread_mutex_t g_emctl_mutex = PTHREAD_MUTEX_INITIALIZER;

static int setparam_val(const char *name, enum dataType_e type, const char *value, int commit)
{
    int rc;
    int comp_cnt = 0;
    const char *subsys = "eRT.";
    const char *id =  "eRT.com.cisco.spvtg.ccsp.CR";
    componentStruct_t **comps = NULL;
    parameterValStruct_t val = { 0 };
    char *fault = NULL;
    int retval = -1;

    rc = CcspBaseIf_discComponentSupportingNamespace(bus_handle, id, name,
        subsys, &comps, &comp_cnt);
    if (rc != CCSP_SUCCESS) {
        log_ssp_e("Unable to find component");
        goto bail;
    }
    if (comp_cnt != 1) {
        log_ssp_e("Invalid search result");
        goto bail;
    }

    val.type = type;
    val.parameterValue = strdup(value);
    val.parameterName = strdup(name);
    rc = CcspBaseIf_setParameterValues(bus_handle, comps[0]->componentName,
        comps[0]->dbusPath, 0, 0, &val, 1,
        commit, &fault);
    if (rc != CCSP_SUCCESS) {
        log_ssp_e("Set parameter values failed");
        goto bail;
    }
    retval = 0;

bail:
    SFREE(fault);
    SFREE(val.parameterName);
    SFREE(val.parameterValue);
    if (comps != NULL) {
        free_componentStruct_t(bus_handle, comp_cnt, comps);
    }

    return retval;
}

static int wifi_notify(update_params_t *update)
{
    char path[128];
    char value[128];
    enum dataType_e type;
    int rc;

    if (strcmp(update->type, "ApplySetting") == 0) {
        snprintf(path, sizeof(path), "Device.WiFi.Radio.%d."
            "X_RDK_ApplySetting", update->index + 1);
        snprintf(value, sizeof(value), update->value);
        type = ccsp_boolean;
    } else if (strcmp(update->type, "ApplySettingSSID") == 0) {
        snprintf(path, sizeof(path), "Device.WiFi.Radio.%d."
            "X_RDK_ApplySettingSSID", update->index + 1);
        snprintf(value, sizeof(value), update->value);
        type = ccsp_int;
    } else {
        snprintf(path, sizeof(path), "Device.WiFi."
            "X_RDK_EasymeshControllerNotification");
        snprintf(value, sizeof(value), "%s,%d,%s",
            update->type, update->index, update->value);
        type = ccsp_string;
    }

    rc = setparam_val(path, type, value, TRUE);
    if (rc < 0) {
        log_ssp_e("Send notification failed");
        return -1;
    }

    return 0;
}

static void *update_handler(void *farg)
{
    UNREFERENCED_PARAMETER(farg);
    struct timeval tval;
    struct timespec tspec;
    update_params_t *update;
    map_profile_cfg_t *modified;
    int rc;

    while (1) {
        pthread_mutex_lock(&g_emctl_mutex);
        g_emctl_ssp->updating = 1;
        gettimeofday(&tval, NULL);
        tspec.tv_sec = tval.tv_sec + 5;
        rc = pthread_cond_timedwait(&g_emctl_cond, &g_emctl_mutex, &tspec);
        if (rc == 0) {
            pthread_mutex_unlock(&g_emctl_mutex);
            continue;
        }

        /* Perform network update */
        map_profile_update();

        /* Clear modified profiles list */
        modified = pop_object(g_emctl_ssp->modified_profiles);
        while (modified != NULL) {
            free(modified);
            modified = pop_object(g_emctl_ssp->modified_profiles);
        }
        delete_array_list(g_emctl_ssp->modified_profiles);
        g_emctl_ssp->modified_profiles = NULL;

        /* Clear update objects list */
        update = pop_object(g_emctl_ssp->updates);
        while (update != NULL) {
            wifi_notify(update);
            free(update->type);
            free(update->value);
            free(update);
            update = pop_object(g_emctl_ssp->updates);
        }
        delete_array_list(g_emctl_ssp->updates);
        g_emctl_ssp->updates = NULL;

        g_emctl_ssp->updating = 0;
        pthread_mutex_unlock(&g_emctl_mutex);
        break;
    }

    return NULL;
}

static bool profile_recall_modified(map_profile_cfg_t **profile)
{
    map_profile_cfg_t *modified = NULL;
    list_iterator_t   *it;

    it = new_list_iterator(g_emctl_ssp->modified_profiles);
    while (it->iter != NULL) {
        modified = (map_profile_cfg_t *)get_next_list_object(it);
        if (modified && ((*profile)->profile_idx == modified->profile_idx)) {
            break;
        }
    }
    free_list_iterator(it);

    if (modified) {
        *profile = modified;
        return false;
    }

    return true;
}

static void profile_store_modified(map_profile_cfg_t **profile)
{
    map_profile_cfg_t *modified = NULL;

    if (!g_emctl_ssp->modified_profiles) {
        g_emctl_ssp->modified_profiles = new_array_list();
    }

    modified = calloc(1, sizeof(map_profile_cfg_t));
    push_object(g_emctl_ssp->modified_profiles, modified);
    map_profile_clone(modified, *profile);

    *profile = modified;
}

static int profile_update(update_params_t *update)
{
    char *value;
    char *sec_type;
    char *auth_mode;
    uint16_t sup_auth_modes;
    uint16_t sup_enc_types;
    map_profile_cfg_t *profile;
    bool store;
    bool enabled;

    log_ssp_e("profile_update");
    if (strcmp(update->type, "SSIDEnable") == 0) {
        if (map_profile_get_by_sidx(update->index + 1, &profile) < 0) {
            log_ssp_e("Matching profile not found");
            return -1;
        }

        if (g_emctl_ssp->modified_profiles) {
            store = profile_recall_modified(&profile);
        } else {
            store = true;
        }

        if (strcmp(update->value, "0") == 0) {
            if (profile->enabled == false) {
                return 0;
            }
            enabled = false;
        } else {
            if (profile->enabled == true) {
                return 0;
            }
            enabled = true;
        }

        if (store) {
            profile_store_modified(&profile);
        }

        profile->enabled = enabled;
        map_profile_save(profile);
    } else if (strcmp(update->type, "SSID") == 0) {
        if (map_profile_get_by_sidx(update->index + 1, &profile) < 0) {
            log_ssp_e("Matching profile not found");
            return -1;
        }

        if (g_emctl_ssp->modified_profiles) {
            store = profile_recall_modified(&profile);
        } else {
            store = true;
        }

        if (strcmp(profile->bss_ssid, update->value) == 0) {
            return 0;
        }

        if (store) {
            profile_store_modified(&profile);
        }

        strncpy(profile->bss_ssid, update->value, sizeof(profile->bss_ssid));
        map_profile_save(profile);
    } else if (strcmp(update->type, "SecMode") == 0) {
        if (map_profile_get_by_sidx(update->index + 1, &profile) < 0) {
            log_ssp_e("Matching profile not found");
            return -1;
        }

        if (g_emctl_ssp->modified_profiles) {
            store = profile_recall_modified(&profile);
        } else {
            store = true;
        }

        value = strdup(update->value);
        sec_type = value;
        auth_mode = strchr(value, ';');
        if (auth_mode) {
            *(auth_mode++) = 0;
        }
        if (strcmp(sec_type, "None") == 0) {
            sup_auth_modes = IEEE80211_AUTH_MODE_OPEN;

            if (auth_mode && strcmp(auth_mode, "None") != 0) {
                log_ssp_e("Unknown auth mode arrived");
                free(value);
                return -1;
            }
            sup_enc_types = IEEE80211_ENCRYPTION_MODE_NONE;
        } else if (strcmp(sec_type, "WPAand11i") == 0) {
            sup_auth_modes = (IEEE80211_AUTH_MODE_WPAPSK | IEEE80211_AUTH_MODE_WPA2PSK);

            if (strcmp(auth_mode, "PSKAuthentication") != 0) {
                log_ssp_e("Unknown auth mode arrived");
                free(value);
                return -1;
            }
            sup_enc_types = IEEE80211_ENCRYPTION_MODE_AES;
        } else if (strcmp(sec_type, "11i") == 0) {
            sup_auth_modes = IEEE80211_AUTH_MODE_WPA2PSK;

            if (strcmp(auth_mode, "PSKAuthentication") != 0) {
                log_ssp_e("Unknown auth mode arrived");
                free(value);
                return -1;
            }
            sup_enc_types = IEEE80211_ENCRYPTION_MODE_AES;
        } else if (strcmp(sec_type, "WPA-WPA2-Personal") == 0) {
            sup_auth_modes = (IEEE80211_AUTH_MODE_WPAPSK | IEEE80211_AUTH_MODE_WPA2PSK);
            sup_enc_types = IEEE80211_ENCRYPTION_MODE_AES;
        } else if (strcmp(sec_type, "WPA2-Personal") == 0) {
            sup_auth_modes = IEEE80211_AUTH_MODE_WPA2PSK;
            sup_enc_types = IEEE80211_ENCRYPTION_MODE_AES;
        } else if (strcmp(sec_type, "WPA3-Personal-Transition") == 0) {
            sup_auth_modes = IEEE80211_AUTH_MODE_WPA2PSK | IEEE80211_AUTH_MODE_SAE;
            sup_enc_types = IEEE80211_ENCRYPTION_MODE_AES;
        } else if (strcmp(sec_type, "WPA3-Personal") == 0) {
            sup_auth_modes = IEEE80211_AUTH_MODE_SAE;
            sup_enc_types = IEEE80211_ENCRYPTION_MODE_AES;
        } else {
            log_ssp_e("Unknown sec type arrived");
            free(value);
            return -1;
        }
        free(value);

        if (profile->supported_auth_modes == sup_auth_modes &&
            profile->supported_encryption_types == sup_enc_types) {
            return 0;
        }

        if (store) {
            profile_store_modified(&profile);
        }

        profile->supported_auth_modes = sup_auth_modes;
        profile->supported_encryption_types = sup_enc_types;
        map_profile_save(profile);
    } else if (strcmp(update->type, "KeyPassphrase") == 0) {
        if (map_profile_get_by_sidx(update->index + 1, &profile) < 0) {
            log_ssp_e("Matching profile not found");
            return -1;
        }

        if (g_emctl_ssp->modified_profiles) {
            store = profile_recall_modified(&profile);
        } else {
            store = true;
        }

        if (strcmp(profile->wpa_key, update->value) == 0) {
            return 0;
        }

        if (store) {
            profile_store_modified(&profile);
        }

        strncpy(profile->wpa_key, update->value, sizeof(profile->wpa_key));
        map_profile_save(profile);
    } else if (strcmp(update->type, "RadioEnable") == 0) {
        return 0;
    } else if (strcmp(update->type, "AutoChannelEnable") == 0) {
        return 0;
    } else if (strcmp(update->type, "Channel") == 0) {
        return 0;
    } else if (strcmp(update->type, "ChannelMode") == 0) {
        return 0;
    } else if (strcmp(update->type, "ApplySetting") == 0) {
    } else if (strcmp(update->type, "ApplySettingSSID") == 0) {
    } else {
        log_ssp_e("Unknown update arrived");
        return -1;
    }

    return 1;
}

int map_apply_wifi_change(char *context)
{
    update_params_t *update;
    int notify_needed;
    char *p_tok, *st;
    pthread_attr_t attr;
    pthread_attr_t *attrp = NULL;
    pthread_t thread;
    int i = 0;

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
    notify_needed = profile_update(update);
    if (notify_needed <= 0) {
        pthread_mutex_unlock(&g_emctl_mutex);
        return notify_needed;
    }
    if (g_emctl_ssp->updating) {
        push_object(g_emctl_ssp->updates, update);
        pthread_cond_signal(&g_emctl_cond);
        pthread_mutex_unlock(&g_emctl_mutex);
        return 0;
    }
    if (g_emctl_ssp->updates != NULL) {
        delete_array_list(g_emctl_ssp->updates);
    }
    g_emctl_ssp->updates = new_array_list();
    push_object(g_emctl_ssp->updates, update);
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

ANSC_HANDLE CosaEmctlCreate(void)
{
    map_emctl_ssp_t *emctl_ssp = NULL;

    emctl_ssp = (map_emctl_ssp_t *)calloc(1, sizeof(map_emctl_ssp_t));

    return emctl_ssp;
}

ANSC_STATUS CosaEmctlInitialize(ANSC_HANDLE hThisObject)
{
    map_emctl_ssp_t *emctl_ssp = (map_emctl_ssp_t *)hThisObject;

    emctl_ssp->cfg = map_cfg_get();

    return ANSC_STATUS_SUCCESS;
}

ANSC_STATUS CosaEmctlDestroy(ANSC_HANDLE hThisObject)
{
    map_emctl_ssp_t *emctl_ssp = (map_emctl_ssp_t *)hThisObject;

    free(emctl_ssp);

    return ANSC_STATUS_SUCCESS;
}

