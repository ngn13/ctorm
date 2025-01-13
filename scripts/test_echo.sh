#!/bin/bash

data=$(curl -X POST 'http://127.0.0.1:8080/post'       \
  -H 'Content-Type: application/x-www-form-urlencoded' \
  --data 'msg=testing' --silent)

if [[ "${data}" != "message: testing" ]]; then
  echo 'fail (1)'
  exit 1
fi

echo 'success'
