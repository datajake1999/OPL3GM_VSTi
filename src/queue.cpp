/*
VST Event Queue
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

#include "queue.h"

EventQueue::EventQueue()
{
	memset(Events, 0, sizeof(Events));
	Write = 0;
	Read = 0;
	Count = 0;
}

bool EventQueue::EnqueueEvent(VstEvent* ev)
{
	if (Count == EVBUFSIZE || !ev)
	{
		return false;
	}
	memcpy(&Events[Write], ev, sizeof(VstEvent));
	Write++;
	Write = Write%EVBUFSIZE;
	Count++;
	if (Count > EVBUFSIZE)
	{
		Count = EVBUFSIZE;
	}
	return true;
}

VstEvent* EventQueue::GetNextEvent()
{
	if (Count == 0)
	{
		return NULL;
	}
	VstEvent* ev = &Events[Read];
	Read++;
	Read = Read%EVBUFSIZE;
	Count--;
	if (Count < 0)
	{
		Count = 0;
	}
	return ev;
}

bool EventQueue::HasEvents()
{
	if (Count > 0)
	{
		return true;
	}
	return false;
}

VstInt32 EventQueue::GetEventCount()
{
	return Count;
}

VstInt32 EventQueue::GetEventTime()
{
	if (Count == 0)
	{
		return -1;
	}
	return Events[Read].deltaFrames;
}

VstInt32 EventQueue::GetEventTimeAt(VstInt32 ahead)
{
	if (Count == 0)
	{
		return -1;
	}
	VstInt32 num = (Read+ahead)%EVBUFSIZE;
	return Events[num].deltaFrames;
}

void EventQueue::Flush(bool clear)
{
	if (clear)
	{
		memset(Events, 0, sizeof(Events));
	}
	Write = 0;
	Read = 0;
	Count = 0;
}
