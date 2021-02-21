//
// Copyright (C) 2015-2017 Alexey Khokholov (Nuke.YKT)
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

#include <string.h>
#include "opl3class.h"


int opl3class::fm_init(unsigned int rate) {
    OPL3_Reset(&chip, rate);

	return 1;
}

void opl3class::fm_writereg(unsigned short reg, unsigned char data) {
    OPL3_WriteRegBuffered(&chip, reg, data);
}

void opl3class::fm_generate(signed short *buffer, unsigned int len) {
    OPL3_GenerateStream(&chip, buffer, len);
}

fm_chip *getchip() {
	opl3class *chip = new opl3class;
	return chip;
}