
/**
@file   add_node.c - Sample application to add a node into Z-Wave network.

@author David Chow

@version    1.0 26-1-15  Initial release

@copyright © 2014-2019 Silicon Laboratories Inc.
The licensor of this software is Silicon Laboratories Inc.  Your use of this software is governed by the terms of
Silicon Labs Z-Wave Development Kit License Agreement.  A copy of the license is available at www.silabs.com.
*/

#include <stdlib.h>
#include <malloc.h>
#include <memory.h>
#include <string.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <time.h>
#include "../../include/zip_api.h"

#define  ALPHA_OFFSET           0x72AA  ///< Alphabet offset for input

#define  MAX_DTLS_PSK           64  ///<Maximum DTLS pre-shared key hex string length

/** Network initialization status*/
#define NW_INI_STS_UNKNOWN      0   ///<Unknown
#define NW_INI_STS_PROGRESS     1   ///<In progress
#define NW_INI_STS_FAIL         2   ///<Failed
#define NW_INI_STS_DONE         3   ///<Done


/** Add node status*/
#define ADD_NODE_STS_UNKNOWN    0   ///<Unknown
#define ADD_NODE_STS_PROGRESS   1   ///<In progress
#define ADD_NODE_STS_DONE       2   ///<Done


/** Remove node status*/
#define RM_NODE_STS_UNKNOWN     0   ///<Unknown
#define RM_NODE_STS_PROGRESS    1   ///<In progress
#define RM_NODE_STS_DONE        2   ///<Done


/** Network update status*/
#define NW_UPDT_STS_UNKNOWN     0   ///<Unknown
#define NW_UPDT_STS_PROGRESS    1   ///<In progress
#define NW_UPDT_STS_DONE        2   ///<Done


/** Node update status*/
#define NODE_UPDT_STS_UNKNOWN   0   ///<Unknown
#define NODE_UPDT_STS_PROGRESS  1   ///<In progress
#define NODE_UPDT_STS_DONE      2   ///<Done


/** Reset network status*/
#define RESET_NW_STS_UNKNOWN    0   ///<Unknown
#define RESET_NW_STS_PROGRESS   1   ///<In progress
#define RESET_NW_STS_DONE       2   ///<Done


/** Network health check status*/
#define NW_HEALTH_CHK_STS_UNKNOWN    0   ///<Unknown
#define NW_HEALTH_CHK_STS_PROGRESS   1   ///<In progress
#define NW_HEALTH_CHK_STS_DONE       2   ///<Done


#define SEC2_ENTER_KEY_REQ      1   ///< Bit-mask for allowing S2 key request callback
#define SEC2_ENTER_DSK          2   ///< Bit-mask for allowing S2 DSK callback

#define     MAX_ALARM_TYPE      ZW_ALRM_HOME_MNTR   //Maximum alarm type number

/** user application context*/
typedef struct
{
    volatile int        init_status;    ///< Network initialization status, NW_INI_STS_XXX
    volatile int        add_status;     ///< Add node status, ADD_NODE_STS_XXX
    volatile int        rm_status;      ///< Remove node status, RM_NODE_STS_XXX
    volatile int        nw_updt_status; ///< Network update status, NW_UPDT_STS_XXX
    volatile int        node_updt_status;        ///< Node update status, NODE_UPDT_STS_XXX
    volatile int        rst_status;     ///< Reset network status, RESET_NW_STS_XXX
    volatile int        nw_health_chk_status;    ///< Network health check status, NW_HEALTH_CHK_STS_XXX
    volatile unsigned   sec2_cb_enter;  ///< Control security 2 callback entry bitmask, see SEC2_ENTER_XXX. bit set = allowed callback, 0 = not allowed
    volatile int        sec2_cb_exit;   ///< Security 2 callback status. 1 = exited callback, 0 = waiting or still in the callback
    uint8_t             sec2_add_node;  ///< Flag to determine whether to use security 2 when adding node
    sec2_add_prm_t      sec2_add_prm;   ///< Add node with security 2 parameters
    int                 psk_len;        ///< DTLS pre-shared key length
    uint8_t             dtls_psk[MAX_DTLS_PSK/2];   //DTLS pre-shared key
    uint16_t            host_port;      //Host listening port
    int                 use_ipv4;       ///< Flag to indicate whether to use IPv4 or IPv6. 1=IPv4; 0=IPv6
    zwnet_p             zwnet;          ///< Network handle
    uint8_t             *gw_addr_buf;   ///< Z/IP gateway addresses buffer
    char                **gw_name;      ///< Z/IP gateway names corresponding to entries in gw_addr_buf
    uint8_t             gw_addr_cnt;    ///< Number of Z/IP gateway addresses in gw_addr_buf
    uint8_t             zip_gw_ip[16];  ///< Z/IP gateway address in IPv4 or IPv6
    struct plt_mtx_t    *hl_mtx;        ///< Mutex for generic use
} hl_appl_ctx_t;

void print_hl_appl_ctx(const hl_appl_ctx_t *ctx)
{
    printf("init_status: %d\n", ctx->init_status);
    printf("add_status: %d\n", ctx->add_status);
    printf("rm_status: %d\n", ctx->rm_status);
    printf("nw_updt_status: %d\n", ctx->nw_updt_status);
    printf("node_updt_status: %d\n", ctx->node_updt_status);
    printf("rst_status: %d\n", ctx->rst_status);
    printf("nw_health_chk_status: %d\n", ctx->nw_health_chk_status);
    printf("sec2_cb_enter: %u\n", ctx->sec2_cb_enter);
    printf("sec2_cb_exit: %d\n", ctx->sec2_cb_exit);
    printf("sec2_add_node: %u\n", ctx->sec2_add_node);
    // Ajoutez ici les instructions printf pour les champs restants...
    printf("sec2_add_prm: %p\n", ctx->sec2_add_prm);
    printf("psk_len: %d\n", ctx->psk_len);
    // Utilisez une boucle pour afficher les éléments de dtls_psk
    printf("dtls_psk: ");
    for (int i = 0; i < MAX_DTLS_PSK/2; i++) {
        printf("%u ", ctx->dtls_psk[i]);
    }
    printf("\n");
    printf("host_port: %u\n", ctx->host_port);
    printf("use_ipv4: %d\n", ctx->use_ipv4);
    printf("zwnet: %p\n", ctx->zwnet);
    printf("gw_addr_buf: %p\n", ctx->gw_addr_buf);
    printf("gw_name: %p\n", ctx->gw_name);
    printf("gw_addr_cnt: %u\n", ctx->gw_addr_cnt);
    // Utilisez une boucle pour afficher les éléments de zip_gw_ip
    printf("zip_gw_ip: ");
    for (int i = 0; i < 16; i++) {
        printf("%u ", ctx->zip_gw_ip[i]);
    }
    printf("\n");
    printf("hl_mtx: %p\n", ctx->hl_mtx);
}


void printZwifd(const zwifd_t *zwifd) {
    printf("CC: %u\n", zwifd->cls);
    printf("CC Version: %u\n", zwifd->ver);
    printf("Real CC Version: %u\n", zwifd->real_ver);
    printf("Interface Properties: %u\n", zwifd->propty);
    printf("Endpoint ID: %u\n", zwifd->epid);
    printf("Node ID: %u\n", zwifd->nodeid);
    printf("Network Handle: %p\n", (void*)zwifd->net);
}



/** Storage for command class and its name string */
typedef struct
{
    uint16_t    cls;    ///< Command class
    char        *str;   ///< Class string
} hl_cls_str_t;

static const hl_cls_str_t cls_str[] = {
                                        {COMMAND_CLASS_APPLICATION_CAPABILITY, "COMMAND_CLASS_APPLICATION_CAPABILITY"},
                                        {COMMAND_CLASS_APPLICATION_STATUS, "COMMAND_CLASS_APPLICATION_STATUS"},
                                        {COMMAND_CLASS_ASSOCIATION, "COMMAND_CLASS_ASSOCIATION"},
                                        {COMMAND_CLASS_ASSOCIATION_GRP_INFO, "COMMAND_CLASS_ASSOCIATION_GRP_INFO"},
                                        {COMMAND_CLASS_BARRIER_OPERATOR, "COMMAND_CLASS_BARRIER_OPERATOR"},
                                        {COMMAND_CLASS_BASIC, "COMMAND_CLASS_BASIC"},
                                        {COMMAND_CLASS_BATTERY, "COMMAND_CLASS_BATTERY"},
                                        {COMMAND_CLASS_CENTRAL_SCENE, "COMMAND_CLASS_CENTRAL_SCENE"},
                                        {COMMAND_CLASS_CLIMATE_CONTROL_SCHEDULE, "COMMAND_CLASS_CLIMATE_CONTROL_SCHEDULE"},
                                        {COMMAND_CLASS_CLOCK, "COMMAND_CLASS_CLOCK"},
                                        {COMMAND_CLASS_CONFIGURATION, "COMMAND_CLASS_CONFIGURATION"},
                                        {COMMAND_CLASS_CONTROLLER_REPLICATION, "COMMAND_CLASS_CONTROLLER_REPLICATION"},
                                        {COMMAND_CLASS_CRC_16_ENCAP, "COMMAND_CLASS_CRC_16_ENCAP"},
                                        {COMMAND_CLASS_DEVICE_RESET_LOCALLY, "COMMAND_CLASS_DEVICE_RESET_LOCALLY"},
                                        {COMMAND_CLASS_DOOR_LOCK, "COMMAND_CLASS_DOOR_LOCK"},
                                        {COMMAND_CLASS_DOOR_LOCK_LOGGING, "COMMAND_CLASS_DOOR_LOCK_LOGGING"},
                                        {COMMAND_CLASS_FIRMWARE_UPDATE_MD, "COMMAND_CLASS_FIRMWARE_UPDATE_MD"},
                                        {COMMAND_CLASS_HAIL, "COMMAND_CLASS_HAIL"},
                                        {COMMAND_CLASS_INCLUSION_CONTROLLER, "COMMAND_CLASS_INCLUSION_CONTROLLER"},
                                        {COMMAND_CLASS_INDICATOR, "COMMAND_CLASS_INDICATOR"},
                                        {COMMAND_CLASS_IP_ASSOCIATION, "COMMAND_CLASS_IP_ASSOCIATION"},
                                        {COMMAND_CLASS_LOCK, "COMMAND_CLASS_LOCK"},
                                        {COMMAND_CLASS_MAILBOX, "COMMAND_CLASS_MAILBOX"},
                                        {COMMAND_CLASS_MANUFACTURER_PROPRIETARY, "COMMAND_CLASS_MANUFACTURER_PROPRIETARY"},
                                        {COMMAND_CLASS_MANUFACTURER_SPECIFIC, "COMMAND_CLASS_MANUFACTURER_SPECIFIC"},
                                        {COMMAND_CLASS_METER, "COMMAND_CLASS_METER"},
                                        {COMMAND_CLASS_METER_PULSE, "COMMAND_CLASS_METER_PULSE"},
                                        {COMMAND_CLASS_METER_TBL_CONFIG, "COMMAND_CLASS_METER_TBL_CONFIG"},
                                        {COMMAND_CLASS_METER_TBL_MONITOR, "COMMAND_CLASS_METER_TBL_MONITOR"},
                                        {COMMAND_CLASS_MULTI_CMD, "COMMAND_CLASS_MULTI_CMD"},
                                        {COMMAND_CLASS_NETWORK_MANAGEMENT_BASIC, "COMMAND_CLASS_NETWORK_MANAGEMENT_BASIC"},
                                        {COMMAND_CLASS_NETWORK_MANAGEMENT_INCLUSION, "COMMAND_CLASS_NETWORK_MANAGEMENT_INCLUSION"},
                                        {COMMAND_CLASS_NETWORK_MANAGEMENT_INSTALLATION_MAINTENANCE, "COMMAND_CLASS_NETWORK_MANAGEMENT_INSTALLATION_MAINTENANCE"},
                                        {COMMAND_CLASS_NETWORK_MANAGEMENT_PRIMARY, "COMMAND_CLASS_NETWORK_MANAGEMENT_PRIMARY"},
                                        {COMMAND_CLASS_NETWORK_MANAGEMENT_PROXY, "COMMAND_CLASS_NETWORK_MANAGEMENT_PROXY"},
                                        {COMMAND_CLASS_NO_OPERATION, "COMMAND_CLASS_NO_OPERATION"},
                                        {COMMAND_CLASS_NODE_NAMING, "COMMAND_CLASS_NODE_NAMING"},
                                        {COMMAND_CLASS_NODE_PROVISIONING, "COMMAND_CLASS_NODE_PROVISIONING"},
                                        {COMMAND_CLASS_POWERLEVEL, "COMMAND_CLASS_POWERLEVEL"},
                                        {COMMAND_CLASS_PROTECTION, "COMMAND_CLASS_PROTECTION"},
                                        {COMMAND_CLASS_SCENE_ACTIVATION, "COMMAND_CLASS_SCENE_ACTIVATION"},
                                        {COMMAND_CLASS_SCENE_ACTUATOR_CONF, "COMMAND_CLASS_SCENE_ACTUATOR_CONF"},
                                        {COMMAND_CLASS_SCENE_CONTROLLER_CONF, "COMMAND_CLASS_SCENE_CONTROLLER_CONF"},
                                        {COMMAND_CLASS_SCHEDULE, "COMMAND_CLASS_SCHEDULE"},
                                        {COMMAND_CLASS_SCHEDULE_ENTRY_LOCK, "COMMAND_CLASS_SCHEDULE_ENTRY_LOCK"},
                                        {COMMAND_CLASS_SECURITY, "COMMAND_CLASS_SECURITY"},
                                        {COMMAND_CLASS_SECURITY_2, "COMMAND_CLASS_SECURITY_2"},
                                        {COMMAND_CLASS_SENSOR_ALARM, "COMMAND_CLASS_SENSOR_ALARM"},
                                        {COMMAND_CLASS_SENSOR_BINARY, "COMMAND_CLASS_SENSOR_BINARY"},
                                        {COMMAND_CLASS_SENSOR_MULTILEVEL, "COMMAND_CLASS_SENSOR_MULTILEVEL"},
                                        {COMMAND_CLASS_SIMPLE_AV_CONTROL, "COMMAND_CLASS_SIMPLE_AV_CONTROL"},
										{COMMAND_CLASS_SOUND_SWITCH, "COMMAND_CLASS_SOUND_SWITCH" },
                                        {COMMAND_CLASS_SUPERVISION, "COMMAND_CLASS_SUPERVISION"},
                                        {COMMAND_CLASS_SWITCH_ALL, "COMMAND_CLASS_SWITCH_ALL"},
                                        {COMMAND_CLASS_SWITCH_BINARY, "COMMAND_CLASS_SWITCH_BINARY"},
                                        {COMMAND_CLASS_SWITCH_COLOR, "COMMAND_CLASS_SWITCH_COLOR"},
                                        {COMMAND_CLASS_SWITCH_MULTILEVEL, "COMMAND_CLASS_SWITCH_MULTILEVEL"},
                                        {COMMAND_CLASS_SWITCH_TOGGLE_BINARY, "COMMAND_CLASS_SWITCH_TOGGLE_BINARY"},
                                        {COMMAND_CLASS_THERMOSTAT_FAN_MODE, "COMMAND_CLASS_THERMOSTAT_FAN_MODE"},
                                        {COMMAND_CLASS_THERMOSTAT_FAN_STATE, "COMMAND_CLASS_THERMOSTAT_FAN_STATE"},
                                        {COMMAND_CLASS_THERMOSTAT_MODE, "COMMAND_CLASS_THERMOSTAT_MODE"},
                                        {COMMAND_CLASS_THERMOSTAT_OPERATING_STATE, "COMMAND_CLASS_THERMOSTAT_OPERATING_STATE"},
                                        {COMMAND_CLASS_THERMOSTAT_SETBACK, "COMMAND_CLASS_THERMOSTAT_SETBACK"},
                                        {COMMAND_CLASS_THERMOSTAT_SETPOINT, "COMMAND_CLASS_THERMOSTAT_SETPOINT"},
                                        {COMMAND_CLASS_TIME, "COMMAND_CLASS_TIME"},
                                        {COMMAND_CLASS_TIME_PARAMETERS, "COMMAND_CLASS_TIME_PARAMETERS"},
                                        {COMMAND_CLASS_TRANSPORT_SERVICE, "COMMAND_CLASS_TRANSPORT_SERVICE"},
                                        {COMMAND_CLASS_USER_CODE, "COMMAND_CLASS_USER_CODE"},
                                        {COMMAND_CLASS_VERSION, "COMMAND_CLASS_VERSION"},
                                        {COMMAND_CLASS_WAKE_UP, "COMMAND_CLASS_WAKE_UP"},
                                        {COMMAND_CLASS_WINDOW_COVERING, "COMMAND_CLASS_WINDOW_COVERING" },
                                        {COMMAND_CLASS_ZIP, "COMMAND_CLASS_ZIP"},
                                        {COMMAND_CLASS_ZIP_GATEWAY, "COMMAND_CLASS_ZIP_GATEWAY"},
                                        {COMMAND_CLASS_ZIP_NAMING, "COMMAND_CLASS_ZIP_NAMING"},
                                        {COMMAND_CLASS_ZIP_ND, "COMMAND_CLASS_ZIP_ND"},
                                        {COMMAND_CLASS_ZIP_PORTAL, "COMMAND_CLASS_ZIP_PORTAL"},
                                        {COMMAND_CLASS_ZWAVEPLUS_INFO, "COMMAND_CLASS_ZWAVEPLUS_INFO"}
};


const char *bsensor_type_str[] =
{
    "unknown",
    "General purpose sensor",
    "Smoke sensor",
    "CO sensor",
    "CO2 sensor",
    "Heat sensor",
    "Water sensor",
    "Freeze sensor",
    "Tamper sensor",
    "AUX sensor",
    "Door/Window sensor",
    "Tilt sensor",
    "Motion sensor",
    "Glass break sensor"
};

const char *alrm_type_str[] =
{
    "Unknown alarm",
    "Smoke alarm",
    "CO alarm",
    "CO2 alarm",
    "Heat alarm",
    "Water alarm",
    "Access control",
    "Home security",
    "Power management",
    "System",
    "Emergencyi alarm",
    "Clock",
    "Appliance",
    "Home health",
    "Siren",
    "Water valve",
    "Weather alarm",
    "Irrigation",
    "Gas alarm",
    "Pest control",
    "Light sensor",
    "Water quality monitoring",
    "Home monitoring"
};

/**
bin_show - show binary string to the user
@param[in] pltfm_ctx    Context
@param[in] buf          The buffer that stores the binary string
@param[in] len          The length of the binary string.
@return
*/
void bin_show(void * buf, uint32_t len)
{
    uint32_t i;

    for (i = 0; i < len; i++)
    {
        printf("%02X ",*((uint8_t *)buf + i));
    }
    printf("\n");
}

/**
hl_class_str_get - Get command class string
@param[in]	cls	        class
@param[in]	ver	        version of the command class
@return		Command class string if found, else return string "UNKNOWN"
*/
static char *hl_class_str_get(uint16_t cls, uint8_t ver)
{
    //Process command class with different names based on version
    if (cls == COMMAND_CLASS_MULTI_CHANNEL_ASSOCIATION_V2)
    {
        if (ver >= 2)
        {
            return "COMMAND_CLASS_MULTI_CHANNEL_ASSOCIATION";
        }
        return "COMMAND_CLASS_MULTI_INSTANCE_ASSOCIATION";
    }

    if (cls == COMMAND_CLASS_MULTI_CHANNEL_V2)
    {
        if (ver >= 2)
        {
            return "COMMAND_CLASS_MULTI_CHANNEL";
        }
        return "COMMAND_CLASS_MULTI_INSTANCE";
    }

    if (cls == COMMAND_CLASS_ALARM)
    {
        if (ver >= 3)
        {
            return "COMMAND_CLASS_NOTIFICATION";
        }
        return "COMMAND_CLASS_ALARM";
    }

    for (size_t i = 0; i < sizeof(cls_str) / sizeof(hl_cls_str_t); i++)
    {
        if (cls == cls_str[i].cls)
        {
            return cls_str[i].str;
        }
    }

    return "UNKNOWN";
}


/**
prompt_str - prompt for a string from user
@param[in] disp_str   The prompt string to display
@param[in] out_buf_sz The size of out_str buffer
@param[out] out_str   The buffer where the user input string to be stored
@return          The out_str if successful; else NULL.
*/
static char  *prompt_str(hl_appl_ctx_t *hl_appl, const char *disp_str, int out_buf_sz, char *out_str)
{
    (void)hl_appl;
    int retry;
    puts(disp_str);
    retry = 3;
    while (retry-- > 0)
    {
        if (fgets(out_str, out_buf_sz, stdin) && (*out_str) && ((*out_str) != '\n'))
        {
            char *newline;
            //Remove newline character

            newline = strchr(out_str, '\n');
            if (newline)
            {
                *newline = '\0';
            }
            return out_str;
        }
    }
    return NULL;
}


/**
prompt_hex - prompt for an hexadecimal unsigned integer input from user
@param[in] str   The prompt string to display
@return          The unsigned integer that user has input
*/
static unsigned prompt_hex(hl_appl_ctx_t *hl_appl, char *str)
{
    char user_input_str[36];
    unsigned  ret;

    if (prompt_str(hl_appl, str, 36, user_input_str))
    {
        if (sscanf(user_input_str, "%x", &ret) == 1)
        {
            return ret;
        }
    }
    return 0;
}


/**
prompt_int - prompt for a signed integer input from user
@param[in] str   The prompt string to display
@return          The signed integer that user has input
*/
static signed prompt_int(hl_appl_ctx_t *hl_appl, char *str)
{
    char user_input_str[36];
    signed  ret;

    if (prompt_str(hl_appl, str, 36, user_input_str))
    {
        if (sscanf(user_input_str, "%u", &ret) == 1)
        {
            return ret;
        }
    }
    return 0;
}


/**
prompt_uint - prompt for an unsigned integer input from user
@param[in] str   The prompt string to display
@return          The unsigned integer that user has input
*/
static unsigned prompt_uint(hl_appl_ctx_t *hl_appl, char *str)
{
    char user_input_str[36];
    unsigned  ret;

    if (prompt_str(hl_appl, str, 36, user_input_str))
    {
        if (sscanf(user_input_str, "%u", &ret) == 1)
        {
            return ret;
        }
    }
    return 0;
}


/**
prompt_char - prompt for a character input from user
@param[in] str   The prompt string to display
@return          The character that user has input. Null character on error.
*/
static char prompt_char(hl_appl_ctx_t *hl_appl, char *str)
{
    char ret[80];

    if (prompt_str(hl_appl, str, 80, ret))
    {
        return ret[0];
    }
    return 0;
}


/**
prompt_yes - prompt for yes or no from user
@param[in] str   The prompt string to display
@return          1 = user has input yes, 0 =  user has input no
*/
static int prompt_yes(hl_appl_ctx_t *hl_appl, char *str)
{
    char c;

    c = prompt_char(hl_appl, str);
    if (c == 'y' || c == 'Y')
    {
        return 1;
    }
    return 0;
}


/**
prompt_menu - prompt for a menu choice from user
@param[in] str   The prompt string to display
@return          The integer value of the choice (for number) or alphabet + ALPHA_OFFSET (for alphabet); -1 on error.
*/
static int prompt_menu(hl_appl_ctx_t *hl_appl, char *str)
{
    char choice[80];
    unsigned choice_number;

    if (prompt_str(hl_appl, str, 80, choice))
    {
        if (sscanf(choice, "%u", &choice_number) == 1)
        {
            return(int)choice_number;
        }
        return ALPHA_OFFSET + choice[0];
    }
    return -1;

}


/**
hex2bin - Convert hex character to binary
@param[in] c        hex character
@return  Value of hex character on success, negative value on failure
*/
static int hex2bin(char c)
{
    if (c >= '0' && c <= '9')
    {
        return c-'0';
    }
    else if (c >= 'a' && c <= 'f')
    {
        return c-'a' + 10;
    }
    else if (c >= 'A' && c <= 'F')
    {
        return c-'A' + 10;
    }
    else
    {
        return -1;
    }
}


/**
hexstring_to_bin - Convert ASCII hexstring to binary string
@param[in] psk_str   ASCII hexstring
@param[in] psk_len   ASCII hexstring length (must be even number)
@param[out] psk_bin  Binary string
@return  Zero on success, non-zero on failure
*/
static int hexstring_to_bin(char *psk_str, int psk_len, uint8_t *psk_bin)
{
    int i = 0;
    int val;

    while(psk_len > 0)
    {
      val = hex2bin(*psk_str++);
      if(val < 0)
          return -1;
      psk_bin[i]  = (val & 0x0F) << 4;

      val = hex2bin(*psk_str++);
      if(val < 0)
          return -1;
      psk_bin[i] |= (val & 0x0F);

      i++;
      psk_len -= 2;
    }

    return 0;
}


/**
config_param_get - get configuration parameters
@param[in] cfg_file     Configuration file name
@param[out] host_port   Host port
@param[out] router      Z/IP router IP address
@param[out] psk         DTLS pre-shared key (PSK)
@return         ZW_ERR_XXX
*/
static int config_param_get(char *cfg_file, uint16_t *host_port, char *router, char *psk)
{
    FILE        *file;
    const char  delimiters[] = " =\r\n";
    char        line[384];
    char        *prm_name;
    char        *prm_val;

    //Initialize output
    *router = '\0';
    *psk = '\0';
    *host_port = 0;

    //Open config file
    if (!cfg_file)
    {
        return ZW_ERR_FILE_OPEN;
    }

    file = fopen(cfg_file, "rt");
    if (!file)
    {
        return ZW_ERR_FILE_OPEN;
    }

    while (fgets(line, 384, file))
    {
        if (*line == '#')
        {   //Skip comment line
            continue;
        }

        //Check if '=' exists
        if (strchr(line, '='))
        {
            //Get the parameter name and value
            prm_name = strtok(line, delimiters);

            if (prm_name)
            {
                prm_val = strtok(NULL, delimiters);

                if (!prm_val)
                {
                    continue;
                }

                //Compare the parameter name
                if (strcmp(prm_name, "ZipLanPort") == 0)
                {
                    unsigned port;
                    if (sscanf(prm_val, "%u", &port) == 1)
                    {
                        *host_port = (uint16_t)port;
                    }
                }
                else if (strcmp(prm_name, "ZipRouterIP") == 0)
                {
                    strcpy(router, prm_val);
                }
                else if (strcmp(prm_name, "DTLSPSK") == 0)
                {
                    strcpy(psk, prm_val);
                }
            }
        }
    }

    fclose(file);

    return 0;
}


/**
hl_nw_tx_cb - Callback function to notify application transmit data status
@param[in]	user	    The high-level api context
param[in]	tx_sts	    Transmit status ZWNET_TX_xx
@return
*/
static void hl_nw_tx_cb(void *user, uint8_t tx_sts)
{
    (void)user;
    static const char    *tx_cmplt_sts[] = {"ok",
        "timeout: no ACK received",
        "system error",
        "destination host needs long response time",
        "frame failed to reach destination host",
        "ok, verified"
    };

    if (tx_sts == TRANSMIT_COMPLETE_OK)
    {
        //printf("Higher level appl send data completed successfully\n");
    }
    else
    {
        printf("Higher level appl send data completed with error:%s\n",
               (tx_sts < sizeof(tx_cmplt_sts)/sizeof(char *))? tx_cmplt_sts[tx_sts]  : "unknown");
    }
}


/**
hl_nw_node_cb - Callback function to notify node is added, deleted, or updated
@param[in]	user	    The high-level api context
@param[in]	noded	Node
@param[in]	mode	    The node status
@return
*/
static void hl_nw_node_cb(void *user, zwnoded_p noded, int mode)
{
    (void)user;
    switch (mode)
    {
        case ZWNET_NODE_ADDED:
            {
                printf("\nNode:%u added\n", (unsigned)noded->nodeid);
            }
            break;

        case ZWNET_NODE_REMOVED:
            {
                printf("\nNode:%u removed\n", (unsigned)noded->nodeid);
            }
            break;
    }
}


/**
gw_intf_get - Search for the Z/IP gateway interface
@param[in]  net     network handle
@param[out] gw_if   Z/IP gateway interface
@return  0 on success; otherwise negative number
*/
static int gw_intf_get(zwnet_p net, zwifd_t *gw_if)
{
    int         result;
    zwnoded_t   node;
    zwepd_t     ep;
    zwifd_t     intf;

    //Get first node (controller node)
    result = zwnet_get_node(net, &node);
    if (result != 0)
    {
        return result;
    }

    if (!zwnode_get_ep(&node, &ep)) //get first endpoint of the node
    {
        if (!zwep_get_if(&ep, &intf)) //get first interface of the endpoint
        {
            do
            {
                if (intf.cls == COMMAND_CLASS_ZIP_GATEWAY)
                {   //Found
                    *gw_if = intf;
                    return 0;
                }

            }while (!zwif_get_next(&intf, &intf)); //get next interface
        }
    }

    return  ZW_ERR_INTF_NOT_FOUND;
}


/**
hl_unsolicited_addr_setup - Setup unsolicited address to receive unsolicited report
@param[in]	hl_appl	    The high-level api context
@return  0 on success, negative error number on failure
*/
static int hl_unsolicited_addr_setup(hl_appl_ctx_t *hl_appl)
{
    int         result;
    uint8_t     local_ip[16];
    uint16_t    local_port;
    zwifd_t     gw_ifd;

    //Get local Z/IP client listening address and port
    result = zwnet_local_addr_get(hl_appl->zwnet, hl_appl->zip_gw_ip, local_ip, hl_appl->use_ipv4);

    if (result != 0)
    {
        printf("Error: couldn't get local Z/IP client listening address: %d\n", result);
        return result;
    }

    local_port = zwnet_listen_port_get(hl_appl->zwnet);

    if (hl_appl->use_ipv4)
    {   //Convert to IPv4-mapped IPv6 address
        uint8_t unsolicit_ipv4[4];

        //Save the IPv4 address
        memcpy(unsolicit_ipv4, local_ip, 4);

        //Convert the IPv4 address to IPv4-mapped IPv6 address
        memset(local_ip, 0, 10);
        local_ip[10] = 0xFF;
        local_ip[11] = 0xFF;
        memcpy(&local_ip[12], unsolicit_ipv4, 4);
    }

    result = gw_intf_get(hl_appl->zwnet, &gw_ifd);
    if (result != 0)
    {
        printf("Error: couldn't find Z/IP gateway interface: %d\n", result);
        return result;
    }

    result = zwif_gw_unsolicit_set(&gw_ifd, local_ip, local_port);

    if (result != 0)
    {
        printf("Error: couldn't set unsolicited address: %d\n", result);
    }

    return result;

}


/**
hl_nw_notify_cb - Callback function to notify the status of current operation
@param[in]  user        user context
@param[in]  op          network operation ZWNET_OP_XXX
@param[in]  sts         status of current operation
@param[in]  info        additional information for the specified op and sts; NULL if there is no additional info
@return
*/
static void hl_nw_notify_cb(void *user, uint8_t op, uint32_t sts, zwnet_sts_t *info)
{
    hl_appl_ctx_t *hl_appl = (hl_appl_ctx_t *)user;

    //Check whether the status is progress status of discovering each detailed node information
    if (OP_GET_NI_TOTAL_NODES(sts))
    {
        printf("Get node info %u/%u completed\n", OP_GET_NI_CMPLT_NODES(sts), OP_GET_NI_TOTAL_NODES(sts));
        return;
    }

    switch (op)
    {
        case ZWNET_OP_INITIALIZE:
            printf("\nInitialization status:%u\n", sts);
            if (sts == OP_DONE)
            {
                zwnetd_p nw_desp;

                //Get network descriptor
                nw_desp = zwnet_get_desc(hl_appl->zwnet);
                printf("Network id:%08X, Z/IP controller id:%u\n", nw_desp->id, nw_desp->ctl_id);
                printf("Network initialized!  Setting up unsolicited address ...\n");

                if (hl_unsolicited_addr_setup(hl_appl) == 0)
                {
                    printf("Done.\n");
                    hl_appl->init_status = NW_INI_STS_DONE;
                    print_hl_appl_ctx(hl_appl);
                    break;
                }
            }
            hl_appl->init_status = NW_INI_STS_FAIL;
            break;

        case ZWNET_OP_ADD_NODE:
            printf("Add node status:%u\n", sts);
            if (sts == OP_DONE)
            {   //Clear add node DSK callback control & status
                hl_appl->sec2_cb_enter = 0;
                hl_appl->sec2_cb_exit = 1;

                hl_appl->add_status = ADD_NODE_STS_DONE;
            }
            else if (sts == OP_FAILED)
            {   //Clear add node DSK callback control & status
                hl_appl->sec2_cb_enter = 0;
                hl_appl->sec2_cb_exit = 1;

                hl_appl->add_status = ADD_NODE_STS_UNKNOWN;
            }
            break;

        case ZWNET_OP_RM_NODE:
            printf("Remove node status:%u\n", sts);
            if (sts == OP_DONE)
            {
                hl_appl->rm_status = RM_NODE_STS_DONE;
            }
            else if (sts == OP_FAILED)
            {
                hl_appl->rm_status = RM_NODE_STS_UNKNOWN;
            }
            break;

        case ZWNET_OP_UPDATE:
            printf("Network update status:%u\n", sts);
            if (sts == OP_DONE)
            {
                hl_appl->nw_updt_status = NW_UPDT_STS_DONE;
            }
            else if (sts == OP_FAILED)
            {
                hl_appl->nw_updt_status = NW_UPDT_STS_UNKNOWN;
            }
            break;

        case ZWNET_OP_NODE_UPDATE:
            printf("Node update status:%u\n", sts);
            if (sts == OP_DONE)
            {
                hl_appl->node_updt_status = NODE_UPDT_STS_DONE;
            }
            else if (sts == OP_FAILED)
            {
                hl_appl->node_updt_status = NODE_UPDT_STS_UNKNOWN;
            }
            break;

        case ZWNET_OP_RESET:
            printf("Reset network status:%u\n", sts);
            if (sts == OP_DONE)
            {
                hl_appl->rst_status = RESET_NW_STS_DONE;
            }
            else if (sts == OP_FAILED)
            {
                hl_appl->rst_status = RESET_NW_STS_UNKNOWN;
            }
            break;

        case ZWNET_OP_HEALTH_CHK:
            printf("Network health check status:%u\n", sts);
            switch (sts)
            {
                case OP_DONE:
                    hl_appl->nw_health_chk_status = NW_HEALTH_CHK_STS_DONE;
                    break;

                case OP_FAILED:
                    hl_appl->nw_health_chk_status = NW_HEALTH_CHK_STS_UNKNOWN;
                    break;

                case OP_HEALTH_CHK_STARTED:
                    break;

                case OP_HEALTH_CHK_PROGRESS:
                    if (info && (info->type == ZWNET_STS_INFO_HEALTH_CHK_PRG))
                    {
                        printf("\tNetwork health check %u/%u completed\n",
                                info->info.health_chk_progress.node_cnt,
                                info->info.health_chk_progress.total);
                    }
                    break;

                case OP_HEALTH_CHK_CMPLT:
                    if (info && (info->type == ZWNET_STS_INFO_HEALTH_CHK_RPT))
                    {
                        int i;
                        printf("\tNetwork health check report (%u nodes):\n",
                                info->info.health_chk_rpt.sts_cnt);

                        for (i=0; i<info->info.health_chk_rpt.sts_cnt; i++)
                        {
                            printf("\t\tNode id: %u, NHV: %u, Category:%u\n",
                                    info->info.health_chk_rpt.sts[i].node_id,
                                    info->info.health_chk_rpt.sts[i].value,
                                    info->info.health_chk_rpt.sts[i].sts_cat);
                        }
                    }
                    break;
            }
            break;

        default:
            printf("hl_nw_notify_cb op:%u, status:%u\n", (unsigned)op, sts);
    }
}


/**
lib_init - Initialize library
@param[in]	hl_appl		        The high-level api context
@param[in]	host_port		    Host listening port
@param[in]	zip_router_ip		Z/IP router IP address in numerical form
@param[in]	use_ipv4		    Flag to indicate zip_router_ip is in IPv4 or IPv6 format. 1= IPv4; 0= IPv6
@param[in]	dev_cfg_file_name	Device specific configuration database file name
@param[in]	dtls_psk		    DTLS pre-shared key
@param[in]	dtls_psk_len		DTLS pre-shared key length (in bytes)
@param[in]	pref_dir		    Full path of directory for storing network/user preference files
@param[in]	cmd_cls_cfg_file    Full path to the command class configuration file
@return  0 on success, negative error number on failure
*/
static int lib_init(hl_appl_ctx_t *hl_appl, uint16_t host_port, uint8_t *zip_router_ip, int use_ipv4,
                    char *dev_cfg_file_name, uint8_t *dtls_psk, uint8_t dtls_psk_len, char *pref_dir, char *cmd_cls_cfg_file)
{
    int                 result;
    zwnet_init_t        zw_init = {0};

    zw_init.user = hl_appl; //high-level application context
    zw_init.node = hl_nw_node_cb;
    zw_init.notify = hl_nw_notify_cb;
    zw_init.appl_tx = hl_nw_tx_cb;
    zw_init.pref_dir = pref_dir;
    zw_init.cmd_cls_cfg_file = cmd_cls_cfg_file;
    zw_init.print_txt_fn = NULL;
    zw_init.net_info_dir = NULL;
    zw_init.host_port = host_port;
    zw_init.use_ipv4 = use_ipv4;
    memcpy(zw_init.zip_router, zip_router_ip, (use_ipv4)? IPV4_ADDR_LEN : IPV6_ADDR_LEN);
    zw_init.dev_cfg_file = dev_cfg_file_name;
    zw_init.dev_cfg_usr = NULL;
    zw_init.dtls_psk_len = dtls_psk_len;
    if (dtls_psk_len)
    {
        memcpy(zw_init.dtls_psk, dtls_psk, dtls_psk_len);
    }
    //Unhandled command handler
    zw_init.unhandled_cmd = NULL;

    //Init ZW network
    result = zwnet_init(&zw_init, &hl_appl->zwnet);

    if (result != 0)
    {
        printf("zwnet_init with error:%d\n", result);

        //Display device configuration file error
        if (zw_init.err_loc.dev_ent)
        {
            printf("Parsing device configuration file error loc:\n");
            printf("Device entry number:%u\n", zw_init.err_loc.dev_ent);
            if (zw_init.err_loc.ep_ent)
            {
                printf("Endpoint entry number:%u\n", zw_init.err_loc.ep_ent);
            }

            if (zw_init.err_loc.if_ent)
            {
                printf("Interface entry number:%u\n", zw_init.err_loc.if_ent);
            }
        }
        return result;
    }

    return 0;
}


/**
nw_init - initialization network
@param[in] hl_appl   user application context
@return  0=success, 1=gateway discovery using IPv4 is required, 2=gateway discovery using IPv6 is required,
         negative error number on failure
*/
static int nw_init(hl_appl_ctx_t *hl_appl)
{
    int         ret;
    char        zip_gw_addr_str[100];
    char        psk_str[384];

    //Read config file to get configuration parameters
    ret = config_param_get("controller_app.cfg", &hl_appl->host_port, zip_gw_addr_str, psk_str);
    if (ret != 0)
    {
        printf("Error: couldn't get config param from file: controller_app.cfg\n");
        return ret;
    }

    //Check DTLS pre-shared key validity
    hl_appl->psk_len = strlen(psk_str);

    if (hl_appl->psk_len > 0)
    {
        if (hl_appl->psk_len > MAX_DTLS_PSK)
        {
            printf("PSK string length is too long\n");
            return ZW_ERR_VALUE;
        }
        if (hl_appl->psk_len % 2)
        {
            printf("PSK string length should be even\n");
            return ZW_ERR_VALUE;
        }
        //Convert ASCII hexstring to binary string
        ret = hexstring_to_bin(psk_str, hl_appl->psk_len, hl_appl->dtls_psk);
        if (ret != 0)
        {
            printf("PSK string is not hex string\n");
            return ZW_ERR_VALUE;
        }

        hl_appl->psk_len >>= 1;
    }

    //Convert IPv4 / IPv6 address string to numeric equivalent
    ret = zwnet_ip_aton(zip_gw_addr_str, hl_appl->zip_gw_ip, &hl_appl->use_ipv4);

    if (ret != 0)
    {
        printf("Invalid Z/IP router IP address:%s\n", zip_gw_addr_str);
        return ZW_ERR_IP_ADDR;
    }

    //Check whether Z/IP gateway discovery operation should be performed
    if (hl_appl->use_ipv4)
    {
        uint8_t zero_addr[4];

        memset(zero_addr, 0, IPV4_ADDR_LEN);

        if (memcmp(hl_appl->zip_gw_ip, zero_addr, IPV4_ADDR_LEN) == 0)
        {
            return 1;//using IPv4
        }
    }
    else
    {
        uint8_t zero_addr[IPV6_ADDR_LEN];

        memset(zero_addr, 0, IPV6_ADDR_LEN);

        if (memcmp(hl_appl->zip_gw_ip, zero_addr, IPV6_ADDR_LEN) == 0)
        {
            return 2;//using IPv6
        }
    }

    //Initialize library
    ret = lib_init(hl_appl, hl_appl->host_port, hl_appl->zip_gw_ip, hl_appl->use_ipv4, "zwave_device_rec.txt" /*device info database file*/,
                   hl_appl->dtls_psk, hl_appl->psk_len, NULL, "cmd_class.cfg");

    if (ret == 0)
    {
        hl_appl->init_status = NW_INI_STS_PROGRESS;
        printf("Initialize network in progress, please wait for status ...\n");
    }
    else
    {
        printf("lib_init with error: %d\n", ret);
    }
    return ret;
}


/**
hl_add_node_s2_cb - callback for add node with security 2 operation to report Device Specific Key (DSK)
@param[in]	usr_param  user supplied parameter when calling zwnet_add
@param[in]	cb_param   DSK related callback parameters
*/
static void hl_add_node_s2_cb(void *usr_param, sec2_add_cb_prm_t *cb_param)
{
    hl_appl_ctx_t *hl_appl = (hl_appl_ctx_t *)usr_param;
    int           res;

    if (cb_param->cb_type == S2_CB_TYPE_REQ_KEY)
    {
        uint8_t granted_key;
        uint8_t grant_csa;
        uint8_t accept_s2;

        if (hl_appl->sec2_cb_enter & SEC2_ENTER_KEY_REQ)
        {   //Requested keys callback is allowed
            hl_appl->sec2_cb_enter &= ~SEC2_ENTER_KEY_REQ;
        }
        else
        {
            printf("\nNot allowed to processed Security 2 requested keys callback!\n");
            return;
        }

        printf("\nDevice requested keys bit-mask: %02Xh\n", cb_param->cb_prm.req_key.req_keys);

        printf("Key (bit-mask in hex) :\n");
        printf("                      Security 2 key 0 (01)\n");
        printf("                      Security 2 key 1 (02)\n");
        printf("                      Security 2 key 2 (04)\n");
        printf("                      Security 0       (80)\n");

        granted_key = prompt_hex(hl_appl, "Grant keys bit-mask (hex):");

        grant_csa = 0;
        if (cb_param->cb_prm.req_key.req_csa)
        {
            printf("Device requested for client-side authentication (CSA)\n");
            if (prompt_yes(hl_appl, "Grant CSA (y/n)?:"))
            {
                grant_csa = 1;
                printf("Please enter this 10-digit CSA Pin into the joining device:%s\n", cb_param->cb_prm.req_key.csa_pin);
            }
            //No DSK callback when in CSA mode
            hl_appl->sec2_cb_enter &= ~SEC2_ENTER_DSK;

        }

        if (!granted_key)
        {   //No granted key
            accept_s2 = prompt_yes(hl_appl, "Allow S2 bootstrapping process to continue (y/n)?:");
        }
        else
        {
            accept_s2 = 1;
        }

        res = zwnet_add_sec2_grant_key(hl_appl->zwnet, granted_key, grant_csa, accept_s2);

        if (res != 0)
        {
            printf("zwnet_add_sec2_grant_key with error: %d\n", res);
        }

        //Check whether if there is DSK callback pending
        if (!(hl_appl->sec2_cb_enter))
        {   //No callback pending
            hl_appl->sec2_cb_exit = 1;
        }
    }
    else
    {
        sec2_dsk_cb_prm_t   *dsk_prm;
        int                 accept;
        char                dsk_str[200];

        if (hl_appl->sec2_cb_enter & SEC2_ENTER_DSK)
        {   //DSK callback is allowed
            hl_appl->sec2_cb_enter &= ~SEC2_ENTER_DSK;
        }
        else
        {
            printf("\nNot allowed to processed Security 2 DSK callback!\n");
            return;
        }

        dsk_prm = &cb_param->cb_prm.dsk;

        if (dsk_prm->pin_required)
        {
            printf("\nReceived DSK: XXXXX%s\n", dsk_prm->dsk);
        }
        else
        {
            printf("\nReceived DSK: %s\n", dsk_prm->dsk);
        }

        accept = prompt_yes(hl_appl, "Do you accept this device to be added securely (y/n)?:");

        printf("You %s the device.\n", (accept)? "accepted" : "rejected");

        if (accept && dsk_prm->pin_required)
        {
            if (prompt_str(hl_appl, "Enter 5-digit PIN that matches the received DSK:", 200, dsk_str))
            {
                strcat(dsk_str, dsk_prm->dsk);
            }
        }

        res = zwnet_add_sec2_accept(hl_appl->zwnet, accept, (dsk_prm->pin_required)? dsk_str : dsk_prm->dsk);

        if (res != 0)
        {
            printf("zwnet_add_sec2_accept with error: %d\n", res);
        }

        hl_appl->sec2_cb_exit = 1;
    }
}


/**
hl_add_node - Add node
@param[in]	hl_appl  Application context
@return zero if successful; else negative error number
*/
static int hl_add_node(hl_appl_ctx_t *hl_appl)
{
    int         res;
    char        dsk_str[200];
    zwnetd_p    netdesc;

    netdesc = zwnet_get_desc(hl_appl->zwnet);

    if (netdesc->ctl_cap & ZWNET_CTLR_CAP_S2)
    {
        printf("Controller supports security 2.\n");
        hl_appl->sec2_add_node = 1;
    }
    else
    {
        hl_appl->sec2_add_node = 0;
    }

    if (hl_appl->sec2_add_node)
    {
        hl_appl->sec2_add_prm.dsk = NULL;

        if (prompt_yes(hl_appl, "Pre-enter Device Specific Key (DSK) (y/n)?:"))
        {
            if (prompt_str(hl_appl, "DSK:", 200, dsk_str))
            {
                hl_appl->sec2_add_prm.dsk = dsk_str;
            }
        }

        hl_appl->sec2_add_prm.usr_param = hl_appl;

        hl_appl->sec2_add_prm.cb = hl_add_node_s2_cb;

    }

    res = zwnet_add(hl_appl->zwnet, 1, (hl_appl->sec2_add_node)? &hl_appl->sec2_add_prm : NULL, 0);

    if (res == 0)
    {   //Change add node status
        hl_appl->add_status = ADD_NODE_STS_PROGRESS;

        //Wait for S2 callbacks
        if (hl_appl->sec2_add_node)
        {
            int wait_count;

            hl_appl->sec2_cb_enter = SEC2_ENTER_KEY_REQ;

            if (!hl_appl->sec2_add_prm.dsk)
            {   //No pre-entered DSK, requires DSK callback
                hl_appl->sec2_cb_enter |= SEC2_ENTER_DSK;
            }

            hl_appl->sec2_cb_exit = 0;

            printf("Waiting for Requested keys and/or DSK callback ...\n");
            printf("NOTE: Operation cannot be cancelled during this period.\n");

            //Wait for S2 callback to exit
            wait_count = 600;    //Wait for 60 seconds
            while (wait_count-- > 0)
            {
                if (hl_appl->sec2_cb_exit == 1)
                    break;
                plt_sleep(100);
            }
        }
    }

    return res;

}


/**
hl_sup_bsensor_show - Show supported binary sensor types
@param[in]	intf	  Binary sensor interface
@return
*/
static void hl_sup_bsensor_show(zwifd_p intf)
{
    int     i;
    uint8_t type;
    uint8_t sensor_cnt;
	uint8_t sup_sensor[255];

    if (zwif_bsensor_sup_cache_get(intf, &sensor_cnt, sup_sensor) != 0)
    {
        return;
    }

    for (i=0; i<sensor_cnt; i++)
    {
        type = sup_sensor[i];

        if (type > ZW_BSENSOR_TYPE_GLASS_BRK)
        {
            type = 0;
        }
        printf("                        Supported binary sensor type:%s\n", bsensor_type_str[type]);

    }
}


/**
hl_grp_info_show - Show detailed group information
@param[in]	intf   interface
@return
*/
static void hl_grp_info_show(zwifd_p intf)
{
    int                 j;
    int                 i;
    int                 result;
    if_grp_info_dat_t   *grp_info;
    zw_grp_info_p       grp_info_ent;

    result = zwif_group_info_get(intf, &grp_info);

    if (result == 0)
    {
        printf("                        Group info type:%s\n", (grp_info->dynamic)? "dynamic" : "static");
        printf("                        Maximum supported groups:%u\n", grp_info->group_cnt);
        printf("                        Valid groups:%u\n", grp_info->valid_grp_cnt);
        for (i=0; i<grp_info->valid_grp_cnt; i++)
        {
            grp_info_ent = grp_info->grp_info[i];
            if (grp_info_ent)
            {
                printf("                        --------------------------------------------\n");
                printf("                        Group id:%u, profile:%04xh, event code:%04xh,\n",
                             grp_info_ent->grp_num, grp_info_ent->profile, grp_info_ent->evt_code);
                printf("                        name:%s, command list:\n",
                             grp_info_ent->name);
                for (j=0; j<grp_info_ent->cmd_ent_cnt; j++)
                {
                        printf("                        command class:%04xh(%s), command:%02xh\n",
                                     grp_info_ent->cmd_lst[j].cls,
                                     hl_class_str_get(grp_info_ent->cmd_lst[j].cls, 1),
                                     grp_info_ent->cmd_lst[j].cmd);
                }
            }
        }
        //Free group info
        zwif_group_info_free(grp_info);
    }
}


/**
hl_intf_show - Show interface info
@param[in]	intf    Interface descriptor
@return
*/
static void hl_intf_show(zwifd_p intf)
{
    printf("              Interface: %02Xh v%u:%s %c%c\n",
                 (unsigned)intf->cls, intf->ver, hl_class_str_get(intf->cls, intf->ver),
                 (intf->propty & IF_PROPTY_SECURE)? '*' : ' ',
                 (intf->propty & IF_PROPTY_UNSECURE)? '^' : ' ');

    switch(intf->cls)
    {
        case COMMAND_CLASS_SENSOR_BINARY:
            printf("                        Capable to sent event clear: %s\n",
                         (intf->propty & IF_PROPTY_BSNSR_EVT_CLR)? "yes" : "no\n");
            hl_sup_bsensor_show(intf);
            break;

        case COMMAND_CLASS_ASSOCIATION_GRP_INFO:
            hl_grp_info_show(intf);
            break;

    }
}


/**
hl_ep_show - Show endpoint info
@param[in]	ep    Endpoint descriptor
@return
*/
static void hl_ep_show(zwepd_p ep)
{
    printf("Endpoint id:%u\n", ep->epid);
    if (ep->aggr_ep_cnt)
    {
        int     i;
        int     str_len = 0;
        char    member_str[MAX_AGGR_MBR * 2 + 1];

        for (i=0; i<ep->aggr_ep_cnt; i++)
        {
            sprintf(member_str + str_len, "%u,\n", ep->aggr_members[i]);
            str_len = strlen(member_str);
        }
        printf("Aggregated end point members:%s\n", member_str);

    }
    printf("Device class: generic:%02Xh, specific:%02Xh\n", ep->generic, ep->specific);
    printf("Endpoint name:%s\n", ep->name);
    printf("Endpoint location:%s\n", ep->loc);
}


/**
hl_node_show - Show node info
@param[in]	node    Node descriptor
@return
*/
static void hl_node_show(zwnoded_p node)
{
    printf("\n__________________________________________________________________________\n");
    printf("Node id:%u\n", (unsigned)node->nodeid);

    if (node->sleep_cap)
    {
        printf("Node is capable to sleep\n");
    }

    if (node->restricted)
    {
        printf("Node is running in restricted mode\n");
    }

    if (node->sensor)
    {
        printf("Node is a listening sleeping slave\n");
    }

    if (*node->s2_dsk)
    {
        printf("Node DSK: %s\n", node->s2_dsk);
    }

    if (node->s2_keys_valid)
    {
        printf("Node security 2 granted keys bitmask: %02Xh\n", node->s2_grnt_keys);

        if (node->s2_grnt_keys & SEC_KEY_BITMSK_S2_K0)
        {
            printf("S2 Unauthenticated,\n");
        }
        if (node->s2_grnt_keys & SEC_KEY_BITMSK_S2_K1)
        {
            printf("S2 Authenticated,\n");
        }
        if (node->s2_grnt_keys & SEC_KEY_BITMSK_S2_K2)
        {
            printf("S2 Access Control,\n");
        }
        if (node->s2_grnt_keys & SEC_KEY_BITMSK_S0)
        {
            printf("S0 Unauthenticated,\n");
        }

    }

    if (node->propty & NODE_PROPTY_IDENTIFY_CAP)
    {
        printf("Node is capable to identify itself\n");
    }

    if (node->propty & NODE_PROPTY_SECURE_CAP_S0)
    {
        printf("Node is security S0 capable\n");
    }

    if (node->propty & NODE_PROPTY_SECURE_CAP_S2)
    {
        printf("Node is security S2 capable\n");
    }

    if (node->propty & NODE_PROPTY_ADD_SECURE)
    {
        printf("Node was added securely to the Z-Wave network\n");
    }

    if (node->propty & NODE_PROPTY_ADD_INSECURE)
    {
        printf("Node was added insecurely to the Z-Wave network\n");
        if ((node->propty & (NODE_PROPTY_SECURE_CAP_S0 | NODE_PROPTY_SECURE_CAP_S2)) != 0)
        {
            printf("*** WARNING *** : The node is security capable, but added insecurely.\nPlease remove and add it again to the network. \n");
        }
    }

    printf("Vendor id:%04Xh\n", node->vid);
    printf("Product type id:%04Xh\n", node->type);
    printf("Product id:%04Xh\n", node->pid);
    printf("Z-Wave library type:%u\n", node->lib_type);
    printf("Z-Wave protocol version:%u.%02u\n", (unsigned)(node->proto_ver >> 8), (unsigned)(node->proto_ver & 0xFF));
    printf("Application version:%u.%02u\n", (unsigned)(node->app_ver >> 8), (unsigned)(node->app_ver & 0xFF));
}


/**
hl_nw_show - Show network info
@param[in]	hl_appl		The high-level api context
@return
*/
static void hl_nw_show(hl_appl_ctx_t *hl_appl)
{
    int         result;
    zwnet_p     net = hl_appl->zwnet;
    zwnetd_p    nw_desp;
    zwnoded_t   node;
    zwepd_t     ep;
    zwifd_t     intf;

    if (!net)
    {
        return;
    }

    //Get network info
    nw_desp = zwnet_get_desc(net);

    if (nw_desp->id == 0)
    {
        printf("\nNo network info available!\n");
        return;
    }

    printf("\nNetwork id:%08X, controller node id:%u", nw_desp->id, nw_desp->ctl_id);

    //Get first node (controller node)
    result = zwnet_get_node(net, &node);

    while (result == 0)
    {
        hl_node_show(&node);

        if (!zwnode_get_ep(&node, &ep)) //get first endpoint of the node
        {
            do
            {
                hl_ep_show(&ep);

                if (!zwep_get_if(&ep, &intf)) //get first interface of the endpoint
                {
                    do
                    {
                        hl_intf_show(&intf);

                    }while (!zwif_get_next(&intf, &intf)); //get next interface
                }
            }while (!zwep_get_next(&ep, &ep)); //get next endpoint
        }

        result = zwnode_get_next(&node, &node); //get next node
    }
}


/**
gw_discvr_cb - gateway discovery callback
@param[in]	gw_addr	    Gateway addresses
@param[in]	gw_cnt	    Number of gateway addresses returned in gw_addr
@param[in]	ipv4        Flag to indicate the addr parameter is IPv4 or IPv6. 1=IPv4; 0=IPv6
@param[in]	usr_param   User defined parameter used in callback function
@param[in]	rpt_num     Report number that this callback is delivering the gateway addresses report; start from 1
@param[in]	total_rpt   Total reports that will be delivered by callbacks. Each callback delivers one report.
@param[in]	gw_name	    Gateway names corresponding to the gw_ip.  If NULL, it means gateway name information is unavailable.
*/
static void gw_discvr_cb(uint8_t *gw_addr, uint8_t gw_addr_cnt, int ipv4, void *usr_param,
                         int rpt_num, int total_rpt, char **gw_name)
{
    hl_appl_ctx_t   *hl_appl = (hl_appl_ctx_t *)usr_param;
    uint8_t         *tmp_buf;
    char            **tmp_gw_name;
    int             gw_addr_buf_sz;
    int             i;
    int             result;
    int             one_ip_addr_len;
    char            addr_str[80];

    printf("\nReceived report:%d/%d with gw count:%u\n", rpt_num, total_rpt, gw_addr_cnt);

    if (total_rpt == 0)
    {
        printf("The system has no valid IP, please configure it.\n");
        return;
    }

    one_ip_addr_len = (ipv4)? IPV4_ADDR_LEN : IPV6_ADDR_LEN;

    if (gw_addr_cnt > 0)
    {
        //Save a copy of the gateway addresses
        gw_addr_buf_sz = (gw_addr_cnt * one_ip_addr_len);

        plt_mtx_lck(hl_appl->hl_mtx);
        if (hl_appl->gw_addr_buf)
        {
            tmp_buf = realloc(hl_appl->gw_addr_buf, (hl_appl->gw_addr_cnt * one_ip_addr_len) + gw_addr_buf_sz);
        }
        else
        {
            tmp_buf = malloc(gw_addr_buf_sz);
        }

        if (gw_name)
        {   //Gateway names are available
            if (hl_appl->gw_name)
            {
                tmp_gw_name = (char **)realloc(hl_appl->gw_name, (hl_appl->gw_addr_cnt + gw_addr_cnt)*sizeof(char *));
            }
            else
            {
                tmp_gw_name = (char **)malloc(gw_addr_cnt * sizeof(char *));
            }
            if (tmp_gw_name)
            {
                hl_appl->gw_name = tmp_gw_name;
                for (i=0; i<gw_addr_cnt; i++)
                {
                    tmp_gw_name[hl_appl->gw_addr_cnt + i] = strdup(gw_name[i]);
                }
            }
        }

        if (tmp_buf)
        {
            hl_appl->gw_addr_buf = tmp_buf;
            memcpy(hl_appl->gw_addr_buf + (hl_appl->gw_addr_cnt * one_ip_addr_len), gw_addr, gw_addr_buf_sz);
            hl_appl->gw_addr_cnt += gw_addr_cnt;
        }
        plt_mtx_ulck(hl_appl->hl_mtx);
    }

    if (hl_appl->gw_addr_cnt > 0)
    {
        //Display gateway ip addresses
        printf("\n---Select Gateway IP address---\n");
        plt_mtx_lck(hl_appl->hl_mtx);
        for (i=0; i<hl_appl->gw_addr_cnt; i++)
        {
            result = zwnet_ip_ntoa(hl_appl->gw_addr_buf + (i * one_ip_addr_len), addr_str, 80, ipv4);
            if (result == 0)
            {
                printf("(%d) %s [%s]\n", i, addr_str, (hl_appl->gw_name)? hl_appl->gw_name[i] : "unknown");
            }
        }
        plt_mtx_ulck(hl_appl->hl_mtx);
        printf("(x) Exit\n");
        printf("Your choice:\n");

    }
}


/**
subm_select_gw - prompt for gateway ip
@param[in] hl_appl   High-level application context
@return  Zero on success, non-zero on failure
*/
static int subm_select_gw(hl_appl_ctx_t *hl_appl)
{
    int     ret;
    int     choice;
    void    *ctx;
    printf("Scanning for Z/IP gateway ...\n");
    printf("(x) Exit program\n");

    //Start gateway discovery
    ctx = zwnet_gw_discvr_start(gw_discvr_cb, hl_appl, hl_appl->use_ipv4, hl_appl->psk_len);

    if (!ctx)
    {
        return -1;
    }

    while(1)
    {
        choice = prompt_menu(hl_appl, "Your choice:");
        if (choice == (ALPHA_OFFSET + 'x'))
        {
            zwnet_gw_discvr_stop(ctx);
            ret = -2;
            break;
        }
        //Check for valid input
        if ((choice >= 0) && (choice < hl_appl->gw_addr_cnt))
        {
            int one_ip_addr_len;

            zwnet_gw_discvr_stop(ctx);

            one_ip_addr_len = (hl_appl->use_ipv4)? IPV4_ADDR_LEN : IPV6_ADDR_LEN;

            //Save Z/IP gateway IP address
            memcpy(hl_appl->zip_gw_ip, hl_appl->gw_addr_buf + (choice * one_ip_addr_len), one_ip_addr_len);

            //Initialize network
            ret = lib_init(hl_appl, hl_appl->host_port, hl_appl->zip_gw_ip,
                           hl_appl->use_ipv4, "zwave_device_rec.txt" /*device info database file*/,
                           hl_appl->dtls_psk, hl_appl->psk_len, NULL, "cmd_class.cfg");

            if (ret == 0)
            {
                hl_appl->init_status = NW_INI_STS_PROGRESS;
                printf("Initialize network in progress, please wait for status ...\n");
            }
            else
            {
                printf("hl_init with error: %d\n", ret);
            }

            break;
        }
    }

    plt_mtx_lck(hl_appl->hl_mtx);

    if (hl_appl->gw_addr_cnt > 0)
    {
        //Free gateway names buffer
        if (hl_appl->gw_name)
        {
            int i;
            for (i=0; i<hl_appl->gw_addr_cnt; i++)
            {
                free(hl_appl->gw_name[i]);
            }
            free(hl_appl->gw_name);
            hl_appl->gw_name = NULL;
        }
        //Free gateway IP address buffer
        free(hl_appl->gw_addr_buf);
        hl_appl->gw_addr_buf = NULL;
        hl_appl->gw_addr_cnt = 0;
    }
    plt_mtx_ulck(hl_appl->hl_mtx);
    return ret;
}


/**
show provisioning list entry information
@param[in]      usr_ctx     user context
@param[in]      plt_ctx     platform context
@param[in]      lst_ent     provisioning list entry
@param[in]      cnt         number of entries in provisioning list
*/
void hl_pl_ent_show(pl_lst_ent_t *lst_ent, uint8_t cnt)
{
    int i;
    int j;

    if (!cnt)
    {
        return;
    }

    for (i=0; i<cnt; i++)
    {
        printf("Device DSK:%s\n", lst_ent[i].dsk);

        for (j=0; j<lst_ent[i].info_cnt; j++)
        {
            switch (lst_ent[i].info[j].type)
            {
                case PL_INFO_TYPE_PROD_TYPE:
                    printf("Generic class:%u, Specific class:%u, Icon type:%u\n",
                                 lst_ent[i].info[j].info.prod_type.generic_cls,
                                 lst_ent[i].info[j].info.prod_type.specific_cls,
                                 lst_ent[i].info[j].info.prod_type.icon_type);
                    break;

                case PL_INFO_TYPE_PROD_ID:
                    printf("Manf id:%04xh, Prod type:%04xh, Prod id:%04xh,App ver:%02xh, App sub-version:%02xh,\n",
                                 lst_ent[i].info[j].info.prod_id.manf_id,
                                 lst_ent[i].info[j].info.prod_id.prod_type,
                                 lst_ent[i].info[j].info.prod_id.prod_id,
                                 lst_ent[i].info[j].info.prod_id.app_ver,
                                 lst_ent[i].info[j].info.prod_id.app_sub_ver);
                    break;

                case PL_INFO_TYPE_INC_INTV:
                    printf("Smart Start inclusion request interval in unit of 128 seconds:%u\n", lst_ent[i].info[j].info.interval);
                    break;

                case PL_INFO_TYPE_UUID16:
                    printf("UUID presentation format:%u\n", lst_ent[i].info[j].info.uuid.pres_fmt);
                    bin_show(lst_ent[i].info[j].info.uuid.uuid, PL_UUID_LEN);
                    break;

                case PL_INFO_TYPE_SUP_PROTO:
                    printf("Supported protocols: Z-Ware: %u, Z-Ware Long Range: %u",
                           lst_ent[i].info[j].info.sup_proto[PL_SUP_PROTO_BYTE_0_Z_WAVE_BYTE] & PL_SUP_PROTO_BYTE_0_Z_WAVE_BIT,
                           lst_ent[i].info[j].info.sup_proto[PL_SUP_PROTO_BYTE_0_Z_WAVE_LR_BYTE] & PL_SUP_PROTO_BYTE_0_Z_WAVE_LR_BIT);
                    break;

                case PL_INFO_TYPE_NAME:
                    printf("Name:%s\n", lst_ent[i].info[j].info.name);
                    break;

                case PL_INFO_TYPE_LOC:
                    printf("Loc:%s\n", lst_ent[i].info[j].info.loc);
                    break;

                case PL_INFO_TYPE_INCL_STS:
                    printf("Inclusion status:%u where 0=pending; 2=passive; 3=ignored\n", lst_ent[i].info[j].info.incl_sts);
                    break;

                case PL_INFO_TYPE_S2_GRNT:
                    printf("S2 grant keys:%02xh\n", lst_ent[i].info[j].info.grnt_keys);
                    break;

                case PL_INFO_TYPE_BOOT_MODE:
                    printf("Boot mode:%u where 0=S2; 1=Smart Start; 2=Long Range Smart Start\n", lst_ent[i].info[j].info.boot_mode);
                    break;

                case PL_INFO_TYPE_NW_STS:
                    printf("Node id:%u\n", lst_ent[i].info[j].info.nw_sts.node_id);
                    printf("Network status:%u where 0=not included; 1=included; 2=failed\n", lst_ent[i].info[j].info.nw_sts.status);
                    break;

                default:
                    printf("Unknown info type:%u\n", lst_ent[i].info[j].type);
                    break;

            }
        }
        printf("--------------------------------------------------------------\n");
    }
}


/**
callback to report all entries in provisioning list
@param[in]      usr_ctx     user context
@param[in]      pl_list     provisioning list
@param[in]      ent_cnt     number of entries in provisioning list 'pl_list'
@param[in]      lst_cmplt   flag to indicate whether all the entries in 'pl_list' represent the whole list in Z/IP gateway. 1=all the entries
                        have been retrieved from Z/IP gateway; 0=some of the entries were not retrieved due to error.
*/
static void hl_pl_list_cb(void *usr_ctx, pl_lst_ent_t *pl_list, uint8_t ent_cnt, int lst_cmplt)
{
    (void)usr_ctx;
    if (!lst_cmplt)
    {
        printf("***** The provisioning list is incomplete! *****\n");
    }
    if (!ent_cnt)
    {
        printf( "The provisioning list is empty\n");
        return;
    }
    printf("===============================================\n");
    printf("============== Provisioning list ==============\n");
    printf("===============================================\n");

    hl_pl_ent_show(pl_list, ent_cnt);
}


/**
hl_pl_add - Add a provisioning list entry
@param[in]      hl_appl  Context
@return
*/
static void hl_pl_add(hl_appl_ctx_t *hl_appl)
{
    pl_info_t   pl_info[PL_MAX_INFO_TYPE];
    char        dsk_str[200];
    char        name[200];
    char        loc[200];
    uint8_t     i = 0;
    int         res;

    if (!prompt_str(hl_appl, "DSK:", 200, dsk_str))
    {
        return;
    }

    if (prompt_yes(hl_appl, "Enter device name (y/n)?:"))
    {
        if (prompt_str(hl_appl, "Name (max. 62 characters):", 200, name))
        {
            name[PL_MAX_NAME] = '\0';
            strcpy(pl_info[i].info.name, name);
            pl_info[i++].type = PL_INFO_TYPE_NAME;
        }
    }

    if (prompt_yes(hl_appl, "Enter device location (y/n)?:"))
    {
        if (prompt_str(hl_appl, "Location (max. 62 characters):", 200, loc))
        {
            loc[PL_MAX_LOC] = '\0';
            strcpy(pl_info[i].info.loc, loc);
            pl_info[i++].type = PL_INFO_TYPE_LOC;
        }
    }

    if (prompt_yes(hl_appl, "Enter product type (y/n)?:"))
    {
        pl_info[i].info.prod_type.generic_cls = prompt_uint(hl_appl, "Generic device class:");
        pl_info[i].info.prod_type.specific_cls = prompt_uint(hl_appl, "Specific device class:");
        pl_info[i].info.prod_type.icon_type = prompt_uint(hl_appl, "Installer icon type:");
        pl_info[i++].type = PL_INFO_TYPE_PROD_TYPE;
    }

    if (prompt_yes(hl_appl, "Enter product id (y/n)?:"))
    {
        pl_info[i].info.prod_id.manf_id = prompt_hex(hl_appl, "Manufacturer ID (hex):");
        pl_info[i].info.prod_id.prod_type = prompt_hex(hl_appl, "Product type (hex):");
        pl_info[i].info.prod_id.prod_id = prompt_hex(hl_appl, "Product ID (hex):");
        pl_info[i].info.prod_id.app_ver = prompt_hex(hl_appl, "Application version (hex):");
        pl_info[i].info.prod_id.app_sub_ver = prompt_hex(hl_appl, "Application sub version (hex):");
        pl_info[i++].type = PL_INFO_TYPE_PROD_ID;
    }

    if (prompt_yes(hl_appl, "Enter Smart Start inclusion request interval (y/n)?:"))
    {
        pl_info[i].info.interval = prompt_uint(hl_appl, "Interval in unit of 128 seconds, range from 5 to 99:");
        pl_info[i++].type = PL_INFO_TYPE_INC_INTV;
    }

    if (prompt_yes(hl_appl, "Enter UUID (y/n)?:"))
    {
        int j;

        pl_info[i].info.uuid.pres_fmt = prompt_uint(hl_appl, "Presentation format (0 ~ 6):");

        for (j=0; j<PL_UUID_LEN; j++)
        {
            pl_info[i].info.uuid.uuid[j] = prompt_hex(hl_appl, "1 byte of UUID (hex):");
        }
        pl_info[i++].type = PL_INFO_TYPE_UUID16;
    }

    if (prompt_yes(hl_appl, "Enter Supported protocols (y/n)?:"))
    {
        memset(pl_info[i].info.sup_proto, 0, PL_SUP_PROTO_LEN);
        if (prompt_yes(hl_appl, "Z-Wave supported (y/n)?:"))
            pl_info[i].info.sup_proto[PL_SUP_PROTO_BYTE_0_Z_WAVE_BYTE] |= (1 << PL_SUP_PROTO_BYTE_0_Z_WAVE_BIT);
        if (prompt_yes(hl_appl, "Z-Wave Long Range supported (y/n)?:"))
            pl_info[i].info.sup_proto[PL_SUP_PROTO_BYTE_0_Z_WAVE_LR_BYTE] |= (1 << PL_SUP_PROTO_BYTE_0_Z_WAVE_LR_BIT);
        pl_info[i++].type = PL_INFO_TYPE_SUP_PROTO;
    }

    if (prompt_yes(hl_appl, "Enter inclusion status (y/n)?:"))
    {
        pl_info[i].info.incl_sts = prompt_uint(hl_appl, "0=pending; 2=passive; 3=ignored:");
        pl_info[i++].type = PL_INFO_TYPE_INCL_STS;
    }

    if (prompt_yes(hl_appl, "Enter S2 grant keys (y/n)?:"))
    {
        pl_info[i].info.grnt_keys = prompt_hex(hl_appl, "Grant key bit-mask or 0 if unused:");
        pl_info[i++].type = PL_INFO_TYPE_S2_GRNT;
    }

    if (prompt_yes(hl_appl, "Boot mode (y/n)?:"))
    {
        pl_info[i].info.boot_mode = prompt_uint(hl_appl, "0=S2; 1=Smart Start; 2=Long Range Smart Start:");
        pl_info[i++].type = PL_INFO_TYPE_BOOT_MODE;
    }

    if (prompt_yes(hl_appl, "Node <testing only> (y/n)?:"))
    {
        pl_info[i].info.nw_sts.node_id = prompt_uint(hl_appl, "Node ID:");
        pl_info[i].info.nw_sts.status = 0;
        pl_info[i++].type = PL_INFO_TYPE_NW_STS;
    }

    res = zwnet_pl_add(hl_appl->zwnet, dsk_str, pl_info, i);

    if (res != 0)
    {
        printf("zwnet_pl_add with error:%d\n", res);
    }
}


/**
callback to report provisioning list entry information
@param[in]      usr_ctx     user context
@param[in]      lst_ent     provisioning list entry
*/
static void hl_pl_info_cb(void *usr_ctx, pl_lst_ent_t *lst_ent)
{
    (void)usr_ctx;
    if (!lst_ent->info_cnt)
    {
        printf("Device DSK:%s not found!\n", lst_ent->dsk);
        return;
    }

    hl_pl_ent_show(lst_ent, 1);
}


/**
network_menu - display and run network menu
@return
*/
static void network_menu(hl_appl_ctx_t *hl_appl)
{
    int to_run = 1;
    int result;
    unsigned node_id;
    zwnoded_t node;
    char dsk_str[200];

    while (to_run)
    {
        printf("\n(1) Cancel current operation\n");
        printf("(2) Add node\n");
        printf("(3) Remove node\n");
        printf("(4) Network update\n");
        printf("(5) Node update\n");
        printf("(6) Reset network\n");
        printf("-----------------------------------------------------------------\n");
        printf("(7) Add a provisioning list entry\n");
        printf("(8) Get a provisioning list entry info\n");
        printf("(9) Remove a provisioning list entry\n");
        printf("(10) Get all provisioning list entries\n");
        printf("(11) Remove all provisioning list entries\n");
        printf("-----------------------------------------------------------------\n");
        printf("(12) Start network health check\n");
        printf("-----------------------------------------------------------------\n");
        printf("(d) Display network information\n");
        printf("(x) Exit menu\n");

        switch (prompt_menu(hl_appl, "Your choice:\n"))
        {
            case 1: /*Cancel current operation*/
                result = zwnet_abort(hl_appl->zwnet);
                if (result == 0)
                {
                    printf("Operation cancelled.\n");
                    hl_appl->add_status = ADD_NODE_STS_UNKNOWN;
                    hl_appl->rm_status = RM_NODE_STS_UNKNOWN;
                    hl_appl->nw_updt_status = NW_UPDT_STS_UNKNOWN;
                    hl_appl->node_updt_status = NODE_UPDT_STS_UNKNOWN;
                    hl_appl->rst_status = RESET_NW_STS_UNKNOWN;
                    hl_appl->nw_health_chk_status = NW_HEALTH_CHK_STS_UNKNOWN;
                }
                else
                {
                    printf("Operation cannot be cancelled, error:%d\n", result);
                }
                break;

            case 2: /*Add node*/
                if ((hl_appl->add_status == ADD_NODE_STS_UNKNOWN) || (hl_appl->add_status == ADD_NODE_STS_DONE))
                {
                    hl_appl->add_status = ADD_NODE_STS_UNKNOWN;

                    result = hl_add_node(hl_appl);

                    if (!hl_appl->sec2_add_node)
                    {
                        if (result == 0)
                        {
                            printf("Add node in progress, please wait for status ...\n");
                        }
                        else
                        {
                            printf("Add node with error:%d\n", result);
                        }
                    }
                }
                else
                {
                    printf("Another add node operation in progress ...\n");
                }
                break;

            case 3: /*Remove node*/
                if ((hl_appl->rm_status == RM_NODE_STS_UNKNOWN) || (hl_appl->rm_status == RM_NODE_STS_DONE))
                {
                    hl_appl->rm_status = RM_NODE_STS_UNKNOWN;

                    result = zwnet_add(hl_appl->zwnet, 0, NULL, 0);
                    if (result == 0)
                    {
                        printf("Remove node in progress, please wait for status ...\n");
                        hl_appl->rm_status = RM_NODE_STS_PROGRESS;
                    }
                    else
                    {
                        printf("Remove node with error:%d\n", result);
                    }
                }
                break;

            case 4: /*Network update*/
                if ((hl_appl->nw_updt_status == NW_UPDT_STS_UNKNOWN) || (hl_appl->nw_updt_status == NW_UPDT_STS_DONE))
                {
                    hl_appl->nw_updt_status = NW_UPDT_STS_UNKNOWN;

                    result = zwnet_update(hl_appl->zwnet);
                    if (result == 0)
                    {
                        printf("Network update in progress, please wait for status ...\n");
                        hl_appl->nw_updt_status = NW_UPDT_STS_PROGRESS;
                    }
                    else
                    {
                        printf("Network update with error:%d\n", result);
                    }
                }
                break;

            case 5: /*Node update*/
                if ((hl_appl->node_updt_status == NODE_UPDT_STS_UNKNOWN) || (hl_appl->node_updt_status == NODE_UPDT_STS_DONE))
                {
                    hl_appl->node_updt_status = NODE_UPDT_STS_UNKNOWN;

                    node_id = prompt_uint(hl_appl, "Node id:");

                    result = zwnet_get_node_by_id(hl_appl->zwnet, node_id, &node);
                    if (result != 0)
                    {
                        printf("Failed to find node. Error:%d\n", result);
                        break;
                    }

                    result = zwnode_update(&node);
                    if (result == 0)
                    {
                        printf("Node update in progress, please wait for status ...\n");
                        hl_appl->node_updt_status = NODE_UPDT_STS_PROGRESS;
                    }
                    else
                    {
                        printf("Node update with error:%d\n", result);
                    }
                }
                break;

            case 6: /*Reset network*/
                if ((hl_appl->rst_status == RESET_NW_STS_UNKNOWN) || (hl_appl->rst_status == RESET_NW_STS_DONE))
                {
                    hl_appl->rst_status = RESET_NW_STS_UNKNOWN;

                    result = zwnet_reset(hl_appl->zwnet);
                    if (result == 0)
                    {
                        printf("Reset network in progress, please wait for status ...\n");
                        hl_appl->rst_status = RESET_NW_STS_PROGRESS;
                    }
                    else
                    {
                        printf("Reset network with error:%d\n", result);
                    }
                }
                break;

            case 7: /*Add a provisioning list entry*/
                hl_pl_add(hl_appl);
                break;

            case 8: /*Get a provisioning list entry info*/
                if (prompt_str(hl_appl, "DSK:", 200, dsk_str))
                {
                    result = zwnet_pl_get(hl_appl->zwnet, dsk_str, hl_pl_info_cb, hl_appl);
                    if (result != 0)
                    {
                        printf("Get a provisioning list entry info with error:%d\n", result);
                    }
                }
                break;

            case 9: /*Remove a provisioning list entry*/
                if (prompt_str(hl_appl, "DSK:", 200, dsk_str))
                {
                    result = zwnet_pl_del(hl_appl->zwnet, dsk_str);
                    if (result != 0)
                    {
                        printf("Remove a provisioning list entry with error:%d\n", result);
                    }
                }
                break;

            case 10: /*Get all provisioning list entries*/
                result = zwnet_pl_list_get(hl_appl->zwnet, hl_pl_list_cb, hl_appl);
                if (result != 0)
                {
                    printf("Get all provisioning list entries with error:%d\n", result);
                }
                break;


            case 11: /*Remove all provisioning list entries*/
                result = zwnet_pl_list_del(hl_appl->zwnet);
                if (result != 0)
                {
                    printf("Remove all provisioning list entries with error:%d\n", result);
                }
                break;

            case 12: /*Network health check*/
                if ((hl_appl->nw_health_chk_status == NW_HEALTH_CHK_STS_UNKNOWN) || (hl_appl->nw_health_chk_status == NW_HEALTH_CHK_STS_DONE))
                {
                    hl_appl->nw_health_chk_status = NW_HEALTH_CHK_STS_UNKNOWN;

                    result = zwnet_health_chk(hl_appl->zwnet);
                    if (result == 0)
                    {
                        printf("Network health check in progress, please wait for status ...\n");
                        hl_appl->nw_health_chk_status = NW_HEALTH_CHK_STS_PROGRESS;
                    }
                    else
                    {
                        printf("Network health check with error:%d\n", result);
                    }
                }
                break;

            case (ALPHA_OFFSET + 'd'):
                hl_nw_show(hl_appl);
                break;

            case (ALPHA_OFFSET + 'x'):
                to_run = 0;
                break;
        }
    }
}


/**
bin_switch_menu - binary switch menu
@return
*/
static void bin_switch_menu(hl_appl_ctx_t *hl_appl)
{
    int         to_run = 1;
    int         result;
    int         choice;
    unsigned    node_id;
    unsigned    ep_id;
    zwifd_t     intf = {0};

    while (to_run)
    {
        printf("\n(1) Select the binary switch to control\n");
        printf("(2) Turn on switch\n");
        printf("(3) Turn off switch\n");
        printf("(d) Display network information\n");
        printf("(x) Exit menu\n");

        choice = prompt_menu(hl_appl, "Your choice:\n");
        switch (choice)
        {
            case 1: /*Select the binary switch*/
                node_id = prompt_uint(hl_appl, "Node id:");
                ep_id = prompt_uint(hl_appl, "Endpoint id:");

                result = zwnet_get_if_by_id(hl_appl->zwnet, node_id, ep_id, COMMAND_CLASS_SWITCH_BINARY, &intf);

                if (result != 0)
                {
                    printf("Failed to find binary switch interface. Error:%d\n", result);
                    intf.nodeid = 0;
                }
                break;

            case 2: /*Turn on switch*/
            case 3: /*Turn off switch*/
                if (intf.nodeid)
                {
                    result = zwif_switch_set(&intf, (choice == 2)? 1 : 0, 0, NULL, NULL);
                    if (result != 0)
                    {
                        printf("zwif_switch_set with error:%d\n", result);
                    }
                }
                else
                {
                    printf("Please select the binary switch to control before using this command.\n");
                }
                break;

            case (ALPHA_OFFSET + 'd'):
                hl_nw_show(hl_appl);
                break;

            case (ALPHA_OFFSET + 'x'):
                to_run = 0;
                break;
        }
    }
}


/**
hl_bin_snsr_rep_cb - binary sensor report callback
@param[in]	ifd	    interface that received the report
@param[in]	state	state of the sensor: 0=idle, else event detected
@param[in]	type	sensor type, ZW_BSENSOR_TYPE_XXX. If type equals to zero, sensor type is unknown.
@param[in]	ts      time stamp.  If this is zero, the callback has no data and hence other parameter
                    values should be ignored.
@return
*/
static void hl_bin_snsr_rep_cb(zwifd_p ifd, uint8_t state, uint8_t type, time_t ts)
{
    (void)ifd;
    if (ts == 0)
    {
        printf("\nBinary sensor report: no cached data. Report get: type=%u\n", type);
        return;
    }

    if (type > ZW_BSENSOR_TYPE_GLASS_BRK)
    {
        type = 0;
    }

    printf("\nBinary sensor (%s) state :%s", bsensor_type_str[type], (state == 0)? "idle" : "event detected");
    printf("\nTime stamp:%s\n", ctime(&ts));

}


/**
hl_bsnsr_sup_report_cb - Report callback for supported sensor types
@param[in]	ifd	        interface
@param[in]	type_len    size of sensor type buffer
@param[in]	type        buffer to store supported sensor types (ZW_SENSOR_TYPE_XXX)
@param[in]	valid       validity of the report. If 1 the report is valid; else this report contains no data and should be ignored.
*/
static void hl_bsnsr_sup_report_cb(zwifd_p ifd, uint8_t type_len, uint8_t *type, int valid)
{
    (void)ifd; (void)type_len; (void)type;
    if (!valid)
    {
        printf("\nNo cache for supported binary sensor types\n");
        return;
    }

    if (type_len > 0)
    {
        int i;
        printf("\nZ-Wave binary sensor types: ");
        for (i=0; i<type_len; i++)
        {
            if (type[i] > ZW_BSENSOR_TYPE_GLASS_BRK)
            {
                type[i] = 0;
            }
            if (i == 0)
            {
                printf("%s", bsensor_type_str[type[i]]);
            }
            else
            {
                printf(", %s", bsensor_type_str[type[i]]);
            }
        }
        printf("\n");
    }
}


/**
bin_sensor_menu - binary sensor menu
@return
*/
static void bin_sensor_menu(hl_appl_ctx_t *hl_appl)
{
    int         to_run = 1;
    int         result;
    int         choice;
    int         i;
    unsigned    node_id;
    unsigned    ep_id;
    unsigned    sensor_type;
    zwifd_t     intf = {0};

    while (to_run)
    {
        printf("\n(1) Select the binary sensor to control\n");
        printf("(2) Get supported binary sensor types\n");
        printf("(3) Get live binary sensor report\n");
        printf("(4) Get cache binary sensor report\n");
        printf("(d) Display network information\n");
        printf("(x) Exit menu\n");

        choice = prompt_menu(hl_appl, "Your choice:\n");
        switch (choice)
        {
            case 1: /*Select the binary sensor*/
                node_id = prompt_uint(hl_appl, "Node id:");
                ep_id = prompt_uint(hl_appl, "Endpoint id:");

                result = zwnet_get_if_by_id(hl_appl->zwnet, node_id, ep_id, COMMAND_CLASS_SENSOR_BINARY, &intf);

                if (result != 0)
                {
                    printf("\nFailed to find binary sensor interface. Error:%d\n", result);
                    intf.nodeid = 0;
                    break;
                }
                printZwifd(&intf);
                result = zwif_bsensor_rpt_set(&intf, hl_bin_snsr_rep_cb);

                if (result != 0)
                {
                    printf("\nBinary sensor report setup with error:%d\n", result);
                }

                break;

            case 2: /*Get supported binary sensor types*/
                if (!intf.nodeid)
                {
                    printf("Please select the binary sensor to control before using this command.\n");
                    break;
                }

                result = zwif_bsensor_sup_get(&intf, hl_bsnsr_sup_report_cb, 0);

                if (result < 0)
                {
                    printf("\nGet supported binary sensor with error:%d\n", result);
                }
                break;

            case 3: /*Get live binary sensor report*/
            case 4: /*Get cache binary sensor report*/
                if (!intf.nodeid)
                {
                    printf("Please select the binary sensor to control before using this command.\n");
                    break;
                }

                printf("\nBinary Sensor types:");
                for (i=1; i<=ZW_BSENSOR_TYPE_GLASS_BRK; i++)
                {
                    printf("\n<%d> %s", i, bsensor_type_str[i]);
                }
                printf("\n<255> First supported sensor\n");
                sensor_type = prompt_uint(hl_appl, "Enter sensor type:");

                result = zwif_bsensor_get(&intf, sensor_type, (choice == 3)? ZWIF_GET_BMSK_LIVE : ZWIF_GET_BMSK_CACHE);

                if (result != 0)
                {
                    printf("\nGet binary sensor reading with error:%d\n", result);
                }
                break;

            case (ALPHA_OFFSET + 'd'):
                hl_nw_show(hl_appl);
                break;

            case (ALPHA_OFFSET + 'x'):
                to_run = 0;
                break;
        }
    }
}


/**
door_lock_menu - door lock menu
@return
*/
static void door_lock_menu(hl_appl_ctx_t *hl_appl)
{
    int         to_run = 1;
    int         result;
    int         choice;
    unsigned    node_id;
    unsigned    ep_id;
    zwifd_t     intf = {0};

    while (to_run)
    {
        printf("\n(1) Select the door lock to control\n");
        printf("(2) Lock door lock\n");
        printf("(3) Unlock door lock\n");
        printf("(d) Display network information\n");
        printf("(x) Exit menu\n");

        choice = prompt_menu(hl_appl, "Your choice:\n");
        switch (choice)
        {
            case 1: /*Select the door lock*/
                node_id = prompt_uint(hl_appl, "Node id:");
                ep_id = prompt_uint(hl_appl, "Endpoint id:");

                result = zwnet_get_if_by_id(hl_appl->zwnet, node_id, ep_id, COMMAND_CLASS_DOOR_LOCK, &intf);

                if (result != 0)
                {
                    printf("Failed to find door lock interface. Error:%d\n", result);
                    intf.nodeid = 0;
                }
                break;

            case 2: /*Lock door lock*/
            case 3: /*Unlock door lock*/
                if (intf.nodeid)
                {
                    result = zwif_dlck_op_set(&intf, (choice == 2)? ZW_DOOR_SEC :ZW_DOOR_UNSEC, NULL, NULL);
                    if (result != 0)
                    {
                        printf("zwif_dlck_op_set with error:%d\n", result);
                    }
                }
                else
                {
                    printf("Please select the door lock to control before using this command.\n");
                }
                break;

            case (ALPHA_OFFSET + 'd'):
                hl_nw_show(hl_appl);
                break;

            case (ALPHA_OFFSET + 'x'):
                to_run = 0;
                break;
        }
    }
}


/**
hl_alrm_report_cb - notification status report callback
@param[in]      ifd             The interface that received the report
@param[in]      alarm_info  alarm info
@param[in]      ts      time stamp.  If this is zero, the callback has no data and hence other parameter
                    values should be ignored.
@return
*/
static void hl_alrm_rep_cb(zwifd_p ifd, zwalrm_p  alarm_info, time_t ts)
{
    if_rec_alarm_match_t    *rec_head;
    char                    *alrm_type_name = NULL;
    char                    *alrm_evt_name = NULL;

    if (ts == 0)
    {
        printf("\nNotification report: no cached data. Report get: vtype=%u, ztype=%u, evt=%u\n",
                alarm_info->type, alarm_info->ex_type, alarm_info->ex_event);
        return;
    }

    if (zwif_alrm_vtype_sup_get(ifd, &rec_head) == 0)
    {
        while (rec_head)
        {
            if (rec_head->type == alarm_info->type)
            {   //Found alarm type
                alrm_type_name = rec_head->pResult->type_name;

                //Find event
                if (rec_head->level_low == alarm_info->level ||
                        rec_head->pResult->level == alarm_info->level)
                {
                    alrm_evt_name = rec_head->pResult->level_name;
                    break;
                }
            }
            rec_head = rec_head->next;
        }
    }

    printf("\nVendor specific alarm type:%u (%s), alarm level:%u (%s)\n",
                 alarm_info->type, (alrm_type_name)? alrm_type_name : "unknown",
                 alarm_info->level, (alrm_evt_name)? alrm_evt_name : "unknown");

    if (alarm_info->ex_info)
    {
        printf("Zensor net source node id:%u,\n", alarm_info->ex_zensr_nodeid);
        printf("Z-Wave defined:");
        printf("alarm type:%s, status:%u, event:%u\n",
                     (alarm_info->ex_type <= MAX_ALARM_TYPE)? alrm_type_str[alarm_info->ex_type] : alrm_type_str[0],
                     alarm_info->ex_status, alarm_info->ex_event);

        if (alarm_info->ex_evt_len)
        {
            switch (alarm_info->ex_evt_type)
            {
                case ZW_ALRM_PARAM_LOC:
                    printf("alarm location:%s\n", alarm_info->ex_evt_prm);
                    break;

                case ZW_ALRM_PARAM_USRID:
                    {
                        uint16_t    user_id;

                        if (alarm_info->ex_evt_len == 1)
                        {
                            user_id = alarm_info->ex_evt_prm[0];
                        }
                        else
                        {
                            user_id = U8_TO_16BIT(alarm_info->ex_evt_prm);
                        }
                        printf("user id:%u\n", user_id);
                    }
                    break;

                case ZW_ALRM_PARAM_OEM_ERR_CODE:
                    printf("OEM proprietary system failure code:\n");
                    bin_show(alarm_info->ex_evt_prm, alarm_info->ex_evt_len);
                    break;

                case ZW_ALRM_PARAM_PROPRIETARY:
                    printf("Proprietary event parameters:\n");
                    bin_show(alarm_info->ex_evt_prm, alarm_info->ex_evt_len);
                    break;

                case ZW_ALRM_PARAM_EVENT_ID:
                    printf("Event id: %u is no more active.\n", alarm_info->ex_evt_prm[0]);
                    break;

                case ZW_ALRM_PARAM_UNKNOWN:
                    printf("Unknown event parameters (Not defined in CC spec, or from a higher version of CC):\n");
                    bin_show(alarm_info->ex_evt_prm, alarm_info->ex_evt_len);
                    break;

                default:
                    printf("Undefined event parameter type: %u\n", alarm_info->ex_evt_type);
                    bin_show(alarm_info->ex_evt_prm, alarm_info->ex_evt_len);
            }
        }

        if(alarm_info->ex_has_sequence)
        {
            printf("Sequence number:%d\n", alarm_info->ex_sequence_no);
        }
        else
        {
            printf("No sequence number\n");
        }
    }

    printf("\nTime stamp:%s\n", ctime(&ts));
}


/**
hl_alrm_sup_report_cb - Report callback for supported alarm types
@param[in]      ifd             interface
@param[in]      have_vtype  flag to indicate whether vendor specific alarm type supported. 1=supported; else 0=unsupported
@param[in]      ztype_len   size of ztype buffer
@param[in]      ztype       buffer to store supported Z-Wave alarm types (ZW_ALRM_XXX)
@param[in]      valid       validity of the report. If 1 the report is valid; else this report contains no data and should be ignored.
*/
static void hl_alrm_sup_report_cb(zwifd_p ifd, uint8_t have_vtype, uint8_t ztype_len, uint8_t *ztype, int valid)
{
    (void)ifd;
    if (!valid)
    {
        printf("\nNo cache for supported alarm types\n");
        return;
    }

    printf("Vendor specific alarm type is %s\n", (have_vtype)? "supported" : "unsupported");

    if (ztype_len > 0)
    {
        int i;
        printf("\nSupported Z-Wave alarm types: ");
        for (i=0; i<ztype_len; i++)
        {
            if (ztype[i] > MAX_ALARM_TYPE)
            {
                ztype[i] = 0;
            }
            if (i == 0)
            {
                printf("%s", alrm_type_str[ztype[i]]);
            }
            else
            {
                printf(", %s", alrm_type_str[ztype[i]]);
            }
        }
        printf("\n");
    }
}


/**
hl_alrm_sup_evt_report_cb - Report callback for supported alarm events
@param[in]      ifd             interface
@param[in]      ztype       Z-Wave alarm type (ZW_ALRM_XXX)
@param[in]      evt_len     size of evt buffer
@param[in]      evt         buffer to store supported event of the alarm type specified in ztype
@param[in]      valid       validity of the report. If 1 the report is valid; else this report contains no data and should be ignored.
*/
void hl_alrm_sup_evt_report_cb(zwifd_p ifd, uint8_t ztype, uint8_t evt_len, uint8_t *evt, int valid)
{
    (void)ifd;
    if (!valid)
    {
        printf("\nNo cache for supported alarm events\n");
        return;
    }

    if (evt_len > 0)
    {
        int i;
        printf("\nZ-Wave alarm type = %s, supported events: ",
                     (ztype <= MAX_ALARM_TYPE)? alrm_type_str[ztype] : alrm_type_str[0]);
        for (i=0; i<evt_len; i++)
        {
            if (i == 0)
            {
                printf("%u", evt[i]);
            }
            else
            {
                printf(", %u", evt[i]);
            }
        }
        printf("\n");
    }
}


/**
notification_menu - notification menu
@return
*/
static void notification_menu(hl_appl_ctx_t *hl_appl)
{
    int         to_run = 1;
    int         result;
    int         choice;
    int         i;
    unsigned    node_id;
    unsigned    ep_id;
    signed      alrm_vtype;
    unsigned    alrm_ztype;
    unsigned    alrm_evt;
    unsigned    alrm_sts;
    if_rec_alarm_match_t    *rec_head;
    uint8_t     *ptmp;
    zwifd_t     intf = {0};

    while (to_run)
    {
        printf("\n(1) Select the notification device to control\n");
        printf("(2) Get supported vendor specific alarm types\n");
        printf("(3) Get supported Z-Wave alarm types\n");
        printf("(4) Get supported Z-Wave alarm events\n");
        printf("(5) Get live notification report\n");
        printf("(6) Get cache notification report\n");
        printf("(7) Set alarm activity\n");
        printf("(d) Display network information\n");
        printf("(x) Exit menu\n");

        choice = prompt_menu(hl_appl, "Your choice:\n");
        switch (choice)
        {
            case 1: /*Select the notification device*/
                node_id = prompt_uint(hl_appl, "Node id:");
                ep_id = prompt_uint(hl_appl, "Endpoint id:");

                result = zwnet_get_if_by_id(hl_appl->zwnet, node_id, ep_id, COMMAND_CLASS_ALARM, &intf);

                if (result != 0)
                {
                    printf("\nFailed to find notification interface. Error:%d\n", result);
                    intf.nodeid = 0;
                    break;
                }

                result = zwif_alrm_rpt_set(&intf, hl_alrm_rep_cb);

                if (result != 0)
                {
                    printf("\nNotification report setup with error:%d\n", result);
                }

                break;

            case 2: /*Get supported vendor specific alarm types*/
                if (!intf.nodeid)
                {
                    printf("Please select the notification device to control before using this command.\n");
                    break;
                }

                result = zwif_alrm_vtype_sup_get(&intf, &rec_head);

                if (result < 0)
                {
                    printf("\nGet supported vendor specific alarm types with error:%d\n", result);
                }
                else
                {
                    while (rec_head)
                    {
                        printf("Matching \n");

                        if (rec_head->type != -1)
                            printf("\tVendor specific alarm type:%u\n", (unsigned)rec_head->type);

                        if (rec_head->isRange == 1)
                            printf("\tAlarm level range:[%u, %u] \n", (unsigned)rec_head->level_low, (unsigned)rec_head->level_high);
                        else if (rec_head->isRange == 0)
                            printf("\tAlarm level:%u \n", (unsigned)rec_head->level_low);

                        if (rec_head->ex_type != -1)
                            printf("\tZ-Wave alarm type:%u\n", (unsigned)rec_head->ex_type);

                        if (rec_head->ex_event != -1)
                            printf("\tZ-Wave alarm event:%u\n", (unsigned)rec_head->ex_event);

                        if (rec_head->ex_evt_len != -1 && rec_head->ex_evt_len != 0)
                        {
                            printf("\tZ-Wave alarm event param:\n");

                            for (i = 0, ptmp = rec_head->pEx_evt_prm; i < rec_head->ex_evt_len; i++, ptmp++)
                                printf("\t%u \n", *ptmp);
                        }

                        printf("To \n");

                        if (rec_head->pResult->type != -1)
                            printf("\tVendor specific alarm type:%u\n", (unsigned)rec_head->pResult->type);

                        if (rec_head->pResult->level != -1)
                            printf("\tAlarm level:%u \n", (unsigned)rec_head->pResult->level);

                        if (rec_head->pResult->ex_type != -1)
                            printf("\tZ-Wave alarm type:%u\n", (unsigned)rec_head->pResult->ex_type);

                        if (rec_head->pResult->type_name)
                            printf("\t%s\n", rec_head->pResult->type_name);

                        if (rec_head->pResult->ex_event != -1)
                            printf("\tZ-Wave alarm event:%u\n", (unsigned)rec_head->pResult->ex_event);

                        if (rec_head->pResult->level_name)
                            printf("\t%s\n", rec_head->pResult->level_name);

                        if (rec_head->pResult->ex_evt_len != -1 && rec_head->pResult->ex_evt_len != 0)
                        {
                            printf("\tZ-Wave alarm event param:\n");

                            for (i = 0; i < rec_head->pResult->ex_evt_len; i++)
                                printf("\t%u \n", (unsigned)rec_head->pResult->pEx_evt_prm[i]);
                        }

                        rec_head = rec_head->next;
                    }
                }
                break;

            case 3: /*Get supported Z-Wave alarm types*/
                if (!intf.nodeid)
                {
                    printf("Please select the notification device to control before using this command.\n");
                    break;
                }

                result = zwif_alrm_sup_get(&intf, hl_alrm_sup_report_cb, 0);

                if (result < 0)
                {
                    printf("\nGet supported Z-Wave alarm types with error:%d\n", result);
                }
                break;

            case 4: /*Get supported alarm events*/
                if (!intf.nodeid)
                {
                    printf("Please select the notification device to control before using this command.\n");
                    break;
                }

                printf("\nZ-Wave alarm types:");
                for (i=1; i<=MAX_ALARM_TYPE; i++)
                {
                    printf("\n<%d> %s", i, alrm_type_str[i]);
                }
                alrm_ztype = prompt_uint(hl_appl, "\nEnter Z-Wave alarm type:");

                result = zwif_alrm_sup_evt_get(&intf, alrm_ztype, hl_alrm_sup_evt_report_cb, 0);

                if (result < 0)
                {
                    printf("\nGet supported alarm events with error:%d\n", result);
                }
                break;

            case 5: /*Get live notification report*/
            case 6: /*Get cache notification report*/
                if (!intf.nodeid)
                {
                    printf("Please select the notification device to control before using this command.\n");
                    break;
                }

                alrm_vtype = prompt_int(hl_appl, "Enter vendor specific alarm type [-1 for don't care]:");

                printf("\nZ-Wave alarm types:");
                for (i=1; i<=MAX_ALARM_TYPE; i++)
                {
                    printf("\n<%d> %s", i, alrm_type_str[i]);
                }
                printf("\n<255> First supported alarm type\n");
                alrm_ztype = prompt_uint(hl_appl, "Enter Z-Wave alarm type:");

                if ((alrm_ztype == 0) || (alrm_ztype == 255))
                {
                    alrm_evt = 0;
                }
                else
                {
                    alrm_evt = prompt_uint(hl_appl, "Z-Wave alarm event:");
                }

                result = zwif_alrm_get(&intf, alrm_vtype, alrm_ztype, alrm_evt, (choice == 5)? ZWIF_GET_BMSK_LIVE : ZWIF_GET_BMSK_CACHE);

                if (result != 0)
                {
                    printf("\nGet alarm reading with error:%d\n", result);
                }
                break;

            case 7: /*Set alarm activity*/
                if (!intf.nodeid)
                {
                    printf("Please select the notification device to control before using this command.\n");
                    break;
                }

                printf("\nZ-Wave alarm types:");
                for (i=1; i<=MAX_ALARM_TYPE; i++)
                {
                    printf("\n<%d> %s", i, alrm_type_str[i]);
                }
                alrm_ztype = prompt_uint(hl_appl, "\nEnter Z-Wave alarm type:");

                printf("\nZ-Wave alarm status:\n<0> Deactivate\n<255> Activate");
                alrm_sts = prompt_uint(hl_appl, "\nEnter Z-Wave alarm status:");

                result = zwif_alrm_set(&intf, alrm_ztype, alrm_sts);

                if (result < 0)
                {
                    printf("\nSet alarm activity with error:%d\n", result);
                }
                break;

            case (ALPHA_OFFSET + 'd'):
                hl_nw_show(hl_appl);
                break;

            case (ALPHA_OFFSET + 'x'):
                to_run = 0;
                break;
        }
    }
}


/**
main_menu - display and run main menu
@return
*/
static void main_menu(hl_appl_ctx_t *hl_appl)
{
    int     to_run = 1;
    int     choice;

    while (to_run)
    {
        printf("\n(1) Manage network\n");
        printf("(2) Binary switch\n");
        printf("(3) Binary sensor\n");
        printf("(4) Door lock\n");
        printf("(5) Notification (Alarm)\n");
        printf("(d) Display network information\n");
        printf("(r) Reload menu\n");
        printf("(x) Exit program\n");

        choice = prompt_menu(hl_appl, "Your choice:\n");

        //Filter out invalid choices when initialization is not done.
        if ((choice > 0) && (choice <= 5))
        {
            if (hl_appl->init_status != NW_INI_STS_DONE)
            {
                printf("Choice (%d) is unavailable in current state.\n", choice);
                continue;
            }
        }

        switch (choice)
        {
            case 1:
                network_menu(hl_appl);
                break;
            case 2:
                bin_switch_menu(hl_appl);
                break;
            case 3:
                bin_sensor_menu(hl_appl);
                break;
            case 4:
                door_lock_menu(hl_appl);
                break;
            case 5:
                notification_menu(hl_appl);
                break;
            case (ALPHA_OFFSET + 'd'):
                hl_nw_show(hl_appl);
                break;
            case (ALPHA_OFFSET + 'r'):
                break;
            case (ALPHA_OFFSET + 'x'):
                to_run = 0;
                break;
        }
    }
}


/**
hl_exit - Clean up and exit
@param[in]	hl_appl		The high-level api context
@return
*/
void    hl_exit(hl_appl_ctx_t *hl_appl)
{
    plt_mtx_destroy(hl_appl->hl_mtx);

    if (hl_appl->init_status >= NW_INI_STS_PROGRESS)
    {
        zwnet_exit(hl_appl->zwnet);
    }
}


int main(int argc, char **argv)
{
    (void)argc; (void)argv;
    hl_appl_ctx_t   appl_ctx = {0};
    int             result;
    print_hl_appl_ctx(&appl_ctx);
    //Initialize
    if (!plt_mtx_init(&appl_ctx.hl_mtx))
    {
        return -1;
    }

    //Initialize network
    result = nw_init(&appl_ctx);
    print_hl_appl_ctx(&appl_ctx);

    if (result > 0)
    {
        result = subm_select_gw(&appl_ctx);
    }
    print_hl_appl_ctx(&appl_ctx);
    if (result == 0)
    {
        main_menu(&appl_ctx);
    }

    hl_exit(&appl_ctx);

    return 0;
}

