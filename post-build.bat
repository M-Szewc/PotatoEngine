@ECHO OFF

REM Run from root directory!
if not exist "%cd%\bin\assets\shaders\" mkdir "%cd%\bin\assets\shaders"

ECHO "Compiling shaders..."

ECHO "assets/shaders/Builtin.ObjectShader.vert.glsl -> bin/assets/shader/Builtin.ObjectShader.vert.spv"
CALL %VULKAN_SDK%\Bin\glslc.exe -fshader-stage=vert assets/shaders/Builtin.ObjectShader.vert.glsl -o bin/assets/shaders/Builtin.ObjectShader.vert.spv
IF %ERRORLEVEL% NEQ 0 (echo ERROR:%ERRORLEVEL% && exit)

ECHO "assets/shaders/Builtin.ObjectShader.frag.glsl -> bin/assets/shader/Builtin.ObjectShader.frag.spv"
CALL %VULKAN_SDK%\Bin\glslc.exe -fshader-stage=frag assets/shaders/Builtin.ObjectShader.frag.glsl -o bin/assets/shaders/Builtin.ObjectShader.frag.spv
IF %ERRORLEVEL% NEQ 0 (echo ERROR:%ERRORLEVEL% && exit)

ECHO "Copying assets..."
ECHO xcopy "assets" "bin\assets" /h /i /c /k /e /r /y
xcopy "assets" "bin\assets" /h /i /c /k /e /r /y

ECHO "Done."