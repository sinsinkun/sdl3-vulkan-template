@REM uses dynamic linking for standard libraries 
@REM as this is only intended to run on developer machines
g++ pack-tool\main.cpp -o build\packer -IC:\Programs\SDL3\include -LC:\Programs\SDL3\lib -lsdl3 -lsdl3_ttf -lsdl3_image
