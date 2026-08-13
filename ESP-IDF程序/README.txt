编译环境：ESP-IDF v5.5.5 + VScode + ESP-IDF扩展
默认配置适用于：ESP32-S3-WROOM-1-N16R8

在IDF中可以修改lwIP协议栈中 TCP 默认发送缓冲区的大小，因此在IDF中编译的固件推流速度比Arduino中编译的快

编译方法：
1、新建一个IDF工程项目，在上方选择信任工作区，把新建项目的main文件夹把里面的文件删除
2、将本项目中的程序文件复制到你新建的项目中
3、参照项目README.md修改代码中你所用的引脚号
4、点击左下角小扳手开始编译，保持网络畅通，编译前会自动下载所需依赖
5、如果编译失败，在managed_components\esp32async__asynctcp\CMakeLists.txt文件中加入REQUIRES arduino-esp32
6、编译完成后，点击左下角插头图标选择烧录方式，点击闪电图标进行烧录