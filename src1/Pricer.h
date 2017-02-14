#pragma once
//#include "Stdafx.h"
#include <cmath>
#ifndef Pi 
#define Pi 3.141592653589793238462643 
#endif 

// The cumulative normal distribution function 
double CND( double X )
{
	double L, K, w ;
	double const a1 = 0.31938153, a2 = -0.356563782, a3 = 1.781477937;
	double const a4 = -1.821255978, a5 = 1.330274429;
	L = fabs(X);
	K = 1.0 / (1.0 + 0.2316419 * L);
	w = 1.0 - 1.0 / sqrt(2 * Pi) * exp(-L *L / 2) * (a1 * K + a2 * K *K + a3 * pow(K,3) + a4 * pow(K,4) + a5 * pow(K,5));
	if (X < 0 ){
		w= 1.0 - w;
	}
	return w;
}
// The Black and Scholes (1973) Stock option formula
double BlackScholes(bool IsCall, double S, double X, double T, double r,double v)
{
	double d1, d2;

	d1=(log(S/X)+(r+v*v/2)*T)/(v*sqrt(T));
	d2=d1-v*sqrt(T);
	double ret = 0;
	if(IsCall)
		ret = S *CND(d1)-X * exp(-r*T)*CND(d2);
	else 
		ret = X * exp(-r * T) * CND(-d2) - S * CND(-d1);

	return ret;
}

double normalDistribution(double x)
{
	static const double RT2PI = sqrt(4.0*acos(0.0));
	static const double SPLIT = 10.0/sqrt(2.0);
	static const double a[] = {220.206867912376,221.213596169931,112.079291497871,33.912866078383,6.37396220353165,0.700383064443688,3.52624965998911e-02};
	static const double b[] = {440.413735824752,793.826512519948,637.333633378831,296.564248779674,86.7807322029461,16.064177579207,1.75566716318264,8.83883476483184e-02};

	const double z = fabs(x);
	double Nz = 0.0;

	// if z outside these limits then value effectively 0 or 1 for machine precision
	if(z<=37.0)
	{
		// NDash = N'(z) * sqrt{2\pi}
		const double NDash = exp(-z*z/2.0)/RT2PI;
		if(z<SPLIT)
		{
			const double Pz = (((((a[6]*z + a[5])*z + a[4])*z + a[3])*z + a[2])*z + a[1])*z + a[0];
			const double Qz = ((((((b[7]*z + b[6])*z + b[5])*z + b[4])*z + b[3])*z + b[2])*z + b[1])*z + b[0];
			Nz = RT2PI*NDash*Pz/Qz;
		}
		else
		{
			const double F4z = z + 1.0/(z + 2.0/(z + 3.0/(z + 4.0/(z + 13.0/20.0))));
			Nz = NDash/F4z;
		}
	}
	return x>=0.0 ? 1-Nz : Nz;
}

// return the value of a call option using the black scholes formula
double callOptionPrice(double S,double t,double X,double r,double sigma)
{
	if(S<1.e-14)return 0.; // check if asset worthless
	if(sigma<1.e-14) // check if sigma zero
	{
		if(S<X*exp(-r*(t)))return 0.;
		else return S-X*exp(-r*(t));
	}
	if(fabs(t)<1.e-14) // check if we are at maturity
	{
		if(S<X)return 0.;
		else return S-X;
	}
	// calculate option price
	double d1=(log(S/X) + (r+sigma*sigma/2.)*(t))/(sigma*sqrt(t));
	double d2=(log(S/X) + (r-sigma*sigma/2.)*(t))/(sigma*sqrt(t));
	double ret = normalDistribution(d1)*S - normalDistribution(d2)*X*exp(-r*(t));
	return ret;
}
