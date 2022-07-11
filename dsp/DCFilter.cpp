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

double DCFilter::Process(double in)
{
	double out = (in - LastIn) + (pole * LastOut);
	LastIn = in;
	LastOut = out;
	if (fabs(out) > threshold)
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
	out = out * envelope;
	return out;
}
