#!/bin/bash

git pull origin master

make clean
make PI=1
make install
