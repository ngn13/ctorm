#!/bin/bash

# crow
g++ -O3 -o crow/out crow/*.cpp
crow/out & fpid=$!
sleep 3 
ab -t 10 -n 10000 -c 100 127.0.0.1:8080/
kill -9 $fpid

# fiber 
pushd fiber > /dev/null
  go build
popd > /dev/null
fiber/fiber & fpid=$!
sleep 3
ab -t 10 -n 10000 -c 100 127.0.0.1:8080/
kill -9 $fpid

# ctorm
gcc -O3 -o ctorm/out -lctorm ctorm/*.c
ctorm/out & fpid=$!
sleep 3 
ab -t 10 -n 10000 -c 100 127.0.0.1:8080/
kill -9 $fpid

# tide 
pushd tide > /dev/null
  cargo run & fpid=$!
popd > /dev/null
sleep 3
ab -t 10 -n 10000 -c 100 127.0.0.1:8080/
kill -9 $fpid

# express
pushd express > /dev/null
  npm install
popd > /dev/null
node express/index.js & fpid=$!
sleep 3
ab -t 10 -n 10000 -c 100 127.0.0.1:8080/
kill -9 $fpid
