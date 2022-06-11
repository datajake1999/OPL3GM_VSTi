@ECHO OFF
set path="C:\Program Files\7-Zip";"C:\Program Files\Inno Setup 5"
call "C:\Program Files\Microsoft Visual Studio 8\VC\bin\vcvars32.bat"
call "C:\Program Files\Microsoft Platform SDK\SetEnv.Cmd" /SRV32 /RETAIL
cl /nologo -c /D _CRT_SECURE_NO_DEPRECATE /D VST_FORCE_DEPRECATED=0 /D demo /D gui /I ".\include\msinttypes" /I ".\VST2_SDK" dsp\*.cpp fmopl3lib\*.cpp src\*.cpp src\editor\*.cpp synthlib_apogee\*.cpp synthlib_doom\*.cpp synthlib_w9x\*.cpp VST2_SDK\public.sdk\source\vst2.x\*.cpp
rc res\OPL3GM.rc
link audioeffect.obj audioeffectx.obj vstplugmain.obj main.obj midiprog.obj OPL3GM.obj OPL3GM_proc.obj queue.obj reaper.obj editor.obj DCFilter.obj resampler.obj opl.obj opl3.obj opl3class.obj al_midi.obj res\OPL3GM.res bufferoverflowU.lib comctl32.lib comdlg32.lib kernel32.lib shell32.lib user32.lib /nologo /dll /def:".\res\vstplug.def" /OUT:OPL3Apogee.dll
link audioeffect.obj audioeffectx.obj vstplugmain.obj main.obj midiprog.obj OPL3GM.obj OPL3GM_proc.obj queue.obj reaper.obj editor.obj DCFilter.obj resampler.obj opl.obj opl3.obj opl3class.obj i_oplmusic.obj res\OPL3GM.res bufferoverflowU.lib comctl32.lib comdlg32.lib kernel32.lib shell32.lib user32.lib /nologo /dll /def:".\res\vstplug.def" /OUT:OPL3Doom.dll
link audioeffect.obj audioeffectx.obj vstplugmain.obj main.obj midiprog.obj OPL3GM.obj OPL3GM_proc.obj queue.obj reaper.obj editor.obj DCFilter.obj resampler.obj opl.obj opl3.obj opl3class.obj opl3midi.obj res\OPL3GM.res bufferoverflowU.lib comctl32.lib comdlg32.lib kernel32.lib shell32.lib user32.lib /nologo /dll /def:".\res\vstplug.def" /OUT:OPL3W9x.dll
del *.obj
call "C:\Program Files\Microsoft Platform SDK\SetEnv.Cmd" /X64 /RETAIL
cl /nologo -c /D _CRT_SECURE_NO_DEPRECATE /D VST_FORCE_DEPRECATED=0 /D demo /D gui /I ".\include\msinttypes" /I ".\VST2_SDK" dsp\*.cpp fmopl3lib\*.cpp src\*.cpp src\editor\*.cpp synthlib_apogee\*.cpp synthlib_doom\*.cpp synthlib_w9x\*.cpp VST2_SDK\public.sdk\source\vst2.x\*.cpp
link audioeffect.obj audioeffectx.obj vstplugmain.obj main.obj midiprog.obj OPL3GM.obj OPL3GM_proc.obj queue.obj reaper.obj editor.obj DCFilter.obj resampler.obj opl.obj opl3.obj opl3class.obj al_midi.obj res\OPL3GM.res bufferoverflowU.lib comctl32.lib comdlg32.lib kernel32.lib shell32.lib user32.lib /nologo /dll /def:".\res\vstplug.def" /OUT:OPL3Apogee_X64.dll
link audioeffect.obj audioeffectx.obj vstplugmain.obj main.obj midiprog.obj OPL3GM.obj OPL3GM_proc.obj queue.obj reaper.obj editor.obj DCFilter.obj resampler.obj opl.obj opl3.obj opl3class.obj i_oplmusic.obj res\OPL3GM.res bufferoverflowU.lib comctl32.lib comdlg32.lib kernel32.lib shell32.lib user32.lib /nologo /dll /def:".\res\vstplug.def" /OUT:OPL3Doom_X64.dll
link audioeffect.obj audioeffectx.obj vstplugmain.obj main.obj midiprog.obj OPL3GM.obj OPL3GM_proc.obj queue.obj reaper.obj editor.obj DCFilter.obj resampler.obj opl.obj opl3.obj opl3class.obj opl3midi.obj res\OPL3GM.res bufferoverflowU.lib comctl32.lib comdlg32.lib kernel32.lib shell32.lib user32.lib /nologo /dll /def:".\res\vstplug.def" /OUT:OPL3W9x_X64.dll
del *.obj
call "C:\Program Files\Microsoft Platform SDK\SetEnv.Cmd" /SRV64 /RETAIL
cl /nologo -c /D _CRT_SECURE_NO_DEPRECATE /D VST_FORCE_DEPRECATED=0 /D demo /D gui /I ".\include\msinttypes" /I ".\VST2_SDK" dsp\*.cpp fmopl3lib\*.cpp src\*.cpp src\editor\*.cpp synthlib_apogee\*.cpp synthlib_doom\*.cpp synthlib_w9x\*.cpp VST2_SDK\public.sdk\source\vst2.x\*.cpp
link audioeffect.obj audioeffectx.obj vstplugmain.obj main.obj midiprog.obj OPL3GM.obj OPL3GM_proc.obj queue.obj reaper.obj editor.obj DCFilter.obj resampler.obj opl.obj opl3.obj opl3class.obj al_midi.obj res\OPL3GM.res bufferoverflowU.lib comctl32.lib comdlg32.lib kernel32.lib shell32.lib user32.lib /nologo /dll /def:".\res\vstplug.def" /OUT:OPL3Apogee_IA64.dll
link audioeffect.obj audioeffectx.obj vstplugmain.obj main.obj midiprog.obj OPL3GM.obj OPL3GM_proc.obj queue.obj reaper.obj editor.obj DCFilter.obj resampler.obj opl.obj opl3.obj opl3class.obj i_oplmusic.obj res\OPL3GM.res bufferoverflowU.lib comctl32.lib comdlg32.lib kernel32.lib shell32.lib user32.lib /nologo /dll /def:".\res\vstplug.def" /OUT:OPL3Doom_IA64.dll
link audioeffect.obj audioeffectx.obj vstplugmain.obj main.obj midiprog.obj OPL3GM.obj OPL3GM_proc.obj queue.obj reaper.obj editor.obj DCFilter.obj resampler.obj opl.obj opl3.obj opl3class.obj opl3midi.obj res\OPL3GM.res bufferoverflowU.lib comctl32.lib comdlg32.lib kernel32.lib shell32.lib user32.lib /nologo /dll /def:".\res\vstplug.def" /OUT:OPL3W9x_IA64.dll
del *.exp *.lib *.obj
del res\OPL3GM.res
iscc /Qp "install.iss"
7z a -tzip output\OPL3GM_X86.zip @zip_X86.lst
7z a -tzip output\OPL3GM_X64.zip @zip_X64.lst
7z a -tzip output\OPL3GM_IA64.zip @zip_IA64.lst
del *.dll
pause > nul
