# lorawan-network-server

lorawan-network-server is lightweigth LoRaWAN network server.

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
