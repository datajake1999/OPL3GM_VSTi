@echo off
echo Starting build process.
echo Setting environment variables.
set path=C:\BCC55\Bin;%path%
set include=C:\BCC55\Include
set lib=C:\BCC55\Lib
echo Compiling source files.
bcc32 -q -c -DVST_FORCE_DEPRECATED=0 -DGUI -D_WIN32_WINNT=0x0400 -I..\include\pstdint;..\VST2_SDK -O2 -w-8004 -w-8057 @src.lst
echo Compiling resource script.
brcc32 -fo OPL3GM.res ..\res\OPL3GM.rc
echo Linking DLLs.
ilink32 -q /Tpd c0d32.obj DCFilter.obj resampler.obj opl.obj opl3.obj opl3class.obj CPU.obj editor.obj guihelp.obj lock.obj main.obj midiprog.obj OPL3GM.obj OPL3GM_proc.obj queue.obj reaper.obj al_midi.obj audioeffect.obj audioeffectx.obj vstplugmain.obj,OPL3Apogee.dll,,cw32mt.lib import32.lib,..\res\borland.def,OPL3GM.res
ilink32 -q /Tpd c0d32.obj DCFilter.obj resampler.obj opl.obj opl3.obj opl3class.obj CPU.obj editor.obj guihelp.obj lock.obj main.obj midiprog.obj OPL3GM.obj OPL3GM_proc.obj queue.obj reaper.obj i_oplmusic.obj audioeffect.obj audioeffectx.obj vstplugmain.obj,OPL3Doom.dll,,cw32mt.lib import32.lib,..\res\borland.def,OPL3GM.res
ilink32 -q /Tpd c0d32.obj DCFilter.obj resampler.obj opl.obj opl3.obj opl3class.obj CPU.obj editor.obj guihelp.obj lock.obj main.obj midiprog.obj OPL3GM.obj OPL3GM_proc.obj queue.obj reaper.obj opl3midi.obj audioeffect.obj audioeffectx.obj vstplugmain.obj,OPL3W9x.dll,,cw32mt.lib import32.lib,..\res\borland.def,OPL3GM.res
echo Cleaning up intermediate build files.
del *.ilc *.ild *.ilf *.ils *.map *.obj *.res *.tds
echo Build finished. Press any key to continue.
pause > nul
