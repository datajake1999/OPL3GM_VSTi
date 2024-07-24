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

#ifndef NVDAHELPER_LOCK_H
#define NVDAHELPER_LOCK_H

/**
* A class that provides a locking mechonism on objects.
* The lock is reeentrant for the same thread.
*/

class LockableObject {
private:
	void* cs;
public:
	LockableObject();
	~LockableObject();
	void acquire();
	void release();
};

#endif
