#!/bin/sh
PATH_PKT_FWD=~/git/rak_common_for_gateway/lora/rak2287/sx1302_hal
cp $PATH_PKT_FWD/libtools/libbase64.a .
cp $PATH_PKT_FWD/libtools/libtinymt32.a .
cp $PATH_PKT_FWD/libtools/libparson.a .
cp $PATH_PKT_FWD/libloragw/libloragw.a .

cp $PATH_PKT_FWD/libloragw/inc/config.h .
cp $PATH_PKT_FWD/libtools/inc/base64.h  .
cp $PATH_PKT_FWD/libtools/inc/parson.h .
cp $PATH_PKT_FWD/packet_forwarder/inc/jitqueue.h .
cp $PATH_PKT_FWD/packet_forwarder/inc/trace.h .
cp $PATH_PKT_FWD/libloragw/inc/loragw_aux.h .
cp $PATH_PKT_FWD/libloragw/inc/loragw_com.h .
cp $PATH_PKT_FWD/libloragw/inc/loragw_gps.h .
cp $PATH_PKT_FWD/libloragw/inc/loragw_hal.h .
cp $PATH_PKT_FWD/libloragw/inc/loragw_reg.h .

cp $PATH_PKT_FWD/packet_forwarder/src/jitqueue.c .
cp $PATH_PKT_FWD/packet_forwarder/src/lora_pkt_fwd.c .


exit 0
