#!/bin/bash

res_200=$(curl --silent -w "%{http_code}" 'http://127.0.0.1:8082/echo/just%20testing/empty')
res_404=$(curl --silent -o /dev/null -w "%{http_code}" 'http://127.0.0.1:8082/echo/test')

if [[ "${res_200}" != "param: just testing200" ]]; then
  echo 'fail (1)'
  exit 1
fi

if [[ "${res_404}" != "404" ]]; then
  echo 'fail (2)'
  exit 1
fi

echo 'success'
