#!/bin/bash

case $1 in
  compile)
    arduino-cli compile --fqbn longan-rp2040:longan-rp2040:canbed2040
    ;;
  upload)
    arduino-cli upload -p /dev/ttyACM0 --fqbn longan-rp2040:longan-rp2040:canbed2040
    ;;
  *)
    echo "./build.sh [compile|upload]"
    ;;
esac
