#!/bin/bash

INFO="[INFO]"
ERROR="[ERROR]"

CFLAGS_RELEASE="-Wall -Werror -O3"

#!/bin/bash
#############################################################################################################
############################################### VARIABLES ###################################################
#############################################################################################################

CLIB_SOCKETCONNECTION="src/socket_connection.c"
ROOT="../src"

CFLAGS_THREAD="-pthread"
CFLAGS_ZLIB="-lz"
CFLAGS_YML="-lyaml"
CFLAGS_RELEASE="-Wall -Werror -O3"


if ! gcc -o build/test_socket_connection src/test_socket_connection.c $CLIB_SOCKETCONNECTION $CFLAGS_RELEASE;
then 
    echo "$ERROR Can't compile 'src/test_socket_connection.c'" >&2
fi

if ! gcc -o build/test_configuration src/test_configuration.c ../lib/configuration.c ../lib/encrypt.c $CFLAGS_YML $CFLAGS_RELEASE;
then 
    echo "$ERROR Can't compile 'src/test_configuration.c'" >&2
fi

if ! gcc -o build/test_encrypt src/test_encrypt.c ../lib/encrypt.c $CFLAGS_RELEASE;
then 
    echo "$ERROR Can't compile 'src/test_encrypt.c'" >&2
fi

if ! gcc -o build/test_dataset_creation src/test_dataset_creation.c ../src/dataset.c ../lib/configuration.c ../lib/encrypt.c $CFLAGS_YML $CFLAGS_THREAD $CFLAGS_RELEASE;
then 
    echo "$ERROR Can't compile 'src/test_dataset_creation.c'" >&2
fi
echo "$INFO Tests are ready"