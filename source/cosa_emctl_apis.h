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

#include <stdbool.h>
#include <pthread.h>

#include "collection.h"

#include "slap_definitions.h"

#ifndef COSA_EMCTL_MAX_PROFILES_COUNT
#define COSA_EMCTL_MAX_PROFILES_COUNT       16
#endif
#ifndef COSA_EMCTL_MAX_FREQUENCY_BANDS_LEN
#define COSA_EMCTL_MAX_FREQUENCY_BANDS_LEN  17
#endif
#ifndef COSA_EMCTL_MAX_WIFI_PASSWORD_LEN
#define COSA_EMCTL_MAX_WIFI_PASSWORD_LEN    65
#endif
#ifndef COSA_EMCTL_MAX_PROFILE_LABEL_LEN
#define COSA_EMCTL_MAX_PROFILE_LABEL_LEN    65
#endif
#ifndef COSA_EMCTL_MAX_WIFI_SSID_LEN
#define COSA_EMCTL_MAX_WIFI_SSID_LEN        33
#endif
#ifndef COSA_EMCTL_MAX_SECURITY_MODE_LEN
#define COSA_EMCTL_MAX_SECURITY_MODE_LEN    17
#endif
#ifndef COSA_EMCTL_MAX_PROFILE_TYPE_LEN
#define COSA_EMCTL_MAX_PROFILE_TYPE_LEN     17
#endif
#ifndef COSA_EMCTL_MAX_IFLIST_LEN
#define COSA_EMCTL_MAX_IFLIST_LEN           33
#endif
#ifndef COSA_EMCTL_MAC_ADDR_LEN
#define COSA_EMCTL_MAC_ADDR_LEN             18
#endif
#ifndef COSA_EMCTL_MAX_CHANNEL_SET
#define COSA_EMCTL_MAX_CHANNEL_SET          192
#endif

typedef struct
{
    char *type;
    unsigned int index;
    char *value;
} update_params_t;

typedef int (*cb_func_t)(queue_t *);

struct _COSA_DML_EMCTL_PROFILE_CFG {
    BOOL    Backhaul;
    BOOL    Extender;
    char    FrequencyBands[COSA_EMCTL_MAX_FREQUENCY_BANDS_LEN];
    BOOL    Fronthaul;
    BOOL    Gateway;
    char    KeyPassphrase[COSA_EMCTL_MAX_WIFI_PASSWORD_LEN];
    char    Label[COSA_EMCTL_MAX_PROFILE_LABEL_LEN];
    char    SecurityMode[COSA_EMCTL_MAX_SECURITY_MODE_LEN];
    char    SSID[COSA_EMCTL_MAX_WIFI_SSID_LEN];
    char    Type[COSA_EMCTL_MAX_PROFILE_TYPE_LEN];
    int     VLANID;
}_struct_pack_;

typedef struct _COSA_DML_EMCTL_PROFILE_CFG COSA_DML_EMCTL_PROFILE_CFG, *PCOSA_DML_EMCTL_PROFILE_CFG;

struct _COSA_DML_EMCTL_CFG {
    char                        AllowedChannelList2G[COSA_EMCTL_MAX_CHANNEL_SET];
    char                        AllowedChannelList5G[COSA_EMCTL_MAX_CHANNEL_SET];
    char                        BandLock5G[COSA_EMCTL_MAX_CHANNEL_SET];
    ULONG                       ConfigRenewInterval;
    ULONG                       ConfigRenewMaxRetry;
    BOOL                        ConfigureBackhaulStation;
    ULONG                       DeadAgentDetectionInterval;
    char                        Default2GPreferredChannelList[COSA_EMCTL_MAX_CHANNEL_SET];
    char                        Default5GPreferredChannelList[COSA_EMCTL_MAX_CHANNEL_SET];
    ULONG                       DefaultPCP;
    BOOL                        Enable;
    char                        InterfaceList[COSA_EMCTL_MAX_IFLIST_LEN];
    BOOL                        IsMaster;
    ULONG                       LinkMetricsQueryInterval;
    char                        LocalAgentMACAddress[COSA_EMCTL_MAC_ADDR_LEN];
    char                        MACAddress[COSA_EMCTL_MAC_ADDR_LEN];
    int                         PrimaryVLANID;
    char                        PrimaryVLANInterfacePattern[COSA_EMCTL_MAX_IFLIST_LEN];
    ULONG                       SSIDProfileNumberOfEntries;
    COSA_DML_EMCTL_PROFILE_CFG  SSIDProfiles[COSA_EMCTL_MAX_PROFILES_COUNT];
    ULONG                       TopologyDiscoveryInterval;
    ULONG                       TopologyQueryInterval;
    ULONG                       TopologyStableCheckInterval;

    unsigned int                updating;
    queue_t                     *updates;
    cb_func_t                   profile_update_cb;
}_struct_pack_;

typedef struct _COSA_DML_EMCTL_CFG COSA_DML_EMCTL_CFG, *PCOSA_DML_EMCTL_CFG;

ANSC_HANDLE CosaEmctlCreate(void);
ANSC_STATUS CosaEmctlInitialize(ANSC_HANDLE hThisObject);

int CosaEmctlGetAllowedChannelList2G(char *value);
int CosaEmctlGetAllowedChannelList5G(char *value);
int CosaEmctlGetBandLock5G(char *value);
int CosaEmctlGetConfigRenewInterval(unsigned int *value);
int CosaEmctlGetConfigRenewMaxRetry(unsigned int *value);
int CosaEmctlGetConfigureBackhaulStation(uint8_t *value);
int CosaEmctlGetDeadAgentDetectionInterval(unsigned int *value);
int CosaEmctlGetDefault2GPreferredChannelList(char *value);
int CosaEmctlGetDefault5GPreferredChannelList(char *value);
int CosaEmctlGetDefaultPCP(uint8_t *value);
int CosaEmctlGetEnable(unsigned int *value);
int CosaEmctlGetInterfaceList(char *value);
int CosaEmctlGetIsMaster(unsigned int *value);
int CosaEmctlGetLinkMetricsQueryInterval(unsigned int *value);
int CosaEmctlGetLocalAgentMACAddress(char *value);
int CosaEmctlGetMACAddress(char *value);
int CosaEmctlGetPrimaryVLANID(int *value);
int CosaEmctlGetPrimaryVLANInterfacePattern(char *value);
int CosaEmctlGetTopologyDiscoveryInterval(unsigned int *value);
int CosaEmctlGetTopologyQueryInterval(unsigned int *value);
int CosaEmctlTopologyStableCheckInterval(unsigned int *value);

int CosaEmctlProfileGetBackhaul(uint8_t index, bool *bh);
int CosaEmctlProfileGetExtender(uint8_t index, uint8_t *extender);
int CosaEmctlProfileGetFrequencyBands(uint8_t index, char **freq_bands);
int CosaEmctlProfileGetFronthaul(uint8_t index, bool *fh);
int CosaEmctlProfileGetGateway(uint8_t index, uint8_t *gateway);
int CosaEmctlProfileGetKeypassphrase(uint8_t index, char **key_passphrase);
int CosaEmctlProfileGetLabel(uint8_t index, char **label);
int CosaEmctlProfileGetSecurityMode(uint8_t index, char **security_mode);
int CosaEmctlProfileGetSSID(uint8_t index, char **ssid);
int CosaEmctlProfileGetType(uint8_t index, char **type);
int CosaEmctlProfileGetVLANID(uint8_t index, int *vlan_id);
int CosaEmctlProfileConfigChangeNotification(update_params_t *update);

int EmctlConfigChangeCB(char *context);
int EmctlRegisterConfigChangeCB(cb_func_t cb_func);

#endif
