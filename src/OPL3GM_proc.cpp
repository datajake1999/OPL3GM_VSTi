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
	lock.acquire();
	AudioEffectX::setSampleRate (sampleRate);
	clearSynth ();
	initSynth ((int)sampleRate);
	lock.release();
}

void OPL3GM::setBlockSize (VstInt32 blockSize)
{
	lock.acquire();
	AudioEffectX::setBlockSize (blockSize);
	clearBuffer ();
	initBuffer (blockSize);
	lock.release();
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
	dcf[0].SetRate(sampleRate);
	dcf[1].SetRate(sampleRate);
#ifdef hqresampler
	resampler = resampler_create();
	if (resampler)
	{
		resampler_set_rate(resampler, 49716.0 / sampleRate);
	}
#endif
	loadInstruments (BankFile, BankName);
}

void OPL3GM::initBuffer (int blockSize)
{
	bufferSize = blockSize;
	buffer = new short[2*bufferSize];
	if (buffer)
	{
		memset(buffer, 0, (2*bufferSize)*sizeof(short));
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
		memset(buffer, 0, (2*bufferSize)*sizeof(short));
		delete[] buffer;
		buffer = NULL;
	}
}

#if !VST_FORCE_DEPRECATED
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

#endif
void OPL3GM::suspend ()
{
	lock.acquire();
	MidiQueue.Flush(true);
#if reaper_extensions
	ParameterQueue.Flush(true);
#endif
	if (synth)
	{
		synth->midi_panic();
	}
	lock.release();
}

void OPL3GM::resume ()
{
	lock.acquire();
	AudioEffectX::resume ();
	dcf[0].ResetState();
	dcf[1].ResetState();
	if (synth)
	{
		synth->midi_reset();
	}
	lock.release();
}

float OPL3GM::getVu ()
{
	if (bypassed)
	{
		return 0;
	}
	return (float)fabs((vu[0] + vu[1]) / 2);
}

#if !VST_FORCE_DEPRECATED
void OPL3GM::process (float** inputs, float** outputs, VstInt32 sampleFrames)
{
	processTemplate (inputs, outputs, sampleFrames);
}

#endif
void OPL3GM::processReplacing (float** inputs, float** outputs, VstInt32 sampleFrames)
{
	processTemplate (inputs, outputs, sampleFrames);
}

#if VST_2_4_EXTENSIONS
void OPL3GM::processDoubleReplacing (double** inputs, double** outputs, VstInt32 sampleFrames)
{
	processTemplate (inputs, outputs, sampleFrames);
}

#endif
template <class sampletype>
void OPL3GM::processTemplate (sampletype** inputs, sampletype** outputs, VstInt32 sampleFrames)
{
	sampletype* in1 = inputs[0];
	sampletype* in2 = inputs[1];
	sampletype* out1 = outputs[0];
	sampletype* out2 = outputs[1];

	lock.acquire();
	if (bypassed || !buffer || !out1 || !out2)
	{
		double begin = GetCPUTime();
		if (out1)
		{
			if (in1)
			{
				memcpy(out1, in1, sampleFrames*sizeof(sampletype));
			}
			else
			{
				memset(out1, 0, sampleFrames*sizeof(sampletype));
			}
		}
		if (out2)
		{
			if (in2)
			{
				memcpy(out2, in2, sampleFrames*sizeof(sampletype));
			}
			else
			{
				memset(out2, 0, sampleFrames*sizeof(sampletype));
			}
		}
		double end = GetCPUTime();
		calculateCPULoad (begin, end, sampleFrames);
		lock.release();
		return;
	}

	VstTimeInfo *timeinfo = getTimeInfo (0);

	if (sampleFrames > bufferSize)
	{
		sampleFrames = bufferSize;
	}

	double begin = GetCPUTime();
	for (VstInt32 i = 0; i < sampleFrames; i++)
	{
		while (MidiQueue.HasEvents() && MidiQueue.GetEventTime() <= i)
		{
			processEvent (MidiQueue.GetNextEvent());
		}
#if reaper_extensions
		while (ParameterQueue.HasEvents() && ParameterQueue.GetEventTime() <= i)
		{
			processEvent (ParameterQueue.GetNextEvent());
		}
#endif
		fillBuffer (buffer, 1, i);
		if (out1)
		{
			out1[i] = buffer[i*2+0] / (sampletype)32768;
			if (in1)
			{
				out1[i] += in1[i];
			}
			if (DCBlock >= 0.5)
			{
				out1[i] = (sampletype)dcf[0].Process(out1[i]);
			}
			out1[i] = out1[i] * Volume;
#ifdef demo
			if (time(NULL) >= startTime + 600)
			{
				out1[i] += ((rand() / (sampletype)RAND_MAX) / (sampletype)256);
			}
#endif
			vu[0] = out1[i];
		}
		if (out2)
		{
			out2[i] = buffer[i*2+1] / (sampletype)32768;
			if (in2)
			{
				out2[i] += in2[i];
			}
			if (DCBlock >= 0.5)
			{
				out2[i] = (sampletype)dcf[1].Process(out2[i]);
			}
			out2[i] = out2[i] * Volume;
#ifdef demo
			if (time(NULL) >= startTime + 600)
			{
				out2[i] += ((rand() / (sampletype)RAND_MAX) / (sampletype)256);
			}
#endif
			vu[1] = out2[i];
		}
	}
	while (MidiQueue.HasEvents())
	{
		processEvent (MidiQueue.GetNextEvent());
	}
#if reaper_extensions
	while (ParameterQueue.HasEvents())
	{
		processEvent (ParameterQueue.GetNextEvent());
	}
#endif
	double end = GetCPUTime();
	calculateCPULoad (begin, end, sampleFrames);
	lock.release();
}

void OPL3GM::calculateCPULoad (double begin, double end, int numsamples)
{
	double freq = GetCPUFrequency();
	double GenerateDuration = (end - begin) / freq;
	double BufferDuration = numsamples * (1.0 / getSampleRate ());
	CPULoad = (GenerateDuration / BufferDuration) * 100.0;
}

void OPL3GM::fillBuffer (short *bufpos, int length, int offset)
{
	if (!bufpos)
	{
		return;
	}
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
			bufpos[i*2+0] = (short)ls;
			bufpos[i*2+1] = (short)rs;
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
	lock.acquire();
	if (bypassed || !ev)
	{
		if (ev && hi.ReceiveEvents)
		{
			sendVstEventsToHost (ev);
		}
		lock.release();
		return 0;
	}

	if (PushMidi >= 0.5)
	{
		VstInt32 eventCount = ev->numEvents;
		if (eventCount > evbufsize)
		{
			eventCount = evbufsize;
		}
		for (VstInt32 i = 0; i < eventCount; i++)
		{
#if VST_2_4_EXTENSIONS
			if (ev->events[i]->type == kVstSysExType)
			{
				processEvent (ev->events[i]);
				continue;
			}
#endif
			if (!MidiQueue.EnqueueEvent (ev->events[i]))
			{
				break;
			}
		}
	}
	else
	{
		for (VstInt32 i = 0; i < ev->numEvents; i++)
		{
			processEvent (ev->events[i]);
		}
	}
	if (hi.ReceiveEvents)
	{
		sendVstEventsToHost (ev);
	}
	lock.release();
	return 1;
}

void OPL3GM::processEvent (VstEvent* ev)
{
	if (!ev)
	{
		return;
	}
	if (ev->type == kVstMidiType)
	{
		VstMidiEvent* event = (VstMidiEvent*)ev;
		char* midiData = event->midiData;
		sendMidi (midiData);
	}
#if VST_2_4_EXTENSIONS
	else if (ev->type == kVstSysExType)
	{
		VstMidiSysexEvent* event = (VstMidiSysexEvent*)ev;
		if (synth)
		{
			synth->midi_write_sysex((unsigned char *)event->sysexDump, event->dumpBytes);
		}
	}
#endif
#if reaper_extensions
	else if (ev->type == kVstParameterType)
	{
		VstParameterEvent* event = (VstParameterEvent*)ev;
		setParameter (event->index, event->value);
	}
#endif
}

void OPL3GM::sendMidi (char *data)
{
	if (!data)
	{
		return;
	}
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
	lock.acquire();
	if (buffer)
	{
		memset(buffer, 0, (2*bufferSize)*sizeof(short));
		lock.release();
		return 1;
	}
	lock.release();
	return 0;
}

VstInt32 OPL3GM::stopProcess ()
{
	lock.acquire();
	if (buffer)
	{
		memset(buffer, 0, (2*bufferSize)*sizeof(short));
		lock.release();
		return 1;
	}
	lock.release();
	return 0;
}
