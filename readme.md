# Introduction
This is a collection of VST instruments that emulate MIDI drivers for OPL3 based sound cards. The synthesizer code is based on [the Windows OPL3 drivers from this thread](https://www.vogons.org/viewtopic.php?t=36667)

# Included synths

* Apogee OPL3: Based on Apogee Sound System source code.
* Doom OPL3: Based on Chocolate Doom source code.
* Windows 9x OPL3: Based on Windows 9x Sound Blaster 16 driver reverse engineering.

# Installation and usage notes

## Included Apogee patches

* built in patch - ROTT 1.3, Build engine games setup utility.
* apogee_blood.tmb - Blood
* apogee_default.tmb - Copy of built in patch
* apogee_duke3d.tmb - Duke Nukem 3D
* apogee_lee.tmb - ROTT 1.0-1.2
* apogee_nam.tmb - NAM
* apogee_sw.tmb - Shadow Warrior(Shareware)

### Default Apogee patch installation

1. Create OPLSynth folder in C:\ disk root.
2. Copy *.tmb files to C:\OPLSynth folder.
3. Copy needed tmb file as apogee.tmb
4. If you want to use built in patch remove apogee.tmb

## Included Doom patches

* built in patch - DMX, Heretic, Hexen, Doom Beta, Strife Demo
* dmx_dmx.op2 - Copy of built in patch
* dmx_doom1.op2 - Doom, Ultimate Doom, Chex Quest
* dmx_doom2.op2 - Doom II, Final Doom
* dmx_strife.op2 - Strife
* dmx_raptor.op2 - Raptor
* dmxopl.op2 - DMXOPL3

### Default Doom patch installation

1. Create OPLSynth folder in C:\ disk root.
2. Copy *.op2 files to C:\OPLSynth folder.
3. Rename needed op2 file to genmidi.op2.
4. If you want to use built in patch remove genmidi.op2

## DMXOPTION toggles

* OPL3 mode enable: Set DMXOPTION environment variable to "-opl3" 
* Doom 1 1.666 mode enable: Set DMXOPTION environment variable to "-doom1" 
* Doom 2 1.666, Heretic, Hexen mode enable: Set DMXOPTION environment variable to "-doom2" 

You also can combine OPL3 mode with Doom 1.666 modes. eg set variable to "-opl3-doom1"

# Automatable parameters

* Volume: Synth master volume.
* VolumeDisplay: Sets the unit for displaying the aforementioned Volume parameter, either dB or %.
* DCBlock: Filters out DC offset from the final output.
* NoiseGate: Gates low level noise from the final output.
* Transpose: Applies an offset to Incoming MIDI notes.
* Emulator: Selects the OPL3 emulator that will be used. Nuked Is more accurate, while DOSBox runs faster.
* HQResample: Internally Runs OPL3 emulation at the native OPL3 sample rate (49716 hZ), then resamples to the host sample rate using a decent algorithm.
* PushMidi: Queue's MIDI events Instead of processing them Immediately. Queued events have sample accurate timing, while immediate events can have jittery playback with large audio buffers.

# What's with the GUI?

The User Interfaces of most audio plugins draw graphics directly to the screen. The problem with this approach Is that these controls aren't seen by assistive technology such as screen readers, which are designed to work with native Operating System controls. This project aims to change that. The UI has been programmed from the ground up using raw Win32 with mostly standard controls, The only exception being the Virtual MIDI Keyboard.

## Guide to the GUI

### Settings

* Preset: sets the currently active preset. Even though this instrument only has a single preset, a framework has been set in place for working with multiple presets.
* Preset Name: sets the name of the currently active preset.
* The next set of options correspond to the previously mentioned automatable parameters, so they will be omitted here. It is worth noting that parameters such as Volume and Transpose have a static text label to the right of the control that shows the current value.
* Bypass: Stops processing audio and MIDI data. It is worth noting that this plugin acts as a pass through device, as it takes audio in and mixes it with the synth output, as well as outputs MIDI back to the host. This is still the case when Bypass is enabled.
* OPL3 Sample Rate: sets the sample rate of the OPL3 emulation. This option is only available when High Quality Resampling is enabled.

### Misc

* Refresh: refreshes the user interface with the plugin’s current settings and state.
* Load Instrument Bank: loads an instrument bank file into the synth. This functionality is disabled in the Windows 9x synth, as it only has a built-in instrument bank. The current bank is shown to the right of this control.
* Panic Synth: stops all notes immediately.
* Reset Synth: stops all notes immediately and resets MIDI parameters. The number of currently active voices is shown to the right of this control.
* About: displays information about the VST plugin, such as credits.
* Plugin statistics: shows information including the host sample rate and block size, as well as the number of active plugin instances. The audio output level is shown to the right of this control.
* Host Info: displays information about the host the plugin is running in.
* Hard Reset: Fully reinitializes the internal synthesis engine. The CPU load is shown to the right of this control.
* Forget Settings: resets all settings to there default values.
* Freeze Meters: This causes the active voice count, audio output level, and CPU load to stay at there last known values. When the user interface is refreshed by the user, these meters are updated regardless of this setting.
* Hide Parameter Display: hides the display of parameter values such as Volume and Transpose. The virtual MIDI keyboard is below this control.
* Channel Mixer: brings up a dialog where MIDI channels can be enabled or disabled. When a channel is disabled from this dialog, all notes are stopped and various controllers are reset on the relevant channel. This dialog also has All and None buttons for quickly enabling or disabling all channels, and there is a Close button to go back to the main window.
* Open Project Page: opens the project page in the user's default web browser. The user is asked if they are connected to the internet before attempting to load the browser.

## Virtual MIDI Keyboard cheat sheet

* Number row: Select octave.
* Top row: Sharp notes.
* Home row: Flat notes.
* Bottom row: Select velocity.
* Shift: Sustain.
* Up arrow: Increase velocity.
* Down arrow: Decrease velocity.
* Right arrow: Increase octave.
* Left arrow: Decrease octave.
* Page down: Move to next Instrument.
* Page up: Move to previous Instrument.
* End: Increase active channel.
* Home: Decrease active channel.
* =: Increase bend MSB.
* -: Decrease bend MSB.
* \: Increase bend LSB.
* `: Decrease bend LSB.
* Backspace: Reset pitch bend and program to current keyboard state.
* Space: Reset keyboard state.

# Extra notes

* This is only a VST2 compatible plug-in. A VST3 version Is not planned for various reasons.

# Building from source
In order to build OPL3GM you will need

* Microsoft Visual Studio 2005
* Windows Server 2003 Platform SDK
* InnoSetup 5.4.3
* 7-Zip

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
