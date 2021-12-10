#ifndef DCFILTER_H
#define DCFILTER_H

class DCFilter {
public:
	DCFilter();
	double Process(double in);
private:
	double LastIn;
	double LastOut;
};

#endif
