@echo off

if not defined VCVAR_INIT (
    call "C:\Program Files\Microsoft Visual Studio\2022\Community\VC\Auxiliary\Build\vcvars64.bat"
    set VCVAR_INIT=1
)

if not exist build mkdir build
pushd build
del * /Q

cl /W2 /WX ..\anti_debug_game.cpp

popd

