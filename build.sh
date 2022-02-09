#!/bin/sh

clear
set -ex

CFLAGS="-Wall -Werror -Wextra"

cc src/main.c -o build/raft -g3 $CFLAGS -fsanitize=address
