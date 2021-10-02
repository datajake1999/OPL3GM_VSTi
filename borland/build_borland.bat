@echo off
set path=C:\BCC55\Bin;%path%
set include=C:\BCC55\Include
set lib=C:\BCC55\Lib
bcc32 -c -I..\pstdint;..\VST2_SDK ..\fmopl3lib\*.cpp ..\src\*.cpp ..\synthlib_apogee\*.cpp ..\synthlib_doom\*.cpp ..\synthlib_w9x\*.cpp ..\VST2_SDK\public.sdk\source\vst2.x\*.cpp
brcc32 ..\src\OPL3GM.rc
move ..\src\OPL3GM.RES
ilink32 /Tpd c0d32.obj opl.obj opl3.obj opl3class.obj main.obj midiprog.obj OPL3GM.obj OPL3GM_proc.obj al_midi.obj audioeffect.obj audioeffectx.obj vstplugmain.obj,OPL3Apogee.dll,,cw32mt.lib import32.lib,vstplug.def,OPL3GM.RES
ilink32 /Tpd c0d32.obj opl.obj opl3.obj opl3class.obj main.obj midiprog.obj OPL3GM.obj OPL3GM_proc.obj i_oplmusic.obj audioeffect.obj audioeffectx.obj vstplugmain.obj,OPL3Doom.dll,,cw32mt.lib import32.lib,vstplug.def,OPL3GM.RES
ilink32 /Tpd c0d32.obj opl.obj opl3.obj opl3class.obj main.obj midiprog.obj OPL3GM.obj OPL3GM_proc.obj opl3midi.obj audioeffect.obj audioeffectx.obj vstplugmain.obj,OPL3W9x.dll,,cw32mt.lib import32.lib,vstplug.def,OPL3GM.RES
del *.ilc *.ild *.ilf *.ils *.map *.obj *.res *.tds
pause > nul
