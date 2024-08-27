/*
OPL3GM VSTi
Copyright (C) 2021-2024  Datajake

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
//#define GUI
#ifdef GUI
#include "editor.h"
#endif

OPL3GM::OPL3GM (audioMasterCallback audioMaster)
: AudioEffectX (audioMaster, kNumPrograms, kNumParams)
{
	setNumInputs (2);
	setNumOutputs (2);
	setUniqueID ('O3GM');
	setInitialDelay (0);
	canProcessReplacing ();
#if VST_2_4_EXTENSIONS
	canDoubleReplacing ();
#endif
	programsAreChunks ();
	isSynth ();
#if !VST_FORCE_DEPRECATED
	hasVu ();
	hasClip ();
	canMono ();
#endif
	synth = NULL;
	buffer = NULL;
	resampler = NULL;
	memset(samples, 0, sizeof(samples));
	initializeSettings (false);
	memset(&chunk, 0, sizeof(chunk));
	memset(&hi, 0, sizeof(hi));
	memset(vu, 0, sizeof(vu));
	CPULoad = 0;
	initSynth ();
	initBuffer ();
#ifdef DEMO
	startTime = time(NULL);
	srand(startTime);
#endif
#ifdef GUI
	editor = new Editor (this);
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
	hi.MasterVersion = getMasterVersion ();
	if (canHostDo ("receiveVstEvents") || canHostDo ("receiveVstMidiEvent"))
	{
		hi.ReceiveEvents = true;
	}
	else
	{
		hi.ReceiveEvents = false;
	}
}

void OPL3GM::close ()
{
	memset(&hi, 0, sizeof(hi));
}

void OPL3GM::setParameter (VstInt32 index, float value)
{
	lock.acquire();
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
		dcf[0].ResetState();
		dcf[1].ResetState();
		break;
	case kNoiseGate:
		NoiseGate = value;
		dcf[0].ResetState();
		dcf[1].ResetState();
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
		suspend ();
		break;
	case kEmulator:
		Emulator = value;
		changeSynthRate ();
		break;
	case kHQResample:
		HQResample = value;
		if (HQResample >= 0.5)
		{
			internalRate = lastRate;
		}
		else
		{
			if (internalRate != (VstInt32)sampleRate)
			{
				lastRate = internalRate;
			}
			internalRate = (VstInt32)sampleRate;
		}
		changeSynthRate ();
		break;
	case kPushMidi:
		PushMidi = value;
		suspend ();
		break;
	}
	lock.release();
#ifdef GUI
	if (editor)
	{
		((Editor*)editor)->refresh ();
	}
#endif
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
	case kNoiseGate:
		value = NoiseGate;
		break;
	case kTranspose:
		value = (Transpose+12.0f)/24.0f;
		break;
	case kEmulator:
		value = Emulator;
		break;
	case kHQResample:
		value = HQResample;
		break;
	case kPushMidi:
		value = PushMidi;
		break;
	}
	return value;
}

void OPL3GM::getParameterDisplay (VstInt32 index, char* text)
{
	if (!text)
	{
		return;
	}
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
	case kNoiseGate:
		if (NoiseGate >= 0.5)
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
			int2string ((VstInt32)Transpose, text, (kVstMaxParamStrLen*2)-1);
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
	case kHQResample:
		if (HQResample >= 0.5)
		{
			vst_strncpy (text, "ON", (kVstMaxParamStrLen*2)-1);
		}
		else
		{
			vst_strncpy (text, "OFF", (kVstMaxParamStrLen*2)-1);
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
	if (!label)
	{
		return;
	}
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

void OPL3GM::getParameterName (VstInt32 index, char* name)
{
	if (!name)
	{
		return;
	}
	switch (index)
	{
	case kVolume:
		vst_strncpy (name, "Volume", (kVstMaxParamStrLen*2)-1);
		break;
	case kVolumeDisplay:
		vst_strncpy (name, "VolumeDisplay", (kVstMaxParamStrLen*2)-1);
		break;
	case kDCBlock:
		vst_strncpy (name, "DCBlock", (kVstMaxParamStrLen*2)-1);
		break;
	case kNoiseGate:
		vst_strncpy (name, "NoiseGate", (kVstMaxParamStrLen*2)-1);
		break;
	case kTranspose:
		vst_strncpy (name, "Transpose", (kVstMaxParamStrLen*2)-1);
		break;
	case kEmulator:
		vst_strncpy (name, "Emulator", (kVstMaxParamStrLen*2)-1);
		break;
	case kHQResample:
		vst_strncpy (name, "HQResample", (kVstMaxParamStrLen*2)-1);
		break;
	case kPushMidi:
		vst_strncpy (name, "PushMidi", (kVstMaxParamStrLen*2)-1);
		break;
	}
}

VstInt32 OPL3GM::setChunk (void* data, VstInt32 byteSize, bool isPreset)
{
	if (!data)
	{
		return 0;
	}
	OPL3GMChunk* chunkData = (OPL3GMChunk*)data;
	VstInt32 i;
	if (chunkData->Size != sizeof(OPL3GMChunk))
	{
		return 0;
	}
	setProgramName (chunkData->ProgramName);
	for (i = 0; i < kNumParams; i++)
	{
		setParameter (i, chunkData->Parameters[i]);
	}
	setBypass (chunkData->bypassed);
	lastRate = chunkData->lastRate;
	if (HQResample >= 0.5)
	{
		setInternalRate (lastRate);
	}
	loadInstruments (chunkData->BankFile, chunkData->BankName);
	for (i = 0; i < 16; i++)
	{
		enableChannel (i, chunkData->ChannelEnabled[i]);
	}
#ifdef GUI
	if (editor)
	{
		((Editor*)editor)->refresh ();
	}
#endif
	return byteSize;
}

VstInt32 OPL3GM::getChunk (void** data, bool isPreset)
{
	if (!data)
	{
		return 0;
	}
	VstInt32 i;
	chunk.Size = sizeof(OPL3GMChunk);
	getProgramName (chunk.ProgramName);
	for (i = 0; i < kNumParams; i++)
	{
		chunk.Parameters[i] = getParameter (i);
	}
	chunk.bypassed = bypassed;
	if (HQResample >= 0.5)
	{
		chunk.lastRate = internalRate;
	}
	else
	{
		chunk.lastRate = lastRate;
	}
	strncpy(chunk.BankFile, BankFile, sizeof(chunk.BankFile));
	strncpy(chunk.BankName, BankName, sizeof(chunk.BankName));
	for (i = 0; i < 16; i++)
	{
		chunk.ChannelEnabled[i] = ChannelEnabled[i];
	}
	*data = &chunk;
	return sizeof(OPL3GMChunk);
}

void OPL3GM::setProgram (VstInt32 program)
{
	if (program >= kNumPrograms || program < 0)
	{
		return;
	}
	lock.acquire();
	curProgram = program;
	lock.release();
#ifdef GUI
	if (editor)
	{
		((Editor*)editor)->refresh ();
	}
#endif
}

VstInt32 OPL3GM::getProgram ()
{
	return curProgram%kNumPrograms;
}

void OPL3GM::setProgramName (char* name)
{
	if (!name)
	{
		return;
	}
	lock.acquire();
	vst_strncpy (ProgramName, name, kVstMaxProgNameLen-1);
	lock.release();
#ifdef GUI
	if (editor)
	{
		((Editor*)editor)->refresh ();
	}
#endif
}

void OPL3GM::getProgramName (char* name)
{
	if (!name)
	{
		return;
	}
	vst_strncpy (name, ProgramName, kVstMaxProgNameLen-1);
}

bool OPL3GM::string2parameter (VstInt32 index, char* text)
{
	if (!text)
	{
		return false;
	}
	float value = (float)atof(text);
	switch (index)
	{
	case kVolume:
		if (VolumeDisplay >= 0.5)
		{
			value = value/100.0f;
		}
		else
		{
			value = (float)pow(10.0, value/20.0);
		}
		break;
	case kTranspose:
		value = (value+12.0f)/24.0f;
		break;
	}
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
	if (!p)
	{
		return false;
	}
	getParameterName (index, p->label);
	p->shortLabel[0] = 0;
	p->flags = kVstParameterUsesFloatStep | kVstParameterSupportsDisplayIndex;
	p->stepFloat = 0.01f;
	p->smallStepFloat = 0.001f;
	p->largeStepFloat = 0.1f;
	p->displayIndex = (VstInt16)index;
	switch (index)
	{
	case kVolumeDisplay:
		p->flags |= kVstParameterIsSwitch;
		break;
	case kDCBlock:
		p->flags |= kVstParameterIsSwitch;
		break;
	case kNoiseGate:
		p->flags |= kVstParameterIsSwitch;
		break;
	case kTranspose:
		p->flags |= (kVstParameterUsesIntegerMinMax | kVstParameterUsesIntStep);
		p->minInteger = -12;
		p->maxInteger = 12;
		p->stepInteger = 1;
		p->largeStepInteger = 2;
		break;
	case kEmulator:
		p->flags |= kVstParameterIsSwitch;
		break;
	case kHQResample:
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
	if (index == 0 && text)
	{
		vst_strncpy (text, ProgramName, kVstMaxProgNameLen-1);
		return true;
	}
	return false;
}

bool OPL3GM::getInputProperties (VstInt32 index, VstPinProperties* properties)
{
	if (!properties)
	{
		return false;
	}
	switch (cEffect.numInputs)
	{
	case 1:
		if (index == 0)
		{
			vst_strncpy (properties->label, "Mono Input", kVstMaxLabelLen-1);
			vst_strncpy (properties->shortLabel, "InM", kVstMaxShortLabelLen-1);
			properties->flags = kVstPinIsActive;
			return true;
		}
		break;
	case 2:
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
		break;
	}
	return false;
}

bool OPL3GM::getOutputProperties (VstInt32 index, VstPinProperties* properties)
{
	if (!properties)
	{
		return false;
	}
	switch (cEffect.numOutputs)
	{
	case 1:
		if (index == 0)
		{
			vst_strncpy (properties->label, "Mono Output", kVstMaxLabelLen-1);
			vst_strncpy (properties->shortLabel, "OutM", kVstMaxShortLabelLen-1);
			properties->flags = kVstPinIsActive;
			return true;
		}
		break;
	case 2:
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
		break;
	}
	return false;
}

bool OPL3GM::setBypass (bool onOff)
{
	lock.acquire();
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
	lock.release();
#ifdef GUI
	if (editor)
	{
		((Editor*)editor)->refresh ();
	}
#endif
	return true;
}

bool OPL3GM::getEffectName (char* name)
{
	if (!name)
	{
		return false;
	}
	if (synth)
	{
		vst_strncpy (name, synth->midi_synthname(), kVstMaxEffectNameLen-1);
	}
	else
	{
		vst_strncpy (name, "OPL3GM", kVstMaxEffectNameLen-1);
	}
	return true;
}

bool OPL3GM::getVendorString (char* text)
{
	if (!text)
	{
		return false;
	}
	vst_strncpy (text, "Datajake", kVstMaxVendorStrLen-1);
	return true;
}

bool OPL3GM::getProductString (char* text)
{
	if (!text)
	{
		return false;
	}
	vst_strncpy (text, "OPL3GM VSTi", kVstMaxProductStrLen-1);
#ifdef DEMO
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
#if REAPER_EXTENSIONS
	case effGetParamDisplay:
		if (lArg2 >= 0 && lArg2 < kNumParams)
		{
			if (ptrArg)
			{
				if (getParameterDisplayValue ((VstInt32)lArg2, (char*)ptrArg, floatArg))
				{
					return 0xbeef;
				}
			}
		}
		break;
	case effString2Parameter:
		if (lArg2 >= 0 && lArg2 < kNumParams)
		{
			if (ptrArg)
			{
				if (string2parameterReplace ((VstInt32)lArg2, (char*)ptrArg))
				{
					return 0xbeef;
				}
			}
		}
		break;
	case kVstParameterUsesIntStep:
		if (lArg2 >= 0 && lArg2 < kNumParams)
		{
			if (isEnumParameter ((VstInt32)lArg2))
			{
				return 0xbeef;
			}
		}
		break;
	case effCanBeAutomated:
		if (lArg2 >= 0 && lArg2 < kNumParams)
		{
			if (automateParameter ((VstInt32)lArg2, floatArg, (VstInt32)ptrArg))
			{
				return 0xbeef;
			}
		}
		break;
	case 0xdeadbef0:
		if (lArg2 >= 0 && lArg2 < kNumParams)
		{
			if (ptrArg)
			{
				if (parameterRange ((VstInt32)lArg2, (double*)ptrArg))
				{
					return 0xbeef;
				}
			}
		}
		break;
	case effGetChunk:
		if (lArg2 && ptrArg)
		{
			vst_strncpy ((char*)ptrArg, (char*)lArg2, (VstInt32)floatArg-1);
			return 0xf00d;
		}
		break;
	case effSetChunk:
		if (lArg2 && ptrArg)
		{
			return 0xf00d;
		}
		break;
	case effGetEffectName:
		if (lArg2 == 0x50 && ptrArg)
		{
			if (renamePlug ((char**)ptrArg, "OPL3GM"))
			{
				return 0xf00d;
			}
		}
		break;
#endif
	}
	return 0;
}

VstInt32 OPL3GM::canDo (char* text)
{
	if (!text)
	return 0;
	if (!strcmp (text, "sendVstEvents"))
	return 1;
	if (!strcmp (text, "sendVstMidiEvent"))
	return 1;
	if (!strcmp (text, "receiveVstEvents"))
	return 1;
	if (!strcmp (text, "receiveVstMidiEvent"))
	return 1;
	if (!strcmp (text, "receiveVstTimeInfo"))
	return 1;
	if (!strcmp (text, "midiProgramNames"))
	return 1;
	if (!strcmp (text, "bypass"))
	return 1;
#if REAPER_EXTENSIONS
	if (!strcmp (text, "hasCockosExtensions"))
	return 0xbeef0000;
	if (!strcmp (text, "hasCockosSampleAccurateAutomation"))
	return 0xbeef0000;
#endif
	return -1;	// explicitly can't do; 0 => don't know
}

#if VST_2_4_EXTENSIONS
VstInt32 OPL3GM::getNumMidiInputChannels ()
{
	return 16;
}

VstInt32 OPL3GM::getNumMidiOutputChannels ()
{
	if (hi.ReceiveEvents)
	{
		return 16;
	}
	return 0;
}
#endif
