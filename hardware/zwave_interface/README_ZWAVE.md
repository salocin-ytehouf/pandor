# ZWAVE
## ZWAVE ubuntu local 
### Z-IP GATEWAY 

#### installation ubuntu 20 LTS 64bit

$sudo apt-get update
$sudo apt-get upgrade

$ sudo apt-get install libusb 
$ sudo apt-get libssl1
$ sudo apt-get radvd
$ sudo apt-get install parprouted
$ sudo apt-get install bridge-utils

Downlaod simplicity studio 
https://www.silabs.com/developers/simplicity-studio
go to SimplicityStudio_v5 folder and run
$ bash SimplicityStudio_v5/setup.sh

from simplicity studio download download the SDK 
 - z-wave SDK
 - z-wave Z-IP gateway SDK

all the documentation and package are found under /SimplicityStudio_v5/developer/sdks/zwave/

$ sudo dpkg --add-architecture i386
$ sudo apt-get update
$ sudo apt-get install -y doxygen graphviz mscgen roffit perl git python3 cmake gcc xsltproc bison flex gcc-9-multilib pkg-config:i386 libssl-dev:i386 libc6-dev:i386 libusb-1.0-0-dev:i386 libjson-c-dev:i386 openjdk-8-jre curl g++-9-multilib libstdc++-9-dev

go in the source z-ip folder

$ tar -xjvf zipgateway-7.18.03-Source.tar.bz2
$ cd zipgateway-7.18.03-Source/usr/local/

$ mkdir build 
$ cd build 
$ cmake ..
$ make
$ make package 
$ ls zipgateway-7.18.03-Linux-i386.deb
$ sudo dpkg -i zipgateway-7.18.03-Linux-i386.deb

#### Z-IP GATEWAY definition 

Z/IP Gateway, also known as Z-Wave over IP Gateway, is a technology that enables communication and interoperability between Z-Wave devices and IP (Internet Protocol) networks. Z-Wave is a wireless communication protocol commonly used for home automation and smart home devices.

The Z/IP Gateway serves as a bridge between Z-Wave devices and IP networks, allowing users to control and manage their Z-Wave devices through IP-based interfaces such as smartphones, tablets, or computers. It essentially extends the reach of Z-Wave devices beyond the limitations of a local Z-Wave network.

Here are some key points about Z/IP Gateway:

1. Z-Wave to IP Communication: The Z/IP Gateway translates Z-Wave commands and messages into IP-based protocols, allowing Z-Wave devices to communicate over an IP network. This enables remote access, control, and monitoring of Z-Wave devices from anywhere with internet connectivity.

2. Interoperability: By utilizing the Z/IP Gateway, Z-Wave devices can be integrated with other IP-based smart home systems, protocols, or platforms. This facilitates interoperability between different smart home technologies and enables the creation of comprehensive and integrated smart home solutions.

3. Remote Access and Control: With a Z/IP Gateway, users can remotely access and control their Z-Wave devices through IP-connected devices such as smartphones or web interfaces. This means they can manage their smart home devices even when they are away from home.

4. Security: Z/IP Gateways typically incorporate security measures to ensure secure communication between Z-Wave devices and IP networks. This may include encryption, authentication, and other security protocols to protect the privacy and integrity of the data transmitted.

5. Management and Configuration: Z/IP Gateways often provide management and configuration interfaces to set up and manage the Z-Wave devices connected to the gateway. This allows users to customize and control their smart home setup according to their preferences.

Overall, a Z/IP Gateway enables communication between Z-Wave devices and IP networks, facilitating remote access, control, and interoperability with other smart home systems. It expands the capabilities and convenience of Z-Wave devices by integrating them into IP-based networks and applications.

#### Z-IP GATEWAY main command 


start zipgateway
sudo /etc/init.d/zipgateway start

stop zipgateway
sudo /etc/init.d/zipgateway stop

restart  zipgateway
sudo /etc/init.d/zipgateway restart

get status zipgateway
sudo /etc/init.d/zipgateway status

get log zipgateway
sudo tail -f /var/log/zipgateway.log

#### Z-IP GATEWAY debug 

The Z-Wave over IP Gateway (Z/IP Gateway) is a device providing a layer above the serial API, and the Z-Wave protocol running on the Z-Wave module. It allows internet "Z/IP clients" connected over the Internet Protocol (IP) to contact and control nodes in a Z-Wave network.

Z/IP Gateway bridges IP UDP frames to Z-Wave, enabling communication between these two technologies. The number of Z-Wave node IDs in a network is a limited resource, with only 232 available. Z/IP Gateway is usually used to represent Z-Wave nodes as IP hosts, and because the IPv4/6 address space is larger than the Z-Wave Node ID space, it can represent all Z-Wave nodes as IP hosts. Additionally, the Gateway also represents IP hosts as Z-Wave nodes but due to ID limitations, it cannot represent every IP host as a Z-Wave node.

Z/IP Gateway key features:

Enables communication between the two technologies by

Representing Z-Wave nodes as IP hosts
Representing IP hosts as Z-Wave nodes

IPv6 Environment
Z/IP Gateway works in both IPv6 and IPv4 environments. The recommendation is to use IPv6, as IPv4 is subject to certain limitations. In an IPv6 environment, the Z/IP Gateway has a preset IPv6 address and creates a separate IPv6 subnet for the Z-Wave nodes with a preconfigured IPv6 prefix.

IPv6 PAN and LAN Connected through Z/IP Gateway

On the left-hand side, a LAN with a Z/IP Gateway-shared prefix is shown. This allows all IP hosts on the LAN to reach Z/IP Gateway, which acts as an IP router for the Z-Wave nodes on the PAN side.

To configure the IP hosts on the network, Z/IP Gateway will, in an IPv6 environment, transmit router advertisements for itself as a router telling other hosts on the IP network within the same broadcast domain, that it provides access to the Z-Wave subnet. If these hosts accept these router advertisements, they will autoconfigure an IPv6 address that has the same prefix as Z/IP Gateway, allowing them to communicate directly with Z/IP Gateway itself.

The router advertisements will also configure a route on the hosts, telling that the PAN subnet, typically fd00:bbbb::/64, is reachable through Z/IP Gateway, allowing the hosts to communicate with the Z-Wave nodes through Z/IP Gateway.


LAN side 
ping6 fd00:aaaa::3

PAN side 
ping6 fd00:bbbb::1

2023-07-01T16:43:58.245569+0200 Lan address         fd00:aaaa::03
2023-07-01T16:43:58.245577+0200 Han address         fd00:bbbb::01
2023-07-01T16:43:58.245584+0200 Gateway address     fd00:aaaa::1234


### Zware

file:///home/n/SimplicityStudio_v5/developer/sdks/zwave/zware/v7.18.3/doc/html/index.html#intro

$ sudo add-apt-repository -y ppa:git-core/ppa
$ sudo apt update
$ sudo apt install -y build-essential git curl wget python rsync doxygen cmake libtool autoconf default-jre-headless g++ unzip pkg-config patch gettext libglib2.0-dev-bin zlib1g-dev flex bison bc groff texinfo patchelf zip clang-10 clang-tools-10 clang-tidy-10 cloc bear libpcre3-dev python-pygments


$ cd /home/<user>/zwportal 
$ ./build/build.sh local [pc|rpi] [release|debug] [noparallel|parallel] 
$ ./build/build-installer.sh 

$ sudo apt-get install gettext binutils
$ cd /home/<user>/installer/
$ tar -zxvf zware-X.XX.X-x86_64.tar.gz
./install.sh /home/<user>/zware/


Below are locations of the relevant log-files.
Apache HTTP Server
Location for access-log:
Location for error-log <install-path>/install/httpd/logs/access_log
<install-path>/install/httpd/logs/error_log
Z-Ware Portal Daemon
Location: <install-path>/install/zwportald/var/log/zwportald.log
Z-Ware Web
Location for access-log:
Location for error-log <install-path>/install/zweb/logs/access_log
<install-path>/install/zweb/logs/error_log



sudo service apache2 stop
sudo tail -f /var/log/zipgateway.log

nano /home/n/SimplicityStudio_v5/developer/sdks/zwave/zware/install/httpd/conf/httpd.conf


 1267  sudo cat /usr/local/etc/zipgateway.cfg
 1268  cd /usr/local/etc/
 1269  ll
 1270  cat Portal.ca_x509.pem 
 1271  sudo cat /usr/local/etc/ZIPR.x509_1024.pem
 1272  sudo cat /usr/local/etc/Portal.ca_x509.pem
 1273  sudo cat /usr/local/etc/ZIPR.key_1024.pem


123456789012345678901234567890AA

Failed to get gateway mode
Failed to get settings because of timeout or the configurations are hidden



ip addr show br-lan  
306  journalctl | grep -i dhcp | tail -n100
307  journalctl | grep -i br-lan | tail -n100

sudo ufw disable
sudo ufw status

sudo /etc/init.d/zipgateway force-reload
sudo ip tuntap add tap0 mode tap user nico



########################################################################################################################
########################################################################################################################
########################################################################################################################
########################################################################################################################
########################################################################################################################
########################################################################################################################


ZIP gateway is a UDP/IP package capable 
- encapsuler les commandes z-wave dans des packet udp-ip
- extraire les commandes 
- encapsuler les commandes dans un serial frame 
- envoyer le serial frame a la puce silicone lab 


https://www.youtube.com/watch?v=uwoD5YsGACg&ab_channel=PowerCertAnimatedVideos
https://www.youtube.com/watch?v=t-a_KuIeBBc&ab_channel=RickGraziani

regarder TCP-UDP/IP tutorial avec wireshark 

A UDP/IP packet refers to a data unit used in computer networking that follows the User Datagram Protocol (UDP) and Internet Protocol (IP) standards. These protocols are part of the TCP/IP protocol suite, which is the foundation of communication on the internet.

UDP is a connectionless protocol that operates at the transport layer of the TCP/IP model. It provides a lightweight and simple method for sending datagrams (packets) of data between devices on a network. Unlike TCP, UDP does not establish a reliable, ordered, and error-checked connection between the sender and receiver. Instead, UDP focuses on sending data as quickly as possible, without any guarantees regarding delivery or sequencing.

IP, on the other hand, is a network layer protocol responsible for routing packets across networks. It provides the addressing and routing mechanisms necessary for data to be transmitted across different networks. IP breaks data into smaller units called packets, which are then transmitted over the network individually and reassembled at the destination.

A UDP/IP packet combines the functionality of both protocols. It consists of a UDP header and a payload section. The UDP header contains information such as source and destination port numbers, which allow the receiving device to identify the application or service to which the packet belongs. The payload section holds the actual data being transmitted.

When a device wants to send data using UDP/IP, it encapsulates the data into UDP packets by attaching the necessary UDP and IP headers. These packets are then routed through the network based on the destination IP address until they reach the intended recipient. Upon arrival, the recipient's device examines the UDP header to determine the appropriate application or service, and the payload section is extracted for further processing.

UDP/IP packets are commonly used for applications that prioritize speed and efficiency over reliability, such as real-time multimedia streaming, online gaming, and DNS (Domain Name System) lookups. However, since UDP does not provide error checking, flow control, or retransmission of lost packets, it is less suitable for applications that require guaranteed delivery and error-free communication.

CONTIKI

Contiki is an open-source operating system designed for resource-constrained embedded systems and Internet of Things (IoT) devices. It is known for its lightweight and low-power consumption characteristics, making it well-suited for small-scale devices with limited processing power, memory, and energy resources.

One of the key features of Contiki is its support for network communication protocols, such as the IPv6 networking stack. It provides built-in implementations of protocols like IPv6, UDP, TCP, CoAP (Constrained Application Protocol), and RPL (Routing Protocol for Low-Power and Lossy Networks), allowing devices running Contiki to connect to the internet and communicate with other devices over networks.

zipgateway service start first contiki 
- open config file
- start tapdev process
- thes start ZIP process 

the serial api port use is ttyusb0

what is TTYUSB0

The term "ttyUSB0" refers to a specific device file in Linux systems that represents a USB-connected serial port. In Linux, devices are accessed through special files located in the /dev directory, and ttyUSB0 is one such file.

USB-to-serial converters are commonly used to connect devices that communicate over a serial interface, such as microcontrollers, sensors, modems, or other embedded systems, to a computer via a USB port. When you connect such a device to a Linux system, the operating system recognizes it and assigns it a device file, typically starting with "ttyUSB" followed by a number.

The "tty" in ttyUSB0 stands for "teletypewriter," which historically refers to a terminal or console device used for text-based communication. Although the term is outdated, it has been retained in the naming convention of serial devices in Linux.

Once the device is recognized and assigned the ttyUSB0 file, you can interact with it using terminal emulation software or programming languages that provide serial communication support. By reading from and writing to the ttyUSB0 device file, you can exchange data with the connected device as if it were directly connected to a physical serial port.

It's worth noting that the specific device file assigned to a USB-to-serial converter may vary depending on the order of connection or the number of other USB serial devices present. For example, if multiple USB-to-serial converters are connected, you might see ttyUSB1, ttyUSB2, and so on for each device.

In summary, ttyUSB0 is the device file representing a USB-connected serial port in a Linux system, allowing communication with devices that use a serial interface via a USB connection.

Virtual NIC TAP0
https://www.youtube.com/watch?v=NZlbyoHgYjk&list=PL2TXDotVKyDCuM7-MShb_9y2g6LQjVDxw&ab_channel=TheLinuxChannel


the tun tap driver realizes the function of the virtual network card.
- TUN indicates that the virtula is point to point device 
- Tap indicates that the virtual is ethernet device 
these two devices implement different encapsulation for packet (udpip)

using the tun/tap driver, the network packet processedby the TCP/IP protocol stack can be sent to any process that use the tun/ttap driver and the process will reprocess it and then send it to the physical link 

In summary, the sentence explains that with the tun/tap driver, network packets processed by the TCP/IP protocol stack can be redirected to specific processes. These processes (SERIALAPI.C) can then manipulate the packets before sending them back to the physical network interface for transmission(LINUX SERIAL API to communicate with the ZWAVEHW). This provides flexibility and customization options for network packet handling within a system.

the tun/tap interface is initiate in tapdev-drv.c file 


tap0 receive data
throw it to uIP protocol stack
get udp_input data
then go to tcpip_uipcall

"udp_data" typically refers to the payload or data section of a UDP (User Datagram Protocol) packet. UDP is a connectionless transport layer protocol in the TCP/IP protocol suite that provides a lightweight and fast method for sending data over IP networks.

In a UDP packet, the UDP header contains information such as source and destination port numbers, length, and checksum. Following the UDP header, the "udp_data" section contains the actual payload or application data being transmitted. The size of the payload can vary and depends on the application or protocol using UDP.

https://blog.csdn.net/huilin9960/article/details/80094130



########################################################################################################################
########################################################################################################################
########################################################################################################################
########################################################################################################################
########################################################################################################################
########################################################################################################################

#CONFIG_FILE
sudo cat /usr/local/etc/zipgateway.cfg

#START_FILE_ZPGW_SERVICE 
sudo /etc/init.d/zipgateway start

avahi-browse _z-wave._udp -r


sudo apt install -y build-essential git curl wget python rsync doxygen cmake libtool autoconf default-jre-headless g++ unzip pkg-config patch gettext libglib2.0-dev-bin zlib1g-dev flex bison bc groff texinfo patchelf zip clang-10 clang-tools-10 clang-tidy-10 cloc bear libpcre3-dev python-pygments



########################################################################################################################
########################################################################################################################
########################################################################################################################
########################################################################################################################
########################################################################################################################
########################################################################################################################

add node to get info 

Get interface with specified node, endpoint and interface id in the network.

Attribute	    Type	    I/O	Description
net	            zwnet_t *	I	Network handle	
nodeid	        node_id_t	I	Node ID	
epid	        uint8_t	    I	Endpoint ID	
cls	            uint16_t	I	Interface ID (CC)	
ifd	            zwifd_t *	O	Interface descriptor (Can be NULL, if the purpose is to verify the existence of an interface)	
return	        int     	O	ZW_ERR_NONE on success; else ZW_ERR_XXX	

The zwifd_t data structure is a read-only descriptor used to access interfaces in the Z-Ware SDK. It provides information about the Z-Wave Command Classes (CCs) coupled with the corresponding Version CC information.

Here is a breakdown of the zwifd_t structure:

Table 140 – zwifd_t Structure

Attribute	Type	    I/O	Description
cls	        uint8_t	    O	CC (Command Class)
ver	        uint8_t	    O	CC version. Can be upgraded by device database.
real_ver	uint8_t	    O	Real Version of the CC that the device supports.
propty	    uint8_t	    O	Properties of the interface (bit-mask)
epid	    uint8_t	    O	Endpoint number
nodeid	    node_id_t	O	Node ID
net	        zwnet_t*	O	Network handle

The properties (propty) of the interface are represented as a bit-mask with the following values:

IF_PROPTY_SECURE: Indicates that the interface can be accessed securely.
IF_PROPTY_UNSECURE: Indicates that the interface can be accessed insecurely.
IF_PROPTY_HIDDEN: Indicates that the interface is hidden (for internal use only, not to be used by user applications).
IF_PROPTY_HIDDEN_POLL: Indicates that the interface is hidden, but device polling is enabled (for internal use only, not to be used by user applications).
IF_PROPTY_ALRM_EVT_CLR: Indicates that the interface is capable of sending event clear notification (valid for Alarm/Notification CC).
IF_PROPTY_BSNSR_EVT_CLR: Indicates that the interface is capable of sending event clear (valid for binary sensor CC).
IF_PROPTY_ALRM_SNSR_EVT_CLR: Indicates that the interface is capable of sending event clear (valid for the Alarm Sensor CC).
The zwifd_t structure provides essential information about interfaces and their properties, allowing developers to interact with specific CCs and endpoints in the Z-Ware SDK.


finally 
Attribute	Type	    I/O	Description
id	        uint32_t	O	Z-Wave Home ID
ctl_id	    node_id_t	O	Z/IP controller node ID
ctl_role	uint8_t	    O	Z/IP controller role. Bit-mask of ZWNET_CTLR_ROLE_XXX:
ctl_cap	    uint8_t	    O	Z/IP controller capabilities. Bit-mask of ZWNET_CTLR_CAP_XXX:
ctl_zw_role	uint8_t	    O	Z/IP controller Z-Wave role. ZW_ROLE_XXX:
user	    void *	    O	User context which was passed to zwnet_init
plt_ctx	    void *	    O	Platform context for printing of output text messages



then the scropt call 

result = zwif_bsensor_rpt_set(&intf, hl_bin_snsr_rep_cb);

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

    result = zwif_bsensor_rpt_set(&intf, hl_bin_snsr_rep_cb);

    if (result != 0)
    {
        printf("\nBinary sensor report setup with error:%d\n", result);
    }

    break;

RESUME 

CHECK IF THE INTERFACE HANDLER api EXIST FOR A SPECIFIC NODE AND ENPOINT 
zwnet_get_if_by_id(hl_appl->zwnet, node_id, ep_id, COMMAND_CLASS_SENSOR_BINARY, &intf)

THEN SET UP BINARY CALL BACK FOR THE SENSOR 
result = zwif_bsensor_rpt_set(&intf, hl_bin_snsr_rep_cb)

hl_bin_snsr_rep_cb
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




CREER UNE DATA STRUCTURE POUR HANDLE LA CONNECTION A SQLITE 

UNE FOIS QUE LE RESEAU EST INITIALISE 
JE PEUX CREER UNE OPTION DAN LE MENU PRINICPAL QUI VA ME CHERCHER NETWORK DESCRIPTION OPTION D 
    TROUVER LA FON,CTION DE L'OPTION D EST LA MODIFIER POUR RENVOYER LE RESULTAT SOUS FORME DE JSON 

CRÉER CETTE FONCTION 
JE PARSE LE RÉSULTAT, CHAQUE NODE EST UN ELEMENT DE MA TABLE QUI REPRÉSENTE UN CAPTEUR
JE CRÉÉE UN ELEMENT PAR NODE = > PK IS NODE ID 

INT     NODE_ID 
INT     NODE_END_POINT 
JSON    NODE_INFO
INT     NODE_SENSOR_TYPE

INT     NODE_BATTERY
TS      LAST_BATTERY_TS

INT     NODE_STATE
TS      LAST_STATE_TS


JE FAIS UNE LOOP SUR TOUS LES NODE EXCEPT LE CONTROLLER 
POUR TROUVER LES WINDOWS SENSOR 
    POUR CHACUN D'EUX
        JE DEMANDE L'INTERFACE API DU NODE EXISTE 
        zwnet_get_if_by_id(hl_appl->zwnet, node_id, ep_id, COMMAND_CLASS_SENSOR_BINARY, &intf);

        THEN SET UP BINARY CALL BACK FOR THE SENSOR 
        result = zwif_bsensor_rpt_set(&intf, hl_bin_snsr_rep_cb)

            DANS LE CALL BACK JUPDATE DANS LA DB LE'ÉLÉMENTS INT NODE_BATTERY LAST_BATTERY_TS NODE_STATE LAST_STATE_TS 
            JE LOG SUCCESS OR FAILED  (SI FAILED GROS PROBLEME VOIR COMMENT GERER ???????)


########################################################################################################################
########################################################################################################################
########################################################################################################################
########################################################################################################################
########################################################################################################################
########################################################################################################################

# sqlite istallation rpi3 

sudo apt-get update
sudo apt-get install sqlite3

Pour créer une base de données appelée "pandore" avec une table appelée "hw_interface_sensor" comprenant les champs que vous avez mentionnés, vous pouvez suivre les étapes ci-dessous :

Ouvrez le terminal de votre Raspberry Pi.

Assurez-vous que SQLite est installé en exécutant la commande sqlite3 --version. Si SQLite n'est pas installé, vous pouvez l'installer en utilisant les commandes mentionnées précédemment.

Pour créer la base de données "pandore", exécutez la commande suivante dans le terminal :

Copy code
sqlite3 pandore.db
Cela va créer une base de données vide appelée "pandore.db". Vous pouvez remplacer "pandore.db" par le nom de fichier de votre choix.

Pour créer la table "hw_interface_sensor" avec les champs spécifiés, vous pouvez exécuter la requête SQL suivante dans le terminal SQLite :

sql
Copy code
CREATE TABLE hw_interface_sensor (
    NODE_ID INTEGER,
    NODE_END_POINT INTEGER,
    NODE_INFO JSON,
    NODE_SENSOR_TYPE INTEGER,
    NODE_BATTERY INTEGER,
    LAST_BATTERY_TS TEXT,
    NODE_STATE INTEGER,
         TEXT
);
Cette requête crée une table avec les champs et les types de données correspondants.

Vous pouvez maintenant utiliser cette base de données et la table pour stocker et interroger vos données à l'aide de requêtes SQL.

Voilà, vous avez créé une base de données "pandore" avec une table "hw_interface_sensor" contenant les champs spécifiés. N'oubliez pas de remplacer "pandore.db" par le nom de fichier souhaité pour votre base de données.

CREATE TABLE hw_interface_sensor (
    NODE_ID INTEGER,
    NODE_END_POINT INTEGER,
    NODE_INFO JSON,
    NODE_SENSOR_TYPE INTEGER,
    NODE_BATTERY INTEGER,
    LAST_BATTERY_TS TEXT,
    NODE_STATE INTEGER,
    LAST_STATE_TS TEXT
);

show table in db 
.tables


.schema hw_interface_sensor

quitter le terminal sqlite 
.quit 



########################################################################################################################
########################################################################################################################
########################################################################################################################
########################################################################################################################
########################################################################################################################
########################################################################################################################

Wakeup Interface API

This corresponds to the Wakeup CC, which implies a sleep-capable node. This interface should
not be used when ZIPGW mailbox is used.


8.7.1 zwif_wakeup_get
Get wakeup information through report callback function. The wakeup information includes
minimum, maximum, default and current wake-up interval, the node that will receive wakeup
notification, and the wakeup interval step size.

zwif_wakeup_get Parameters
+-----------------------------------+----------------------------------+-------+-------------------------------------------------+
|  Attribute  |  Type        | I/O  |  Description                                    |
|-----------------------------------|----------------------------------|-------|-------------------------------------------------|
| ifd         | zwifd_t *    | I    | Wakeup interface handle.                        |
| cb          | zwrep_wakeup_fn | I    | Report callback function.                       |
| flag        | int          | I    | Flag, see ZWIF_GET_BMSK_XXX.                     |
| return      | int          | O    | ZW_ERR_NONE on success; else ZW_ERR_XXX.         |
+-----------------------------------+----------------------------------+-------+-------------------------------------------------+
zwrep_wakeup_fn Parameters
+-----------------------------------+------------------------------------+-------+-------------------------------------------------+
|  Attribute  |  Type        | I/O   |  Description                                    |
|-----------------------------------|------------------------------------|-------|-------------------------------------------------|
| ifd         | zwifd_t *    | I     | Wakeup interface handle.                        |
| cap         | zwif_wakeup_t * | I     | Capabilities report, NULL for wake up notification. |
| ts          | time_t       | I     | Time stamp. If this is zero, the callback has no data and hence other parameter values should be ignored. |
+-----------------------------------+------------------------------------+-------+-------------------------------------------------+
zwif_wakeup_t Structure
+-----------------------------------+-------------------------------------+-------+-------------------------------------------------+
|  Attribute  |  Type        | I/O   |  Description                                    |
|-----------------------------------|-------------------------------------|-------|-------------------------------------------------|
| min         | uint32_t     | O     | Minimum in seconds.                             |
| max         | uint32_t     | O     | Maximum in seconds.                             |
| def         | uint32_t     | O     | Default in seconds.                             |
| interval    | uint32_t     | O     | Steps between min and max.                      |
| cur         | uint32_t     | O     | Current setting in seconds.                     |
| node        | zwnoded_t    | O     | Node to send wakeup notification.               |
+-----------------------------------+-------------------------------------+-------+-------------------------------------------------+

8.7.2 zwif_wakeup_set
Set wakeup interval and node to notify on wakeup.
+-----------------------------------+-------------------------------------+
|  Attribute  |  Type       | I/O   |  Description                      |
|-----------------------------------|-------------------------------------|
| ifd         | zwifd_t *   | I     | Wake up interface handle.          |
| secs        | uint32_t    | I     | Interval in seconds (24 bit).      |
| node        | zwnoded_t * | I     | Node to notify.                    |
| return      | int         | O     | ZW_ERR_NONE on success;            |
|             |             |       | else ZW_ERR_XXX.                   |
+-----------------------------------+-------------------------------------+




ifd

        zwnet_get_if_ by_id  Get interface with specified node, endpoint and interface id in the network

        zwifd_t     intf    = {0};
        int         node_id = 8;
        int         ep_id   = 0;
        int         result;

        //Select the binary sensor wake up command class interface
        result = zwnet_get_if_by_id(appl_ctx->zwnet, node_id, ep_id, COMMAND_CLASS_WAKE_UP, &intf);

node
        Get the first node (local controller) in the network.
        Table 96 – zwnet_get_node Parameters
        +-----------------------------------+-------------------------------------+-------+-------------------------------------------------+
        |  Attribute  |  Type        | I/O   |  Description                                    |
        |-----------------------------------|-------------------------------------|-------|-------------------------------------------------|
        | net         | zwnet_t *    | I     | Network handle.                                |
        | noded       | zwnoded_t *  | O     | Node handle.                                   |
        | return      | Int          | O     | ZW_ERR_NONE on success; else ZW_ERR_XXX.       |
        +-----------------------------------+-------------------------------------+-------+-------------------------------------------------+



    printf("Press Enter to continue...");
    getchar(); // Wait for the user to press Enter
    
    int         result;
    zwnoded_t   node;
    zwepd_t     ep;
    zwifd_t     intf;

    int         node_id = 8;
    int         ep_id = 0;
    int         w_int = 60;


    //Get first node (controller node)
    result = zwnet_get_node(&appl_ctx.zwnet, &node);
    if (result != 0)
    {
        printf("Error: couldn't find Z/IP gateway interface: %d\n", result);
    }
    //Get wake up interface node 
    result = zwnet_get_if_by_id(&appl_ctx.zwnet, node_id, ep_id, COMMAND_CLASS_WAKE_UP, &intf);

    if (result != 0)
    {
        printf("\nFailed to find wakeup class. Error:%d\n", result);
    }
    //set wake up mode 
    result = zwif_wakeup_set(&intf, w_int, &node);
    if (result != 0)
    {
        printf("\nFailed to setup wakeup class. Error:%d\n", result);
    }
    printf("sucees to set up wakeup class. Error:%d\n", result));


    // Call the zwif_wakeup_get function to retrieve the wakeup report
    result = zwif_wakeup_get(&intf, zwrep_wakeup_fn, ZWIF_GET_BMSK_CACHE);

    if (result == 0) {
        printf("Wakeup report retrieval successful.\n");
    } else {
        printf("Error retrieving wakeup report.\n");
    }







int zwrep_wakeup_fn(zwifd_t *ifd, zwif_wakeup_t *cap, time_t ts) {
    // Check if the timestamp is zero (no data)
    if (ts == 0) {
        printf("No data received in the wakeup report callback.\n");
        return 0;
    }

    // Process the received wakeup report data
    printf("Received wakeup report:\n");
    printf("Minimum interval: %u seconds\n", cap->min);
    printf("Maximum interval: %u seconds\n", cap->max);
    printf("Default interval: %u seconds\n", cap->def);
    printf("Interval steps: %u seconds\n", cap->interval);
    printf("Current interval: %u seconds\n", cap->cur);
    printf("Node ID to notify: %u\n", cap->node.node_id);

    return 0;
}