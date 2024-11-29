#!/bin/bash

wrk -t12 -c400 -d10s 'http://127.0.0.1:8080/'
