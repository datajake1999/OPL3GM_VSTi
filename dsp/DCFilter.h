/*
DC Blocking Filter and Noise Gate
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

#ifndef DCFILTER_H
#define DCFILTER_H

#define fltfreq 10.0
#define thresh -66.0
#define fadeinlen 1.0
#define fadeoutlen 100.0
#define holdlen 10.0

class DCFilter {
public:
	DCFilter();
	void ResetState();
	void SetRate(double rate);
	double ProcessDC(double in);
	double ProcessGate(double in);
private:
	double pole;
	double LastIn;
	double LastOut;
	bool open;
	double envelope;
	double threshold;
	double FadeIn;
	double FadeOut;
	double HoldLength;
	double HoldTime;
};

#endif
