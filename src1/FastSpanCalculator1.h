// Header for SPAN calculation without using objects...
#include <map>
#include <iostream>
#include <fstream>
#include "SpanNative.h"
#include "R.h"
#include "Rmath.h"
#include "Rinternals.h"
#define HRESULT int
#define S_OK 0
#define S_FALSE -1
#define __stdcall __attribute__((stdcall))

int		__stdcall InitMarginCalculator(char* XMLFileName);
int		__stdcall AddtoPortfolio(char* Symbol, bool IsOption, bool IsCall, bool IsAmerican, double Strike, char* Maturity, double Count, double Price = 0, bool IsOrder = false, bool IsIntraDay = true, bool Effectpremium = true);
int		__stdcall AddtoPortfoliowithOrderPrice(char* Symbol, bool IsOption, bool IsCall, bool IsAmerican, double Strike, char* Maturity, double Count, double Price = -1, double OrderExecutionPrice = -1, bool IsOrder = false, bool IsIntraDay = true, bool Effectpremium = true);
int		__stdcall AddtoPortfolioT(int PortfolioID, char* Symbol, bool IsOption, bool IsCall, bool IsAmerican, double Strike, char* Maturity, double Count, double Price = 0, bool IsOrder = false, bool IsIntraDay = true, bool Effectpremium = true);
int		__stdcall GetPriceScanRate(char* Symbol, bool IsOption, bool IsCall, bool IsAmerican, double Strike, char* Maturity, double& PriceScan);
void	__stdcall ClearPortfolio();
void	__stdcall ClearPortfolioT(int PortfolioID);
int		__stdcall CalculateMargin(double& Initial,double& Maintenance, double& NetOptionValue, double& NetOrderOptionValue, double& NetIntraDayOptionValue, bool IncludeDMC = false);
int		__stdcall CalculateWorstCase(double& Initial,double& Maintenance, double& NetOptionValue, double& NetOrderOptionValue, double& NetIntraDayOptionValue, bool DoBrute, char** CarryOut, bool IncludeDMC = false, bool withGrouping = false);
int		__stdcall CalculateWorstCaseOld(double& Initial,double& Maintenance, double& NetOptionValue, double& NetOrderOptionValue, double& NetIntraDayOptionValue, bool DoBrute, bool IncludeDMC = false, bool withGrouping = false);

int		__stdcall CalculateMarginT(int PortfolioID, double& Initial,double& Maintenance, double& NetOptionValue, double& NetOrderOptionValue, double& NetIntraDayOptionValue, bool IncludeDMC = false);
int		__stdcall CalculateWorstCaseT(int PortfolioID, double& Initial,double& Maintenance, double& NetOptionValue, double& NetOrderOptionValue, double& NetIntraDayOptionValue, bool DoBrute, char** CarryOut, bool IncludeDMC = false, bool withGrouping = false);
int		__stdcall CalculateWorstCaseOldT(int PortfolioID, double& Initial,double& Maintenance, double& NetOptionValue, double& NetOrderOptionValue, double& NetIntraDayOptionValue, bool DoBrute, bool IncludeDMC = false, bool withGrouping = false);
int		__stdcall CalculateDetailedMargin(double& Scenario,double& IntraSpread,double& InterSpread,double& SOM,double& DMC,double& Total,double& TotalMaintenance);

//int		__stdcall UpdateOptionScenarioValues(char* Symbol, bool IsCall, bool IsAmerican, double Strike, char* Maturity, double Values[17]);
//int		__stdcall UpdateFuturesScenarioValues(char* Symbol, char* Maturity, double Values[17]);

int		__stdcall SetRT(bool IsRealTime = true);

int		__stdcall UpdateOptionScenarios(char* Symbol, bool IsCall, bool IsAmerican, double Strike, char* Maturity, double UnderlyingPrice, double Price, double Volatility, double RiskFreeRate);

int		__stdcall UpdateOptionDelta(char* Symbol, bool IsCall, bool IsAmerican, double Strike, char* Maturity, double Delta);

void	__stdcall TerminateMarginCalculator();

int		__stdcall NewPortfolio(int newID);
