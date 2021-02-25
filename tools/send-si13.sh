#!/bin/sh
for j in {1..100}; do
  cat si13.bin | nc -u 84.237.104.128 8003 -w 0
  sleep 1m
done
