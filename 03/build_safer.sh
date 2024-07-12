#!/bin/bash

lsof -i :7777 | grep LISTEN | awk '{print $2}' | xargs kill -9

gcc -fsanitize=address -g -o build/database main.c server.c distributor.c http.c requests/*.c utils/*.c \
    -I/opt/homebrew/opt/libuv/include \
    /opt/homebrew/opt/libuv/lib/libuv.a \
    -framework CoreFoundation -framework CoreServices