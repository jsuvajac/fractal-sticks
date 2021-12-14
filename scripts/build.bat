@echo off
if not exist build mkdir build
pushd build
if not exist SDL2.dll xcopy ..\lib\SDL\lib\win64\SDL2.dll
cl -nologo -EHsc -Z7 -FC -MP ^
    ..\lsystem.cpp ..\lib\GLAD\src\glad.c^
	-Fe:lsystem.exe^
    -I ../lib/SDL/include -I ../lib/GLAD/include -I ../include^
    Shell32.lib SDL2.lib^
    -link -LIBPATH:../lib/SDL/lib/win64 -SUBSYSTEM:CONSOLE
popd
