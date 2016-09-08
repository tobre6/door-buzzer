#!/bin/bash
set -e

ARDUINO_IDE_VERSION=1.6.9
BUILD_DIR="build"

if [ $(uname) == 'Darwin' ]
then
    PLATFORM='macosx'
    FILE_EXT='zip'
    ARDUINO_DIR='Arduino.app/Contents/Java'
else
    PLATFORM='linux64'
    FILE_EXT='tar.xz'
    ARDUINO_DIR="arduino-$ARDUINO_IDE_VERSION"
fi

if [ ! -e "$BUILD_DIR" ]
then
    mkdir $BUILD_DIR
fi
cd $BUILD_DIR

if [ ! -e "$ARDUINO_DIR" ]
then
    ARDUINO_FILE=arduino-${ARDUINO_IDE_VERSION}-$PLATFORM.$FILE_EXT
    wget -O $ARDUINO_FILE http://arduino.cc/download.php?f=/$ARDUINO_FILE

    if [ $FILE_EXT == 'zip' ]
    then
        unzip $ARDUINO_FILE
    else
        tar xf $ARDUINO_FILE
    fi

    rm -f $ARDUINO_FILE
fi

if [ ! -e "esp8266" ]
then
    git clone https://github.com/esp8266/Arduino.git
    mv Arduino esp8266
    cd esp8266/tools
    python get.py
    cd ../../
fi

if [ ! -e "makeEspArduino" ]
then
    git clone https://github.com/plerup/makeEspArduino.git
fi

export ESP_ROOT=$(pwd)/esp8266
#export VERBOSE=1
export SINGLE_THREAD=1

if [ ! -e "pubsubclient" ]
then
git clone https://github.com/Imroy/pubsubclient
fi

cd makeEspArduino

export SKETCH=Door-Buzzer.ino
export UPLOAD_PORT=/dev/cu.usbserial-A92HD3JZ
export LIBS="$ESP_ROOT/libraries/ESP8266WiFi/ ../pubsubclient/src/"

cp ../../*.ino .

if [ $(uname) == 'Darwin' ]
then
    make -f makeEspArduino.mk upload
    screen $UPLOAD_PORT 115200
else
    make -f makeEspArduino.mk all
fi
