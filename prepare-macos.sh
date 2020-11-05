#!/bin/sh
brew install boost
brew install openssl
brew install nlohmann-json
mkdir -p libs/third
cd libs/third
git clone --recurse-submodules https://github.com/socketio/socket.io-client-cpp.git
cd socket.io-client-cpp
cmake -DOPENSSL_INCLUDE_DIR:STRING=/usr/local/opt/openssl@1.1/include ./
cd lib/websocketpp
git pull origin master
cd ../..
make install
cd ../../..