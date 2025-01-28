@REM Compile standalone exe with SDL3.dll dependency
g++ -O2 -o build\release.exe -g **.cpp -std=c++17 -Iexternal -DPLATFORM_DESKTOP -static -lsdl3 -lgdi32 -lwinmm -mwindows