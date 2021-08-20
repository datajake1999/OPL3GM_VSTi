#include "OPL3GM.h"

OPL3GM::OPL3GM (audioMasterCallback audioMaster)
: AudioEffectX (audioMaster, kNumPrograms, kNumParams)
{
	setNumInputs (0);
	setNumOutputs (2);
	setUniqueID ('W9XO');
	canProcessReplacing ();
	canDoubleReplacing ();
	isSynth ();
	volume = 1;
	strcpy (ProgramName, "Default");
	synth = NULL;
	synth = getsynth();
	if (synth)
	{
		if (!synth->midi_init((int)sampleRate))
		{
			delete synth;
			synth = NULL;
		}
	}
	buffer = NULL;
	bufferSize = blockSize;
	buffer = new short[2*bufferSize];
	if (buffer)
	{
		memset(buffer, 0, 4*bufferSize);
	}
}

OPL3GM::~OPL3GM ()
{
	if (synth)
	{
		synth->midi_close();
		delete synth;
		synth = NULL;
	}
	if (buffer)
	{
		delete buffer;
		buffer = NULL;
	}
}

void OPL3GM::setParameter (VstInt32 index, float value)
{
	switch(index)
	{
	case kVolume:
		volume = value;
		break;
	}
}

float OPL3GM::getParameter (VstInt32 index)
{
	float value = 0;
	switch(index)
	{
	case kVolume:
		value = volume;
		break;
	}
	return value;
}

void OPL3GM::getParameterDisplay (VstInt32 index, char* text)
{
	switch(index)
	{
	case kVolume:
		float2string (volume, text, kVstMaxParamStrLen);
		break;
	}
}

void OPL3GM::getParameterLabel (VstInt32 index, char* label)
{
	switch(index)
	{
	case kVolume:
		strcpy (label, "F");
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
	if (synth)
	{
		strcpy (text, synth->midi_synthname());
		return true;
	}
	return false;
}

VstInt32 OPL3GM::getVendorVersion ()
{ 
	return 1000; 
}

VstInt32 OPL3GM::canDo (char* text)
{
	if (!strcmp (text, "receiveVstEvents"))
	return 1;
	if (!strcmp (text, "receiveVstMidiEvent"))
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
