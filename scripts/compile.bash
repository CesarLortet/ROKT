#!/bin/bash

INFO="[INFO]"
ERROR="[ERROR]"

CFLAGS_RELEASE="-Wall -Werror -O3"

#!/bin/bash
#############################################################################################################
############################################### VARIABLES ###################################################
#############################################################################################################

CFLAGS_THREAD="-pthread"

CFLAGS_RELEASE="-Wall -Werror -O3"


if ! gcc -o build/main src/main.c src/gateway.c $CFLAGS_RELEASE;
then 
    echo "$ERROR Can't compile 'src/main.c'" >&2
    exit 1
fi

echo "$INFO App is ready"