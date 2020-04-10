#!/bin/sh 
#这个脚本的作用是自动安装相关的软件、安装相关的库
#需要root权限来运行这个脚本


echo "=====================安装WiringNP=========================="
cd ./lib/WiringNP/
chmod 755 build
./build

echo "===============编译mjpg-streamer及安装相关依赖=============="
sudo apt-get update
sudo apt-get install -y cmake libjpeg8-dev
cd ../mjpg-streamer-experimental/
make
sudo make install

echo "=======================编译easylogger======================"
cd ../easylogger
make
sudo cp libeasylogger.so ..
sudo cp libeasylogger.so /usr/lib/

cd ../..

echo "=======================使编译脚本可执行====================="
chmod +x compile.sh

echo "=======================使推流脚本可执行====================="
chmod +x video.sh


exit 0
