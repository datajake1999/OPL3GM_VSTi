# Introduction
This is a collection of VST instruments that emulate MIDI drivers for OPL3 based sound cards. The synthesizer code is based on [the Windows OPL3 drivers from this thread](https://www.vogons.org/viewtopic.php?t=36667)

# Included Synths

* Apogee OPL3: Based on Apogee Sound System source code.
* Doom OPL3: Based on Chocolate Doom source code.
* Windows 9x OPL3: Based on Windows 9x Sound Blaster 16 driver reverse engineering.

# Installation and Usage Notes
Note: Apogee and Doom instrument bank files can be loaded from the plug-in's user interface. The following sections describe how to install a default instrument bank.

## Included Apogee Patches

* built in patch - ROTT 1.3, Build engine games setup utility.
* apogee_blood.tmb - Blood
* apogee_default.tmb - Copy of built in patch
* apogee_duke3d.tmb - Duke Nukem 3D
* apogee_lee.tmb - ROTT 1.0-1.2
* apogee_nam.tmb - NAM
* apogee_sw.tmb - Shadow Warrior(Shareware)

### Default Apogee Patch Installation

1. Create OPLSynth folder in C:\ disk root.
2. Copy *.tmb files to C:\OPLSynth folder.
3. Copy needed tmb file as apogee.tmb
4. If you want to use built in patch remove apogee.tmb

## Included Doom Patches

* built in patch - DMX, Heretic, Hexen, Doom Beta, Strife Demo
* dmx_dmx.op2 - Copy of built in patch
* dmx_doom1.op2 - Doom, Ultimate Doom, Chex Quest
* dmx_doom2.op2 - Doom II, Final Doom
* dmx_strife.op2 - Strife
* dmx_raptor.op2 - Raptor
* dmxopl.op2 - DMXOPL3

### Default Doom Patch Installation

1. Create OPLSynth folder in C:\ disk root.
2. Copy *.op2 files to C:\OPLSynth folder.
3. Rename needed op2 file to genmidi.op2.
4. If you want to use built in patch remove genmidi.op2

## DMXOPTION Toggles

* OPL3 mode enable: Set DMXOPTION environment variable to "-opl3" 
* Doom 1 1.666 mode enable: Set DMXOPTION environment variable to "-doom1" 
* Doom 2 1.666, Heretic, Hexen mode enable: Set DMXOPTION environment variable to "-doom2" 

You also can combine OPL3 mode with Doom 1.666 modes. eg set variable to "-opl3-doom1"

# Known Bugs
The original MIDI drivers that this project replicates have known issues. For historical accuracy, these are replicated and will not be addressed.

* When the Apogee driver receives a command to reset all controllers, the MIDI programs on all channels are also reset.
* The stereo channels in the Doom driver are reversed when in OPL3 mode.
* The Windows 9x driver has a pitch bend underflow bug where the lowest pitch bend value wraps to the highest possible value.

# Automatable Parameters

* Volume: Synth master volume.
* VolumeDisplay: Sets the unit for displaying the aforementioned Volume parameter, either dB or %.
* DCBlock: Filters out DC offset from the final output.
* NoiseGate: Gates low level noise from the final output.
* Transpose: Applies an offset to incoming MIDI notes.
* Emulator: Selects the OPL3 emulator that will be used. Nuked is more accurate, while DOSBox runs faster.
* HQResample: Internally Runs OPL3 emulation at the native OPL3 sample rate (49716 hZ), then resamples to the host sample rate using a decent algorithm.
* PushMidi: Queue's MIDI events instead of processing them immediately. Queued events have sample accurate timing, while immediate events can have jittery playback with large audio buffers.

# What's with the GUI?
The user interfaces of most audio plug-ins draw graphics directly to the screen. The problem with this approach is that these controls aren't seen by assistive technology such as screen readers, which are designed to work with native operating system controls. This project aims to change that. The UI has been programmed from the ground up using raw Win32 with mostly standard controls, the only exception being the virtual MIDI keyboard.

## Guide to the GUI

### Settings

* Preset: sets the currently active preset. Even though this instrument only has a single preset, a framework has been set in place for working with multiple presets.
* Preset Name: sets the name of the currently active preset.
* The next set of options correspond to the previously mentioned automatable parameters, so they will be omitted here. It is worth noting that parameters such as Volume and Transpose have a static text label to the right of the control that shows the current value.
* Bypass: Stops processing audio and MIDI data. It is worth noting that this plug-in acts as a pass through device, as it takes audio in and mixes it with the synth output, as well as outputs MIDI back to the host. This is still the case when Bypass is enabled.
* OPL3 Sample Rate: sets the sample rate of the OPL3 emulation. This option is only available when High Quality Resampling is enabled.

### Misc

* Refresh: refreshes the user interface with the plug-in’s current settings and state.
* Load Instrument Bank: loads an instrument bank file into the synth. This functionality is disabled in the Windows 9x synth, as it only has a built-in instrument bank. The current bank is shown to the right of this control.
* Panic Synth: stops all notes immediately.
* Reset Synth: stops all notes immediately and resets MIDI parameters. The number of currently active voices is shown to the right of this control.
* About: displays information about the VST plug-in, such as credits.
* Plug-in Statistics: shows information including the host sample rate and block size, as well as the number of active plug-in instances. The audio output level is shown to the right of this control.
* Host Info: displays information about the host the plug-in is running in.
* Hard Reset: Fully reinitializes the internal synthesis engine. The CPU load is shown to the right of this control.
* Forget Settings: resets all settings to there default values.
* Freeze Meters: This causes the active voice count, audio output level, and CPU load to stay at there last known values. When the user interface is refreshed by the user, these meters are updated regardless of this setting.
* Hide Parameter Display: hides the display of parameter values such as Volume and Transpose. The virtual MIDI keyboard is below this control.
* Channel Mixer: brings up a dialog where MIDI channels can be enabled or disabled. When a channel is disabled from this dialog, all notes are stopped and various controllers are reset on the relevant channel. This dialog also has All and None buttons for quickly enabling or disabling all channels, and there is a Close button to go back to the main window.
* Open Project Page: opens the project page in the user's default web browser. The user is asked if they are connected to the internet before attempting to load the browser.

## Virtual MIDI Keyboard Cheat Sheet

* Number Row: Select octave.
* Top Row: Sharp notes.
* Home Row: Flat notes.
* Bottom Row: Select velocity.
* Shift: Sustain.
* Up Arrow: Increase velocity.
* Down Arrow: Decrease velocity.
* Right Arrow: Increase octave.
* Left Arrow: Decrease octave.
* Page Down: Move to next instrument.
* Page Up: Move to previous instrument.
* End: Increase active channel.
* Home: Decrease active channel.
* =: Increase bend MSB.
* -: Decrease bend MSB.
* \: Increase bend LSB.
* `: Decrease bend LSB.
* Backspace: Reset pitch bend and program to current keyboard state.
* Space: Reset keyboard state.

# Extra Notes

* This is only a VST2 compatible plug-in. A VST3 version is not planned for various reasons.

# Building from Source
In order to build OPL3GM you will need

* Microsoft Visual Studio 2005
* Windows Server 2003 Platform SDK
* InnoSetup 5.4.3
* 7-Zip (any version)

After installing, just run build.bat. You may need to change the paths defined in this script to correspond with your installation of the build tools.

# License
    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

# Credits

* Nukeykt, for writing the Nuked OPL3 emulator and the MIDI drivers that this project is based on.
* Wohlstand, for applying various updates to the aforementioned MIDI drivers.
* The DOSBox Team, for providing a faster alternative to the Nuked OPL3 emulator.
* Kode54, for providing a high quality resampler.
* Me (Datajake), for writing the VST interface for this synth.
* Developers in the 90s, for creating the original MIDI drivers that this project replicates, as well as the included instrument banks.
* Sneakernets, for creating the DMXOPL patch set.
* Old blogs and forums, for tips on VST development.
* Charles Petzold, for writing Programming Windows, Fifth Edition.
* Veli-Pekka Tätilä, for writing a set of recommended accessibility practices to keep in mind when designing VST user interfaces.
