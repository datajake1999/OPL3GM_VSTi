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

void OPL3GM::setSampleRate (float sampleRate)
{
	AudioEffectX::setSampleRate (sampleRate);
	clearSynth ();
	initSynth ((int)sampleRate);
}

void OPL3GM::setBlockSize (VstInt32 blockSize)
{
	AudioEffectX::setBlockSize (blockSize);
	clearBuffer ();
	initBuffer (blockSize);
}

void OPL3GM::initSynth (int sampleRate)
{
	synth = getsynth();
	if (synth)
	{
#ifdef hqresampler
		if (!synth->midi_init(49716))
#else
		if (!synth->midi_init(sampleRate))
#endif
		{
			delete synth;
			synth = NULL;
		}
	}
#ifdef hqresampler
	resampler = resampler_create();
	if (resampler)
	{
		resampler_set_rate(resampler, 49716.0 / sampleRate);
	}
#endif
}

void OPL3GM::initBuffer (int blockSize)
{
	bufferSize = blockSize;
	buffer = new short[2*bufferSize];
	if (buffer)
	{
		memset(buffer, 0, 4*bufferSize);
	}
}

void OPL3GM::clearSynth ()
{
	if (synth)
	{
		synth->midi_close();
		delete synth;
		synth = NULL;
	}
#ifdef hqresampler
	if (resampler)
	{
		resampler_clear(resampler);
		resampler_destroy(resampler);
		resampler = NULL;
	}
#endif
}

void OPL3GM::clearBuffer ()
{
	if (buffer)
	{
		memset(buffer, 0, 4*bufferSize);
		delete[] buffer;
		buffer = NULL;
	}
}

void OPL3GM::resume ()
{
	AudioEffectX::resume ();
	if (synth)
	{
		synth->midi_reset();
	}
}

void OPL3GM::processReplacing (float** inputs, float** outputs, VstInt32 sampleFrames)
{
	float* out1 = outputs[0];
	float* out2 = outputs[1];

	if (buffer == NULL)
	{
		return;
	}
	if (sampleFrames > bufferSize)
	{
		sampleFrames = bufferSize;
	}
	fillBuffer (sampleFrames);
	for (int i = 0; i < sampleFrames; i++)
	{
		out1[i] = buffer[0] / 32768.0f;
		out2[i] = buffer[1] / 32768.0f;
		out1[i] = out1[i] * Volume;
		out2[i] = out2[i] * Volume;
		buffer += 2;
	}
	buffer -= sampleFrames*2;
}

void OPL3GM::processDoubleReplacing (double** inputs, double** outputs, VstInt32 sampleFrames)
{
	double* out1 = outputs[0];
	double* out2 = outputs[1];

	if (buffer == NULL)
	{
		return;
	}
	if (sampleFrames > bufferSize)
	{
		sampleFrames = bufferSize;
	}
	fillBuffer (sampleFrames);
	for (int i = 0; i < sampleFrames; i++)
	{
		out1[i] = buffer[0] / 32768.0;
		out2[i] = buffer[1] / 32768.0;
		out1[i] = out1[i] * Volume;
		out2[i] = out2[i] * Volume;
		buffer += 2;
	}
	buffer -= sampleFrames*2;
}

void OPL3GM::fillBuffer (int length)
{
#ifdef hqresampler
	if (resampler)
	{
		for (int i = 0; i < length; i++)
		{
			sample_t ls, rs;
			int to_write = resampler_get_min_fill(resampler);
			for (int j = 0; j < to_write; j++)
			{
				short samples[2];
				if (synth)
				{
					if (Emulator >= 0.5)
					{
						synth->midi_generate(samples, 1);
					}
					else
					{
						synth->midi_generate_dosbox(samples, 1);
					}
				}
				resampler_write_pair(resampler, samples[0], samples[1]);
			}
			resampler_peek_pair(resampler, &ls, &rs);
			resampler_read_pair(resampler, &ls, &rs);
			if ((ls + 0x8000) & 0xFFFF0000) ls = (ls >> 31) ^ 0x7FFF;
			if ((rs + 0x8000) & 0xFFFF0000) rs = (rs >> 31) ^ 0x7FFF;
			buffer[0] = (short)ls;
			buffer[1] = (short)rs;
			buffer += 2;
		}
		buffer -= length*2;
	}
#else
	if (synth)
	{
		if (Emulator >= 0.5)
		{
			synth->midi_generate(buffer, length);
		}
		else
		{
			synth->midi_generate_dosbox(buffer, length);
		}
	}
#endif
}

VstInt32 OPL3GM::processEvents (VstEvents* ev)
{
	for (VstInt32 i = 0; i < ev->numEvents; i++)
	{
		if ((ev->events[i])->type == kVstMidiType)
		{
			VstMidiEvent* event = (VstMidiEvent*)ev->events[i];
			char* midiData = event->midiData;
			sendMidi (midiData);
		}
		else if ((ev->events[i])->type == kVstSysExType)
		{
			VstMidiSysexEvent* event = (VstMidiSysexEvent*)ev->events[i];
			if (synth)
			{
				synth->midi_write_sysex(event->sysexDump, event->dumpBytes);
			}
		}
	}
	return 1;
}

void OPL3GM::sendMidi (char *data)
{
	unsigned char byte1 = data[0];
	unsigned char byte2 = data[1];
	unsigned char byte3 = data[2];
	if (Transpose >= 1 || Transpose <= -1)
	{
		unsigned char type = byte1 & 0xf0;
		unsigned char channel = byte1 & 0x0f;
		if (type == 0x80 || type == 0x90)
		{
			if (channel != 9)
			{
				int note = byte2 + (int)Transpose;
				if (note > 127)
				{
					note = 127;
				}
				else if (note < 0)
				{
					note = 0;
				}
				byte2 = (unsigned char)note;
			}
		}
	}
	unsigned int msg = (byte3<<16) | (byte2<<8) | byte1;
	if (synth)
	{
		synth->midi_write(msg);
	}
}
