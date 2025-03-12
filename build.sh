#!/bin/bash

fqbn=longan-rp2040:longan-rp2040:canbed2040
port=/dev/ttyACM0

case $1 in
  compile)
    arduino-cli compile --fqbn $fqbn
    ;;
  debug)
    arduino-cli compile --fqbn $fqbn --build-property build.extra_flags=-DSCAN7_DEBUG=1
    ;;
  monitor)
    arduino-cli monitor -p /dev/ttyACM0
    ;;
  upload)
    arduino-cli upload -p /dev/ttyACM0 --fqbn $fqbn
    ;;
  *)
    echo "./build.sh [compile|debug|monitor|upload]"
    ;;
esac
