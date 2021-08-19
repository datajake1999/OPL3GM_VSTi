@ECHO OFF
call "c:\program files\microsoft visual studio 8\vc\bin\vcvars32.bat"
call "C:\program files\Microsoft Platform SDK\SetEnv.Cmd" /SRV32 /RETAIL
cl -c /I ".\msinttypes" /I ".\VST2_SDK" fmopl3lib\*.cpp src\*.cpp synthlib\*.cpp VST2_SDK\public.sdk\source\vst2.x\*.cpp
rc src\OPL3GM.rc
link *.obj src\OPL3GM.res bufferoverflowU.lib kernel32.lib user32.lib /dll /def:".\src\vstplug.def" /OUT:OPL3GM.dll
del *.obj
call "C:\program files\Microsoft Platform SDK\SetEnv.Cmd" /X64 /RETAIL
cl -c /I ".\msinttypes" /I ".\VST2_SDK" fmopl3lib\*.cpp src\*.cpp synthlib\*.cpp VST2_SDK\public.sdk\source\vst2.x\*.cpp
link *.obj src\OPL3GM.res bufferoverflowU.lib kernel32.lib user32.lib /dll /def:".\src\vstplug.def" /OUT:OPL3GM_X64.dll
del *.obj
del src\OPL3GM.res
