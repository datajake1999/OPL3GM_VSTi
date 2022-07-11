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
	void SetRate(double rate);
	double Process(double in);
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
