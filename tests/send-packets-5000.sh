#!/bin/bash
while read -r line
do
    echo "$line" | xxd -r -p | nc -4u -q1 localhost 5000
	sleep 1
done < <(cat <<EOM
029bb40000006cc3743eed467b227278706b223a5b7b22746d7374223a3138363333343238332c226368616e223a342c2272666368223a302c2266726571223a3836372e3330303030302c2273746174223a312c226d6f6475223a224c4f5241222c2264617472223a225346374257313235222c22636f6472223a22342f35222c226c736e72223a31302e302c2272737369223a2d35342c2273697a65223a33372c2264617461223a22514455794d5453414241424b34626453303633796353727a59566d666b496f506a6e6d4546696e7266556e3477546b7062673d3d227d5d7d
020e820000006cc3743eed467b227278706b223a5b7b22746d7374223a3230353334323630342c226368616e223a362c2272666368223a302c2266726571223a3836372e3730303030302c2273746174223a312c226d6f6475223a224c4f5241222c2264617472223a225346374257313235222c22636f6472223a22342f35222c226c736e72223a31302e302c2272737369223a2d35342c2273697a65223a33372c2264617461223a22514455794d5453414251424b31324475376556596879787467336e6c43495838696e65716177586a7a5971703063556853413d3d227d5d7d
0270e90000006cc3743eed467b227278706b223a5b7b22746d7374223a3232343332333831392c226368616e223a332c2272666368223a302c2266726571223a3836372e3130303030302c2273746174223a312c226d6f6475223a224c4f5241222c2264617472223a225346374257313235222c22636f6472223a22342f35222c226c736e72223a392e382c2272737369223a2d35352c2273697a65223a32312c2264617461223a22514455794d5453414267424b5253304e472f33444166597035366267227d5d7d
EOM
)