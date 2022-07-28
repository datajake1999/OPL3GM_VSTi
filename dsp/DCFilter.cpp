/*
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

#include "DCFilter.h"
#include <math.h>
#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

DCFilter::DCFilter()
{
	pole = 1;
	LastIn = 0;
	LastOut = 0;
	open = false;
	envelope = 0;
	threshold = 0;
	FadeIn = 0;
	FadeOut = 0;
	HoldLength = 0;
	HoldTime = 0;
}

void DCFilter::ResetState()
{
	LastIn = 0;
	LastOut = 0;
	open = false;
	envelope = 0;
	HoldTime = 0;
}

void DCFilter::SetRate(double rate)
{
	double wn = M_PI * (fltfreq / rate);
	double b0 = 1.0 / (1.0 + wn);
	pole = (1.0 - wn) * b0;
	threshold = pow(10.0, thresh / 20.0);
	FadeIn = 1.0 / pow(10.0, 1.0 / (rate * fadeinlen / 1000.0));
	FadeOut = 1.0 / pow(10.0, 1.0 / (rate * fadeoutlen / 1000.0));
	HoldLength = rate * holdlen / 1000.0;
}

double DCFilter::ProcessDC(double in)
{
	double out = (in - LastIn) + (pole * LastOut);
	LastIn = in;
	LastOut = out;
	return out;
}

double DCFilter::ProcessGate(double in)
{
	if (fabs(in) > threshold)
	{
		open = true;
		HoldTime = 0;
	}
	else
	{
		if (HoldTime++ > HoldLength)
		{
			open = false;
		}
	}
	if (open)
	{
		envelope = (envelope * FadeIn) + (1.0 - FadeIn);
	}
	else
	{
		envelope = envelope * FadeOut;
	}
	return in * envelope;
}
