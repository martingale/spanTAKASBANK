// Header for SPAN calculation without using objects...
#include <map>
#include <iostream>
#include <fstream>
#include "SpanNative.h"
//#include "R.h"
//#include "Rmath.h"
//#include "Rinternals.h"
#define HRESULT int
#define S_OK 0
#define S_FALSE -1

int		 InitMarginCalculator(char *XMLFileName);
bool   isMarginCalculatorLoaded();
void unLoadMarginCalculator();
int		 AddtoPortfolio              (char* Symbol, bool IsOption, bool IsCall, bool IsAmerican, double Strike, char* Maturity, double Count, double Price = 0, bool IsOrder = false, bool IsIntraDay = true, bool Effectpremium = true);
int		 AddtoPortfoliowithOrderPrice(char* Symbol, bool IsOption, bool IsCall, bool IsAmerican, double Strike, char* Maturity, double Count, double Price = -1, double OrderExecutionPrice = -1, bool IsOrder = false, bool IsIntraDay = true, bool Effectpremium = true);
int		 AddtoPortfolioT(int PortfolioID, char* Symbol, bool IsOption, bool IsCall, bool IsAmerican, double Strike, char* Maturity, double Count, double Price = 0, bool IsOrder = false, bool IsIntraDay = true, bool Effectpremium = true);
int		 GetPriceScanRate(char* Symbol, bool IsOption, bool IsCall, bool IsAmerican, double Strike, char* Maturity, double& PriceScan);
void	 ClearPortfolio();
void	 ClearPortfolioT(int PortfolioID);
int		 CalculateMargin(double& Initial,double& Maintenance, double& NetOptionValue, double& NetOrderOptionValue, double& NetIntraDayOptionValue, bool IncludeDMC = false);
int		 CalculateWorstCase(double& Initial,double& Maintenance, double& NetOptionValue, double& NetOrderOptionValue, double& NetIntraDayOptionValue, bool DoBrute, char** CarryOut, bool IncludeDMC = false, bool withGrouping = false);
int		 CalculateWorstCaseOld(double& Initial,double& Maintenance, double& NetOptionValue, double& NetOrderOptionValue, double& NetIntraDayOptionValue, bool DoBrute, bool IncludeDMC = false, bool withGrouping = false);

int		 CalculateMarginT(int PortfolioID, double& Initial,double& Maintenance, double& NetOptionValue, double& NetOrderOptionValue, double& NetIntraDayOptionValue, bool IncludeDMC = false);
int		 CalculateWorstCaseT(int PortfolioID, double& Initial,double& Maintenance, double& NetOptionValue, double& NetOrderOptionValue, double& NetIntraDayOptionValue, bool DoBrute, char** CarryOut, bool IncludeDMC = false, bool withGrouping = false);
int		 CalculateWorstCaseOldT(int PortfolioID, double& Initial,double& Maintenance, double& NetOptionValue, double& NetOrderOptionValue, double& NetIntraDayOptionValue, bool DoBrute, bool IncludeDMC = false, bool withGrouping = false);
int		 CalculateDetailedMargin(double& Scenario,double& IntraSpread,double& InterSpread,double& SOM,double& DMC,double& Total,double& TotalMaintenance);

//int		 UpdateOptionScenarioValues(char* Symbol, bool IsCall, bool IsAmerican, double Strike, char* Maturity, double Values[17]);
//int		 UpdateFuturesScenarioValues(char* Symbol, char* Maturity, double Values[17]);

int		 SetRT(bool IsRealTime = true);

int		 UpdateOptionScenarios(char* Symbol, bool IsCall, bool IsAmerican, double Strike, char* Maturity, double UnderlyingPrice, double Price, double Volatility, double RiskFreeRate);

int		 UpdateOptionDelta(char* Symbol, bool IsCall, bool IsAmerican, double Strike, char* Maturity, double Delta);

void	 TerminateMarginCalculator();

int		 NewPortfolio(int newID);
