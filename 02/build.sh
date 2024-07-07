#!/bin/bash

lsof -i :7777 | grep LISTEN | awk '{print $2}' | xargs kill -9

gcc -o server server.c -I/opt/homebrew/opt/libuv/include -L/opt/homebrew/opt/libuv/lib -luv