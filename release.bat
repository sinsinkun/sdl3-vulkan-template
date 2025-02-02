@REM Compile standalone exe with SDL3.dll dependency
g++ -O2 -o build\release.exe -g **.cpp src\**.cpp -std=c++17 ^
-IC:\Programs\SDL3\include -LC:\Programs\SDL3\lib ^
-IC:/Programs/freetype-2.13.3/include -LC:/Programs/freetype-2.13.3/win32 ^
-DPLATFORM_DESKTOP -lsdl3 -lfreetype -static -lgdi32 -lwinmm -mwindows