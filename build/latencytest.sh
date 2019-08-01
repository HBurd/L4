#!/bin/bash

./L4server 4444 &
ID1=$!
./latenc 4443 4444 500 500 65536 &
ID2=$!
./L4client 127.0.0.1 4443

kill $ID1
kill $ID2
