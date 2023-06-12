#! /bin/sh

mkdir -p quick-run/
g++ bosch-assign.cpp -o ./quick-run/bosch-assign -lpthread

./quick-run/bosch-assign
