# mkdir -p build_unix && cd build_unix
# cmake -DLUAC_COMPATIBLE_FORMAT=ON ../
# cd ..
# cmake --build build_unix --config Release

# https://github.com/Tencent/xLua/blob/master/Assets/XLua/Doc/compatible_bytecode.md

mkdir -p build_unix && cd build_unix
cmake ../
cd ..
cmake --build build_unix --config Release

