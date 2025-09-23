#!/bin/bash

set -e

# sudo usermod -a -G dialout $USER
if [ $# -ne 1 ]; then
    PROJECT="IDF_MOKUKU"
else
    PROJECT=$1
fi

echo "monitor project " $PROJECT

cd $PROJECT

idf.py monitor
