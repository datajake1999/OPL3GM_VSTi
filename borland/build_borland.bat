@echo off
echo Starting build process.
echo Setting environment variables.
set path=C:\BCC55\Bin;%path%
set include=C:\BCC55\Include
set lib=C:\BCC55\Lib
echo Compiling source files.
bcc32 -q -c -I..\pstdint;..\VST2_SDK -w-8004 -w-8057 ..\fmopl3lib\*.cpp ..\src\*.cpp ..\synthlib_apogee\*.cpp ..\synthlib_doom\*.cpp ..\synthlib_w9x\*.cpp ..\VST2_SDK\public.sdk\source\vst2.x\*.cpp
echo Compiling resource script.
brcc32 -fo OPL3GM.res ..\src\OPL3GM.rc
echo Linking DLLs.
ilink32 -q /Tpd c0d32.obj opl.obj opl3.obj opl3class.obj main.obj midiprog.obj OPL3GM.obj OPL3GM_proc.obj al_midi.obj audioeffect.obj audioeffectx.obj vstplugmain.obj,OPL3Apogee.dll,,cw32mt.lib import32.lib,vstplug.def,OPL3GM.res
ilink32 -q /Tpd c0d32.obj opl.obj opl3.obj opl3class.obj main.obj midiprog.obj OPL3GM.obj OPL3GM_proc.obj i_oplmusic.obj audioeffect.obj audioeffectx.obj vstplugmain.obj,OPL3Doom.dll,,cw32mt.lib import32.lib,vstplug.def,OPL3GM.res
ilink32 -q /Tpd c0d32.obj opl.obj opl3.obj opl3class.obj main.obj midiprog.obj OPL3GM.obj OPL3GM_proc.obj opl3midi.obj audioeffect.obj audioeffectx.obj vstplugmain.obj,OPL3W9x.dll,,cw32mt.lib import32.lib,vstplug.def,OPL3GM.res
echo Cleaning up intermediate build files.
del *.ilc *.ild *.ilf *.ils *.map *.obj *.res *.tds
echo Build finished. Press any key to continue.
pause > nul
