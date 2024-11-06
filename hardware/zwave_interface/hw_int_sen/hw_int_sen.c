#include <malloc.h>
#include <memory.h>
#include <string.h>
#include <stdint.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <ctype.h>
#include <time.h>
#include <pthread.h>
#include <sqlite3.h>
#include <cjson/cJSON.h>

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

/** Mutex for battery update worker updating db*/
pthread_mutex_t db_mutex = PTHREAD_MUTEX_INITIALIZER;


/** sqlite node data context*/
typedef struct 
{
    int32_t     node_id;                
    int32_t     node_end_point;
    cJSON       *node_info;  		    ///< Use cJSON pointer to represent hl node data
    int32_t     node_sensor_type;       ///< 0 for door windows sensor 1 for tilt sensor
    int32_t     node_battery;           ///< value battery level express from 0-100
    char        last_battery_ts[20];    ///< time stamp las battery level update
    int32_t     node_state;             ///< state sensor 0 idle 255 event detected (open or shock)
    char        last_state_ts[20];      ///< time stamp last event 
} hl_sqlite_node_ctx_t;


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


/** print function of fundamentals data structures used by zware c api */
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
    //printf("sec2_add_prm: %p\n", ctx->sec2_add_prm);
    printf("psk_len: %d\n", ctx->psk_len);
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
    printf("zip_gw_ip: ");
    for (int i = 0; i < 16; i++) {
        printf("%u ", ctx->zip_gw_ip[i]);
    }
    printf("\n");
    printf("hl_mtx: %p\n", ctx->hl_mtx);
}


void print_hl_zwifd(const zwifd_t *zwifd) 
{
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
            return 2;//using IPv6guestguest
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
is_key_exists - Check if a key already exists in a cJSON object
@param[in]	cJSON	  json object
@param[in]	char      pointer of string representing key of dict
@return     int
*/
int is_key_exists(cJSON *object, const char *key) {
    cJSON* item = cJSON_GetObjectItemCaseSensitive(object, key);
    return (item != NULL);
}


/**
is_key_exists - Add a key-value pair to a cJSON object only if the key does not already exist
@param[in]	cJSON	  json object
@param[in]	char      pointer of string representing key of dict
@param[in]  cJSON     pointer of the value of a dictionnary key
@return     
*/
void add_json_key_value(cJSON *object, const char *key, cJSON *value) {
    if (!is_key_exists(object, key)) {
        cJSON_AddItemToObject(object, key, value);
    }
    else
   {
   cJSON_ReplaceItemInObject(object, key, value);
   }
}



/**
hl_sup_bsensor_show - Show supported binary sensor types - create node info dictionnary - dict resume node info -> store in sqlite db
@param[in]	intf	  Binary sensor interface
@param[in]  *sql_node_tmp_ctx pointer of node info descriptor (sqlite DB)
@return
*/
static void hl_sup_bsensor_show(zwifd_p intf, hl_sqlite_node_ctx_t *sql_node_tmp_ctx)
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
	if ((strcmp(bsensor_type_str[type], "Door/Window sensor") == 0))
        {
            add_json_key_value(sql_node_tmp_ctx->node_info, "sensortype:", cJSON_CreateString(bsensor_type_str[type]));
            sql_node_tmp_ctx->node_sensor_type = 0;
        }
        else if ((strcmp(bsensor_type_str[type], "Tamper sensor") == 0))
        {
            add_json_key_value(sql_node_tmp_ctx->node_info, "sensortype:", cJSON_CreateString(bsensor_type_str[type]));
            sql_node_tmp_ctx->node_sensor_type = 1;
        }
    }
}


/**
hl_grp_info_show - Show detailed group information
@param[in]	intf   interface
@param[in]  *sql_node_tmp_ctx pointer of node info descriptor (sqlite DB)
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
@param[in]  *sql_node_tmp_ctx pointer of node info descriptor (sqlite DB)
@return
*/
static void hl_intf_show(zwifd_p intf, hl_sqlite_node_ctx_t *sql_node_tmp_ctx)
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
            hl_sup_bsensor_show(intf, sql_node_tmp_ctx);
            break;

        case COMMAND_CLASS_ASSOCIATION_GRP_INFO:
            hl_grp_info_show(intf);
            break;

    }
}


/**
hl_ep_show - Show endpoint info - create node info dictionnary - dict resume node info -> store in sqlite db
@param[in]	ep    Endpoint descriptor
@param[in]  *sql_node_tmp_ctx pointer of node info descriptor (sqlite DB)
@return
*/
static void hl_ep_show(zwepd_p ep, hl_sqlite_node_ctx_t *sql_node_tmp_ctx)
{
    printf("Endpoint id:%u\n", ep->epid);
    add_json_key_value(sql_node_tmp_ctx->node_info, "endpoint_id", cJSON_CreateNumber(ep->epid));
    sql_node_tmp_ctx->node_end_point = ep->epid;
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
    add_json_key_value(sql_node_tmp_ctx->node_info, "device_class_generic", cJSON_CreateNumber(ep->generic));
    add_json_key_value(sql_node_tmp_ctx->node_info, "device_class_specific", cJSON_CreateNumber(ep->specific));

    printf("Endpoint name:%s\n", ep->name);
    add_json_key_value(sql_node_tmp_ctx->node_info, "endpoint_name", cJSON_CreateString(ep->name));

    printf("Endpoint location:%s\n", ep->loc);
    add_json_key_value(sql_node_tmp_ctx->node_info, "endpoint_locatio", cJSON_CreateString(ep->loc));
}


/**
hl_node_show - Show node info - create node info dictionnary - dict resume node info -> store in sqlite db
@param[in]	node    Node descriptor
@param[in]  *sql_node_tmp_ctx pointer of node info descriptor (sqlite DB)
@return
*/
static void hl_node_show(zwnoded_p node, hl_sqlite_node_ctx_t *sql_node_tmp_ctx)
{
    printf("\n__________________________________________________________________________\n");
    printf("Node id:%u\n", (unsigned)node->nodeid);
    sql_node_tmp_ctx->node_id = node->nodeid;
    add_json_key_value(sql_node_tmp_ctx->node_info, "nodeID", cJSON_CreateNumber(node->nodeid));
    //printf("JSON Object: %s\n", cJSON_Print(sql_node_tmp_ctx->node_info));

    if (node->sleep_cap)
    {
        printf("Node is capable to sleep\n");
    }
    add_json_key_value(sql_node_tmp_ctx->node_info, "node_capable_to_sleep", cJSON_CreateNumber(node->sleep_cap));

    if (node->restricted)
    {
        printf("Node is running in restricted mode\n");
    }
    add_json_key_value(sql_node_tmp_ctx->node_info, "node_is_running_inrestricted_mode", cJSON_CreateNumber(node->restricted));

    if (node->sensor)
    {
        printf("Node is a listening sleeping slave\n");
    }
    add_json_key_value(sql_node_tmp_ctx->node_info, "node_is_LSS", cJSON_CreateNumber(node->sensor));

    if (*node->s2_dsk)
    {
        printf("Node DSK: %s\n", node->s2_dsk);
    }
    add_json_key_value(sql_node_tmp_ctx->node_info, "node_DSK", cJSON_CreateString(node->s2_dsk));

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
    add_json_key_value(sql_node_tmp_ctx->node_info, "vendor_id", cJSON_CreateNumber(node->vid));

    printf("Product type id:%04Xh\n", node->type);
    add_json_key_value(sql_node_tmp_ctx->node_info, "product_type", cJSON_CreateNumber(node->type));

    printf("Product id:%04Xh\n", node->pid);
    add_json_key_value(sql_node_tmp_ctx->node_info, "product_id", cJSON_CreateNumber(node->pid));

    printf("Z-Wave library type:%u\n", node->lib_type);
    add_json_key_value(sql_node_tmp_ctx->node_info, "zwave_lib_type", cJSON_CreateNumber(node->lib_type));

    printf("Z-Wave protocol version:%u.%02u\n", (unsigned)(node->proto_ver >> 8), (unsigned)(node->proto_ver & 0xFF));

    printf("Application version:%u.%02u\n", (unsigned)(node->app_ver >> 8), (unsigned)(node->app_ver & 0xFF));
}


/**
hl_nw_show - Show network info
@param[in]	hl_appl		The high-level api context
@param[in]  *sql_node_tmp_ctx pointer of node info descriptor (sqlite DB)
@return
*/
static void hl_nw_show(hl_appl_ctx_t *hl_appl, hl_sqlite_node_ctx_t *sql_node_tmp_ctx)
{
    int         result;
    zwnet_p     net = hl_appl->zwnet;
    zwnetd_p    nw_desp;
    zwnoded_t   node;
    zwepd_t     ep;
    zwifd_t     intf;

    // initialize sql_node_tmp_ctx->node_sensor_type to unknow
    sql_node_tmp_ctx->node_sensor_type = -1;

    // sqlite open connection
    sqlite3* db;
    int rc;
    rc = sqlite3_open("pandore.db", &db);
    printf("code  open database. Return code: %d\n", rc);
    if (rc != SQLITE_OK) {
        printf("Failed to open database: %s\n", sqlite3_errmsg(db));
    }

    // Prepare and execute the DELETE statement
    char* deleteQuery = "DELETE FROM hw_interface_sensor";
    rc = sqlite3_exec(db, deleteQuery, 0, 0, 0);
    if (rc != SQLITE_OK) {
        printf("Failed to delete rows. Error: %s\n", sqlite3_errmsg(db));
    }
    // Prepare the insert statement
    const char* sql = "INSERT INTO hw_interface_sensor (NODE_ID, NODE_END_POINT, NODE_INFO, NODE_SENSOR_TYPE, NODE_BATTERY, LAST_BATTERY_TS, NODE_STATE, LAST_STATE_TS) VALUES (?, ?, ?, ?, ?, ?, ?, ?)";

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
        hl_node_show(&node, sql_node_tmp_ctx);

        if (!zwnode_get_ep(&node, &ep)) //get first endpoint of the node
        {
            do
            {
                hl_ep_show(&ep, sql_node_tmp_ctx);

                if (!zwep_get_if(&ep, &intf)) //get first interface of the endpoint
                {
                    do
                    {
                        hl_intf_show(&intf, sql_node_tmp_ctx);

                    }while (!zwif_get_next(&intf, &intf)); //get next interface
                }
            }while (!zwep_get_next(&ep, &ep)); //get next endpoint
        }
        printf("JSON Object: %s\n", cJSON_Print(sql_node_tmp_ctx->node_info));

        sqlite3_stmt* stmt;
        rc = sqlite3_prepare_v2(db, sql, -1, &stmt, NULL);
        if (rc != SQLITE_OK) {
            printf("Failed to prepare statement: %s\n", sqlite3_errmsg(db));
            sqlite3_close(db);
        }

        if (sql_node_tmp_ctx->node_sensor_type != -1)
        {
            // Bind values to the parameters in the prepared statement
            sqlite3_bind_int(stmt, 1, sql_node_tmp_ctx->node_id); // Example value for NODE_ID
            sqlite3_bind_int(stmt, 2, sql_node_tmp_ctx->node_end_point); // Example value for NODE_END_POINT
            sqlite3_bind_text(stmt, 3, cJSON_Print(sql_node_tmp_ctx->node_info), -1, SQLITE_STATIC); // Example value for NODE_INFO as JSON
            sqlite3_bind_int(stmt, 4, sql_node_tmp_ctx->node_sensor_type); // Example value for NODE_SENSOR_TYPE
            sqlite3_bind_int(stmt, 5, 4); // Example value for NODE_BATTERY
            sqlite3_bind_text(stmt, 6, "1900-01-01 00:00:00", -1, SQLITE_STATIC); // Example value for LAST_BATTERY_TS
            sqlite3_bind_int(stmt, 7, 5); // Example value for NODE_STATE
            sqlite3_bind_text(stmt, 8, "1900-01-01 00:00:00", -1, SQLITE_STATIC); // Example value for LAST_STATE_TS

            // Execute the statement
            rc = sqlite3_step(stmt);
            if (rc != SQLITE_DONE) {
                printf("Failed to execute statement: %s\n", sqlite3_errmsg(db));
            } else {
                printf("Data inserted successfully.\n");
            }
        }
        add_json_key_value(sql_node_tmp_ctx->node_info, "sensortype:", cJSON_CreateString("unknown"));
        result = zwnode_get_next(&node, &node); //get next node
    }
}


/**
sql_update_database - update sqlite db field
@param[in]	node_id
@param[in]	update_type	battery level or sensor state (idle or event)
@param[in]  new_state
@param[in]  ts          time stamp of update
@return
*/
void sql_update_database(int node_id, const char *update_type, int new_state, time_t ts) {
    sqlite3 *db;
    char sql[200];
    int rc;

    // Convert timestamp to a formatted string without special characters
    char timestamp_str[20];
    strftime(timestamp_str, sizeof(timestamp_str), "%Y-%m-%d %H:%M:%S", localtime(&ts));
    // Lock the mutex before accessing the database
    pthread_mutex_lock(&db_mutex);

    rc = sqlite3_open("pandore.db", &db);
    if (rc) {
        fprintf(stderr, "Failed to open database: %s\n", sqlite3_errmsg(db));
        sqlite3_close(db);
        return;
    }

    // Construct the SQL statement based on the update_type
    if (strcmp(update_type, "sensor_state") == 0) {
        snprintf(sql, sizeof(sql), "UPDATE hw_interface_sensor SET NODE_STATE = %d, LAST_STATE_TS = '%s' WHERE NODE_ID = %d;", new_state, timestamp_str, node_id);
    } else if (strcmp(update_type, "battery_state") == 0) {
        printf("inside sql battery ypdate statement\n");
        snprintf(sql, sizeof(sql), "UPDATE hw_interface_sensor SET NODE_BATTERY = %d, LAST_BATTERY_TS = '%s' WHERE NODE_ID = %d;", new_state, timestamp_str, node_id);
    } else {
        printf("Invalid update_type. Please specify either 'sensor_state' or 'battery_state'.\n");
        sqlite3_close(db);
        return;
    }

    rc = sqlite3_exec(db, sql, 0, 0, 0);
    if (rc != SQLITE_OK) {
        fprintf(stderr, "Failed to update data: %s\n", sqlite3_errmsg(db));
    } else {
        printf("Update successful!\n");
    }

    sqlite3_close(db);
    pthread_mutex_unlock(&db_mutex);
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
    printf("node id:%d\n", ifd->nodeid);
    // update sqlite db 
    sql_update_database(ifd->nodeid, "sensor_state", state, ts);
}


// Battery report callback function
void bat_rpt_cb(zwifd_t *ifd, zwbatt_dat_t *data, time_t ts) {
    const char *update_type = "battery_state";
    // Check if the data is valid
    char buffer[26];
    ctime_r(&ts, buffer); // ctime_r is a thread-safe version of ctime

    // Remove the newline character from the ctime output
    char *newline_pos = strchr(buffer, '\n');
    if (newline_pos != NULL) {
        *newline_pos = '\0';
    }

    // Print the battery data and timestamp
    printf("\nBattery Level report node ID: %d\n", ifd->nodeid);
    printf("Battery Level: %d%%\n", data->level);
    printf("Timestamp: %s\n", buffer);
    //update sqlite db
    sql_update_database(ifd->nodeid, update_type, data->level, ts);
}


typedef struct {
    int node_id;
    hl_appl_ctx_t *appl_ctx;
} WorkerArgs;


/**
worker_function -   worker use to implement cb for battery level 
@param[in]	*arg	data structure contain node_id and hl application context (other data structure )
@return
*/
void *worker_function(void *arg) {
    //dont understand ask chatgpt
    WorkerArgs *args = (WorkerArgs *)arg;
    int             node_id = args->node_id;
    hl_appl_ctx_t   *appl_ctx = args->appl_ctx;
    zwifd_t         intf_b = {0};
    int             result;
    int		        ep_id = 0;

    //Get battery interface node
    result = zwnet_get_if_by_id(appl_ctx->zwnet, node_id, ep_id, COMMAND_CLASS_BATTERY, &intf_b);
    if (result != 0)
    {
        printf("\n Failed to find battery class interface. Error:%d\n", result);
    }
    printf("\n Find  battery class interface success result : %d\n", result);

    //Set battery report callback
    result = zwif_battery_rpt_set(&intf_b, bat_rpt_cb);
    if (result != 0)
    {
        printf("\nFailed to setup battery report cb. Error:%d\n", result);
    }
    printf("Battery interface rpt cb setup success result:%d\n", result);

    printf("Launching worker thread battery report for NODE_ID: %d\n", node_id);
    while (1) {
	    //result = zwif_battery_get(&intf_b, ZWIF_GET_BMSK_LIVE);
        result = zwif_battery_get(&intf_b, ZWIF_GET_BMSK_CACHE);
        //result = zwif_battery_get(&intf_b, ZWIF_GET_BMSK_LIVE);
        sleep(2);
        if (result != ZW_ERR_NONE) {
            printf("Error retrieving battery level cache for NODE_ID: %d\n", node_id);
        }
        else{
            printf("battery level for NODE_ID retrieved: %d\n", node_id);
        }
    }
    return NULL;
}


/**
sqlite_node_init_cb -   loop in each node_id and launch battery worker, sensor state callback 
@param[in]	*arg	data structure contain node_id and hl application context (other data structure )
@return
*/
static int sqlite_node_init_cb(void *data, int argc, char **argv, char **azColNam) {
    // Loop through the query results and print the NODE_ID values
    hl_appl_ctx_t   *appl_ctx = (hl_appl_ctx_t*)data;
    zwnoded_t       node;
    int		    result;
    int  	    num_workers = argc - 1;

    pthread_t *worker_threads = (pthread_t *)malloc(num_workers * sizeof(pthread_t));

    //Get first node (controller node)
    result = zwnet_get_node(appl_ctx->zwnet, &node);
    if (result != 0)
    {
        printf("Error: couldn't find Z/IP gateway interface: %d\n", result);
    }
    printf("print Z/IP gateway interface result : %d\n", result);

    for (int i = 0; i < argc; i++) {
        printf("NODE_ID: %s\n", argv[i]);
        zwifd_t     intf = {0};
        int         node_id = atoi(argv[i]);
        int         ep_id   = 0;
        int         result;
        //BINARY SENSOR CALLBACK REPORT 
        //Select the binary sensor
        result = zwnet_get_if_by_id(appl_ctx->zwnet, node_id, ep_id, COMMAND_CLASS_SENSOR_BINARY, &intf);
        if (result != 0)
        {
            printf("\nFailed to find binary sensor interface. Error:%d\n", result);
            intf.nodeid = 0;
        }
        else{
            printf("\nFind binary sensor interface for node:%s\n", argv[i]);
        }
        //set callback function for binary sensor
        result = zwif_bsensor_rpt_set(&intf, hl_bin_snsr_rep_cb);
        if (result != 0)
        {
            printf("\nBinary sensor report setup with error:%d\n", result);
        }
        else{
            printf("\nBinary sensor report setup with success for node:%s\n", argv[i]);
        }
        //BINARY BATTERY CALLBACK REPORT
        // Allocate memory for WorkerArgs struct
        WorkerArgs *args = (WorkerArgs *)malloc(sizeof(WorkerArgs));
        args->node_id = node_id;
        args->appl_ctx = appl_ctx;

        // Create the worker thread
        int create_result = pthread_create(&worker_threads[i], NULL, worker_function, args);
        if (create_result) {
            printf("Error creating worker thread %d\n", i);
            return 1;
        }
        else{
            printf("Success creating worker thread %d\n", i);
        }
    }
    return 0;
}


int main(int argc, char **argv)
{
    (void)argc; (void)argv;

    hl_appl_ctx_t           appl_ctx = {0};
    hl_sqlite_node_ctx_t    sql_node_tmp_ctx = {0};
    sqlite3                 *db;
    int                     rc;

    // Create a cJSON object and assign it to the node_info member
    sql_node_tmp_ctx.node_info = cJSON_CreateObject();
    //Initialize
    if (!plt_mtx_init(&appl_ctx.hl_mtx))
    {
        return -1;
    }
    //Initialize network
    nw_init(&appl_ctx);
    while (appl_ctx.init_status != 3)
    {
        // Wait for intialization callback success
        if (appl_ctx.init_status == 3)
        {
            printf("initialisation success");
            break; // Exit the loop if appl_ctx is 3
        }
    }
    printf("init_status: %d\n", appl_ctx.init_status);
    hl_nw_show(&appl_ctx, &sql_node_tmp_ctx);
    // Open the database
    rc = sqlite3_open("pandore.db", &db);
    if (rc) {
        fprintf(stderr, "Failed to open database: %s\n", sqlite3_errmsg(db));
        sqlite3_close(db);
        return 1;
    }
    // Construct the SQL statement to select all NODE_ID values from hw_interface_sensor table
    char sql[100];
    snprintf(sql, sizeof(sql), "SELECT NODE_ID FROM hw_interface_sensor;");
    // Execute the query with the callback function to process the results
    rc = sqlite3_exec(db, sql, sqlite_node_init_cb, &appl_ctx, 0);
    if (rc != SQLITE_OK) {
        fprintf(stderr, "Failed to execute query: %s\n", sqlite3_errmsg(db));
    }
    // Close the database
    sqlite3_close(db);

    while (1)
    {
    sleep(20);
    }
    return 0;
}


