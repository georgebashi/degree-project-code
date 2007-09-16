#!/bin/sh
echo "../test/Fischerspooner/#1/#1-03-Emerge.mp3" > test.m3u
./generator -s 10 -c sorted --dir=../test/ ../test/Fischerspooner/\#1/\#1-03-Emerge.mp3.vec >> test.m3u
totem test.m3u &
./generator -v -s 10 -c sorted --dir=../test/ ../test/Fischerspooner/\#1/\#1-03-Emerge.mp3.vec

