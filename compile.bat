@echo off

@REM compile all shader files into SPIRV binary and store in SPIRV folder
for %%i in (shaders\*) do echo Compiling %%i to assets\SPIRV\%%~nxi & ^
C:\Programs\VulkanSDK_1_4_304\Bin\glslc.exe %%i -o assets\SPIRV\%%~nxi.spv
echo Finished