/*
OPL3GM VSTi
Copyright (C) 2021-2025  Datajake

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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
//#define DEMO
#ifdef DEMO
#include <time.h>
#endif
#include "../interface.h"
#include "../dsp/DCFilter.h"
#include "../dsp/resampler.h"
#include <public.sdk/source/vst2.x/audioeffectx.h>
#include "queue.h"
#define REAPER_EXTENSIONS 1
#if REAPER_EXTENSIONS
#include "parmstruct.h"
#endif
#include "lock.h"
#include "CPU.h"

enum
{
	// Global
	kNumPrograms = 1,

	// Parameters Tags
	kVolume = 0,
	kVolumeDisplay,
	kDCBlock,
	kNoiseGate,
	kTranspose,
	kEmulator,
	kHQResample,
	kPushMidi,

	kNumParams
};

struct HostInfo
{
	char VendorString[kVstMaxVendorStrLen];
	char ProductString[kVstMaxProductStrLen];
	VstInt32 VendorVersion;
	VstInt32 MasterVersion;
	bool ReceiveEvents;
};

struct OPL3GMChunk
{
	VstInt32 Size;
	char ProgramName[kVstMaxProgNameLen];
	float Parameters[kNumParams];
	bool bypassed;
	VstInt32 lastRate;
	char BankFile[256];
	char BankName[256];
	bool ChannelEnabled[16];
	bool FreezeMeters;
	bool HideParameters;
};

class OPL3GM : public AudioEffectX
{
public:
	OPL3GM (audioMasterCallback audioMaster);
	~OPL3GM ();
	virtual void open ();
	virtual void close ();
	virtual void setParameter (VstInt32 index, float value);
	virtual float getParameter (VstInt32 index);
	virtual void getParameterDisplay (VstInt32 index, char* text);
	virtual void getParameterLabel (VstInt32 index, char* label);
	virtual void getParameterName (VstInt32 index, char* name);
	virtual VstInt32 setChunk (void* data, VstInt32 byteSize, bool isPreset);
	virtual VstInt32 getChunk (void** data, bool isPreset);
	virtual void setProgram (VstInt32 program);
	virtual VstInt32 getProgram ();
	virtual void setProgramName (char* name);
	virtual void getProgramName (char* name);
	virtual bool string2parameter (VstInt32 index, char* text);
	virtual bool getParameterProperties (VstInt32 index, VstParameterProperties* p);
	virtual bool getProgramNameIndexed (VstInt32 category, VstInt32 index, char* text);
	virtual bool getInputProperties (VstInt32 index, VstPinProperties* properties);
	virtual bool getOutputProperties (VstInt32 index, VstPinProperties* properties);
	virtual bool setBypass (bool onOff);
	virtual bool getEffectName (char* name);
	virtual bool getVendorString (char* text);
	virtual bool getProductString (char* text);
	virtual VstInt32 getVendorVersion ();
	virtual VstPlugCategory getPlugCategory ();
	virtual VstIntPtr vendorSpecific (VstInt32 lArg, VstIntPtr lArg2, void* ptrArg, float floatArg);
	virtual VstInt32 canDo (char* text);
#if VST_2_4_EXTENSIONS
	virtual VstInt32 getNumMidiInputChannels ();
	virtual VstInt32 getNumMidiOutputChannels ();
#endif
	virtual void setSampleRate (float sampleRate);
	virtual void setBlockSize (VstInt32 blockSize);
	virtual void setBlockSizeAndSampleRate (VstInt32 blockSize, float sampleRate);
	virtual bool getErrorText (char* text);
	virtual void suspend ();
	virtual void resume ();
	virtual float getVu ();
#if !VST_FORCE_DEPRECATED
	virtual void process (float** inputs, float** outputs, VstInt32 sampleFrames);
#endif
	virtual void processReplacing (float** inputs, float** outputs, VstInt32 sampleFrames);
#if VST_2_4_EXTENSIONS
	virtual void processDoubleReplacing (double** inputs, double** outputs, VstInt32 sampleFrames);
#endif
	virtual VstInt32 processEvents (VstEvents* events);
	virtual VstInt32 startProcess ();
	virtual VstInt32 stopProcess ();
	virtual VstInt32 getMidiProgramName (VstInt32 channel, MidiProgramName* midiProgramName);
	virtual VstInt32 getCurrentMidiProgram (VstInt32 channel, MidiProgramName* currentProgram);
	virtual VstInt32 getMidiProgramCategory (VstInt32 channel, MidiProgramCategory* category);
	virtual bool hasMidiProgramsChanged (VstInt32 channel);
	virtual bool getMidiKeyName (VstInt32 channel, MidiKeyName* keyName);
	virtual void initializeSettings (bool resetSynth);
	virtual void setInternalRate (VstInt32 rate);
	virtual VstInt32 getInternalRate ();
	virtual bool loadInstruments (char* filename, char* display);
	virtual void enableChannel (VstInt32 channel, bool enable);
	virtual bool isChannelEnabled (VstInt32 channel);
	virtual void setFreezeMeters (bool value);
	virtual bool getFreezeMeters ();
	virtual void setHideParameters (bool value);
	virtual bool getHideParameters ();
	virtual void hardReset ();
	virtual VstInt32 getActiveVoices ();
	virtual void getBankName (char* text, VstInt32 size);
	virtual bool getBypass ();
	virtual HostInfo* getHostInfo ();
	virtual double getCPULoad ();
private:
#if REAPER_EXTENSIONS
	bool getParameterDisplayValue (VstInt32 index, char* text, float value);
	bool string2parameterReplace (VstInt32 index, char* text);
	bool isEnumParameter (VstInt32 index);
	bool automateParameter (VstInt32 index, float value, VstInt32 timestamp);
	bool parameterRange (VstInt32 index, double* range);
	bool renamePlug (char** text, const char* newName);
	void adjustParameterIndex (VstInt32 index, VstInt32 adjust);
#endif
	void initSynth ();
	void initBuffer ();
	void clearSynth ();
	void clearBuffer ();
	void changeSynthRate ();
	template <class sampletype>
	void processTemplate (sampletype** inputs, sampletype** outputs, VstInt32 sampleFrames);
	void calculateCPULoad (double begin, double end, VstInt32 numsamples);
	void fillBuffer (short* bufpos, VstInt32 length);
	void processEvent (VstEvent* event);
	void sendMidi (char* data);
	void fillProgram (VstInt32 channel, VstInt32 prg, MidiProgramName* mpn);
	midisynth* synth;
	short* buffer;
	void* resampler;
	short samples[2];
	DCFilter dcf[2];
	EventQueue MidiQueue;
#if REAPER_EXTENSIONS
	EventQueue ParameterQueue;
#endif
	char ProgramName[kVstMaxProgNameLen];
	float Volume;
	float VolumeDisplay;
	float DCBlock;
	float NoiseGate;
	float Transpose;
	float Emulator;
	float HQResample;
	float PushMidi;
	bool bypassed;
	VstInt32 internalRate;
	VstInt32 lastRate;
	char BankFile[256];
	char BankName[256];
	bool ChannelEnabled[16];
	bool FreezeMeters;
	bool HideParameters;
	OPL3GMChunk chunk;
	HostInfo hi;
	LockableObject lock;
	double vu[2];
	double CPULoad;
	VstInt32 channelPrograms[16];
#ifdef DEMO
	time_t startTime;
#endif
};

#endif
