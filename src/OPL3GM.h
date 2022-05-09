/*
OPL3GM VSTi
Copyright (C) 2021-2022  Datajake

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
#include "../dsp/DCFilter.h"
#define hqresampler
#ifdef hqresampler
#include "../dsp/resampler.h"
#endif
#include <public.sdk/source/vst2.x/audioeffectx.h>
#include "queue.h"
#define reaper_extensions 1
#if reaper_extensions
#include "parmstruct.h"
#endif
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
//#define demo
#ifdef demo
#include <time.h>
#endif

enum
{
	// Global
	kNumPrograms = 1,

	// Parameters Tags
	kVolume = 0,
	kVolumeDisplay,
	kDCBlock,
	kTranspose,
	kEmulator,
	kPushMidi,

	kNumParams
};

struct HostInfo
{
	char VendorString[kVstMaxVendorStrLen];
	char ProductString[kVstMaxProductStrLen];
	VstInt32 VendorVersion;
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
	virtual void getParameterName (VstInt32 index, char* text);
	virtual VstInt32 setChunk (void* data, VstInt32 byteSize, bool isPreset);
	virtual VstInt32 getChunk (void** data, bool isPreset);
	virtual void setProgram (VstInt32 program);
	virtual VstInt32 getProgram ();
	virtual void setProgramName (char* name);
	virtual void getProgramName (char* name);
	virtual bool canParameterBeAutomated (VstInt32 index);
	virtual bool string2parameter (VstInt32 index, char* text);
	virtual bool getParameterProperties (VstInt32 index, VstParameterProperties* p);
	virtual bool getProgramNameIndexed (VstInt32 category, VstInt32 index, char* text);
	virtual bool beginSetProgram ();
	virtual bool endSetProgram ();
	virtual VstInt32 beginLoadBank (VstPatchChunkInfo* ptr);
	virtual VstInt32 beginLoadProgram (VstPatchChunkInfo* ptr);
	virtual bool getInputProperties (VstInt32 index, VstPinProperties* properties);
	virtual bool getOutputProperties (VstInt32 index, VstPinProperties* properties);
	virtual bool setBypass (bool onOff);
	virtual bool setPanLaw (VstInt32 type, float val);
#if VST_2_4_EXTENSIONS
	virtual bool setProcessPrecision (VstInt32 precision);
#endif
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
#if !VST_FORCE_DEPRECATED
	virtual void setBlockSizeAndSampleRate (VstInt32 blockSize, float sampleRate);
	virtual bool getErrorText (char* text);
#endif
	virtual void suspend ();
	virtual void resume ();
#if !VST_FORCE_DEPRECATED
	virtual float getVu ();
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
private:
#if reaper_extensions
	void getParameterDisplayValue (VstInt32 index, char* text, float value);
	void string2parameterReplace (VstInt32 index, char* text);
	bool isEnumParameter (VstInt32 index);
	bool automateParameter (VstInt32 index, float value, VstInt32 timestamp);
	void parameterRange (VstInt32 index, double *range);
	void adjustParameterIndex (VstInt32 index, VstInt32 adjust);
	void renamePlug (char **text, char *newName);
#endif
	void initSynth (int sampleRate);
	void initBuffer (int blockSize);
	void clearSynth ();
	void clearBuffer ();
	template <class sampletype>
	void processTemplate (sampletype** inputs, sampletype** outputs, VstInt32 sampleFrames);
	void fillBuffer (short *bufpos, int length, int offset);
	void processEvent (VstEvent* event);
	void sendMidi (char* data);
	void fillProgram (VstInt32 channel, VstInt32 prg, MidiProgramName* mpn);
	midisynth *synth;
	DCFilter dcf[2];
	short *buffer;
	int bufferSize;
#ifdef hqresampler
	void *resampler;
	short samples[2];
#endif
	EventQueue MidiQueue;
#if reaper_extensions
	EventQueue ParameterQueue;
#endif
	bool bypassed;
#if !VST_FORCE_DEPRECATED
	double vu[2];
#endif
	float Volume;
	float VolumeDisplay;
	float DCBlock;
	float Transpose;
	float Emulator;
	float PushMidi;
	float ParameterChunk[kNumParams];
	char ProgramName[kVstMaxProgNameLen];
	HostInfo hi;
#ifdef demo
	time_t startTime;
#endif
};

#endif
