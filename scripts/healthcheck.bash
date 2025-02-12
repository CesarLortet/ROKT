#!bin/sh
result=0

#### ARRAYS
#if ! pidof ccs_arrays.out >/dev/null
#then
#    let "result+=1"
#### DISPATCHER
if [[ "$(ps | grep ./build/gateway | wc -l)" -lt 2 ]]
then
    let "result+=1"
    echo "Dispatcher Issue"
fi

if [ $result -gt 0 ]
then
    #ps axf | ./build/gateway | grep -v grep | awk '{print "kill -2 " $1}'
    #pkill --signal SIGINT ./build/gateway
    exit 1
fi