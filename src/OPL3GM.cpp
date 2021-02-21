#include "OPL3GM.h"

OPL3GM::OPL3GM (audioMasterCallback audioMaster)
: AudioEffectX (audioMaster, 0, 0)
{
	setNumInputs (0);
	setNumOutputs (2);
	canProcessReplacing ();
	isSynth ();
	synth = NULL;
	buffer = NULL;
	resume ();
}

OPL3GM::~OPL3GM ()
{
	if (synth)
	{
		delete synth;
		synth = NULL;
	}
	if (buffer)
	{
		delete buffer;
		buffer = NULL;
	}
}

bool OPL3GM::getEffectName (char* name)
{
	strcpy (name, "Windows 9X OPL3");
	return true;
}

bool OPL3GM::getVendorString (char* text)
{
	strcpy (text, "Datajake");
	return true;
}

bool OPL3GM::getProductString (char* text)
{
	strcpy (text, "Windows 9X OPL3");
	return true;
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
