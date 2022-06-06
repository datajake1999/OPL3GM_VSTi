@echo off
g++ -c -DVST_FORCE_DEPRECATED=0 -Dgui -I..\VST2_SDK ..\dsp\*.cpp ..\fmopl3lib\*.cpp ..\src\*.cpp ..\src\editor\*.cpp ..\synthlib_apogee\*.cpp ..\synthlib_doom\*.cpp ..\synthlib_w9x\*.cpp ..\VST2_SDK\public.sdk\source\vst2.x\*.cpp
windres ..\res\OPL3GM.rc resource.o
g++ DCFilter.o resampler.o opl.o opl3.o opl3class.o main.o midiprog.o OPL3GM.o OPL3GM_proc.o queue.o reaper.o editor.o al_midi.o audioeffect.o audioeffectx.o vstplugmain.o resource.o -s -static -shared -lcomctl32 -lkernel32 -lshell32 -luser32 -o OPL3Apogee.dll
g++ DCFilter.o resampler.o opl.o opl3.o opl3class.o main.o midiprog.o OPL3GM.o OPL3GM_proc.o queue.o reaper.o editor.o i_oplmusic.o audioeffect.o audioeffectx.o vstplugmain.o resource.o -s -static -shared -lcomctl32 -lkernel32 -lshell32 -luser32 -o OPL3Doom.dll
g++ DCFilter.o resampler.o opl.o opl3.o opl3class.o main.o midiprog.o OPL3GM.o OPL3GM_proc.o queue.o reaper.o editor.o opl3midi.o audioeffect.o audioeffectx.o vstplugmain.o resource.o -s -static -shared -lcomctl32 -lkernel32 -lshell32 -luser32 -o OPL3W9x.dll
del *.o
pause > nul
