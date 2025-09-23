#!/bin/bash

# Installation steps for emscripten:
# 1) git clone https://github.com/emscripten-core/emsdk.git
# 2) cd smdsk
# 3) ./emsdk install latest
# 4) ./emsdk activate latest
# 5) source ./emsdk_env.sh

if [ $(which emcmake) ] && [ $(which emcc) ]; then
	echo "Found emcmake: $(which emcmake)";
	echo "Found emcc: $(which emcc)";
else
	echo "No emcmake and emcc found! Make sure you activated the emsdk environment by calling: source emsdk_env.sh";
	exit 1;
fi

mkdir -p embuild
cd embuild
emcmake cmake .. -DPLATFORM=Web
cmake --build . --parallel