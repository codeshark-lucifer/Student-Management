@echo off
REM Create build folder if not exists
if not exist build mkdir build
cd build

REM Run cmake with MinGW Makefiles
cmake -G "MinGW Makefiles" ..
mingw32-make
cd ..
pause
