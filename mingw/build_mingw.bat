@echo off
g++ -c -I..\VST2_SDK ..\fmopl3lib\*.cpp ..\src\*.cpp ..\synthlib_apogee\*.cpp ..\synthlib_doom\*.cpp ..\synthlib_w9x\*.cpp ..\VST2_SDK\public.sdk\source\vst2.x\*.cpp
windres ..\src\OPL3GM.rc resource.o
g++ opl.o opl3.o opl3class.o main.o midiprog.o OPL3GM.o OPL3GM_proc.o resampler.o al_midi.o audioeffect.o audioeffectx.o vstplugmain.o resource.o -s -static -shared -o OPL3Apogee.dll
g++ opl.o opl3.o opl3class.o main.o midiprog.o OPL3GM.o OPL3GM_proc.o resampler.o i_oplmusic.o audioeffect.o audioeffectx.o vstplugmain.o resource.o -s -static -shared -o OPL3Doom.dll
g++ opl.o opl3.o opl3class.o main.o midiprog.o OPL3GM.o OPL3GM_proc.o resampler.o opl3midi.o audioeffect.o audioeffectx.o vstplugmain.o resource.o -s -static -shared -o OPL3W9x.dll
del *.o
pause > nul
