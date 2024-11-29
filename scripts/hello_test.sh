#!/bin/bash

res_200=$(curl --silent -w "%{http_code}" 'http://127.0.0.1:8080/')
res_404=$(curl --silent -o /dev/null -w "%{http_code}" 'http://127.0.0.1:8080/none')

if [[ "${res_200}" != "Hello world!200" ]]; then
  echo 'fail (1)'
  exit 1
fi

if [[ "${res_404}" != "404" ]]; then
  echo 'fail (2)'
  exit 1
fi

echo 'success'
