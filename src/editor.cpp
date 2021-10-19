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

#include "editor.h"

Editor::Editor (AudioEffect* effect)
: AEffEditor (effect)
{
	memset(&vstrect, 0, sizeof(vstrect));
	if (effect)
	{
		effect->setEditor (this);
	}
}

Editor::~Editor ()
{
}

bool Editor::getRect (ERect** rect)
{
	*rect = &vstrect;
	return true;
}

bool Editor::open (void* ptr)
{
	AEffEditor::open (ptr);
	return true;
}

void Editor::close ()
{
	AEffEditor::close ();
}

void Editor::idle ()
{
}

bool Editor::onKeyDown (VstKeyCode& keyCode)
{
	return false;
}

bool Editor::onKeyUp (VstKeyCode& keyCode)
{
	return false;
}