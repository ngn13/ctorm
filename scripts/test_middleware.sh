#!/bin/bash

test_users() {
  users=$(curl --silent 'http://127.0.0.1:8084/users' | \
    jq -r '.list[0]| "\(.name) \(.age)"')

  [[ "${users}" == "${1}" ]] && return 0
  return 1
}

if ! test_users 'John 23'; then
  echo 'fail (1)'
  exit 1
fi

curl --silent 'http://127.0.0.1:8084/user/add' \
  -H 'Content-Type: application/json'          \
  -H 'Authorization: secretpassword'           \
  --data '{"name": "test", "age": 42}' -o /dev/null

curl -X DELETE --silent 'http://127.0.0.1:8084/user/delete?name=John' \
  -H 'Authorization: secretpassword' -o /dev/null

if ! test_users 'test 42'; then
  echo 'fail (2)'
  exit 1
fi

echo 'success'
