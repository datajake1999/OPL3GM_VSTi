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

#include "OPL3GM.h"
//#define gui
#ifdef gui
#include "editor/editor.h"
#endif

OPL3GM::OPL3GM (audioMasterCallback audioMaster)
: AudioEffectX (audioMaster, kNumPrograms, kNumParams)
{
	setNumInputs (0);
	setNumOutputs (2);
	setUniqueID ('O3GM');
	setInitialDelay (0);
	canProcessReplacing ();
	canDoubleReplacing ();
	programsAreChunks ();
	isSynth ();
#if !VST_FORCE_DEPRECATED
	hasVu ();
	hasClip ();
#endif
	synth = NULL;
	buffer = NULL;
#ifdef hqresampler
	resampler = NULL;
	memset(samples, 0, sizeof(samples));
#endif
	bypassed = false;
#if !VST_FORCE_DEPRECATED
	memset(vu, 0, sizeof(vu));
#endif
	Volume = 1;
	VolumeDisplay = 0;
	DCBlock = 0;
	Transpose = 0;
	Emulator = 1;
	PushMidi = 1;
	memset(ParameterChunk, 0, sizeof(ParameterChunk));
	vst_strncpy (ProgramName, "Default", kVstMaxProgNameLen-1);
	initSynth ((int)sampleRate);
	initBuffer (blockSize);
#ifdef gui
	editor = new Editor (this);
#endif
#ifdef demo
	startTime = time(NULL);
	srand(startTime);
#endif
}

OPL3GM::~OPL3GM ()
{
	clearSynth ();
	clearBuffer ();
}

void OPL3GM::open ()
{
	getHostVendorString (hi.VendorString);
	getHostProductString (hi.ProductString);
	hi.VendorVersion = getHostVendorVersion ();
}

void OPL3GM::close ()
{
	memset(&hi, 0, sizeof(hi));
}

void OPL3GM::setParameter (VstInt32 index, float value)
{
	switch (index)
	{
	case kVolume:
		Volume = value;
		break;
	case kVolumeDisplay:
		VolumeDisplay = value;
		break;
	case kDCBlock:
		DCBlock = value;
		break;
	case kTranspose:
		Transpose = (value*24.0f)-12.0f;
		if (Transpose > 12)
		{
			Transpose = 12;
		}
		else if (Transpose < -12)
		{
			Transpose = -12;
		}
		break;
	case kEmulator:
		Emulator = value;
		break;
	case kPushMidi:
		PushMidi = value;
		break;
	}
	if (index >= kTranspose && index < kNumParams)
	{
		suspend ();
	}
}

float OPL3GM::getParameter (VstInt32 index)
{
	float value = 0;
	switch (index)
	{
	case kVolume:
		value = Volume;
		break;
	case kVolumeDisplay:
		value = VolumeDisplay;
		break;
	case kDCBlock:
		value = DCBlock;
		break;
	case kTranspose:
		value = (Transpose+12.0f)/24.0f;
		break;
	case kEmulator:
		value = Emulator;
		break;
	case kPushMidi:
		value = PushMidi;
		break;
	}
	return value;
}

void OPL3GM::getParameterDisplay (VstInt32 index, char* text)
{
	switch (index)
	{
	case kVolume:
		if (VolumeDisplay >= 0.5)
		{
			float2string (Volume*100, text, (kVstMaxParamStrLen*2)-1);
		}
		else
		{
			dB2string (Volume, text, (kVstMaxParamStrLen*2)-1);
		}
		break;
	case kVolumeDisplay:
		if (VolumeDisplay >= 0.5)
		{
			vst_strncpy (text, "%", (kVstMaxParamStrLen*2)-1);
		}
		else
		{
			vst_strncpy (text, "dB", (kVstMaxParamStrLen*2)-1);
		}
		break;
	case kDCBlock:
		if (DCBlock >= 0.5)
		{
			vst_strncpy (text, "ON", (kVstMaxParamStrLen*2)-1);
		}
		else
		{
			vst_strncpy (text, "OFF", (kVstMaxParamStrLen*2)-1);
		}
		break;
	case kTranspose:
		if (Transpose >= 1 || Transpose <= -1)
		{
			int2string ((int)Transpose, text, (kVstMaxParamStrLen*2)-1);
		}
		else
		{
			vst_strncpy (text, "0", (kVstMaxParamStrLen*2)-1);
		}
		break;
	case kEmulator:
		if (Emulator >= 0.5)
		{
			vst_strncpy (text, "Nuked", (kVstMaxParamStrLen*2)-1);
		}
		else
		{
			vst_strncpy (text, "DOSBox", (kVstMaxParamStrLen*2)-1);
		}
		break;
	case kPushMidi:
		if (PushMidi >= 0.5)
		{
			vst_strncpy (text, "ON", (kVstMaxParamStrLen*2)-1);
		}
		else
		{
			vst_strncpy (text, "OFF", (kVstMaxParamStrLen*2)-1);
		}
		break;
	}
}

void OPL3GM::getParameterLabel (VstInt32 index, char* label)
{
	switch (index)
	{
	case kVolume:
		if (VolumeDisplay >= 0.5)
		{
			vst_strncpy (label, "%", (kVstMaxParamStrLen*2)-1);
		}
		else
		{
			vst_strncpy (label, "dB", (kVstMaxParamStrLen*2)-1);
		}
		break;
	case kTranspose:
		if (floor(Transpose) == 1 || ceil(Transpose) == -1)
		{
			vst_strncpy (label, "Semitone", (kVstMaxParamStrLen*2)-1);
		}
		else
		{
			vst_strncpy (label, "Semitones", (kVstMaxParamStrLen*2)-1);
		}
		break;
	}
}

void OPL3GM::getParameterName (VstInt32 index, char* text)
{
	switch (index)
	{
	case kVolume:
		vst_strncpy (text, "Volume", (kVstMaxParamStrLen*2)-1);
		break;
	case kVolumeDisplay:
		vst_strncpy (text, "VolumeDisplay", (kVstMaxParamStrLen*2)-1);
		break;
	case kDCBlock:
		vst_strncpy (text, "DCBlock", (kVstMaxParamStrLen*2)-1);
		break;
	case kTranspose:
		vst_strncpy (text, "Transpose", (kVstMaxParamStrLen*2)-1);
		break;
	case kEmulator:
		vst_strncpy (text, "Emulator", (kVstMaxParamStrLen*2)-1);
		break;
	case kPushMidi:
		vst_strncpy (text, "PushMidi", (kVstMaxParamStrLen*2)-1);
		break;
	}
}

VstInt32 OPL3GM::setChunk (void* data, VstInt32 byteSize, bool isPreset)
{
	float *chunkData = (float *)data;
	for (VstInt32 i = 0; i < kNumParams; i++)
	{
		setParameter (i, chunkData[i]);
	}
	return byteSize;
}

VstInt32 OPL3GM::getChunk (void** data, bool isPreset)
{
	for (VstInt32 i = 0; i < kNumParams; i++)
	{
		ParameterChunk[i] = getParameter (i);
	}
	*data = ParameterChunk;
	return kNumParams*sizeof(float);
}

void OPL3GM::setProgram (VstInt32 program)
{
	if (program >= kNumPrograms || program < 0)
	{
		return;
	}
	curProgram = program;
}

VstInt32 OPL3GM::getProgram ()
{
	return curProgram%kNumPrograms;
}

void OPL3GM::setProgramName (char *name)
{
	vst_strncpy (ProgramName, name, kVstMaxProgNameLen-1);
}

void OPL3GM::getProgramName (char *name)
{
	vst_strncpy (name, ProgramName, kVstMaxProgNameLen-1);
}

bool OPL3GM::canParameterBeAutomated (VstInt32 index)
{
	return true;
}

bool OPL3GM::string2parameter (VstInt32 index, char* text)
{
	float value = (float)atof(text);
	if (value > 1)
	{
		value = 1;
	}
	else if (value < 0)
	{
		value = 0;
	}
	setParameter (index, value);
	return true;
}

bool OPL3GM::getParameterProperties (VstInt32 index, VstParameterProperties* p)
{
	getParameterName (index, p->label);
	p->shortLabel[0] = 0;
	p->flags = kVstParameterUsesFloatStep;
	p->stepFloat = 0.01f;
	p->smallStepFloat = 0.001f;
	p->largeStepFloat = 0.1f;
	switch (index)
	{
	case kVolumeDisplay:
		p->flags |= kVstParameterIsSwitch;
		break;
	case kDCBlock:
		p->flags |= kVstParameterIsSwitch;
		break;
	case kTranspose:
		p->flags |= (kVstParameterUsesIntegerMinMax | kVstParameterUsesIntStep);
		p->minInteger = -12;
		p->maxInteger = 12;
		p->stepInteger = 1;
		p->largeStepInteger = 1;
		break;
	case kEmulator:
		p->flags |= kVstParameterIsSwitch;
		break;
	case kPushMidi:
		p->flags |= kVstParameterIsSwitch;
		break;
	}
	return true;
}

bool OPL3GM::getProgramNameIndexed (VstInt32 category, VstInt32 index, char* text)
{
	if (index == 0)
	{
		vst_strncpy (text, ProgramName, kVstMaxProgNameLen-1);
		return true;
	}
	return false;
}

bool OPL3GM::beginSetProgram ()
{
	suspend ();
	stopProcess ();
	return true;
}

bool OPL3GM::endSetProgram ()
{
	resume ();
	startProcess ();
	return true;
}

VstInt32 OPL3GM::beginLoadBank (VstPatchChunkInfo* ptr)
{
	return 0;
}

VstInt32 OPL3GM::beginLoadProgram (VstPatchChunkInfo* ptr)
{
	return 0;
}

bool OPL3GM::getInputProperties (VstInt32 index, VstPinProperties* properties)
{
	if (index == 0)
	{
		vst_strncpy (properties->label, "Left Input", kVstMaxLabelLen-1);
		vst_strncpy (properties->shortLabel, "InL", kVstMaxShortLabelLen-1);
		properties->flags = kVstPinIsActive | kVstPinIsStereo;
		return true;
	}
	else if (index == 1)
	{
		vst_strncpy (properties->label, "Right Input", kVstMaxLabelLen-1);
		vst_strncpy (properties->shortLabel, "InR", kVstMaxShortLabelLen-1);
		properties->flags = kVstPinIsActive | kVstPinIsStereo;
		return true;
	}
	return false;
}

bool OPL3GM::getOutputProperties (VstInt32 index, VstPinProperties* properties)
{
	if (index == 0)
	{
		vst_strncpy (properties->label, "Left Output", kVstMaxLabelLen-1);
		vst_strncpy (properties->shortLabel, "OutL", kVstMaxShortLabelLen-1);
		properties->flags = kVstPinIsActive | kVstPinIsStereo;
		return true;
	}
	else if (index == 1)
	{
		vst_strncpy (properties->label, "Right Output", kVstMaxLabelLen-1);
		vst_strncpy (properties->shortLabel, "OutR", kVstMaxShortLabelLen-1);
		properties->flags = kVstPinIsActive | kVstPinIsStereo;
		return true;
	}
	return false;
}

bool OPL3GM::setBypass (bool onOff)
{
	bypassed = onOff;
	if (bypassed)
	{
		suspend ();
		stopProcess ();
	}
	else
	{
		resume ();
		startProcess ();
	}
	return true;
}

bool OPL3GM::setPanLaw (VstInt32 type, float val)
{
	Volume = val;
	return true;
}

bool OPL3GM::setProcessPrecision (VstInt32 precision)
{
	return false;
}

bool OPL3GM::getEffectName (char* name)
{
	if (synth)
	{
		vst_strncpy (name, synth->midi_synthname(), kVstMaxEffectNameLen-1);
		return true;
	}
	return false;
}

bool OPL3GM::getVendorString (char* text)
{
	vst_strncpy (text, "Datajake", kVstMaxVendorStrLen-1);
	return true;
}

bool OPL3GM::getProductString (char* text)
{
	vst_strncpy (text, "OPL3GM VSTi", kVstMaxProductStrLen-1);
#ifdef demo
	vst_strncat (text, " Demo", kVstMaxProductStrLen-1);
#endif
	return true;
}

VstInt32 OPL3GM::getVendorVersion ()
{ 
	return 1000; 
}

VstPlugCategory OPL3GM::getPlugCategory ()
{
	return kPlugCategSynth;
}

VstIntPtr OPL3GM::vendorSpecific (VstInt32 lArg, VstIntPtr lArg2, void* ptrArg, float floatArg)
{
	switch (lArg)
	{
#ifdef reaper
	case effGetParamDisplay:
		if (lArg2 >= 0 && lArg2 < kNumParams)
		{
			if (ptrArg)
			{
				sprintf((char *)ptrArg, "%f", floatArg);
				return 0xbeef;
			}
		}
		break;
	case effString2Parameter:
		if (lArg2 >= 0 && lArg2 < kNumParams)
		{
			if (ptrArg)
			{
				float value = (float)atof((char *)ptrArg);
				if (value > 1)
				{
					value = 1;
				}
				else if (value < 0)
				{
					value = 0;
				}
				sprintf((char *)ptrArg, "%f", value);
				return 0xbeef;
			}
		}
		break;
	case kVstParameterUsesIntStep:
		if (lArg2 >= 0 && lArg2 < kNumParams)
		{
			switch (lArg2)
			{
			case kVolumeDisplay:
				return 0xbeef;
			case kDCBlock:
				return 0xbeef;
			case kTranspose:
				return 0xbeef;
			case kEmulator:
				return 0xbeef;
			case kPushMidi:
				return 0xbeef;
			}
		}
		break;
	case effCanBeAutomated:
		if (lArg2 >= 0 && lArg2 < kNumParams)
		{
			VstParameterEvent ev;
			ev.type = kVstParameterType;
			ev.byteSize = sizeof(VstParameterEvent);
			ev.deltaFrames = (VstInt32)ptrArg;
			ev.flags = 0;
			ev.index = (VstInt32)lArg2;
			ev.value = floatArg;
			if (!ParameterQueue.EnqueueEvent ((VstEvent *)&ev))
			{
				return 0;
			}
			return 0xbeef;
		}
		break;
#endif
	}
	return 0;
}

VstInt32 OPL3GM::canDo (char* text)
{
	if (!strcmp (text, "receiveVstEvents"))
	return 1;
	if (!strcmp (text, "receiveVstMidiEvent"))
	return 1;
	if (!strcmp (text, "midiProgramNames"))
	return 1;
	if (!strcmp (text, "bypass"))
	return 1;
#ifdef reaper
	if (!strcmp (text, "hasCockosExtensions"))
	return 0xbeef0000;
	if (!strcmp (text, "hasCockosSampleAccurateAutomation"))
	return 0xbeef0000;
#endif
	return -1;	// explicitly can't do; 0 => don't know
}

VstInt32 OPL3GM::getNumMidiInputChannels ()
{
	return 16;
}

VstInt32 OPL3GM::getNumMidiOutputChannels ()
{
	return 0;
}
