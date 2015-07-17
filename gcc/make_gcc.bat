@echo *****************************************************************************************
g++ -funsafe-math-optimizations -std=c++0x -Wno-multichar -Ofast ..\src\windows\*.cpp ..\src\common\*.cpp -o ..\build\ReflaxMan_gcc.exe -lgdi32 
..\build\ReflaxMan_gcc.exe