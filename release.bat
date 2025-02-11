@REM Compile standalone exe with SDL3.dll dependency
g++ -O2 -o build\release.exe -g **.cpp src\**.cpp -std=c++17 ^
-IC:\Programs\SDL3\include -LC:\Programs\SDL3\lib ^
-DPLATFORM_DESKTOP -lsdl3 -lsdl3_ttf -lsdl3_image -static -lgdi32 -lwinmm -mwindows
