
Ninja:

1. Ensure you have gcc c compiler and ninja installed and set to path, use cmake .. -DCMAKE_CXX_COMPILER=gcc -G "Ninja"
2. cmake --build . -j10

MSVC:

1. cmake .. -G "Visual Studio 16 2019"
2. Use the produced solution file