#! /bin/sh
cd ./build/

ctest -j6 -C Debug -T test --output-on-failure -R ^tests$

cd ../ 