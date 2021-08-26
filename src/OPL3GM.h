#ifndef OPL3GM_H
#define OPL3GM_H

#include "../interface.h"
#include <public.sdk/source/vst2.x/audioeffectx.h>

enum
{
	// Global
	kNumPrograms = 1,

	// Parameters Tags
	kVolume = 0,
	kTranspose,

	kNumParams
};

class OPL3GM : public AudioEffectX
{
public:
	OPL3GM (audioMasterCallback audioMaster);
	~OPL3GM ();
	virtual void setParameter (VstInt32 index, float value);
	virtual float getParameter (VstInt32 index);
	virtual void getParameterDisplay (VstInt32 index, char* text);
	virtual void getParameterLabel (VstInt32 index, char* label);
	virtual void getParameterName (VstInt32 index, char* text);
	virtual void setProgramName (char* name);
	virtual void getProgramName (char* name);
	virtual bool getProgramNameIndexed (VstInt32 category, VstInt32 index, char* text);
	virtual bool getOutputProperties (VstInt32 index, VstPinProperties* properties);
	virtual bool getEffectName (char* name);
	virtual bool getVendorString (char* text);
	virtual bool getProductString (char* text);
	virtual VstInt32 getVendorVersion ();
	virtual VstInt32 canDo (char* text);
	virtual VstInt32 getNumMidiInputChannels ();
	virtual VstInt32 getNumMidiOutputChannels ();
	virtual void setSampleRate (float sampleRate);
	virtual void setBlockSize (VstInt32 blockSize);
	virtual void resume ();
	virtual void processReplacing (float** inputs, float** outputs, VstInt32 sampleFrames);
	virtual void processDoubleReplacing (double** inputs, double** outputs, VstInt32 sampleFrames);
	virtual VstInt32 processEvents (VstEvents* events);
private:
	midisynth *synth;
	short *buffer;
	int bufferSize;
	float Volume;
	float Transpose;
	char ProgramName[32];
};

#endif
