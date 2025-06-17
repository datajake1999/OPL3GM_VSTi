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

#include "OPL3GM.h"

void OPL3GM::initializeSettings (bool resetSynth)
{
	lock.acquire();
	vst_strncpy (ProgramName, "Default", kVstMaxProgNameLen-1);
	Volume = 1;
	VolumeDisplay = 0;
	DCBlock = 0;
	NoiseGate = 0;
	Transpose = 0;
	Emulator = 1;
	HQResample = 1;
	PushMidi = 1;
	bypassed = false;
	internalRate = 49716;
	lastRate = internalRate;
	memset(BankFile, 0, sizeof(BankFile));
	strncpy(BankName, "Default", sizeof(BankName));
	for (VstInt32 i = 0; i < 16; i++)
	{
		ChannelEnabled[i] = true;
	}
	FreezeMeters = false;
	HideParameters = false;
	if (resetSynth)
	{
		suspend ();
		dcf[0].ResetState();
		dcf[1].ResetState();
		changeSynthRate ();
		if (synth)
		{
			synth->midi_resetbank();
		}
	}
	lock.release();
}

bool OPL3GM::getBypass ()
{
	return bypassed;
}

void OPL3GM::setInternalRate (VstInt32 rate)
{
	lock.acquire();
	if (rate != internalRate)
	{
		internalRate = rate;
		if (internalRate > 1000000)
		{
			internalRate = 1000000;
		}
		else if (internalRate < 1000)
		{
			internalRate = 1000;
		}
		changeSynthRate ();
	}
	lock.release();
}

VstInt32 OPL3GM::getInternalRate ()
{
	return internalRate;
}

bool OPL3GM::loadInstruments (char* filename, char* display)
{
	if (!filename || !display)
	{
		return false;
	}
	lock.acquire();
	if (synth)
	{
		if (synth->midi_loadbank(filename))
		{
			strncpy(BankFile, filename, sizeof(BankFile));
			strncpy(BankName, display, sizeof(BankName));
			lock.release();
			return true;
		}
	}
	lock.release();
	return false;
}

void OPL3GM::enableChannel (VstInt32 channel, bool enable)
{
	lock.acquire();
	channel = channel & 0x0f;
	if (ChannelEnabled[channel] && !enable)
	{
		char data[3];
		data[0] = 0xb0 + (char)channel;
		data[1] = 0x40;
		data[2] = 0;
		sendMidi (data);
		data[1] = 0x7b;
		sendMidi (data);
		data[1] = 0x79;
		sendMidi (data);
		data[0] = 0xe0 + (char)channel;
		data[1] = 0;
		data[2] = 0x40;
		sendMidi (data);
	}
	ChannelEnabled[channel] = enable;
	lock.release();
}

bool OPL3GM::isChannelEnabled (VstInt32 channel)
{
	channel = channel & 0x0f;
	return ChannelEnabled[channel];
}

void OPL3GM::setFreezeMeters (bool value)
{
	lock.acquire();
	FreezeMeters = value;
	lock.release();
}

bool OPL3GM::getFreezeMeters ()
{
	return FreezeMeters;
}

void OPL3GM::setHideParameters (bool value)
{
	lock.acquire();
	HideParameters = value;
	lock.release();
}

bool OPL3GM::getHideParameters ()
{
	return HideParameters;
}

void OPL3GM::hardReset ()
{
	lock.acquire();
	clearSynth ();
	clearBuffer ();
	initSynth ();
	initBuffer ();
	lock.release();
}

VstInt32 OPL3GM::getActiveVoices ()
{
	if (synth)
	{
		return synth->midi_getvoicecount();
	}
	return 0;
}

void OPL3GM::getBankName (char* text, VstInt32 size)
{
	if (!text)
	{
		return;
	}
	strncpy(text, BankName, size);
}

HostInfo* OPL3GM::getHostInfo ()
{
	return &hi;
}

double OPL3GM::getCPULoad ()
{
	return CPULoad;
}
