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
}

void DCFilter::SetRate(double rate)
{
	double wn = M_PI * (10.0 / rate);
	double b0 = 1.0 / (1.0 + wn);
	pole = (1.0 - wn) * b0;
}

double DCFilter::Process(double in)
{
	double out = (in - LastIn) + (pole * LastOut);
	if (fabs(out) < 0.0001)
	{
		out = 0;
	}
	LastIn = in;
	LastOut = out;
	return out;
}
