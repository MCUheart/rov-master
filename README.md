

<div align="center">
  <a href="https://github.com/zengwangfa/rov-master"><img src="https://zengwangfa.oss-cn-shanghai.aliyuncs.com/rov/rovmaster1.jpg" alt=""></a>
  <a href="https://github.com/zengwangfa/rov-master"><h1>Rov-Master</h1></a>
</div>


<div align="center">
  <a href="http://wiki.friendlyarm.com/wiki/index.php/NanoPi_NEO_Core/zh"><img src="https://img.shields.io/badge/Device-Nanopi NEO Core-brigreen.svg?style=flat-square" alt="Device"></a>
  <a href="https://img.shields.io"><img src="https://img.shields.io/github/repo-size/zengwangfa/rov-master?style=flat-square" alt="Size"></a>
</div>

# 1. 介绍

- 本项目用到了以下软件包：
  - [WiringNP](https://github.com/friendlyarm/WiringNP) This is a GPIO access library for NanoPi. It is based on the WiringOP for Orange PI which is based on original WiringPi for Raspberry Pi.
  - [EasyLogger](https://github.com/armink/EasyLogger) 是一款超轻量级(ROM<1.6K, RAM<0.3K)、高性能的 C/C++ 日志库。

# 2. 使用
- 运行环境：NanoPi NEO
```shell
$ git clone https://github.com/zengwangfa/rovmaster
$ cd rovmaster
```

## 2.1 安装相关依赖
在第一次运行程序之前，务必执行(`build.sh` 自动化安装依赖库)):

```shell
$ chmod +x build.sh
$ ./build.sh

```
build.sh 中安装相关依赖库

## 2.2 编译与执行
由于使用 WiringNP 库，需要 root 权限：

```shell
$ make 
$ sudo ./rovmaster
```

## 2.3 注意事项

### 2.3.1 设备未打开
若执行时提示无法打开某设备，输入 `sudo npi-config` > `Advanced Options` 中使能相关设备：

```shell
$ sudo npi-config
```

若时区不对，也可以在 `npi-config` > `Localisation Options` > `Change Timezone` 中选择修改。
有时修改会后编译执行会出问题，未出现修改部分效果，此时 `make clean` 后再编译运行即可。

视频推流命令在 `video.sh` 脚本中，指定相关视频参数，在其中修改 **摄像头视频推流参数**即可。

### 2.3.2 easylogger编译
对easylogger编辑后，需要进行重新编译时：
切换至 `lib/easylogger/` 目录，输入 `make` 进行编译后，需要重新拷贝生成的链接库 `libeasylogger.so`

```shell
$ cd lib/easylogger/
$ make
$ sudo cp libeasylogger.so ..
$ sudo cp libeasylogger.so /usr/lib/
```

### 2.3.3 wiringNP编译
对wiringNP编辑后，需要进行重新编译时：
切换至 `lib/WiringNP/` 目录，启动 `./build.sh` 脚本进行自动编译即可

```shell
$ ./build.sh
```

# 3. 进度
- 驱动模块
	- [x] ADS1118 (ADC)
		- [x] 电压检测 (待实际测试)
		- [x] 电流检测 (待实际测试)
	- [ ] PCA9685 (PWM) (待实际测试)
	- [x] 深度传感器
		- [x] SPL1301 (待实际测试)
		- [x] MS5837  (待实际测试)
	- [x] 系统设备状态
		- [x] CPU使用率、温度
		- [x] 内存状态
		- [x] 硬盘状态
		- [x] 实时网速
	- [x] IO设备
		- [x] RGB
		- [x] 按键
		- [x] 蜂鸣器
	- [ ] PWM设备 (待实际测试)
		- [x] 云台
		- [x] 探照灯
		- [x] 机械臂
	- [x] OLED
	- [x] 九轴
	- [ ] 音频MIC

- 功能模块
	- [x] OLED状态显示
	- [x] 视频推流
	- [x] 数据通信
		- [x] 下行-控制数据
		- [x] 上行-ROV状态数据
	- [ ] 系统自检
	- [ ] 日志记录
	- [ ] 传感器融合

- 控制模块
	- [x] 手柄控制 (待实际测试)
	- [x] 定深控制 (待实际测试)
	- [ ] 定向控制
	- [ ] 变焦摄像头控制


