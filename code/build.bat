@echo off

set CommonCompilerFlags= -MT -nologo -Gm- -GR- -EHa- -Oi -WX -W4 -wd4505 -wd4201 -wd4100 -wd4189 -DSLOW=1 -FC -Z7 -Fmwin32_handmade.map

set CommonLinkerFlags= -opt:ref user32.lib Gdi32.lib Winmm.lib

IF NOT EXIST ..\..\build mkdir ..\..\build
pushd ..\..\build

cl %CommonCompilerFlags% w:\graphics\code\win32_graphics.cpp /link %CommonLinkerFlags%

popd