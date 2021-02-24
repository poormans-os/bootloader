#!/bin/bash

xdpyinfo > /dev/null 2>&1 &
sleep 1
kill $! > /dev/null 2>&1 && cmd.exe /c $XSERVER_PATH > /dev/null 2>&1
true
