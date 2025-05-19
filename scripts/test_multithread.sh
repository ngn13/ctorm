#!/bin/bash

curl -s 'http://127.0.0.1:8085/?name=test' -o /dev/null
res=$(curl -s 'http://127.0.0.1:8086')

if [[ "${res}" == "hello test" ]]; then
  echo "success"
  exit 0
fi

echo "fail"
exit 1
