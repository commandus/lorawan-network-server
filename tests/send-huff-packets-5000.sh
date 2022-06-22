#!/bin/bash
#
# Send Huffman packets to the port 5000
#
INIT_TEMPERATURE=-10.0
STEP=0.0625
DELTA=0.5
TH="../logger-huffman/test-huffman"
fcnt=100
for i in {0..1}
do
  C="$TH -i $INIT_TEMPERATURE -s $STEP -d $DELTA"
  PKTS=`$C`
  for P in $PKTS
  do
     echo $P
    ./dev-payload -i identity.json -e 3231323549304c0a  -g 6cc3743eed46 -j -c $fcnt "$P"
    ./dev-payload -i identity.json -e 3231323549304c0a  -g 6cc3743eed46 -a "127.0.0.1:5000" -c $fcnt "$P"
    let "fcnt=fcnt+1"
  done
done
