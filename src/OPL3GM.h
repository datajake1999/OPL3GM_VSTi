#ifndef OPL3GMVST
#define OPL3GMVST

#include "..\interface.h"
#include <public.sdk/source/vst2.x/audioeffectx.h>

enum
{
	kVolume = 0,
	kNumParams
};

class OPL3GM : public AudioEffectX
{
public:
	OPL3GM (audioMasterCallback audioMaster);
	~OPL3GM ();
	virtual void setSampleRate (float sampleRate);
	virtual void setBlockSize (VstInt32 blockSize);
	virtual void processReplacing (float** inputs, float** outputs, VstInt32 sampleFrames);
	virtual VstInt32 processEvents (VstEvents* events);
	virtual void setParameter (VstInt32 index, float value);
	virtual void setParameterAutomated (VstInt32 index, float value);
	virtual float getParameter (VstInt32 index);
	virtual void getParameterDisplay (VstInt32 index, char* text);
	virtual void getParameterLabel (VstInt32 index, char* label);
	virtual void getParameterName (VstInt32 index, char* text);
	virtual void setProgramName (char* name);
	virtual void getProgramName (char* name);
	virtual bool getEffectName (char* name);
	virtual bool getVendorString (char* text);
	virtual bool getProductString (char* text);
	virtual VstInt32 getVendorVersion ();
	virtual VstInt32 canDo (char* text);
	virtual VstInt32 getNumMidiInputChannels ();
	virtual VstInt32 getNumMidiOutputChannels ();
private:
	midisynth *synth;
	short *buffer;
	int bufferSize;
	float volume;
	char ProgramName[32];
};

#endif
