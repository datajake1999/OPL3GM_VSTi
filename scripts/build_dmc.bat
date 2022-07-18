@echo off
echo Starting build process.
echo Setting environment variables.
set path=C:\DM\Bin;%path%
set include=C:\DM\Include
set lib=C:\DM\Lib
echo Compiling source files.
dmc -c -DVST_FORCE_DEPRECATED=0 -Dgui -I..\VST2_SDK -o @src.lst
echo Compiling resource script.
rcc -32 ..\res\OPL3GM.rc
echo Linking DLLs.
link /exetype:nt DCFilter.obj resampler.obj opl.obj opl3.obj opl3class.obj editor.obj guihelp.obj lock.obj main.obj midiprog.obj OPL3GM.obj OPL3GM_proc.obj queue.obj reaper.obj al_midi.obj audioeffect.obj audioeffectx.obj vstplugmain.obj,OPL3Apogee.dll,,advapi32.lib comctl32.lib comdlg32.lib gdi32.lib kernel32.lib shell32.lib user32.lib,..\res\vstplug.def,OPL3GM.res
link /exetype:nt DCFilter.obj resampler.obj opl.obj opl3.obj opl3class.obj editor.obj guihelp.obj lock.obj main.obj midiprog.obj OPL3GM.obj OPL3GM_proc.obj queue.obj reaper.obj i_oplmusic.obj audioeffect.obj audioeffectx.obj vstplugmain.obj,OPL3Doom.dll,,advapi32.lib comctl32.lib comdlg32.lib gdi32.lib kernel32.lib shell32.lib user32.lib,..\res\vstplug.def,OPL3GM.res
link /exetype:nt DCFilter.obj resampler.obj opl.obj opl3.obj opl3class.obj editor.obj guihelp.obj lock.obj main.obj midiprog.obj OPL3GM.obj OPL3GM_proc.obj queue.obj reaper.obj opl3midi.obj audioeffect.obj audioeffectx.obj vstplugmain.obj,OPL3W9x.dll,,advapi32.lib comctl32.lib comdlg32.lib gdi32.lib kernel32.lib shell32.lib user32.lib,..\res\vstplug.def,OPL3GM.res
echo Cleaning up intermediate build files.
del *.map *.obj *.res
echo Build finished. Press any key to continue.
pause > nul
