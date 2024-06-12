#!/bin/bash -e

header_url="https://github.com/CrowCpp/Crow/releases/download/v1.2.0/crow_all.h"
header="crow_all.h"

if [ ! -f "$header" ]; then
  wget "$header_url" -O "$header"
fi

g++ -O3 -o out *.cpp
./out & fpid=$!
sleep 3

ab -t 10 -n 10000 -c 100 127.0.0.1:8080/

sleep 3
kill -9 $fpid
