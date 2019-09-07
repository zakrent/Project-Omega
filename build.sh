#!/bin/sh
rm -rf ./build
mkdir build
gcc -Wall -O0 -g `pkg-config --cflags glfw3` `pkg-config --static --cflags glew` -o build/omega.out src/system_linux.c `pkg-config --static --libs glfw3` `pkg-config --static --libs glew` -lm -lpthread -ldl
gcc -Wall -O0 -g -shared -nostdlib -o build/game.so -fPIC src/game.c
cp -rf ./data ./build
