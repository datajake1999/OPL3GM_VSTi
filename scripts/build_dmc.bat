@echo off
echo Starting build process.
echo Setting environment variables.
set path=C:\DM\Bin;%path%
set include=C:\DM\Include
set lib=C:\DM\Lib
echo Compiling source files.
dmc -c -I..\VST2_SDK @src.lst
echo Compiling resource script.
rcc -32 ..\res\OPL3GM.rc
echo Linking DLLs.
link /exetype:nt DCFilter.obj resampler.obj opl.obj opl3.obj opl3class.obj main.obj midiprog.obj OPL3GM.obj OPL3GM_proc.obj al_midi.obj audioeffect.obj audioeffectx.obj vstplugmain.obj,OPL3Apogee.dll,,kernel32.lib user32.lib,..\res\vstplug.def,OPL3GM.res
link /exetype:nt DCFilter.obj resampler.obj opl.obj opl3.obj opl3class.obj main.obj midiprog.obj OPL3GM.obj OPL3GM_proc.obj i_oplmusic.obj audioeffect.obj audioeffectx.obj vstplugmain.obj,OPL3Doom.dll,,kernel32.lib user32.lib,..\res\vstplug.def,OPL3GM.res
link /exetype:nt DCFilter.obj resampler.obj opl.obj opl3.obj opl3class.obj main.obj midiprog.obj OPL3GM.obj OPL3GM_proc.obj opl3midi.obj audioeffect.obj audioeffectx.obj vstplugmain.obj,OPL3W9x.dll,,kernel32.lib user32.lib,..\res\vstplug.def,OPL3GM.res
echo Cleaning up intermediate build files.
del *.map *.obj *.res
echo Build finished. Press any key to continue.
pause > nul
