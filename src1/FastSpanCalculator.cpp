//#include "Stdafx.h"
#include "FastSpanCalculator.h"
#include <R.h>
#include <Rinternals.h>
#include <Rdefines.h> 
#include <Rcpp.h>


using namespace MARGIN;
using namespace std;
using namespace Rcpp;

// Objects
std::map<int, Portfolio*> Portfolios;
//std::map<int, MarginCalculator* > Calculators;
Portfolio* PORTFOLIO = NULL;
MarginCalculator* CALCULATOR = NULL;

int  InitMarginCalculator(char *cXMLFileName)
{
	string XMLFileName(cXMLFileName);
	if (CALCULATOR==NULL)
	{
		CALCULATOR = new MarginCalculator(XMLFileName);
		return 0;
	}
	else
	{	
		cout<<"Calculator already initialized." << endl;
		return -1;
	}
}

bool isMarginCalculatorLoaded(){
  return(CALCULATOR==NULL ? false : true);
}
void unLoadMarginCalculator() {
  CALCULATOR=NULL;
}

int		 NewPortfolio(int newID)
{
	Portfolio* pPortfolio = new Portfolio();
	
	Portfolios.insert(make_pair(newID,pPortfolio));
	
	return newID;
}

int		 SetRT(bool IsRealTime /*= true*/)
{
	if (CALCULATOR == NULL)
		return -1; 
	CALCULATOR->SetRealTime(IsRealTime);
	return 0;
}


//int		_ UpdateOptionScenarioValues(char* Symbol, bool IsCall, bool IsAmerican, double Strike, char* Maturity, double Values[17])
//{
//	string strSymbol(Symbol); 
//	string strMaturity(Maturity);
//	if (CALCULATOR!= NULL)
//	{
//		ExerciseType eType;
//		if (IsAmerican)
//			eType = AMERICAN;
//		else
//			eType = EUROPIAN;
//		return CALCULATOR->SetOptionScenarioValues(strSymbol,IsCall,strMaturity,eType,Strike,Values);
//	}
//	return -2;
//}
//int		_ UpdateFuturesScenarioValues(char* Symbol, char* Maturity, double Values[17])
//{
//	string strSymbol(Symbol); 
//	string strMaturity(Maturity);
//	if (CALCULATOR!= NULL)
//		return CALCULATOR->SetFuturesScenarioValues(strSymbol,strMaturity,Values);
//	return -2;
//}

int		 UpdateOptionScenarios(char* Symbol, bool IsCall, bool IsAmerican, double Strike, char* Maturity, double UnderlyingPrice, double Price, double Volatility, double RiskFreeRate)
{
	string strSymbol(Symbol); 
	string strMaturity(Maturity);
	if (CALCULATOR!= NULL)
	{
		ExerciseType eType;
		if (IsAmerican)
			eType = AMERICAN;
		else
			eType = EUROPIAN;
		return CALCULATOR->ReCalculateOptionScenarioValues(strSymbol,IsCall,strMaturity,eType,Strike,UnderlyingPrice,Price,Volatility,RiskFreeRate);
	}
	return -3;
}

int		 UpdateOptionDelta(char* Symbol, bool IsCall, bool IsAmerican, double Strike, char* Maturity, double Delta)
{
	string strSymbol(Symbol); 
	string strMaturity(Maturity);
	if (CALCULATOR!= NULL)
	{
		ExerciseType eType;
		if (IsAmerican)
			eType = AMERICAN;
		else
			eType = EUROPIAN;
		return CALCULATOR->SetOptionDelta(strSymbol,IsCall,strMaturity,eType,Strike, Delta);
	}
	return -3;
}

int  AddtoPortfolio(char* cSymbol, bool IsOption, bool IsCall, bool IsAmerican, double Strike, char* cMaturity, double Count, double Price/* = 0*/, bool IsOrder, bool IsIntraDay, bool Effectpremium /*= true*/)
{
	try
	{
		string Symbol(cSymbol); 
		string Maturity(cMaturity);
		if (CALCULATOR!=NULL)
		{
			if (PORTFOLIO == NULL)
				PORTFOLIO = new Portfolio();
		
			Instrument* pInstrument = new Instrument();
			pInstrument->Count = Count;
		
			if (IsOption)
				pInstrument->Type = OPTION;
			else
				pInstrument->Type = FUTURES;
		
			if (IsAmerican)
				pInstrument->EType = AMERICAN;
			else
				pInstrument->EType = EUROPIAN;
		
			pInstrument->IsCall = IsCall;
			pInstrument->Maturity = Maturity;
			pInstrument->Strike = Strike;
			pInstrument->Symbol = Symbol;
			pInstrument->IsOrder = IsOrder;
			pInstrument->IsIntraDay = IsIntraDay;
			pInstrument->Price = Price;
			pInstrument->EffectPremium = Effectpremium;

			PORTFOLIO->AddInstrument(pInstrument);

			return 0;
		}
		else
		{
			cout<<"Calculator not initialized!!!" << endl;
			return -1;
		}
	}
	catch(const std::exception& ex)
	{
		ofstream* outputFile = new ofstream("hataAdd.txt", fstream::app);
		*outputFile << ex.what()<< endl;
		delete outputFile;
		return -1;
	}
}

int	 AddtoPortfoliowithOrderPrice(char* cSymbol, bool IsOption, bool IsCall,
                                  bool IsAmerican, double Strike, char* cMaturity, 
                                  double Count, double Price /*= -1*/, double OrderExecutionPrice /*= -1*/, 
                                  bool IsOrder /*= false*/, bool IsIntraDay /*= true*/, bool Effectpremium /*= true*/)
{
	try
	{
		string Symbol(cSymbol); 
		string Maturity(cMaturity);
		if (CALCULATOR!=NULL)
		{
			if (PORTFOLIO == NULL)
				PORTFOLIO = new Portfolio();

			Instrument* pInstrument = new Instrument();
			pInstrument->Count = Count;

			if (IsOption)
				pInstrument->Type = OPTION;
			else
				pInstrument->Type = FUTURES;

			if (IsAmerican)
				pInstrument->EType = AMERICAN;
			else
				pInstrument->EType = EUROPIAN;

			pInstrument->IsCall = IsCall;
			pInstrument->Maturity = Maturity;
			pInstrument->Strike = Strike;
			pInstrument->Symbol = Symbol;
			pInstrument->IsOrder = IsOrder;
			pInstrument->IsIntraDay = IsIntraDay;
			pInstrument->Price = Price;
			pInstrument->OrderExecutionPrice = OrderExecutionPrice;
			pInstrument->EffectPremium = Effectpremium;

			PORTFOLIO->AddInstrument(pInstrument);

			return 0;
		}
		else
		{
			cout<<"Calculator not initialized!!!" << endl;
			return -1;
		}
	}
	catch(const std::exception& ex)
	{
		ofstream* outputFile = new ofstream("hataAdd.txt", fstream::app);
		*outputFile << ex.what()<< endl;
		delete outputFile;
		return -1;
	}
}

int  AddtoPortfolioT(int PortfolioID, char* cSymbol, bool IsOption, bool IsCall, bool IsAmerican, double Strike, char* cMaturity, double Count, double Price/* = 0*/, bool IsOrder, bool IsIntraDay, bool Effectpremium /*= true*/)
{
	std::map<int,Portfolio*>::iterator it;
	it = Portfolios.find(PortfolioID);
	if (it == Portfolios.end())
	{
		cout<<"Wrong portfolio ID!!!" << endl;
		return -2;
	}
	Portfolio* pPortfolio = it->second;

	string Symbol(cSymbol); 
	string Maturity(cMaturity);
	if (CALCULATOR!=NULL)
	{
	/*	if (PORTFOLIO == NULL)
			PORTFOLIO = new Portfolio();*/

		Instrument* pInstrument = new Instrument();
		pInstrument->Count = Count;

		if (IsOption)
			pInstrument->Type = OPTION;
		else
			pInstrument->Type = FUTURES;

		if (IsAmerican)
			pInstrument->EType = AMERICAN;
		else
			pInstrument->EType = EUROPIAN;

		pInstrument->IsCall = IsCall;
		pInstrument->Maturity = Maturity;
		pInstrument->Strike = Strike;
		pInstrument->Symbol = Symbol;
		pInstrument->IsOrder = IsOrder;
		pInstrument->IsIntraDay = IsIntraDay;
		pInstrument->Price = Price;
		pInstrument->EffectPremium = Effectpremium;

		pPortfolio->AddInstrument(pInstrument);

		return 0;
	}
	else
	{
		cout<<"Calculator not initialized!!!" << endl;
		return -1;
	}
}

int	 GetPriceScanRate(char* cSymbol, bool IsOption, bool IsCall, bool IsAmerican, double Strike, char* cMaturity, double& PriceScan)
{
	string Symbol(cSymbol); 
	string Maturity(cMaturity);
	PriceScan = 0;
	if (CALCULATOR!=NULL)
	{
		Instrument* pInstrument = new Instrument();
		if (IsOption)
			pInstrument->Type = OPTION;
		else
			pInstrument->Type = FUTURES;

		if (IsAmerican)
			pInstrument->EType = AMERICAN;
		else
			pInstrument->EType = EUROPIAN;

		pInstrument->IsCall = IsCall;
		pInstrument->Maturity = Maturity;
		pInstrument->Strike = Strike;
		pInstrument->Symbol = Symbol;
		HRESULT hr = CALCULATOR->GetPriceScan(pInstrument,PriceScan);
		if (hr == S_OK)
			return 0;
	}
	return -1;
}

void   ClearPortfolio()
{
	try     
	{
	 
		if (PORTFOLIO != NULL)
		{
			PORTFOLIO->Clear();
			delete PORTFOLIO;
			PORTFOLIO = NULL;
		}
	}
	catch(const std::exception& ex)
	{
		ofstream* outputFile = new ofstream("hataClear.txt", fstream::app);
		*outputFile << ex.what()<< endl;
		delete outputFile;
		return;
	}
}
void   ClearPortfolioT(int PortfolioID)
{
	std::map<int,Portfolio*>::iterator it;
	it = Portfolios.find(PortfolioID);
	if (it == Portfolios.end())
	{
		cout<<"Wrong portfolio ID!!!" << endl;
		return;
	}
	Portfolio* pPortfolio = it->second;
	if (pPortfolio  != NULL)
	{
		pPortfolio ->Clear();
		delete pPortfolio ;
		pPortfolio  = NULL;
	}
}

int   CalculateMargin(double& Initial,double& Maintenance, double& NetOptionValue, 
                      double& NetOrderOptionValue, double& NetIntraDayOptionValue,bool IncludeDMC /*= false*/)
{
	try{
	Margin Result;
	HRESULT hr = CALCULATOR->Calculate(PORTFOLIO,Result,IncludeDMC);
	if (hr != S_OK)
	{
		cout << "A position or order was not set properly!!" <<endl;
		return -1;
	}
	Initial = Result.Initial;
	Maintenance = Result.Maintenance;
	NetOptionValue = Result.NetOptionValue;
	NetOrderOptionValue = Result.NetOrderOptionValue;
	NetIntraDayOptionValue = Result.NetIntraDayOptionValue;
	return 0;
	}
	catch(const std::exception& ex)
	{
		ofstream* outputFile = new ofstream("hata.txt", fstream::app);
		*outputFile << ex.what()<< endl;
		delete outputFile;
		return -1;
	}
}

int   CalculateMarginT(int PortfolioID, double& Initial,double& Maintenance, double& NetOptionValue, double& NetOrderOptionValue, double& NetIntraDayOptionValue,bool IncludeDMC /*= false*/)
{
	std::map<int,Portfolio*>::iterator it;
	it = Portfolios.find(PortfolioID);
	if (it == Portfolios.end())
	{
		cout<<"Wrong portfolio ID!!!" << endl;
		return -2;
	}
	Portfolio* pPortfolio = it->second;
	Margin Result;
	HRESULT hr = CALCULATOR->Calculate(pPortfolio,Result,IncludeDMC);
	if (hr != S_OK)
	{
		cout << "A position or order was not set properly!!" <<endl;
		return -1;
	}
	Initial = Result.Initial;
	Maintenance = Result.Maintenance;
	NetOptionValue = Result.NetOptionValue;
	NetOrderOptionValue = Result.NetOrderOptionValue;
	NetIntraDayOptionValue = Result.NetIntraDayOptionValue;
	return 0;
}

int   CalculateDetailedMargin(double& Scenario,double& IntraSpread,double& InterSpread,double& SOM,double& DMC,double& Total,double& TotalMaintenance)
{
	DetailedMargin Result;
	HRESULT hr = CALCULATOR->CalculateDetailed(PORTFOLIO,Result);
	if (hr != S_OK)
	{
		cout << "Error: One or more position(s) or order(s) are not set properly." <<endl;
		return -1;
	}
	Scenario = Result.Scenario;
	IntraSpread = Result.IntraSpread;
	InterSpread = Result.InterSpread;
	SOM = Result.SOM ;
	DMC = Result.DeliveryMonthCharge;
	Total = Result.Total;
	TotalMaintenance= Result.TotalMaintenance;
	return 0;
}

int	  CalculateWorstCaseOld(double& Initial,double& Maintenance, double& NetOptionValue, double& NetOrderOptionValue, double& NetIntraDayOptionValue, bool DoBrute, bool IncludeDMC, bool withGrouping /*= false*/)
{try{
	Margin Result;
	HRESULT hr;
	MarginEstimator* Estimator = NULL;
	try{
	Estimator = new MarginEstimator(CALCULATOR, PORTFOLIO, withGrouping);
	}
	catch(const std::exception& ex)
	{
		ofstream* outputFile = new ofstream("hataOld.txt", fstream::app);
		*outputFile << ex.what()<< endl;
		delete outputFile;
		return -1;
	}
	try{
	if (DoBrute)
		hr = Estimator->Calculate(BRUTE,Result);
	else
		hr = Estimator->Calculate(ITERATION,Result);
	}
	catch(const std::exception& ex)
	{
		ofstream* outputFile = new ofstream("hataold2.txt", fstream::app);
		*outputFile << ex.what()<< endl;
		delete outputFile;
		return -1;
	}
	Initial = Result.Initial;
	Maintenance = Result.Maintenance;
	NetOptionValue = Result.NetOptionValue;
	NetOrderOptionValue = Result.NetOrderOptionValue;
	NetIntraDayOptionValue = Result.NetIntraDayOptionValue;
	//char *a=new char[Result.CarryOut.size()+1];
	//a[Result.CarryOut.size()]=0;
	//memcpy(a,Result.CarryOut.c_str(),Result.CarryOut.size());
	//*CarryOut = a;
	delete Estimator;
	Estimator = NULL;
	if (hr == S_OK)
		return 0;
	else
		return -1;
}
	catch(const std::exception& ex)
	{
		ofstream* outputFile = new ofstream("hataWorstoldBig.txt", fstream::app);
		*outputFile << ex.what()<< endl;
		delete outputFile;
		return -1;
	}
	

}

int	  CalculateWorstCaseOldT(int PortfolioID, double& Initial,double& Maintenance, double& NetOptionValue, double& NetOrderOptionValue, double& NetIntraDayOptionValue, bool DoBrute, bool IncludeDMC, bool withGrouping /*= false*/)
{
	std::map<int,Portfolio*>::iterator it;
	it = Portfolios.find(PortfolioID);
	if (it == Portfolios.end())
	{
		cout<<"Wrong portfolio ID!!!" << endl;
		return -2;
	}
	Portfolio* pPortfolio = it->second;

	Margin Result;
	HRESULT hr;
	MarginEstimator* Estimator = new MarginEstimator(CALCULATOR, PORTFOLIO, withGrouping);
	if (DoBrute)
		hr = Estimator->Calculate(BRUTE,Result);
	else
		hr = Estimator->Calculate(ITERATION,Result);
	Initial = Result.Initial;
	Maintenance = Result.Maintenance;
	NetOptionValue = Result.NetOptionValue;
	NetOrderOptionValue = Result.NetOrderOptionValue;
	NetIntraDayOptionValue = Result.NetIntraDayOptionValue;
	//char *a=new char[Result.CarryOut.size()+1];
	//a[Result.CarryOut.size()]=0;
	//memcpy(a,Result.CarryOut.c_str(),Result.CarryOut.size());
	//*CarryOut = a;
	delete Estimator;
	Estimator = NULL;
	if (hr == S_OK)
		return 0;
	else
		return -1;

}

int   CalculateWorstCase(double& Initial,double& Maintenance, double& NetOptionValue, double& NetOrderOptionValue, double& NetIntraDayOptionValue, bool DoBrute,char** CarryOut, bool IncludeDMC /*= false*/, bool withGrouping /*= false*/)
{
	try{
	Margin Result;
	HRESULT hr;
	MarginEstimator* Estimator = new MarginEstimator(CALCULATOR, PORTFOLIO, withGrouping);
	if (DoBrute)
		hr = Estimator->Calculate(BRUTE,Result);
	else
		hr = Estimator->Calculate(ITERATION,Result);
	Initial = Result.Initial;
	Maintenance = Result.Maintenance;
	NetOptionValue = Result.NetOptionValue;
	NetOrderOptionValue = Result.NetOrderOptionValue;
	NetIntraDayOptionValue = Result.NetIntraDayOptionValue;
	char *a=new char[Result.CarryOut.size()+1];
	a[Result.CarryOut.size()]=0;
	memcpy(a,Result.CarryOut.c_str(),Result.CarryOut.size());
	*CarryOut = a;
	std::cout<<"WORSTCASE: "<<Result.CarryOut.c_str()<<std::endl;
	delete Estimator;
	Estimator = NULL;
	if (hr == S_OK)
		return 0;
	else
		return -1;
	}
	catch(const std::exception& ex)
	{
		ofstream* outputFile = new ofstream("hataWorst.txt", fstream::app);
		*outputFile << ex.what()<< endl;
		delete outputFile;
		return -1;
	}
}

int   CalculateWorstCaseT(int PortfolioID, double& Initial,double& Maintenance, double& NetOptionValue, double& NetOrderOptionValue, double& NetIntraDayOptionValue, bool DoBrute,char** CarryOut, bool IncludeDMC /*= false*/, bool withGrouping /*= false*/)
{
	std::map<int,Portfolio*>::iterator it;
	it = Portfolios.find(PortfolioID);
	if (it == Portfolios.end())
	{
		cout<<"Wrong portfolio ID!!!" << endl;
		return -2;
	}
	Portfolio* pPortfolio = it->second;

	Margin Result;
	HRESULT hr;
	MarginEstimator* Estimator = new MarginEstimator(CALCULATOR, PORTFOLIO, withGrouping);
	if (DoBrute)
		hr = Estimator->Calculate(BRUTE,Result);
	else
		hr = Estimator->Calculate(ITERATION,Result);
	Initial = Result.Initial;
	Maintenance = Result.Maintenance;
	NetOptionValue = Result.NetOptionValue;
	NetOrderOptionValue = Result.NetOrderOptionValue;
	NetIntraDayOptionValue = Result.NetIntraDayOptionValue;
	char *a=new char[Result.CarryOut.size()+1];
	a[Result.CarryOut.size()]=0;
	memcpy(a,Result.CarryOut.c_str(),Result.CarryOut.size());
	*CarryOut = a;
	delete Estimator;
	Estimator = NULL;
	if (hr == S_OK)
		return 0;
	else
		return -1;
}

void   TerminateMarginCalculator()
{
	ClearPortfolio();
	if (CALCULATOR)
	{
		delete CALCULATOR;
		CALCULATOR = NULL;
	}
	Portfolios.clear();
}
