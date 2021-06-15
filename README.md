# lorawan-network-server

lorawan-network-server is lightweigth LoRaWAN network server.

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

Automake, autoconf, libtool, gcc or cmake must be installed first.

```
apt install autoconf build-essential libtool
```

You can use

- autoconf or
- cmake

build system.

### autoconf

Generate auttomake files, configure and make:

```
autogen.sh
./configure
make
sudo make install
```

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
- server Network server properties, including end-device list served by the server
- configFileName (optional) Redirect config file to another one

Server config:

- identityStorageName end-device list file. Default is ~/identity.json
- listenAddressIPv4 IPv4address:port
- listenAddressIPv6 IPv6address:port
- readBufferSize UDP buffer size Default 4096.
- verbosity 0..3 error logging verbosity (0- error only, 3- debug info)
- daemonize false..true Indicates does network server starts as deamon or not

configFileName can used to load configuration from different location. 
Do not use this parameter except when you really need it.

identityStorageName is property of the server because network server is responsible for end-device.
In contrast gatewaysFileName property points to the gateways list which send messages to one or more network servers.

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

### gateway.json

Gateways list gateways.

Each entry has an gateway identifier, address, name and gateway statistics.

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

Instead of an host address, you can use host name (domain name). 

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
- name         optional device name (max 8 chars)

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

## mac-gw utility

Send a command to a class C device bypassing the network server directly through the selected gateway.

Parameters:

- -g gateway identifier or -G gateway name
- -e end-device identifier or -E dev-device name
- -p payload (optional)

Options:

-x --regex use regilar expresseion in -g, -e options instead of wildcards ('*', '?').

Options -g, -e can contain "*" and "?" wildcards, or regular expression lile ".*" if -x option is set.
Regular expressions grammar is similar to the grep.

Option -p requires hex string, for instance "0faa0167" is valid parameter value.

MAC commands has short and long names.
For example, "a" is a short name of "linkadr" command.
Most commands have one or more parameteres.
For example, "linkadr" command has 5 parameters:

```
linkadr 2 7 255 1 0
```
where 2 is tx power, 7- data rate, 255- channel mask, 1-  transmissions per messege, 0- mask control.

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

## Packet types sent by Semtech gateway

Tag values:

- 0 PUSH_DATA
- 1 PUSH_ACK
- 2 PULL_DATA
- 3 PULL_RESP
- 4 PULL_ACK

BS identifier 00006cc3743eed46

- PV protocol version 1 byte
- TOKE token 2 bytes long
- TG tag 1 byte
- GatewayIdentifie gateway identifier

### Ping?

Tag 2 (PULL_DATA)
021be80200006cc3743eed46
PV      GatewayIdentifie
  TOKE
      TG

### Stat

Tag 0 (PUSH_DATA)

02e3460000006cc3743eed467b2273746174223a7b2274696d65223a22323032312d30322d32342030343a35343a303120474d54222c226c617469223a36322e30323737342c226c6f6e67223a3132392e37323838332c22616c7469223a3334382c2272786e62223a302c2272786f6b223a302c2272786677223a302c2261636b72223a302e302c2264776e62223a302c2274786e62223a307d7d

02e3460000006cc3743eed46
PV      GatewayIdentifie
  TOKE
      TG

{"stat":{"time":"2021-02-24 04:54:01 GMT","lati":62.02774,"long":129.72883,"alti":348,"rxnb":0,"rxok":0,"rxfw":0,"ackr":0.0,"dwnb":0,"txnb":0}}


021be80200006cc3743eed46
02e3460000006cc3743eed46

## Database backend

Database configuration file dbs.js is Javascript declaration of "databases" array.

Each element is object with members

- name
- type
- connection
- table_aliases
- field_aliases

Name is used to find out appropriate database.

Valid values for "type" are "sqlite3", "postgresql".

Connection for database type "sqlite3" is file name of SQLite database.

Connection for database type "postgresql" looks like:

postgresql://irthermometer:************@localhost:5432/irthermometer",

See [Connection URIs](https://www.postgresql.org/docs/10/libpq-connect.html)

table_aliases is an array of array of two elements. First element is protobuf package.message.
Second element is an alias used in the database for table name.

field_aliases is an array of array of two elements. First element is protobuf package.message.field
Second element is an alias used in the database for table column.

If second element is empty string, this message or field does not put to the database.

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
		name: "sqlite",
		type: "sqlite3",
		connection: "lns.data.sqlite.db",
		table_aliases: sqlite_table_aliases,
		field_aliases: sqlite_field_aliases
	},
	{
		name: "postgres",
		type: "postgresql",
		connection: "postgresql://irthermometer:************@localhost:5432/irthermometer",
		table_aliases: sqlite_table_aliases,
		field_aliases: sqlite_field_aliases
	},
    	{
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

In the Protobuf there are no date or time types, but it can return formatted date and time stamp as text.

This field may have integer type not text.


If proto defines some fields as timestamp, proto-db utility return MySQL error

```
Error insert record into SQL table 1 database mysql: Data truncated for column 'recvtime' at row 1
```

Change rable's field type in the dstabase like this:

```
ALTER TABLE iridium_packet drop column recvtime;
ALTER TABLE iridium_packet add COLUMN recvtime timestamp with time zone; 

-- firebird
ALTER TABLE "iridium_packet" drop "recvtime";
ALTER TABLE "iridium_packet" add "recvtime" VARCHAR(32); 

ALTER TABLE "iridium_packet" drop "recvtime";
ALTER TABLE "iridium_packet" add "recvtime" VARCHAR(32); 

ALTER TABLE "iridium_packet" drop "gps_time";
ALTER TABLE "iridium_packet" add "gps_time" VARCHAR(32); 

```

### proto-db utility

```
proto-db helper utility
  <command>                 list|create|insert. Default list
  -p, --proto=<path>        proto files directory. Default 'proto'
  -c, --dbconfig=<file>     database config file name. Default 'dbs.js'
  -d, --dbname=<database-name> database name, Default all
  -m, --message=<packet.message> Message type packet and name
  -x, --hex=<hex-string>    insert command, payload data.
  -o, --offset=<number>     list command, offset. Default 0.
  -l, --limit=<number>      list command, limit size. Default 10.
  -s, --asc=<field-name>    list command, sort by field ascending.
  -S, --desc=<field-name>   list command, sort by field descending.
  -v, --verbose             Set verbosity level
  -?, --help                Show this help
```

Create table for iridium.IEPacket packet in the "mysql_1" database:

Pass message type in the -m option:

```
./proto-db -d mysql_1 -m iridium.IEPacket create
```

Detremine message type by the payload using -x <payload-hex>

```
./proto-db -d mysql_1 -x 0100213887c1601c000000004a0000000000000000000000
```

Print "iridium.IEPacket" messages stored in the "mysql_1" database:

```
./proto-db -d mysql_1 -m iridium.IEPacket list
```

Insert data from payload

```
./proto-db -d mysql -m iridium.IEPacket insert -x 014c00011c00e8444601333030323334303639323030383530001a070000e199205e030b00003eea3781fbcc05000000021c00c068b50328f1bd078999205e07050000009f1be60ca313f432000000
```

### MySQL

Install
```
sudp apt install mysql-server
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
...
/usr/lib/x86_64-linux-gnu/libfbclient.so
...
```

### LMDB

```
sudo apt install liblmdb-dev 
```

[Semtech LoRaWAN-lib](https://os.mbed.com/teams/Semtech/code/LoRaWAN-lib//file/2426a05fe29e/LoRaMacCrypto.cpp/) uses

[arduino aes implementation](https://raw.githubusercontent.com/arduino-libraries/LoraNodeShield/master/src/system/crypto/cmac.h)

[Typescript implementation](https://github.com/anthonykirby/lora-packet/blob/master/src/lib/crypto.ts)


Device 	DevEUI           NwkSKey                          AppSKey                          devAddr
SI-13-232	3434383566378112 313747123434383535003A0066378888 35003A003434383531374712656B7F47 01450330
sh-2-1	323934344A386D0C 3338470C32393434170026004A386D0C 17002600323934343338470C65717B40 00550116
pak811-1  3231323549304C0A 34313235343132353431323534313235 34313235343132353431323534313235 34313235

52 bytes
d09ed09bd0afd0a0d09c2120d093d09ed09bd090d09ad0a2d095d09ad09e20d09ed09fd090d0a1d09dd09ed0a1d0a2d098212131