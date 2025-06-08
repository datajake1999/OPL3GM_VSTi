/*
This file is a part of the NVDA project.
URL: http://www.nvda-project.org/
Copyright 2006-2010 NVDA contributers.

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

#include "lock.h"
#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#endif

LockableObject::LockableObject()
{
#ifdef _WIN32
	cs = NULL;
	cs = new CRITICAL_SECTION;
	if (cs)
	{
		InitializeCriticalSection((LPCRITICAL_SECTION)cs);
	}
#endif
}

LockableObject::~LockableObject()
{
#ifdef _WIN32
	if (cs)
	{
		DeleteCriticalSection((LPCRITICAL_SECTION)cs);
		delete cs;
		cs = NULL;
	}
#endif
}

/**
* Acquires access (possibly waighting until its free).
*/

void LockableObject::acquire()
{
#ifdef _WIN32
	if (cs)
	{
		EnterCriticalSection((LPCRITICAL_SECTION)cs);
	}
#endif
}

/**
* Releases exclusive access of the object.
*/

void LockableObject::release()
{
#ifdef _WIN32
	if (cs)
	{
		LeaveCriticalSection((LPCRITICAL_SECTION)cs);
	}
#endif
}

/**
* Attempts to acquire access (returns true on success).
*/

bool LockableObject::tryAcquire()
{
#ifdef _WIN32
	if (cs)
	{
		if (TryEnterCriticalSection((LPCRITICAL_SECTION)cs))
		{
			return true;
		}
	}
#endif
	return false;
}
