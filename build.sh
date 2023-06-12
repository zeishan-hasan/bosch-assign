#! /bin/sh

cd ./build/

cmake --build . --config Debug --target all -j 6 --

cd ../