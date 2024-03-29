//
// Copyright (C) 2015 Alexey Khokholov (Nuke.YKT)
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//

#ifndef INTERFACE_H
#define INTERFACE_H

class fm_chip {
public:
	virtual int fm_init(unsigned int rate) = 0;
	virtual void fm_writereg(unsigned short reg, unsigned char data) = 0;
	virtual void fm_generate(signed short *buffer, unsigned int length) = 0;
	virtual void fm_generate_dosbox(signed short *buffer, unsigned int length) = 0;
	virtual void fm_close() = 0;
};

class midisynth {
public:
	virtual int midi_init(unsigned int rate) = 0;
	virtual void midi_changerate(unsigned int rate) = 0;
	virtual void midi_write(unsigned int data) = 0;
	virtual void midi_write_sysex(unsigned char *buffer, unsigned int length) = 0;
	virtual void midi_panic() = 0;
	virtual void midi_reset() = 0;
	virtual void midi_generate(signed short *buffer, unsigned int length) = 0;
	virtual void midi_generate_dosbox(signed short *buffer, unsigned int length) = 0;
	virtual void midi_close() = 0;
	virtual const char *midi_synthname(void) = 0;
	virtual int midi_getprogram(unsigned int channel) = 0;
	virtual bool midi_loadbank(const char *filename) { return false; }
	virtual void midi_resetbank() { }
	virtual int midi_getvoicecount() = 0;
};

midisynth* getsynth();
fm_chip* getchip();

#endif
