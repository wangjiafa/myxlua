rem mkdir build64 & pushd build64
rem cmake -DLUAC_COMPATIBLE_FORMAT=ON -G "Visual Studio 15 2017 Win64" ..
rem IF %ERRORLEVEL% NEQ 0 cmake -DLUAC_COMPATIBLE_FORMAT=ON -G "Visual Studio 15 2017 Win64" ..
rem popd
rem cmake --build build64 --config Release

rem https://github.com/Tencent/xLua/blob/master/Assets/XLua/Doc/compatible_bytecode.md

mkdir build64 & pushd build64
cmake -G "Visual Studio 15 2017 Win64" ..
IF %ERRORLEVEL% NEQ 0 cmake -G "Visual Studio 15 2017 Win64" ..
popd
cmake --build build64 --config Release
pause