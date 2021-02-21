#include "OPL3GM.h"

AudioEffect* createEffectInstance (audioMasterCallback audioMaster)
{
	return new OPL3GM (audioMaster);
}
