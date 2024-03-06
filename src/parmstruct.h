/*
VST Parameter Structure
Copyright (C) 2021-2024  Datajake

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

#ifndef PARMSTRUCT_H
#define PARMSTRUCT_H

#include <pluginterfaces/vst2.x/aeffectx.h>

#if VST_FORCE_DEPRECATED
#define kVstParameterType DECLARE_VST_DEPRECATED (kVstParameterType)
#endif

struct VstParameterEvent
{
	VstInt32 type;
	VstInt32 byteSize;
	VstInt32 deltaFrames;
	VstInt32 flags;
	VstInt32 index;
	float value;
	char data[8];
};

#endif
