#!/bin/bash -e

npm install
node index.js & fpid=$!
sleep 3

ab -t 10 -n 10000 -c 100 127.0.0.1:8080/

sleep 3
kill -9 $fpid
