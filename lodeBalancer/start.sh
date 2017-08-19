#!/bin/sh
./server 127.0.0.2 6002 &
./server 127.0.0.4 6004 &
./server 127.0.0.3 6003 &
