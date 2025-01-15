#!/bin/bash

no_user=$(curl --silent -w "%{http_code}" 'http://127.0.0.1:8083/')
with_user=$(curl --silent -w "%{http_code}" 'http://127.0.0.1:8083/?username=ngn')

if [[ "${no_user}" != "no username provided200" ]]; then
  echo 'fail (1)'
  exit 1
fi

if [[ "${with_user}" != "username: ngn200" ]]; then
  echo 'fail (2)'
  exit 1
fi

echo 'success'
