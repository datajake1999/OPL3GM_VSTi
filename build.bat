@ECHO OFF
set path="C:\Program Files\7-Zip";"C:\Program Files\Inno Setup 5"
call "c:\program files\microsoft visual studio 8\vc\bin\vcvars32.bat"
call "C:\program files\Microsoft Platform SDK\SetEnv.Cmd" /SRV32 /RETAIL
cl -c /I ".\msinttypes" /I ".\VST2_SDK" fmopl3lib\*.cpp src\*.cpp synthlib_w9x\*.cpp VST2_SDK\public.sdk\source\vst2.x\*.cpp
rc src\OPL3GM.rc
link audioeffect.obj audioeffectx.obj vstplugmain.obj main.obj OPL3GM.obj OPL3GM_proc.obj opl3.obj opl3class.obj opl3midi.obj src\OPL3GM.res bufferoverflowU.lib kernel32.lib user32.lib /dll /def:".\src\vstplug.def" /OUT:OPL3W9X.dll
del *.obj
call "C:\program files\Microsoft Platform SDK\SetEnv.Cmd" /X64 /RETAIL
cl -c /I ".\msinttypes" /I ".\VST2_SDK" fmopl3lib\*.cpp src\*.cpp synthlib_w9x\*.cpp VST2_SDK\public.sdk\source\vst2.x\*.cpp
link audioeffect.obj audioeffectx.obj vstplugmain.obj main.obj OPL3GM.obj OPL3GM_proc.obj opl3.obj opl3class.obj opl3midi.obj src\OPL3GM.res bufferoverflowU.lib kernel32.lib user32.lib /dll /def:".\src\vstplug.def" /OUT:OPL3W9X_X64.dll
del *.obj
del src\OPL3GM.res
iscc "install.iss"
7z a -tzip output\OPL3GM.zip OPL3W9X.dll OPL3W9X_X64.dll license.txt readme.md
