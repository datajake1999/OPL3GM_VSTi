#include "DCFilter.h"
#include <math.h>

DCFilter::DCFilter()
{
	LastIn = 0;
	LastOut = 0;
}

double DCFilter::Process(double in)
{
	double out = (in - LastIn) + (0.999 * LastOut);
	if (fabs(out) < 0.0001)
	{
		out = 0;
	}
	LastIn = in;
	LastOut = out;
	return out;
}
