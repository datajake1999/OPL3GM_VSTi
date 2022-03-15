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
}

VstEvent *EventQueue::GetNextEvent()
{
	VstEvent *ev = &Events[Read];
	Read++;
	Read = Read%evbufsize;
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

VstInt32 EventQueue::GetEventTime()
{
	return Events[Read].deltaFrames;
}

VstInt32 EventQueue::GetEventTimeAt(VstInt32 ahead)
{
	VstInt32 num = (Read+ahead)%evbufsize;
	return Events[num].deltaFrames;
}

void EventQueue::Flush()
{
	memset(Events, 0, sizeof(Events));
	Write = 0;
	Read = 0;
}
