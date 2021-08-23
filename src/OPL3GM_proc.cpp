#include "OPL3GM.h"

void OPL3GM::setSampleRate (float sampleRate)
{
	AudioEffectX::setSampleRate (sampleRate);
	if (synth)
	{
		synth->midi_close();
		delete synth;
		synth = NULL;
	}
	synth = getsynth();
	if (synth)
	{
		if (!synth->midi_init((int)sampleRate))
		{
			delete synth;
			synth = NULL;
		}
	}
}

void OPL3GM::setBlockSize (VstInt32 blockSize)
{
	AudioEffectX::setBlockSize (blockSize);
	if (buffer)
	{
		delete buffer;
		buffer = NULL;
	}
	bufferSize = blockSize;
	buffer = new short[2*bufferSize];
	if (buffer)
	{
		memset(buffer, 0, 4*bufferSize);
	}
}

void OPL3GM::resume ()
{
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
	if (synth)
	{
		synth->midi_generate(buffer, sampleFrames);
	}
	for (int i = 0; i < sampleFrames; i++)
	{
		*out1 = buffer[0] / 32768.0f;
		*out2 = buffer[1] / 32768.0f;
		*out1 = *out1 * Volume;
		*out2 = *out2 * Volume;
		buffer += 2;
		*out1++;
		*out2++;
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
	if (synth)
	{
		synth->midi_generate(buffer, sampleFrames);
	}
	for (int i = 0; i < sampleFrames; i++)
	{
		*out1 = buffer[0] / 32768.0;
		*out2 = buffer[1] / 32768.0;
		*out1 = *out1 * Volume;
		*out2 = *out2 * Volume;
		buffer += 2;
		*out1++;
		*out2++;
	}
	buffer -= sampleFrames*2;
}

VstInt32 OPL3GM::processEvents (VstEvents* ev)
{
	for (VstInt32 i = 0; i < ev->numEvents; i++)
	{
		if ((ev->events[i])->type == kVstMidiType)
		{
			VstMidiEvent* event = (VstMidiEvent*)ev->events[i];
			char* midiData = event->midiData;
			unsigned char byte1 = midiData[0];
			unsigned char byte2 = midiData[1];
			unsigned char byte3 = midiData[2];
			if (Transpose != 0)
			{
				unsigned char type = byte1 & 0xf0;
				unsigned char channel = byte1 & 0x0f;
				if (type == 0x80 || type == 0x90)
				{
					if (channel != 9)
					{
						float note = byte2;
						note = note + (int)Transpose;
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
	}
	return 1;
}
