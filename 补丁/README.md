该补丁用于修复ESPAsyncWebServer与AsyncTCP库存在的bug。问题现象：开启服务后，通过浏览器访问服务器，随即长按BOOT键关闭服务，再立刻长按BOOT键重新开启服务，此时会输出报错 `[AsyncTCP.cpp:1542] begin(): bind error: -8`，并且服务无法正常访问。

补丁使用方法：
1. 请确认库版本：`ESPAsyncWebServer` 版本为 **3.12.0**，`AsyncTCP` 版本为 **3.5.0**。
2. Arduino程序使用方式：
打开路径 `%USERPROFILE%\Documents\Arduino\libraries`，将本项目下 `/补丁/arduino` 目录内的全部文件复制到此目录，完成覆盖替换。
3. ESP‑IDF程序使用方式：
完成首次编译后，项目会生成 `managed_components` 文件夹，将本项目下 `/补丁/esp-idf` 目录内的全部文件复制到该目录，完成覆盖替换。