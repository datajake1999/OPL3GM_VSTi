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

void OPL3GM::setBlockSizeAndSampleRate (VstInt32 blockSize, float sampleRate)
{
	setBlockSize (blockSize);
	setSampleRate (sampleRate);
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

bool OPL3GM::getErrorText (char* text)
{
	if (!synth)
	{
		sprintf(text, "Error initializing synth.\n");
		return true;
	}
#ifdef hqresampler
	if (!resampler)
	{
		sprintf(text, "Error initializing resampler.\n");
		return true;
	}
#endif
	if (!buffer)
	{
		sprintf(text, "Error initializing buffer.\n");
		return true;
	}
	return false;
}

void OPL3GM::suspend ()
{
	if (synth)
	{
		synth->midi_panic();
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

float OPL3GM::getVu ()
{
	if (bypassed)
	{
		return 0;
	}
	double value = fabs((vu[0] + vu[1]) / 2);
	if (value > 1)
	{
		value = 1;
	}
	else if (value < 0)
	{
		value = 0;
	}
	return (float)value;
}

void OPL3GM::process (float** inputs, float** outputs, VstInt32 sampleFrames)
{
	processReplacing (inputs, outputs, sampleFrames);
}

void OPL3GM::processReplacing (float** inputs, float** outputs, VstInt32 sampleFrames)
{
	float* out1 = outputs[0];
	float* out2 = outputs[1];

	if (bypassed)
	{
		memset(out1, 0, sampleFrames*sizeof(float));
		memset(out2, 0, sampleFrames*sizeof(float));
		return;
	}
	if (buffer == NULL)
	{
		return;
	}
	if (sampleFrames > bufferSize)
	{
		sampleFrames = bufferSize;
	}
	render (buffer, sampleFrames);
	for (int i = 0; i < sampleFrames; i++)
	{
		out1[i] = buffer[0] / 32768.0f;
		out2[i] = buffer[1] / 32768.0f;
		out1[i] = out1[i] * Volume;
		out2[i] = out2[i] * Volume;
		if (DCBlock >= 0.5)
		{
			out1[i] = (float)dcf[0].Process(out1[i]);
			out2[i] = (float)dcf[1].Process(out2[i]);
		}
#ifdef demo
		if (time(NULL) >= startTime + 600)
		{
			out1[i] += ((rand() / (float)RAND_MAX) / 256.0f);
			out2[i] += ((rand() / (float)RAND_MAX) / 256.0f);
		}
#endif
		vu[0] = out1[i];
		vu[1] = out2[i];
		buffer += 2;
	}
	buffer -= sampleFrames*2;
}

void OPL3GM::processDoubleReplacing (double** inputs, double** outputs, VstInt32 sampleFrames)
{
	double* out1 = outputs[0];
	double* out2 = outputs[1];

	if (bypassed)
	{
		memset(out1, 0, sampleFrames*sizeof(double));
		memset(out2, 0, sampleFrames*sizeof(double));
		return;
	}
	if (buffer == NULL)
	{
		return;
	}
	if (sampleFrames > bufferSize)
	{
		sampleFrames = bufferSize;
	}
	render (buffer, sampleFrames);
	for (int i = 0; i < sampleFrames; i++)
	{
		out1[i] = buffer[0] / 32768.0;
		out2[i] = buffer[1] / 32768.0;
		out1[i] = out1[i] * Volume;
		out2[i] = out2[i] * Volume;
		if (DCBlock >= 0.5)
		{
			out1[i] = dcf[0].Process(out1[i]);
			out2[i] = dcf[1].Process(out2[i]);
		}
#ifdef demo
		if (time(NULL) >= startTime + 600)
		{
			out1[i] += ((rand() / (float)RAND_MAX) / 256.0);
			out2[i] += ((rand() / (float)RAND_MAX) / 256.0);
		}
#endif
		vu[0] = out1[i];
		vu[1] = out2[i];
		buffer += 2;
	}
	buffer -= sampleFrames*2;
}

void OPL3GM::render (short *bufpos, int length)
{
	for (int i = 0; i < length; i++)
	{
		while (evq.GetEventCount())
		{
			if (evq.GetEventTime() > i)
			{
				break;
			}
			processEvent (evq.GetNextEvent());
		}
		fillBuffer (bufpos, 1, i);
	}
}

void OPL3GM::fillBuffer (short *bufpos, int length, int offset)
{
	bufpos += 2*offset;
#ifdef hqresampler
	if (resampler)
	{
		for (int i = 0; i < length; i++)
		{
			sample_t ls, rs;
			int to_write = resampler_get_min_fill(resampler);
			for (int j = 0; j < to_write; j++)
			{
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
			bufpos[0] = (short)ls;
			bufpos[1] = (short)rs;
			bufpos += 2;
		}
	}
#else
	if (synth)
	{
		if (Emulator >= 0.5)
		{
			synth->midi_generate(bufpos, length);
		}
		else
		{
			synth->midi_generate_dosbox(bufpos, length);
		}
	}
#endif
}

VstInt32 OPL3GM::processEvents (VstEvents* ev)
{
	if (bypassed)
	{
		return 0;
	}
	for (VstInt32 i = 0; i < ev->numEvents; i++)
	{
		evq.EnqueueEvent (ev->events[i]);
	}
	return 1;
}

void OPL3GM::processEvent (VstEvent* ev)
{
	if (ev->type == kVstMidiType)
	{
		VstMidiEvent* event = (VstMidiEvent*)ev;
		char* midiData = event->midiData;
		sendMidi (midiData);
	}
	else if (ev->type == kVstSysExType)
	{
		VstMidiSysexEvent* event = (VstMidiSysexEvent*)ev;
		if (synth)
		{
			synth->midi_write_sysex(event->sysexDump, event->dumpBytes);
		}
	}
}

void OPL3GM::sendMidi (char *data)
{
	unsigned char byte1 = data[0] & 0xff;
	unsigned char byte2 = data[1] & 0x7f;
	unsigned char byte3 = data[2] & 0x7f;
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

VstInt32 OPL3GM::startProcess ()
{
	if (buffer)
	{
		memset(buffer, 0, 4*bufferSize);
		return 1;
	}
	return 0;
}

VstInt32 OPL3GM::stopProcess ()
{
	if (buffer)
	{
		memset(buffer, 0, 4*bufferSize);
		return 1;
	}
	return 0;
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
