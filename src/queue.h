/*
VST Event Queue
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

#ifndef QUEUE_H
#define QUEUE_H

#include <pluginterfaces/vst2.x/aeffectx.h>

#define EVBUFSIZE 1024

class EventQueue
{
public:
	EventQueue();
	bool EnqueueEvent(VstEvent* ev);
	VstEvent* GetNextEvent();
	bool HasEvents();
	VstInt32 GetEventCount();
	VstInt32 GetEventTime();
	VstInt32 GetEventTimeAt(VstInt32 ahead);
	void Flush(bool clear);
private:
	VstEvent Events[EVBUFSIZE];
	VstInt32 Write;
	VstInt32 Read;
	VstInt32 Count;
};

#endif
