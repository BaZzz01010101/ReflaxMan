@echo *****************************************************************************************
g++ -funsafe-math-optimizations -std=c++0x -Wno-multichar -Ofast -o build\ReflaxMan_gcc.exe windows\Main.cpp airly\*.cpp lib\libgdi32.a
build\ReflaxMan_gcc.exe