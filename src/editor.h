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

#ifndef EDITOR_H
#define EDITOR_H

#include <public.sdk/source/vst2.x/aeffeditor.h>

class Editor : public AEffEditor
{
public:
	Editor (AudioEffect* effect);
	~Editor ();
	virtual bool getRect (ERect** rect);
	virtual bool open (void* ptr);
	virtual void close ();
	virtual void idle ();
	virtual bool onKeyDown (VstKeyCode& keyCode);
	virtual bool onKeyUp (VstKeyCode& keyCode);
private:
	ERect vstrect;
};

#endif
