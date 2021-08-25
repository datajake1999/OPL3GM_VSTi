# Introduction
This is a collection of VST instruments that emulates the way MIDI sounds on an OPL3 based sound card. The synthesizer code is based on [the Windows OPL3 drivers from this thread](https://www.vogons.org/viewtopic.php?t=36667)

#Installation and usage notes

##Included Apogee patches

* built in patch - ROTT 1.3, Build engine games setup utility.
* apogee_blood.tmb - Blood
* apogee_duke3d.tmb - Duke Nukem 3D
* apogee_lee.tmb - ROTT 1.0-1.2
* apogee_nam.tmb - NAM
* apogee_sw.tmb - Shadow Warrior(Shareware)

##Apogee Patch installation

* 1. Create OPLSynth folder in C:\ disk root.
* 2. Copy *.tmb files to C:\OPLSynth folder.
* 3. Copy needed tmb file as apogee.tmb
* 4. If you want to use built in patch remove apogee.tmb

##Included Doom patches

* dmx_dmx.op2 - DMX, Heretic, Hexen, Doom Beta, Strife Demo
* dmx_doom1.op2 - Doom, Ultimate Doom, Chex Quest
* dmx_doom2.op2 - Doom II, Final Doom
* dmx_strife.op2 - Strife
* dmx_raptor.op2 - Raptor

##Doom Patch installation

* 1. Create OPLSynth folder in C:\ disk root.
* 2. Copy *.op2 files to C:\OPLSynth folder.
* 3. Rename needed op2 file to genmidi.op2.

##DMXOPTION toggles

* OPL3 mode enable: Set DMXOPTION environment variable to "-opl3" 
* Doom 1 1.666 mode enable: Set DMXOPTION environment variable to "-doom1" 
* Doom 2 1.666, Heretic, Hexen mode enable: Set DMXOPTION environment variable to "-doom2" 

You also can combine OPL3 mode with Doom 1.666 modes. eg set variable to "-opl3-doom1"
