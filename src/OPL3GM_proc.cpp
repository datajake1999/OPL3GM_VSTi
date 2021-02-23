#include "OPL3GM.h"

void OPL3GM::resume ()
{
	if (synth)
	{
		delete synth;
		synth = NULL;
	}
	synth = getsynth();
	if (synth)
	{
		synth->midi_init((unsigned int)AudioEffectX::updateSampleRate());
	}
	if (buffer)
	{
		delete buffer;
		buffer = NULL;
	}
	bufferSize = AudioEffectX::updateBlockSize();
	buffer = new short[2*bufferSize];
	if (buffer)
	{
		memset(buffer, 0, sizeof(buffer));
	}
	AudioEffectX::resume();
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
		*out1 = *out1 * volume;
		*out2 = *out2 * volume;
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
			unsigned char byte1 = event->midiData[0] & 0xFF;
			unsigned char byte2 = event->midiData[1] & 0x7F;
			unsigned char byte3 = event->midiData[2] & 0x7F;
			if (synth)
			{
				synth->midi_write((byte3<<16) | (byte2<<8) | byte1);
			}
		}
	}
	return 1;
}
