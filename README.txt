扩展库：
1:ffi (lua语言调用底层C语言的一系列接口)
2:lpeg (基于 Parsing Expression Grammars 的模式匹配库)
3:lua-protobuf (解析google protobuf的库)
4:lua-rapidjson (快速解析json的库)
5.zlib
以上库来自：https://github.com/chexiongsheng/build_xlua_with_libs    Release:v2.1.15


1.编译XLua源码注意事项
a.一定要使用:android-ndk-r10e,下载地址:https://developer.android.google.cn/ndk/downloads/older_releases
b.通过android studio下载SDK platform(android版本是2.3):android-9
c.通过android studio下载cmake:3.10.2.4988404
d.并且配置环境变量:
    把camke的路径添加到系统的Path的环境变量里cmake 3.10.2.4988404
    添加环境变量：ANDROID_SDK-->E:\Android\Sdk
     添加环境变量： ANDROID_NDK-->E:\Android\Sdk\android-ndk-r10e
e.安装vs2017,并且安装vs2017里面的windowsUWP(Universal Windows Platform) windows 10 SDK
f.运行make_uwp.bat编译UWP ARM时会出现fopen unsafe(lua-rapidjson中有使用fopen)的错误,CMakeLists.txt中添加以下预处理器定义
if(MSVC)
    add_definitions(-D_CRT_SECURE_NO_WARNINGS)
endif()
g.编译jit时需要事先编译luajit-2.1.0b3,使用vs2017的开发人员命令行cmd切换到luajit-2.1.0b3/src下执行msvcbuild.bat或msvcbuild_mt.bat
h.编译windows平台时如果遇到缺少MSBuild.exe错误，需要在VS安装目录下的MsBuild所在的bin目录添加到系统的Path的环境变量里
i.如果编译windows平台没有出现vs2017的cmake支持说明cmake版本太旧需要升级到3.10.2.4988404及以上