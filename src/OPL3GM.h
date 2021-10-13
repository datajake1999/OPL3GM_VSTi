/*
OPL3GM VSTi
Copyright (C) 2021  Datajake

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License along
with this program; if not, write to the Free Software Foundation, Inc.,
51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
*/

#ifndef OPL3GM_H
#define OPL3GM_H

#include "../interface.h"
#define hqresampler
#ifdef hqresampler
#include "resampler.h"
#endif
#include <public.sdk/source/vst2.x/audioeffectx.h>

enum
{
	// Global
	kNumPrograms = 1,

	// Parameters Tags
	kVolume = 0,
	kVolumeDisplay,
	kTranspose,
	kEmulator,

	kNumParams
};

class OPL3GM : public AudioEffectX
{
public:
	OPL3GM (audioMasterCallback audioMaster);
	~OPL3GM ();
	virtual void setParameter (VstInt32 index, float value);
	virtual float getParameter (VstInt32 index);
	virtual void getParameterDisplay (VstInt32 index, char* text);
	virtual void getParameterLabel (VstInt32 index, char* label);
	virtual void getParameterName (VstInt32 index, char* text);
	virtual void setProgramName (char* name);
	virtual void getProgramName (char* name);
	virtual bool getProgramNameIndexed (VstInt32 category, VstInt32 index, char* text);
	virtual bool getOutputProperties (VstInt32 index, VstPinProperties* properties);
	virtual bool getEffectName (char* name);
	virtual bool getVendorString (char* text);
	virtual bool getProductString (char* text);
	virtual VstInt32 getVendorVersion ();
	virtual VstPlugCategory getPlugCategory ();
	virtual VstInt32 canDo (char* text);
	virtual VstInt32 getNumMidiInputChannels ();
	virtual VstInt32 getNumMidiOutputChannels ();
	virtual void setSampleRate (float sampleRate);
	virtual void setBlockSize (VstInt32 blockSize);
	virtual void resume ();
	virtual void processReplacing (float** inputs, float** outputs, VstInt32 sampleFrames);
	virtual void processDoubleReplacing (double** inputs, double** outputs, VstInt32 sampleFrames);
	virtual VstInt32 processEvents (VstEvents* events);
	virtual VstInt32 getMidiProgramName (VstInt32 channel, MidiProgramName* midiProgramName);
	virtual VstInt32 getCurrentMidiProgram (VstInt32 channel, MidiProgramName* currentProgram);
	virtual VstInt32 getMidiProgramCategory (VstInt32 channel, MidiProgramCategory* category);
	virtual bool hasMidiProgramsChanged (VstInt32 channel);
	virtual bool getMidiKeyName (VstInt32 channel, MidiKeyName* keyName);
private:
	void initSynth (int sampleRate);
	void initBuffer (int blockSize);
	void clearSynth ();
	void clearBuffer ();
	void fillBuffer (int length);
	void sendMidi (char* data);
	void fillProgram (VstInt32 channel, VstInt32 prg, MidiProgramName* mpn);
	midisynth *synth;
#ifdef hqresampler
	void *resampler;
#endif
	short *buffer;
	int bufferSize;
	float Volume;
	float VolumeDisplay;
	float Transpose;
	float Emulator;
	char ProgramName[32];
};

#endif
