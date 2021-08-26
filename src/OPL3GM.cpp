#include "OPL3GM.h"

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
	Transpose = 0;
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
		Volume = value;
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
		if (synth)
		{
			synth->midi_panic();
		}
		break;
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
	case kTranspose:
		value = (Transpose+12.0f)/24.0f;
		break;
	}
	return value;
}

void OPL3GM::getParameterDisplay (VstInt32 index, char* text)
{
	switch(index)
	{
	case kVolume:
		dB2string (Volume, text, kVstMaxParamStrLen);
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
	}
}

void OPL3GM::getParameterLabel (VstInt32 index, char* label)
{
	switch(index)
	{
	case kVolume:
		strcpy (label, "dB");
		break;
	case kTranspose:
		strcpy (label, "Semitones");
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
	case kTranspose:
		strcpy (text, "Transpose");
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
		properties->flags = kVstPinIsActive;
		properties->flags |= kVstPinIsStereo;
		return true;
	}
	else if (index == 1)
	{
		strcpy (properties->label, "Right Output");
		strcpy (properties->shortLabel, "RightOut");
		properties->flags = kVstPinIsActive;
		properties->flags |= kVstPinIsStereo;
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
