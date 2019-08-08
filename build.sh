#!/bin/sh
rm -rf ./build
mkdir build
gcc -O0 `pkg-config --cflags glfw3` `pkg-config --static --cflags glew` -o build/omega.out src/system_linux.c `pkg-config --static --libs glfw3` `pkg-config --static --libs glew`
gcc -O0 -shared -o build/game.so -fPIC src/game.c
cp -rf ./res ./build
