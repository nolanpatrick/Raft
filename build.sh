#!/bin/sh

clear
set -ex

cc src/main.c -o build/raft -static
