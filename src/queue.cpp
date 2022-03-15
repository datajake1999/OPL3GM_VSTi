/*
Event Queue
Copyright (C) 2021-2022  Datajake

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
	Flush();
}

void EventQueue::EnqueueEvent(VstEvent *ev)
{
	memcpy(&Events[Write], ev, sizeof(VstEvent));
	Write++;
	Write = Write%evbufsize;
	Count++;
	if (Count > evbufsize)
	{
		Count = evbufsize;
	}
}

VstEvent *EventQueue::GetNextEvent()
{
	if (Read == Write)
	{
		return NULL;
	}
	VstEvent *ev = &Events[Read];
	Read++;
	Read = Read%evbufsize;
	Count--;
	if (Count < 0)
	{
		Count = 0;
	}
	return ev;
}

bool EventQueue::HasEvents()
{
	if (Read == Write)
	{
		return false;
	}
	return true;
}

VstInt32 EventQueue::GetEventCount()
{
	return Count;
}

VstInt32 EventQueue::GetEventTime()
{
	if (Read == Write)
	{
		return -1;
	}
	return Events[Read].deltaFrames;
}

VstInt32 EventQueue::GetEventTimeAt(VstInt32 ahead)
{
	if (Read == Write)
	{
		return -1;
	}
	VstInt32 num = (Read+ahead)%evbufsize;
	return Events[num].deltaFrames;
}

void EventQueue::Flush()
{
	memset(Events, 0, sizeof(Events));
	Write = 0;
	Read = 0;
	Count = 0;
}
