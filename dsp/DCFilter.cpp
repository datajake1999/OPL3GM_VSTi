#include "DCFilter.h"

DCFilter::DCFilter()
{
	LastIn = 0;
	LastOut = 0;
}

double DCFilter::Process(double in)
{
	double out = (in - LastIn) + (0.999 * LastOut);
	LastIn = in;
	LastOut = out;
	return out;
}
