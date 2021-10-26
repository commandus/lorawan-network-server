# lorawan-network-server


lorawan-network-server is lightweight monolith LoRaWAN network server.

The payload is parsed according to the description of the data structure of the packet
sent by the end-device and is stored to the database in the table specified in the 
configuration file.

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

Finally

```
make libpkt2.a libpkt2util.a
```

Automake, autoconf, libtool, gcc or cmake must be installed first.


Also must install at leat one backend database library, ot insttl all of them:

```
sudo apt install liblmdb-dev sqlite3 libsqlite3-dev libmysqlclient-dev firebird-dev
```

Full set of libraries:
```
sudo apt install autoconf build-essential libtool libprotobuf-dev liblmdb-dev sqlite3 libsqlite3-dev libmysqlclient-dev firebird-dev libcurl4-openssl-dev protobuf-compiler libgoogle-glog-dev libsnmp-dev libnanomsg-dev libprotoc-dev
```

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

By default this database keep in memory and flushes to the disk as JSON file.

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
- deviceStatStorageName e.g. "device-counters.json"
- listenAddressIPv4 array of IPv4address:port e.g. ["127.0.0.1:5000","84.237.104.128:5000"]
- listenAddressIPv6 array of IPv6address:port
- gwStatStorageType log gateway statistics storage type, "file" or "post". Default none
- logGWStatisticsFileName e.g. log gateway statistics file name "gateway-stat.json"
- logDeviceStatisticsFileName e.g. log device statistics file name "device-stat.json"

- readBufferSize UDP buffer size Default 4096.
- verbosity 0..3 error logging verbosity (0- error only, 3- debug info)
- daemonize false..true Indicates does network server starts as daemon or not
- controlFPort 0: no remote control, 1..223- FPort values used by network service to control server. Default 0.

configFileName can used to load configuration from different location. 
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

Option "storageType" values are:

- json (default)
- lmdb (file database)
- txt (directory with files)

Default value is "json". The identifiers are stored in memory.

Option "lmdb" is a little safer and suitable for low-memory installations.

Option "txt" is slow and useful for debug only.

Option "gwStatStorageType" values are

- file
- post 

File example:
```
        "gwStatStorageType": "file",
        "logGWStatisticsFileName": "gw-stat.json"
```

post example:
```
        "gwStatStorageType": "post",
        "logGWStatisticsFileName": "http://localhost:50002/post.php"
```

#### Message queue

Received messages are sent to the database(s) as soon as possible. In case the database system is not avaliable
for some reason, received messages stay in the queue until database has up.

Option "messageQueueStorageName" set name of file name (or directory name).

There messageQueueStorageType option determines how to keep received messages in the temporary queue.

Option "messageQueueStorageType" values are:

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

Other programs can put files to this directory and lorawan-network-server will parse files and put messages to the databases.

- ".bin" - binary payload, as-is
- ".hex" - payload each byte represented as hexadecimal two digits number
- ".b64" - base64 encoded payload

Option "messageQueueDirFormat" values are:

- 0 or "bin" (default)
- 1 or "hex"
- 2 or "base64"

lorawan-network-server try to parse payload and insert parsed data to database(s). Does not matter success or fail is database insertaion, file is deleted.

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

## mac-gw utility

Send a command to a class C device bypassing the network server directly through the selected gateway.

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
- GatewayId gateway identifier

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
- properties

Name is used to find out appropriate database.

Valid values for "type" are "sqlite3", "postgresql".

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

Create table for iridium.IEPacket packet in the "mysql_1" database:

Pass message type in the -m option:

```
./proto-db -d mysql_1 -m iridium.IEPacket create
```

Determine message type by the payload using -x <payload-hex>

```
./proto-db create -d mysql -x 0100213887c1601c000000004a0000000000000000000000
./proto-db -d sqlite -x 010021b8b06b581f000000004a0000000000000000000000 create
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
...
/usr/lib/x86_64-linux-gnu/libfbclient.so
...

### LMDB

```
sudo apt install liblmdb-dev 
```

## Known bugs

Javascript parser does not support merge using the spread operator like

```
const mergeResult = [...array1, ...array2]
```

in the dbs.js configuration file

### References

[Semtech LoRaWAN-lib](https://os.mbed.com/teams/Semtech/code/LoRaWAN-lib//file/2426a05fe29e/LoRaMacCrypto.cpp/) uses

[arduino aes implementation](https://raw.githubusercontent.com/arduino-libraries/LoraNodeShield/master/src/system/crypto/cmac.h)

[Typescript implementation](https://github.com/anthonykirby/lora-packet/blob/master/src/lib/crypto.ts)


Device 	DevEUI           NwkSKey                          AppSKey                          devAddr
SI-13-232	3434383566378112 313747123434383535003A0066378888 35003A003434383531374712656B7F47 01450330
sh-2-1	323934344A386D0C 3338470C32393434170026004A386D0C 17002600323934343338470C65717B40 00550116
pak811-1  3231323549304C0A 34313235343132353431323534313235 34313235343132353431323534313235 34313235

52 bytes
d09ed09bd0afd0a0d09c2120d093d09ed09bd090d09ad0a2d095d09ad09e20d09ed09fd090d0a1d09dd09ed0a1d0a2d098212131


проверка покрытия

Message received 84.237.104.16:59233 (194 bytes): 02c99a0000006cc3743eed467b227278706b223a5b7b22746d7374223a39363133393232382c226368616e223a322c2272666368223a302c2266726571223a3836342e3530303030302c2273746174223a312c226d6f6475223a224c4f5241222c2264617472223a22534631324257313235222c22636f6472223a22342f35222c226c736e72223a312e302c2272737369223a2d3130362c2273697a65223a31342c2264617461223a225144414452514741657759414f51736a7730593d227d5d7d

## Known bugs

2021-07-23T11:39:28+09 Message received 84.237.104.16:47273 (198 bytes): 02f2fb0000006cc3743eed467b227278706b223a5b7b22746d7374223a34373235393339362c226368616e223a332c2272666368223a302c2266726571223a3836342e3730303030302c2273746174223a312c226d6f6475223a224c4f5241222c2264617472223a22534631324257313235222c22636f6472223a22342f35222c226c736e72223a332e382c2272737369223a2d3130382c2273697a65223a31382c2264617461223a225144414452514741686838452f594c2b364d342b46437752227d5d7d
2021-07-23T11:39:28+09 Sent ACK to 84.237.104.16:47273
2021-07-23T11:39:28+09 rxpk device 01450330: ffef2bfa60
2021-07-23T11:39:28+09 2021-07-23T11:39:28+09.376298Device EUI 3434383566378112, 84.237.104.16:47273
received packet {"activation":"ABP","class":"A","eui":"3434383566378112","nwkSKey":"313747123434383535003a0066378888","appSKey":"35003a003434383531374712656b7f47","name":"SI-13-23"}: ffef2bfa60
Javascript error: uncaught: 'cannot read property \x27time5\x27 of ...' in 
new Date(((field.packet46420.time5.day_month_year >> 9) & 0x7f) + 2000, ((field.packet46420.time5.day_month_year >> 5) & 0xf) - 1, (field.packet46420.time5.day_month_year & 0x1f), field.packet46420.time5.hour, field.packet46420.time5.minute, field.packet46420.time5.second, 00).getTime() / 1000
Aborted

## Implementation 

### MAC processing chain

udp-listener.cpp              UDPListener::listen()
lora-packet-handler-impl.cpp  LoraPacketProcessor::put()
lora-packet-handler-impl.cpp  LoraPacketProcessor::enqueueMAC() LoraPacketProcessor::enqueueControl()
packet-queue.cpp              PacketQueue::push()
packet-queue.cpp              PacketQueue::runner()
packet-queue.cpp              PacketQueue::replyMAC()
utillora.cpp                  SemtechUDPPacket::mkPullResponse()
utillora.cpp                  SemtechUDPPacket::toTxImmediatelyJsonString()
