# lorawan-network-server

lorawan-network-server is lightweight monolith LoRaWAN network server.

The payload is parsed according to the description of the data structure of the packet
sent by the end-device and is stored to the database in the table specified in the 
configuration file.

Clone [Github repository](https://github.com/commandus/lorawan-network-server):
```
git clone git@github.com:commandus/lorawan-network-server.git
```
or
```
git clone https://github.com/commandus/lorawan-network-server.git
```

First install dependencies (see below) and then configure and make project using Autotools.

```
cd lorawan-network-server
./autogen.sh
./configure
make
```

You cam use CMake with care, it may be inconsistent. You need check missed sources in the CMake script.

## Components

```
+------------------+   +------------------+
| Semtech Base St. |   | Semtech Base St. |
+------------------+   +------------------+
          | Private             |
          | Network +------------
          |         |
+---------------------+
|     UDP socket,     |     
|     UDP listener    |
+---------------------+
          |
+---------------------+          
| LoraPacketHandler/  |<------+
| LoraPacketProcessor |->     |
+---------------------+ |     |
          |             |     |
          | Network     |     |
          | Address     |     |   +----------------+
          |             | +-------|   File JSON    |
+---------------------+ | |   |   +----------------+
|   Identity service  |-->|   |
+---------------------+ | |   |   +----------------+
          |             | +-------| Key/Value LMDB |
          | Keys        |     |   +----------------+
          |             |     |
+---------------------+ | waits packets from all BS
|     Packet Queue    |-+     |
+---------------------+       |
          |                   |
          | wait time expired |
          |                   |
          +-------------------+
```

Gateways

```
+---------------------+
|     GatewayList     |
+---------------------+
           |
+---------------------+
|     GatewayStat     |
+---------------------+
```

[Basic communication protocol between Lora gateway and server UDP "JSON" protocol](https://github.com/Lora-net/packet_forwarder/blob/master/PROTOCOL.TXT)


## Run

```
./lorawan-network-server 84.237.104.128:2000
```

## Build

You can use

- Automake
- CMake

build system.

Make sure you have automake installed:
```
apt install autoconf build-essential libtool libprotobuf-dev
```

Before you start, first you need download  [pkt2 library](https://github.com/commandus/pkt2.git)
([git](git@github.com:commandus/pkt2.git) 

Then install libcurl4-openssl-dev protobuf-compiler libgoogle-glog-dev libsnmp-dev libnanomsg-dev 
libprotoc-dev libunwind-dev dependencies:
```
apt install libcurl4-openssl-dev protobuf-compiler libgoogle-glog-dev libsnmp-dev libnanomsg-dev libprotoc-dev
```

Finally, make all
```
make
```

or just two static libraries:

```
make pkt2.pb.h libpkt2.a libpkt2util.a
```

pkt2.pb.h target is required to generate protobuf c++ files.

Automake, autoconf, libtool, gcc or cmake must be installed first.

Also, must install at least one backend database library, ot install all of them:

```
sudo apt install liblmdb-dev sqlite3 libsqlite3-dev libmysqlclient-dev firebird-dev
```

Full set of libraries:
```
sudo apt install autoconf build-essential libtool libprotobuf-dev liblmdb-dev sqlite3 libsqlite3-dev libmysqlclient-dev \
firebird-dev libcurl4-openssl-dev protobuf-compiler libgoogle-glog-dev libsnmp-dev libnanomsg-dev libprotoc-dev
```

If 'libmysqlclient-dev' package is not available in repository, replace with package 'libmariadb-dev'.

To do cmake installed first run:

```
apt install cmake
```

### autoconf

Generate automake files, configure and make:

```
autogen.sh
./configure
make
sudo make install
```

./configure has several options to enable backend database support:

- --enable-db-sqlite=true (on by default)
- --enable-db-postgres
- --enable-db-mysql
- --enable-db-firebird

Configure all supported databases:

```
./configure --enable-db-sqlite --enable-db-postgres --enable-db-mysql --enable-db-firebird
```

You must have database client and developer's tools (include files and libraries at least) installed on the computer.

lorawan-network-server uses internal database to keep device's authentication information.

By default, this database keep in memory and flushes to the disk as JSON file.

In some scenarios it is better store device's authentication information on the disk not in memory.

lorawan-network-server can store device's authentication information in the LMDB or MDBX (clne of the LMDB) database.

./configure has options to choose how to store device's authentication information:

- --enable-json in memory database (by default)
- --enable-lmdb or --enable-mdbx (on the disk file)

For clang:

```
./configure CC=clang CXX=clang++
```

### cmake

For instance, you can use Clang instead of gcc:

```
mkdir build
cd build
export CC=/usr/bin/clang;export CXX=/usr/bin/clang++;cmake ..
make
```

## Configuration files

### server config ~/.lorawan-network-server.json

- gatewaysFileName Gateways list. Default ~/gateway.json
- databaseConfigFileName databases list default "dbs.js"
- protoPath protobuf message description files directory path. Default "proto"
- server Network server properties, including end-device list served by the server
- configFileName (optional) Redirect config file to another one

Server config:

- identityStorageName end-device list file. Default is ~/identity.json
- storageType e.g. "json"
- queueStorageName e.g. "queue.json",
- messageQueueStorageType e.g. "json"
- deviceStatStorageType device statistics storage type, "file" or "post". Default none
- logDeviceStatisticsFileName e.g. log device statistics file name "device-stat.json"
- deviceHistoryStorageName e.g. "device-history.json"
- regionalSettingsStorageName file name of regional settings, e.g. "regional-parameters.json". Default none
- regionalSettingsChannelPlanName If set, overrides default regional band plan value in the regional settings file.   
- listenAddressIPv4 array of IPv4address:port e.g. ["127.0.0.1:5000","84.237.104.128:5000"]
- listenAddressIPv6 array of IPv6address:port
- gwStatStorageType log gateway statistics storage type, "file" or "post". Default none
- logGWStatisticsFileName e.g. log gateway statistics file name "gateway-stat.json"
- netId LoraWAN network identifier. Decimal number e.g. 0 or 1 or hexadecimal number string e.g. "60000C" or "c00007"

- readBufferSize UDP buffer size Default 4096.
- verbosity 0..3 error logging verbosity (0- error only, 3- debug info)
- daemonize false, true. Indicate does network server starts as daemon or not
- controlFPort 0: no remote control, 1..223- FPort bands used by network service to control server. Default 0.

netId parameter is 3 bytes long network identifier, leading zeroes can be omitted. 

Please note NetId 0 and 1 are reserved for private use.   

configFileName may be used to load configuration from different location. 
Do not use this parameter except when you really need it.

identityStorageName is property of the server because network server is responsible for end-device.
In contrast, gatewaysFileName property points to the gateways list which send messages to one or more network servers.

Example of lorawan-network-server.json:
```
{
        "gatewaysFileName": "./gateway.json";
        "server": {
                "identityStorageName": "./identity.json",
                "listenAddressIPv4": "*:"
        }
}
```

Option "storageType" bands are:

- json (default)
- lmdb (file database)
- txt (directory with files)

Default value is "json". The identifiers are stored in memory.

Option "lmdb" is a little safer and suitable for low-memory installations.

Option "txt" is slow and useful for debug only.

Options "gwStatStorageType", "deviceStatStorageType" bands are

- file
- post 

File example:
```
		"deviceStatStorageType": "file",
		"logDeviceStatisticsFileName": "device-stat.txt",

        "gwStatStorageType": "file",
        "logGWStatisticsFileName": "gw-stat.json"
```

post example:
```
		"deviceStatStorageType": "post",
		"logDeviceStatisticsFileName": "http://localhost:50002/post.php",

        "gwStatStorageType": "post",
        "logGWStatisticsFileName": "http://localhost:50002/post.php"
```

#### Message queue

Received messages are sent to the database(s) as soon as possible. In case the database system is not available
for some reason, received messages stay in the queue until database has up.

Option "messageQueueStorageName" set name of file name (or directory name).

There messageQueueStorageType option determines how to keep received messages in the temporary queue.

Option "messageQueueStorageType" bands are:

- json (default)
- lmdb (file database)
- txt (directory with files)

Default value is "json". Received messages are stored in memory.

If lorawan-network-server is down, queue are stored to the file.

When lorawan-network-server is up, queue loaded from the file.

Option "lmdb" is a little safer. You can avoid memory consumption in case of 
external database is down.

Option "txt" is slow and useful for debug only.

If option messageQueueStorageType value is "txt" then option "messageQueueStorageName" set directory name
with ".bin", ".hex", ".b64" files.

Other programs can put files to this directory and lorawan-network-server will parseRX files and put messages
to the databases.

- ".bin" - binary payload, as-is
- ".hex" - payload each byte represented as hexadecimal two digits number
- ".b64" - base64 encoded payload

Option "messageQueueDirFormat" bands are:

- 0 or "bin" (default)
- 1 or "hex"
- 2 or "base64"

lorawan-network-server try to parseRX payload and insert parsed data to database(s). Does not matter success or 
fail is database insertion, file is deleted.

### gateway.json

Gateways list gateways.

Each entry has a gateway identifier, address, name and gateway statistics.

  - gwid Gateway identifier (hex number string)
  - addr gateway address:port (IPv4 or IPv6).
  - name gateway name
  - time UTC time of pkt RX, us precision, ISO 8601 'compact' format
  - lati latitude
  - long longitude
  - alti altitude, meters, integer
  - rxnb Number of radio packets received
  - rxok Number of radio packets received with a valid PHY CRC
  - rxfw Number of radio packets forwarded
  - ackr Percentage of upstream datagrams that were acknowledged
  - dwnb Number of downlink datagrams received
  - txnb Number of packets emitted

Instead of host address, you can use host name (domain name). 

Example:
```
[
  {
     "gwid": "00006cc3743eed46"
     "addr": "127.0.0.1:6000",
     "name": "gw01",
   	"time": 0,
     "lati": 62.02774,
     "long": 129.72883,
     "alti": 348,
     "rxnb": 0,
     "rxok": 0,
     "rxfw": 0,
     "ackr": 0.0,
     "dwnb": 0,
     "txnb": 0
  }
]
```

Server updates the gateway statistics on shutdown.

### identity.json

- addr        network address (hex string, 4 bytes)
- activation  ABP or OTAA
- eui         device identifier (hex string, 8 bytes)
- nwkSKey     shared session key (hex string, 16 bytes)
- appSKey     private key (hex string, 16 bytes)
- class 	    LoraWAN class "A", "B" or "C"
- name        optional device name (max 8 chars). If name longer than 8 characters, name is truncated.
- version     LoraWAN version e.g. "1.0.0"
- flags       default 0- no rights. 1- device can send "control service" messages to the network service

Example:
```
[
  {
    "addr": "xx..",
    "activation": "ABP",
    "eui": "xx..:",
    "nwkSKey": "..",
    "appSKey": "..",
    "name": "dev01"
  },
  ..
]
```

### Regional parameters file   

LoRaWAN regional parameters described in the document named like "RP002-1.0.3"
where 1.0.3 is version number, in the example shown it is "regionalParametersVersion" value.

By default, name of file is "regional-parameters.json"

"RegionBands" array contains regional settings. 

Each region has mnemonic name, e.g. "RU864-870" corresponds to the Russian Federation region.

Numbers 864 and 870 means central frequency in MHz.

Region can have mnemonic name like EU433 if channels frequencies all in 433Mhz.

In accordance to 2.1 Regional Parameter Channel Plan Common Names 

Id  | Channel Plan | Common Name
----|--------------|-------------
 1  | EU863-870    | EU868
 2  | US902-928    | US915
 3  | CN779-787    | CN779
 4  | EU433        | EU433
 5  | AU915-928    | AU915
 6  | CN470-510    | CN470
 7  | AS923-1      | AS923 
 8  | AS923-2      | AS923-2
 9  | AS923-3      | AS923-3
 10 | KR920-923    | KR920
 11 | IN865-867    | IN865
 12 | RU864-870    | RU864
 13 | AS923-4      | AS923-4


- supportsExtraChannels true or false
- defaultRegion true or false. If true, this regional settings sued by default 
  - bandDefaults default values
    - RX2Frequency RX2 frequency in Hz
      - RX2DataRate
      - ReceiveDelay1 delay in seconds
      - ReceiveDelay2 delay in seconds
      - JoinAcceptDelay1 delay in seconds
      - JoinAcceptDelay2 delay in seconds
  - dataRates array of 8 elements": [{
    - uplink true or false
    - downlink true or false
    - modulation "LORA" or "FSK"
    - bandwidth in kHz (Lora only)
    - spreadingFactor e.g. 12 (Lora only)
    - bps (FSK only)
  - uplinkChannels array if one or more uplink channel
  - downlinkChannels array if one or more downlink channel
    - frequency frequency in Hz, e.g. 868900000,
    - minDR minimum data rate
    - maxDR maximum data rate
    - enabled true or false
    - custom true or false
  - maxPayloadSizePerDataRate array of 8 elements
  - maxPayloadSizePerDataRate array of 8 elements
    - m
    - n
  - rx1DataRateOffsets array of 8 elements of array of 1..N elements (unsigned integers)
  - txPowerOffsets array of 8 integers
  
```
{
	"regionalParametersVersion": "1.0.1",
	"RegionBands": [{
		"name": "RU864-870",
		"supportsExtraChannels": true,
		"bandDefaults": {
			"RX2Frequency": 869100000,
			"RX2DataRate": 0,
			"ReceiveDelay1": 1,
			"ReceiveDelay2": 2,
			"JoinAcceptDelay1": 5,
			"JoinAcceptDelay2": 6
		},
		"dataRates": [{
			"uplink": true,
			"downlink": true,
			"modulation": "LORA",
			"bandwidth": 125,
			"spreadingFactor": 12,
			"bps": 0
		},
			...
		],
		"uplinkChannels": [{
			"frequency": 868900000,
			"minDR": 0,
			"maxDR": 5,
			"enabled": true,
			"custom": false
		}, {
			"frequency": 869100000,
			"minDR": 0,
			"maxDR": 5,
			"enabled": true,
			"custom": false
		}],
		"downlinkChannels": [{
			"frequency": 868900000,
			"minDR": 0,
			"maxDR": 5,
			"enabled": true,
			"custom": false
		}, {
			"frequency": 869100000,
			"minDR": 0,
			"maxDR": 5,
			"enabled": true,
			"custom": false
		}],
		"maxPayloadSizePerDataRate": [{
			"m": 59,
			"n": 51
		},
			...
		],
		"maxPayloadSizePerDataRateRepeater": [{
			"m": 59,
			"n": 51
		},
			...
		],
		"rx1DataRateOffsets": [
			[0, 0, 0, 0, 0, 0],
			[1, 0, 0, 0, 0, 0],
			[2, 1, 0, 0, 0, 0],
			[3, 2, 1, 0, 0, 0],
			[4, 3, 2, 1, 0, 0],
			[5, 4, 3, 2, 1, 0],
			[6, 5, 5, 4, 3, 2],
			[7, 6, 5, 4, 3, 2]
		],
		"txPowerOffsets": [0, -2, -4, -6, -8, -10, -12, -14]
	}]
}
```

## Tools

- print-netid Print NetId details
- lora-print
- proto-db
- mac-gw
- mac-ns

### print-netid

NetId if 3 bytes long network identifier contains:

- type 0..7
- identifier itself

Please note LoraWAN address (4 bytes long) also contains type and short version of network address (NwkId).

print-netid utility print NetId details by value (3 bytes):
```
print-netid C0004A
c0004f	6	4f	4f	fc013c00	fc013fff
```
First column (tab delimited) show NetId in hex.

Column 2 show NetType value in rage of 0..7.

Column 3 show network identifier.

Column 4 show NwkId.

It is same network identifier used in the network address except it can be shorter than network identifier.

Column 4 show minimum possible NwkAddr.

Column 5 show maximum possible NwkAddr.

To print header use -v option:

```
print-netid -vv c0004f
NetId   Type Id NwkId DevAddr min  DevAddr max
c0004f  6    4f 4f    fc013c00     fc013fff
```

To print bit fields use -vv option:
```
./print-netid -vv C0004F
NetId	Type	Id	NwkId	DevAddr min	DevAddr max
c0004f	6	4f	4f	fc013c00	fc013fff	

binary:
110000000000000001001111
TTTNNNNNNNNNNNNNNNNNNNNN

DevAddr:
Min 11111100000000010011110000000000 NwkId:   4f NetAddr: 0
    TTTTTTTnnnnnnnnnnnnnnnAAAAAAAAAA
Max 11111100000000010011111111111111 NwkId:   4f NetAddr: 3ff
    TTTTTTTnnnnnnnnnnnnnnnAAAAAAAAAA
```

where T means NetType value in range 0..7,
N- network identifier,
n- NwkId,
A- NwkAddr

## lora-print

lora-print utility parseRX packet received from the Semtech gateway and try to decode payload.

Mandatory option is one of

- -x, --hex=<hex-string>      LoraWAN packet to decode, hexadecimal string.
- -6, --base64=<base64>       same, base64 encoded.

lora-print utility parseRX payload by the packet description in one proto file.

You can specify folder path where proto file stored using option:

- -p, --proto=<path>          proto file directory. Default 'proto'

You can force specific packet description by selecting specific proto message: 

- -m, --message=<pkt.msg>     force message type packet and name

You can chenge output format by option -f <number>:

- 0- json(default)
- 1- csv w/o header
- 2- tab delimited w/o header
- 3- sql
- 4- sql (version 2)
- 5- pbtext
- 6- debug output
- 7- hex string
- 8- binary string
- 11- csv header
- 12- tab header

Option "sql" insert data into databases. By default lora-print just print packet to stdout.

Example:

```
./lora-print -x 024c7e0000006cc3743eed467b227278706b223a5b7b22746d7374223a313237353533303937322c226368616e223a362c2272666368223a312c2266726571223a3836382e3930303030302c2273746174223a312c226d6f6475223a224c4f5241222c2264617472223a22534631324257313235222c22636f6472223a22342f35222c226c736e72223a2d392e352c2272737369223a2d3131352c2273697a65223a33372c2264617461223a2251444144525147416e5259436b4c72715672703677324a55547958744a4467315669464a354d44666b756e336f762f5653513d3d227d2c7b22746d7374223a313237353533303938302c226368616e223a342c2272666368223a302c2266726571223a3836342e3930303030302c2273746174223a312c226d6f6475223a224c4f5241222c2264617472223a22534631324257313235222c22636f6472223a22342f35222c226c736e72223a31302e382c2272737369223a2d32372c2273697a65223a33372c2264617461223a2251444144525147416e5259436b4c72715672703677324a55547958744a4467315669464a354d44666b756e336f762f5653513d3d227d5d7d

{"prefix": {"version":2, "token":32332, "tag":0, "mac": "00006cc3743eed46"}, "addr": "01450330", "id": {"activation":"ABP","class":"C","deveui":"3434383566378112","nwkSKey":"313747123434383535003a0066378888","appSKey":"35003a003434383531374712656b7f47","version":"1.0.0","appeui":"0000000000000000","appKey":"00000000000000000000000000000000","devNonce":"0000","joinNonce":"000000","name":"SI-13-23"}, "metadata": {"rxpk":[{"time":"2022-01-18T12:22:38Z","tmms":1326511376,"tmst":1275530980,"freq":864.900000,"chan":4,"rfch":0,"stat":1,"modu":"LORA","datr":"SF12BW125","codr":"4/5","rssi":-27,"lsnr":10.8,"size":37,"data":"QDADRQGAnRYCkLrqVrp6w2JUTyXtJDg1ViFJ5MDfkun3ov/VSQ=="}]}, "rfm": {"fport": 2, "fopts": "", "header": {"fcnt": 5789, "fctrl": {"foptslen": 0, "fpending": 0, "ack": 0, "adr": 1}, "addr": "01450330", "mac": {"major": 0, "mtype": "unconfirmed-data-up"}}}, "payload_size": 24, "payload": "010021a0c082581c000000004a0000000000000000000000"}
{"vega.SI13p1":{"vega.SI13p1.temperature": 28, "vega.SI13p1.counter1": 0, "vega.SI13p1.counter2": 74, "vega.SI13p1.activation": 1, "vega.SI13p1.ackrequest": 0, "vega.SI13p1.timeout": 0, "vega.SI13p1.input1": 0, "vega.SI13p1.input2": 1}}
```

Please note Semtech packet can contain 1, 2 or more payloads with different download channels.

### proto-db utility

```
Usage: proto-db
 [-v?] [<command>] [-p <path>] [-c <file>] [-d <database-name>]... [-m <packet.message>] [-x <hex-string>] [-6 <base64-string>] [-o <number>] [-l <number>] [-s <field-name>]... [-S <field-name>]...
proto-db helper utility
  <command>                 print|list|create|insert. Default print
  -p, --proto=<path>        proto files directory. Default 'proto'
  -c, --dbconfig=<file>     database config file name. Default 'dbs.js'
  -d, --dbname=<database>   database name, Default all
  -m, --message=<pkt.msg>   Message type packet and name
  -x, --hex=<hex-string>    print, insert command, payload data.
  -6, --base64=<base64>     print, insert command, payload data.
  -o, --offset=<number>     list command, offset. Default 0.
  -l, --limit=<number>      list command, limit size. Default 10.
  -s, --asc=<field-name>    list command, sort by field ascending.
  -S, --desc=<field-name>   list command, sort by field descending.
  -v, --verbose             Set verbosity level
  -?, --help                Show this help```
```

Create table for iridium.IEPacket packet in the "mysql_1" database:

```
./proto-db -d mysql_1 -m iridium.IEPacket create
```
Message type name passed in the -m option.

Message type may be detected by payload. In this case -m message can be omitted.

Tin this case you must provide payload using -x <hex string> or -6 <base64 string> option.

Determine message type by the payload using -x <payload-hex>:

```
./proto-db create -d mysql -x 0100213887c1601c000000004a0000000000000000000000
./proto-db -d sqlite -x 010021b8b06b581f000000004a0000000000000000000000 create
```

Print "iridium.IEPacket" messages stored in the "mysql_1" database:

```
./proto-db -d mysql_1 -m iridium.IEPacket list
```

Print "vega.SI13p1" messages stored in the "mysql_1" database:

```
./proto-db -d sqlite -m vega.SI13p1 list

28|0|74|1|0|0|0|1|01450330|3434383566378112|SI-13-23|1635389922|
28|0|74|1|0|0|0|1|01450330|3434383566378112|SI-13-23|1635390222|
...
```

Print "vega.SI13p1" messages stored in the "mysql_1" database:

```
./proto-db -d sqlite -m vega.SI13p1 list

28|0|74|1|0|0|0|1|01450330|3434383566378112|SI-13-23|1635389922|
28|0|74|1|0|0|0|1|01450330|3434383566378112|SI-13-23|1635390222|
...
```

Print "vega.SI13p1" two messages stored in the "mysql_1" database skipping last 1000 records:

```
./proto-db -d sqlite -m vega.SI13p1 list -o 1000 -l 2
29|0|74|1|0|0|0|1|01450330|3434383566378112|SI-13-23|1636457951|
29|0|74|1|0|0|0|1|01450330|3434383566378112|SI-13-23|1636458251|
```

Insert data from payload to mysql database, force type to iridium.IEPacket 

```
./proto-db -d mysql -m iridium.IEPacket insert -x 014c00011c00e8444601333030323334303639323030383530001a070000e199205e030b00003eea3781fbcc05000000021c00c068b50328f1bd078999205e07050000009f1be60ca313f432000000
```

## mac-gw, mac-ns utilities

mac-gw send a command to a class C device bypassing the network server directly through the selected gateway.

mac-ns send a command to a class C device via the network server through the selected gateway. 
If gateway is not specified in the mac-ns command line parameters, network server use the best gateway.

For example, this command
```
./mac-ns -a 84.237.104.128:5000 -g "6cc3743eed46" -M "SI-13-2" -E "SI-13-23" s -vvv
```
send "devstatus" command via network server to the device named "SI-13-23".

Option -M "SI-13-2" set credentials from "master" device.

Network server accept "control" messages from "master" device(s).   

Parameters:

- -g gateway identifier or -G gateway name
- -e end-device identifier or -E dev-device name
- -p payload (optional)

Options:

-x --regex use regular expression in -g, -e options instead of wildcards ('*', '?').

Options -g, -e can contain "*" and "?" wildcards, or regular expression like ".*" if -x option is set.
Regular expressions' grammar is similar to the grep.

Option -p requires hex string, for instance "0faa0167" is valid parameter value.

MAC commands has short and long names.
For example, "a" is a short name of "linkadr" command.
Most commands have one or more parameteres.
For example, "linkadr" command has 5 parameters:

```
linkadr 2 7 255 1 0
```
where 2 is tx power, 7- data rate, 255- channel mask, 1-  transmissions per message, 0- mask control.

There are special parameter value "asis" for the first two parameters of the "linkadr" command , you can use it as shown:

```
linkadr asis asis 255 1 0
```

List MAC commands:
```
./mac-gw -?
...
MAC commands:
a  linkadr      Rate adaptation
     tx power: 0..7, asis(15)
     data rate: 0..7, asis(15)
     channel mask: 1..255
     transmissions per message: 1..15
     mask control: 0..7
d  dutycycle    Limit transmit duty cycle
     limit: 0..15
rx rxparamsetup Change frequency/data RX2
     frequency: 0..9999999 * 100Hz
     RX1 offset: 0..7s
     data rate: 0..7, asis(15)
s  devstatus    Request device battery, temperature
n  newchannel   Set channel frequency/ data rate
     channel index: 0..15
     frequency: 0..9999999 * 100Hz
     min data rate: 0..7
     max data rate: 0..7
rx rxtiming     Set delay between TX and RX1
     TX - RX delay: 0..15 + 1s
tx dwelltime    Set maximum allowed dwell time
     downlink dwell time: no-limit(0), 400(1)
     uplink dwell time: no-limit(0), 400(1)
     max EIRP: 0..15
dl dlchannel    Set RX1 slot frequency
     channel index: 0..15
     frequency: 0..9999999 * 100Hz
k  rekey        Answer security OTA key update
al acklimit     Set ADR_ACK_LIMIT, ADR_ACK_DELAY
     ADR ACK limit: 0..15
     ADR ACK delay: 0..15
j  forcerejoin  Request immediately Rejoin-Request
     retransmission delay: 0..7
     max retransmission: 0..7
     rejoin type : 0(0), 2(2)
js rejoinsetup  Request periodically send Rejoin-Request
     max time : 0..15
     max count : 0..15
p  ping         Answer to unicast ping slot
pc pingchannel  Set ping slot channel
     frequency: 0..9999999 * 100Hz
     data rate: 0..7, asis(15)
bf beaconfreq   Set beacon frequency
     frequency: 0..9999999 * 100Hz
```

Examples:
```
./mac-gw -c mac-gw.json -g "*" -e "*" -p "0fa1cc"
./mac-gw -c mac-gw.json -g "01*" -g "09*" -e "ff*" -e "aa*" -p "0fa1cc"
./mac-gw -c mac-gw.json -G "gw-sub*"  -E "dev*"  -p "0fa1cc" al 1 2 d 3
```

Configuration file ~/.mac-gw.json same as server config ~/.lorawan-network-server.json.

You can use symlink ~/.mac-gw.json to the ~/.lorawan-network-server.json.

- server
- configFileName
- gatewaysFileName

## Trapped signals

- SIGUSR2 (12) flush files to the disk

## Database backend

Database configuration file dbs.js is Javascript declaration of "databases" array.

Each element is object with members

- name
- type
- connection
- table_aliases
- field_aliases
- properties

Name is used to find out appropriate database.

Valid bands for "type" are 

- "sqlite3"
- "postgresql"
- "firebird"
- "json"

Connection for database type "sqlite3" is file name of SQLite database.

Connection for database type "postgresql" looks like:

postgresql://irthermometer:************@localhost:5432/irthermometer",

See [PostgreSQL Connection URIs](https://www.postgresql.org/docs/10/libpq-connect.html)

table_aliases is an array of array of two elements. First element is protobuf package.message.
Second element is an alias used in the database for table name.

field_aliases is an array of array of two elements. First element is protobuf package.message.field
Second element is an alias used in the database for table column.

If second element is empty string, this message or field does not put to the database.

properties array consist of [key, value] pairs.

Keys are:

- addr network address string
- eui global end-device identifier in IEEE EUI64 address space
- fport application port number (1..223). 0- MAC, 224- test, 225..255- reserved
- name device name
- time (32 bit integer, seconds since Unix epoch)
- timestamp string
- activation (ABP|OTAA)
- class A|B|C

Optional property "id" is a number of packet received by the server (packets received from gateways
deduplicated, first of them has a number, others omitted).

The only integer key is time. All others are string.

Values are set by receiver processor before data is inserted into database.

Optional parameters:

- login (reserved)
- password (reserved)

Example of dbs.js file:

```
/*
 * It's Javascipt not JSON.
 * You can use any evaluatation to produce "databases" array.
 */
sqlite_table_aliases = [
	["iridium.IEPacket", "iridium_packet"]
];

sqlite_field_aliases = [
	["iridium.IEPacket.iridium_version", "version"],
	["iridium.IEPacket.iridium_size", ""],
     ...
];

/*
 * var databases must be declared in the config file.
 */
databases = [
	{
          id: 1,
		name: "sqlite",
		type: "sqlite3",
		connection: "lns.data.sqlite.db",
		table_aliases: sqlite_table_aliases,
		field_aliases: sqlite_field_aliases
	},
	{
          id: 2,
		name: "postgres",
		type: "postgresql",
		connection: "postgresql://irthermometer:************@localhost:5432/irthermometer",
		table_aliases: sqlite_table_aliases,
		field_aliases: sqlite_field_aliases
	},
    	{
          id: 3,
		name: "mysql",
		type: "mysql",
		connection: "localhost",
		db: "irthermometer",
		login: 'irthermometer',
		password: "*******",
		table_aliases: db_table_aliases,
		field_aliases: db_field_aliases
	}

];
```

### How to write database 'driver' roadmap

You can write your own database 'driver'.

To do this, override DatabaseIntf class methods.

Most important class method is open():

```
	virtual int open(
		const std::string &connection,
		const std::string &login,
		const std::string &password,
		const std::string &db,
		int port
```

In case of web service, connection parameter is POST URL to post JSON data,
and connection db is URL to authorize using GET request. 

If 'db' parameter empty, no authorization is required.

### MySQL

In the Protobuf there are no date or time types, but it can return formatted date and time stamp as text.

This field may have integer type not text.


If proto defines some fields as timestamp, proto-db utility return MySQL error

```
Error insert record into SQL table 1 database mysql: Data truncated for column 'recvtime' at row 1
```

Change table's field type in the database like this:

```
ALTER TABLE iridium_packet drop column recvtime;
ALTER TABLE iridium_packet add COLUMN recvtime timestamp with time zone; 
```

### Firebird

```
ALTER TABLE "iridium_packet" drop "recvtime";
ALTER TABLE "iridium_packet" add "recvtime" VARCHAR(32); 

ALTER TABLE "iridium_packet" drop "recvtime";
ALTER TABLE "iridium_packet" add "recvtime" VARCHAR(32); 

ALTER TABLE "iridium_packet" drop "gps_time";
ALTER TABLE "iridium_packet" add "gps_time" VARCHAR(32); 

```

Each table's row must have unique identifier.

You need manually add column id of "autoincrement" type.

For instance, for column id create sequence generator:

```
create sequence gen_vega_id
```

Then in the trigger before insert put generated value to the "id" column:

Add create time. Add column:

```
ALTER TABLE "vega_SI13" ADD CREATED TIMESTAMP;
```
Then modify trigger:

```
ALTER TRIGGER trg_vega_bi
as
begin
  if ((new.id is null) or (new.id = 0)) then
  begin
    new.id = gen_id(gen_vega_id, 1);
    new.created = current_timestamp;
  end
end
```

```
CREATE TRIGGER trg_vega_bi for "vega_SI13"
active before insert position 0
as
begin
  if ((new.id is null) or (new.id = 0)) then
  begin
    new.id = gen_id(gen_vega_id, 1);
  end
end
```

### MySQL

Install
```
sudo apt install mysql-server
```

Create user and give priveleges using root account:

```
sudo mysql -u root -p
CREATE USER 'irthermometer'@'localhost' IDENTIFIED BY '********';
GRANT ALL PRIVILEGES ON * . * TO 'irthermometer'@'localhost';
\q
```

Login as SQL user and create database:

```
mysql -u irthermometer -p
CREATE DATABASE irthermometer;
\q
```

### Firebird

Configs in the /etc/firebird/2.5

Start
```
/etc/init.d/firebird2.5-superclassic start
```

### rxpk

0269730000006cc3743eed467b227278706b223a5b7b22746d7374223a31303334343430332c2274696d65223a22323032312d30322d32345432313a31363a34322e3930363737345a222c22746d6d73223a313239383233363632313930362c226368616e223a332c2272666368223a302c2266726571223a3836372e3130303030302c2273746174223a312c226d6f6475223a224c4f5241222c2264617472223a225346374257313235222c22636f6472223a22342f35222c226c736e72223a362e322c2272737369223a2d3130382c2273697a65223a35332c2264617461223a225141514241414b414b51414335594947664d654b7455354a70546975365551783161516b6977485743686c373162517878764d6c4f3271696c6e494562764737346849526a32593d227d5d7d

0269730000006cc3743eed46
{"rxpk":[{"tmst":10344403,"time":"2021-02-24T21:16:42.906774Z","tmms":1298236621906,"chan":3,"rfch":0,"freq":867.100000,"stat":1,"modu":"LORA","datr":"SF7BW125","codr":"4/5","lsnr":6.2,"rssi":-108,"size":53,"data":"QAQBAAKAKQAC5YIGfMeKtU5JpTiu6UQx1aQkiwHWChl71bQxxvMlO2qilnIEbvG74hIRj2Y="}]}


Error -535 84.237.104.16:34297: Send ACK error 
 84.237.104.16:34297
Message received 84.237.104.16:47998: 0251ff0200006cc3743eed46
Error -535 84.237.104.16:47998: Send ACK error 
 84.237.104.16:47998
Message received 84.237.104.16:47998: 024aec0200006cc3743eed46
Error -535 84.237.104.16:47998: Send ACK error 
 84.237.104.16:47998

00006cc3743eed46

027cc20000006cc3743eed467b2273746174223a7b2274696d65223a22323032312d30322d32352030313a30313a343320474d54222c226c617469223a36322e30323737342c226c6f6e67223a3132392e37323838332c22616c7469223a3334382c2272786e62223a312c2272786f6b223a302c2272786677223a302c2261636b72223a302e302c2264776e62223a302c2274786e62223a307d7d
Error -535 84.237.104.16:40173: Send ACK error 
Gateway statistics 1970-01-01T09:33:41 (62.02774, 129.72883, 348), rxnb: 1, rxok: 0, rxfw: 0, ackr: 0.0, dwnb: 0, txnb: 0
Message received 84.237.104.16:55288: 0254f80200006cc3743eed46
Error -535 84.237.104.16:55288: Send ACK error 
Gateway statistics 1970-01-01T09:33:41 (62.02774, 129.72883, 348), rxnb: 1, rxok: 0, rxfw: 0, ackr: 0.0, dwnb: 0, txnb: 0
Message received 84.237.104.16:55288: 021be80200006cc3743eed46
Error -535 84.237.104.16:55288: Send ACK error 



021be80000006cc3743eed467b227278706b223a5b7b22746d7374223a38313731323935362c2274696d65223a22323032312d30322d32365430323a30353a33342e3237323937355a222c22746d6d73223a313239383334303335333237322c226368616e223a342c2272666368223a302c2266726571223a3836342e3930303030302c2273746174223a312c226d6f6475223a224c4f5241222c2264617472223a22534631324257313235222c22636f6472223a22342f35222c226c736e72223a362e302c2272737369223a2d3130352c2273697a65223a31322c2264617461223a2251444144525147413041445730565479227d5d7d

{"rxpk":[{"tmst":81712956,"time":"2021-02-26T02:05:34.272975Z","tmms":1298340353272,"chan":4,"rfch":0,"freq":864.900000,"stat":1,"modu":"LORA","datr":"SF12BW125","codr":"4/5","lsnr":6.0,"rssi":-105,"size":12,"data":"QDADRQGA0ADW0VTy"}]}

20 bytes
0101 7000 988f 2158 0000 0000 0000 0000
0000 0000 0000 0000 0000 0000 0000 0000
0000 0000 0008 0008                    

449f387f3b305cdb049d

### txpk

JSON down:
```
{
	"txpk": {
		"codr": "4/5",
		"data": "0J7Qm9Cv0KDQnCEg0JPQntCb0JDQmtCi0JXQmtCeINCe0J/QkNCh0J3QntCh0KLQmCEhMQ==",
		"datr": "SF12BW125",
		"freq": 868,
		"ipol": true,
		"modu": "LORA",
		"ncrc": false, 300234069204980 
		"size": 52,
		"tmst": 1
	}
}
```

data:
d09ed09bd0afd0a0d09c2120d093d09ed09bd090d09ad0a2d095d09ad09e20d09ed09fd090d0a1d09dd09ed0a1d0a2d098212131

### 
```
JSON down: {"txpk":{"codr":"4/5","data":"0J7Qm9Cv0KDQnCEg0JPQntCb0JDQmtCi0JXQmtCeINCe0J/QkNCh0J3QntCh0KLQmCEhMQ==","datr":"SF12BW125","freq":868,"ipol":true,"modu":"LORA","ncrc":false,"powe":0,"rfch":1,"size":52,"tmst":1}}
ERROR: Packet REJECTED, unsupported frequency - 868000000 (min:0,max:0)
INFO: [down] PULL_ACK received in 1 ms
INFO: [down] PULL_ACK received in 0 ms
INFO: [down] PULL_ACK received in 1 ms
```

### Log format

Extract payload
```
cat lorawan-network-server.log | gawk '{ match($0, /a\"\:\"([0-9A-Za-z\=]+)\"/, arr); if(arr[1] != "") print arr[1] }'
```


024aec0000006cc3743eed467b227278706b223a5b7b22746d7374223a32373035303337322c2274696d65223a22323032312d30332d30325430363a32363a33372e3631313036355a222c22746d6d73223a313239383730313631363631302c226368616e223a362c2272666368223a312c2266726571223a3836382e3930303030302c2273746174223a312c226d6f6475223a224c4f5241222c2264617472223a22534631324257313235222c22636f6472223a22342f35222c226c736e72223a342e302c2272737369223a2d3130372c2273697a65223a31322c2264617461223a2251444144525147416a67567142776d67227d5d7d

base64_decode ============4030034501808e056a0709a0============
Sent ACK to 84.237.104.16:41095
rxpk device network address 01450330: {"rxpk":[{"time":"2021-03-02T06:26:37.00000Z","tmms":1298669215,"tmst":826637176,"freq":868.900000,"chan":6,"rfch":1,"stat":1,"modu":"LORA","datr":"SF12BW125","codr":"4/5","rssi":-107,"lsnr":5.1,"size":16,"data":"oAFFAzAABY4ABwmgJIPU0A=="}]}
Request identity service r: 0, device id: 3434383566378112
Message received 84.237.104.16:41095: 02baab0000006cc3743eed467b2273746174223a7b2274696d65223a22323032312d30332d30322030363a32353a333420474d54222c226c617469223a36322e30323737342c226c6f6e67223a3132392e37323838332c22616c7469223a3334382c2272786e62223a322c2272786f6b223a312c2272786677223a312c2261636b72223a3130302e302c2264776e62223a302c2274786e62223a307d7d
Sent ACK to 84.237.104.16:41095
Gateway statistics 6cc3743eed46 2021-03-02T06:25:34 (62.02774, 129.72883, 348), rxnb: 2, rxok: 1, rxfw: 1, ackr: 0.0, dwnb: 0, txnb: 0
Message received 84.237.104.16:54820: 02f2fb0200006cc3743eed46
Sent ACK to 84.237.104.16:54820


## Third-party dependencies

- lmdb
- [Filewatch](https://github.com/ThomasMonkman/filewatch)
- sqlite3
- mysql
- firebird

```
sudo apt install sqlite3 libsqlite3-dev libmysqlclient-dev firebird-dev
```

Firebird can use different library name. Check files in package:

```
dpkg-query -L firebird-dev
/usr/lib/x86_64-linux-gnu/libfbclient.so
```

### LMDB

```
sudo apt install liblmdb-dev 
```

## Known bugs

Javascript parser does not support merge using the spread operator like

```
const mergeResult = [...array1, ...array2]
```

in the dbs.js configuration file.

## Tips

### Check sqlite3 records

```
echo ".q" | sqlite3 -cmd "attach \"lns.data.sqlite.db\" as lns;select devname, temperature, datetime(received, 'unixepoch', 'localtime') received, rowid from vega_SI13 order by received desc limit 50;"
```

### Send packet to the network server

```
echo 02bbe50000006cc3743eed467b227278706b223a5b7b22746d7374223a343032333131313534302c226368616e223a332c2272666368223a302c2266726571223a3836342e3730303030302c2273746174223a312c226d6f6475223a224c4f5241222c2264617472223a22534631324257313235222c22636f6472223a22342f35222c226c736e72223a2d31382e352c2272737369223a2d3132312c2273697a65223a33372c2264617461223a22514441445251474151774143334749312b374553394d697030356a436c6f536f464e367a634b65437877394d7357457634513d3d227d5d7d | xxd -r -p | nc -q1 -4u 10.2.104.57 5000

# Send Join request
# 00111213141516171801020304050607088aaacbeb32a2
echo 02030b0000006cc3743eed467b227278706b223a5b7b22746d7374223a313236313338333435322c226368616e223a362c2272666368223a312c2266726571223a3836382e3930303030302c2273746174223a312c226d6f6475223a224c4f5241222c2264617472223a225346374257313235222c22636f6472223a22342f35222c226c736e72223a31302e302c2272737369223a2d33372c2273697a65223a32332c2264617461223a2241424553457851564668635941514944424155474277694b717376724d71493d227d5d7d| xxd -r -p | nc -q1 -4u 84.237.104.128 5000
```

## Extra files

List of registered LoraWAN networks "tests/netid-list.txt" copied from 
[NetID and DevAddr Prefix Assignments](https://www.thethingsnetwork.org/docs/lorawan/prefix-assignments/)

Shell script "tests/netid-list.sh" tests print-netid utility by the tests/netid-list.txt list.

## Implementation details 

### MAC processing chain

udp-listener.cpp              UDPListener::listen()
lora-packet-handler-impl.cpp  LoraPacketProcessor::put()
lora-packet-handler-impl.cpp  LoraPacketProcessor::enqueueMAC() LoraPacketProcessor::enqueueControl()
packet-queue.cpp              PacketQueue::push()
packet-queue.cpp              PacketQueue::runner()
packet-queue.cpp              PacketQueue::replyMAC()             PacketQueue::replyJoinRequest()
lorawan-mac.cpp               MacPtr::mkResponseMACs()            identity-service.cpp IdentityService::joinAccept()
lorawan-mac.cpp               MacPtr::mkResponseMAC()
utillora.cpp                  SemtechUDPPacket::mkPullResponse()
utillora.cpp                  SemtechUDPPacket::toTxJsonString()

### References

- [Semtech LoRaWAN-lib](https://os.mbed.com/teams/Semtech/code/LoRaWAN-lib//file/2426a05fe29e/LoRaMacCrypto.cpp/)
- [arduino aes implementation](https://raw.githubusercontent.com/arduino-libraries/LoraNodeShield/master/src/system/crypto/cmac.h)
- [Typescript implementation](https://github.com/anthonykirby/lora-packet/blob/master/src/lib/crypto.ts)

