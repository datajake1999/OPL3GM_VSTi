#ifndef DCFILTER_H
#define DCFILTER_H

class DCFilter {
public:
	DCFilter();
	void SetRate(double rate);
	double Process(double in);
private:
	double pole;
	double LastIn;
	double LastOut;
};

#endif
