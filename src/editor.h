/*
OPL3GM VSTi Editor
Copyright (C) 2021-2026  Datajake

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

#ifndef EDITOR_H
#define EDITOR_H

#include <public.sdk/source/vst2.x/aeffeditor.h>

struct KeyboardInfo
{
	AudioEffectX* Effect;
	VstInt32 Channel;
	VstInt32 Octave;
	VstInt32 Velocity;
	VstInt32 Program;
	VstInt32 BendMSB;
	VstInt32 BendLSB;
};

class Editor : public AEffEditor
{
public:
	Editor (AudioEffect* effect);
	~Editor ();
	virtual bool getRect (ERect** rect);
	virtual bool open (void* ptr);
	virtual void close ();
	virtual void idle ();
	virtual void refresh ();
private:
	ERect vstrect;
	void* dlg;
	bool dirty;
	KeyboardInfo keyboard;
};

#endif
