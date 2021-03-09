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

## Run

```
./lorawan-network-server 84.237.104.128:2000
```


## Build

automake:

```
autogen.sh
./configure
make
sudo make install
```

cmake (Clang):

```
mkdir build
cd build
export CC=/usr/bin/clang;export CXX=/usr/bin/clang++;cmake ..
make
```

## Configuration files

### gateway.json

Array of gateways identifier and last statistics

  - gwid Gateway identifier (hex number string)
  - time UTC time of pkt RX, us precision, ISO 8601 'compact' format
  - lati latitude
  - long longitude
  - alti altitude, meters, integer
  - rxnb Number of radio packets received (unsigned integer)
  - rxok Number of radio packets received with a valid PHY CRC
  - rxfw Number of radio packets forwarded (unsigned integer)
  - ackr Percentage of upstream datagrams that were acknowledged
  - dwnb Number of downlink datagrams received (unsigned integer)
  - txnb Number of packets emitted (unsigned integer)

Example:
```
[
  {
    "gwid": "00006cc3743eed46"
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


## Thitd-party dependencies

[Semtech LoRaWAN-lib](https://os.mbed.com/teams/Semtech/code/LoRaWAN-lib//file/2426a05fe29e/LoRaMacCrypto.cpp/) uses

[arduino aes implementation](https://raw.githubusercontent.com/arduino-libraries/LoraNodeShield/master/src/system/crypto/cmac.h)

[Typescript implementation](https://github.com/anthonykirby/lora-packet/blob/master/src/lib/crypto.ts)
