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

bool OPL3GM::getBypass ()
{
	return bypassed;
}

void OPL3GM::setInternalRate (VstInt32 rate)
{
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
		setSampleRate (getSampleRate ());
	}
}

VstInt32 OPL3GM::getInternalRate ()
{
	return internalRate;
}

bool OPL3GM::loadInstruments (char *filename, char *display)
{
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

VstInt32 OPL3GM::getActiveVoices ()
{
	if (synth)
	{
		return synth->midi_getvoicecount();
	}
	return 0;
}

void OPL3GM::getBankName (char *text, VstInt32 size)
{
	strncpy(text, BankName, size);
}

HostInfo *OPL3GM::getHostInfo ()
{
	return &hi;
}

double OPL3GM::getCPULoad ()
{
	return CPULoad;
}
