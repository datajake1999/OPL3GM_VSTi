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

#include "OPL3GM.h"
#include <math.h>

OPL3GM::OPL3GM (audioMasterCallback audioMaster)
: AudioEffectX (audioMaster, kNumPrograms, kNumParams)
{
	setNumInputs (0);
	setNumOutputs (2);
	setUniqueID ('O3GM');
	canProcessReplacing ();
	canDoubleReplacing ();
	isSynth ();
	Volume = 1;
	VolumeDisplay = 0;
	Transpose = 0;
	Emulator = 1;
	strcpy (ProgramName, "Default");
	synth = NULL;
	buffer = NULL;
	initSynth ((int)sampleRate);
	initBuffer (blockSize);
}

OPL3GM::~OPL3GM ()
{
	clearSynth ();
	clearBuffer ();
}

void OPL3GM::setParameter (VstInt32 index, float value)
{
	switch(index)
	{
	case kVolume:
		Volume = value;
		break;
	case kVolumeDisplay:
		VolumeDisplay = value;
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
	}
	if (index >= kTranspose && index < kNumParams)
	{
		if (synth)
		{
			synth->midi_panic();
		}
	}
}

float OPL3GM::getParameter (VstInt32 index)
{
	float value = 0;
	switch(index)
	{
	case kVolume:
		value = Volume;
		break;
	case kVolumeDisplay:
		value = VolumeDisplay;
		break;
	case kTranspose:
		value = (Transpose+12.0f)/24.0f;
		break;
	case kEmulator:
		value = Emulator;
		break;
	}
	return value;
}

void OPL3GM::getParameterDisplay (VstInt32 index, char* text)
{
	switch(index)
	{
	case kVolume:
		if (VolumeDisplay >= 0.5)
		{
			float2string (Volume*100, text, kVstMaxParamStrLen);
		}
		else
		{
			dB2string (Volume, text, kVstMaxParamStrLen);
		}
		break;
	case kVolumeDisplay:
		if (VolumeDisplay >= 0.5)
		{
			strcpy (text, "%");
		}
		else
		{
			strcpy (text, "dB");
		}
		break;
	case kTranspose:
		if (Transpose >= 1 || Transpose <= -1)
		{
			int2string ((int)Transpose, text, kVstMaxParamStrLen);
		}
		else
		{
			strcpy (text, "0");
		}
		break;
	case kEmulator:
		if (Emulator >= 0.5)
		{
			strcpy (text, "Nuked");
		}
		else
		{
			strcpy (text, "DOSBox");
		}
		break;
	}
}

void OPL3GM::getParameterLabel (VstInt32 index, char* label)
{
	switch(index)
	{
	case kVolume:
		if (VolumeDisplay >= 0.5)
		{
			strcpy (label, "%");
		}
		else
		{
			strcpy (label, "dB");
		}
		break;
	case kTranspose:
		if (floor(Transpose) == 1 || ceil(Transpose) == -1)
		{
			strcpy (label, "Semitone");
		}
		else
		{
			strcpy (label, "Semitones");
		}
		break;
	}
}

void OPL3GM::getParameterName (VstInt32 index, char* text)
{
	switch(index)
	{
	case kVolume:
		strcpy (text, "Volume");
		break;
	case kVolumeDisplay:
		strcpy (text, "VolumeDisplay");
		break;
	case kTranspose:
		strcpy (text, "Transpose");
		break;
	case kEmulator:
		strcpy (text, "Emulator");
		break;
	}
}

void OPL3GM::setProgramName (char *name)
{
	strcpy (ProgramName, name);
}

void OPL3GM::getProgramName (char *name)
{
	strcpy (name, ProgramName);
}

bool OPL3GM::getProgramNameIndexed (VstInt32 category, VstInt32 index, char* text)
{
	if (index == 0)
	{
		strcpy (text, ProgramName);
		return true;
	}
	return false;
}

bool OPL3GM::getOutputProperties (VstInt32 index, VstPinProperties* properties)
{
	if (index == 0)
	{
		strcpy (properties->label, "Left Output");
		strcpy (properties->shortLabel, "LeftOut");
		properties->flags = kVstPinIsActive | kVstPinIsStereo;
		return true;
	}
	else if (index == 1)
	{
		strcpy (properties->label, "Right Output");
		strcpy (properties->shortLabel, "RightOut");
		properties->flags = kVstPinIsActive | kVstPinIsStereo;
		return true;
	}
	return false;
}

bool OPL3GM::getEffectName (char* name)
{
	if (synth)
	{
		strcpy (name, synth->midi_synthname());
		return true;
	}
	return false;
}

bool OPL3GM::getVendorString (char* text)
{
	strcpy (text, "Datajake");
	return true;
}

bool OPL3GM::getProductString (char* text)
{
	return getEffectName (text);
}

VstInt32 OPL3GM::getVendorVersion ()
{ 
	return 1000; 
}

VstPlugCategory OPL3GM::getPlugCategory ()
{
	return kPlugCategSynth;
}

VstInt32 OPL3GM::canDo (char* text)
{
	if (!strcmp (text, "receiveVstEvents"))
	return 1;
	if (!strcmp (text, "receiveVstMidiEvent"))
	return 1;
	if (!strcmp (text, "midiProgramNames"))
	return 1;
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
