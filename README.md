

## 一个RS232转串口ESP32通过MQTT发送数据的小DEMO

程序使用VScode+PlatformIO开发

目前main.cpp中是向仪表发送一个0x20 收到15个字节数据然后转BCD码发送

在Lib文件夹添加了Modbus库文件，需要Modbus协议可自行参考库文件中的Example

![](3.Images/1.jpg)



