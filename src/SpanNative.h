#pragma once

//#include "Stdafx.h"
#include "SpanXMLParser.h"
#include <vector>
#include <cmath>
#include <inttypes.h>
#include <iostream>
#include <fstream>
#include <string>


typedef long long __int64_t;

using namespace std;
namespace MARGIN
{
	enum ESTIMATION{ BRUTE = 0 , ITERATION = 1};

#pragma region STRUCTS

	struct Margin
	{
	  Margin(){Initial = 0, Maintenance =0;NetOptionValue= 0;NetOrderOptionValue=0;NetIntraDayOptionValue =0; CarryOut = "";}
	  double Initial;
		double Maintenance;
		double NetOptionValue;
		double NetOrderOptionValue;
		double NetIntraDayOptionValue;
		string CarryOut;
		
	};

	struct DetailedMargin
	{
	  double Scenario;
		double IntraSpread;
		double InterSpread;
		double SOM;
		double DeliveryMonthCharge;
		double OptionPrice;
		double Total;
		double TotalMaintenance;
		
	};

	struct Instrument
	{
		string Symbol;
		string CC;
		ContractType Type;
		int ID;
		int UnderlyingID;
		bool IsCall;
		ExerciseType EType;
		double Strike;
		string Maturity;
		double Count;
		double Delta;
		double DeltaRT;
		double ra[17];
		double raRT[17];
		bool IsOrder;
		double Price;
		double PriceRT;
		double OrderExecutionPrice;
		double OptionValue;
		double OrderOptionValue;
		double IntraDayOptionValue;
		bool IsIntraDay;
		bool EffectPremium;

		Instrument(){Price =0;OrderExecutionPrice = 0;OptionValue =0;OrderOptionValue =0;IntraDayOptionValue=0;}

		Instrument & operator=(const Instrument &rhs)
		{
			Symbol = rhs.Symbol;
			CC = rhs.CC;
			Type = rhs.Type;
			IsCall = rhs.IsCall;
			EType = rhs.EType;
			Strike = rhs.Strike;
			Maturity = rhs.Maturity;
			Count = rhs.Count;
			Delta = rhs.Delta;
			DeltaRT = rhs.DeltaRT;
			
			IsOrder = rhs.IsOrder;
			Price = rhs.Price;
			PriceRT = rhs.PriceRT;
			OrderExecutionPrice = rhs.OrderExecutionPrice;
			IsIntraDay = rhs.IsIntraDay;
			EffectPremium = rhs.EffectPremium;
			return *this;
		}
		/*public:
		Instrument(string Symbol, ContractType Type, int ID, string Underlying, bool IsCall, double Strike, date Maturity, bool IsShort );*/
	};
	struct LegContainer 
	{
		int tn;
		double Longs;
		double Shorts;
		
	};
	struct CCLink
	{
		CCLink(){name = ""; CC = NULL; NetDelta = 0;VAR=0;TimeRisk=0;WFPR=0;Count=0; VolRisk = 0; Scan= 0; Intra= 0; Inter= 0; SOM=0;Risk=0;}
		string name;
		vector<Instrument*> Elements;
		map<int, LegContainer*> Tiers;
		CombinedCommodity *CC;
		double NetDelta;
		double TimeRisk;
		double VAR;
		double WFPR;
		double Count;
		double VolRisk;

		// For SOM Correction!!!!!!! - 8.3.2016
		double Scan;
		double Intra;
		double Inter;
		double SOM;
		double Risk;

		map<string,double> MonthDeltas;
		LegContainer* FindTier(int tn)
		{
			map<int,LegContainer*>::iterator it;
			it = Tiers.find(tn);
			if (it != Tiers.end())
				return it->second;
			return NULL;
			/*for(int i = 0; i < Tiers.size(); i++)
			{
				if (Tiers[i]->tn == tn)
				{
					return Tiers[i];
				}
			}
			return NULL;*/
		}
		~CCLink()
		{
			for(map<int, LegContainer*>::iterator iter = Tiers.begin();iter != Tiers.end();iter++)
			{
				pair<int, LegContainer*> temp = *iter;

				delete temp.second;
				temp.second = NULL;
			}
			Tiers.clear();
			MonthDeltas.clear();
			Elements.clear();
			/*for(int i = 0; i < Tiers.size(); i++)
			{
				delete Tiers[i];
			}
			Tiers.clear();*/

		}
	};

#pragma endregion

#pragma region CLASSES
	
	#pragma region PORTFOLIO
	class Portfolio
	{
		vector<Instrument*> Elements;
		
	public:
		 Portfolio(){OptionPrice = 0; NetOptionValue =0;NetOrderOptionValue = 0;NetIntraDayOptionValue=0;};
		 void AddInstrument(Instrument* item);
		 vector<Instrument*> GetInstruments(){return Elements;};
		 vector<CCLink*> GetCCs(){return CC;};
		void printportfolio();
		CCLink* FindCC(string name)
		{
			for(size_t i = 0; i < CC.size(); i++)
			{
				if (CC[i]->name == name)
				{
					return CC[i];
				}
			}
			return NULL;		
		};
		 void Clear();
		double OptionPrice;
		double NetOptionValue;
		double NetOrderOptionValue;
		double NetIntraDayOptionValue;
		 ~Portfolio(){
			Clear();
			for(size_t i = 0; i < CC.size(); i++)
			{
				delete CC[i];
				CC[i] = NULL;
			}
			CC.clear();
		};
		vector<CCLink*> CC;
	};
	#pragma endregion

	#pragma region SPANCALCULATOR 
	


	class MarginCalculator
	{
		XMLParser* Parser;
		bool DoJit;
		char* _ErrorPath;
		bool _DumpErrors;
		bool _IsRealTime;
		
	public:
		 MarginCalculator(){_IsRealTime = false;};
		 MarginCalculator(string XMLFileName);
		 int Calculate(Portfolio* portfolio, Margin &Result, bool IncludeDMC = false);
		 int CalculateDetailed(Portfolio* portfolio, DetailedMargin &Result);
		 int GetPriceScan(Instrument* instrument, double& PriceScan);
		 int GetDelta(Instrument* instrument, double& Delta);
		 int SetErrorPath(char* ErrorPath);
		 void SetRealTime(bool IsRealTime = true){_IsRealTime = IsRealTime;}



		 int SetOptionScenarioValues(string Symbol, bool IsCall, string Maturity, ExerciseType ExType, double Strike, double Values[17]);
		 int SetFuturesScenarioValues(string Symbol, string Maturity, double Values[17]);

		 int ReCalculateOptionScenarioValues(string Symbol, bool IsCall, string Maturity, ExerciseType ExType, double Strike, double UnderlyingPrice, double Price, double Volatility, double RiskFreeRate);
		 int ResetOptionScenarioValues(string Symbol, bool IsCall, string Maturity, ExerciseType ExType, double Strike);

		 int ReCalculateFutureScenarioValues(string Symbol, string Maturity, double Price);

		 int SetOptionDelta(string Symbol, bool IsCall, string Maturity, ExerciseType ExType, double Strike, double Delta);
		 int ResetOptionDelta(string Symbol, bool IsCall, string Maturity, ExerciseType ExType, double Strike);


		 ~MarginCalculator();
	private:
		int CalculateScenario(Portfolio* portfolio ,Margin &Result);
		int CalculateCCScenario(CCLink* CC, Margin &Result);
		int CalculateIntraSpread(Portfolio* portfolio, Margin &Result);
		int CalculateCCIntraSpread(CCLink* CC, double &Result);
		int CalculateCCIntraSpreadforDSpread(CCLink* CC, DSpread* dSpread, double &Result);
		int CalculateInterSpread(Portfolio* portfolio, Margin &Result);
		int CalculateInterSpreadforDSpread(Portfolio* portfolio, DSpread* dSpread, double &Result);
		int CalculateSOM(Portfolio* portfolio ,Margin &Result);
		int CalculateCCSOM(CCLink* CC, double &dResult);
		int CalculateDMC(Portfolio* portfolio ,Margin &Result);
		int CalculateCCDMC(CCLink* CC, double &dResult);

		int FillIDs(Portfolio* portfolio);
		int UpdateCCLink(CCLink* CC, CombinedCommodity* ParserCC, Instrument* instrument);
		
		int FillIDsModified(Portfolio* portfolio);
		int UpdateCCLinkModified(CCLink* CC, CombinedCommodity* ParserCC);
		int UpdateCCLinkModified2(CCLink* CC, CombinedCommodity* ParserCC, Instrument* ins);
		int UpdateCCLinkModifiedContinued(CCLink* CC);

		int FillIDsModifiedwithOrderPrice(Portfolio* portfolio);

		void DumpInvalidInstruments(vector<Instrument*> InvalidList);

		bool CheckCCNetDelta(Portfolio* portfolio);
		bool CheckPortfolio(Portfolio* portfolio);
	};	
	#pragma endregion

	struct InstrumentsItem
	{
		Instrument* instrument;
		size_t index;
	};

	struct InstrumentsContainer 
	{
		string UNDERLYING;
		bool isLong;
		vector<InstrumentsItem*> MEMBERS;
		~InstrumentsContainer()
		{
			for( vector<InstrumentsItem*>::iterator iter= MEMBERS.begin(); iter!=MEMBERS.end(); iter++)
			{
				delete *iter;
			}
			MEMBERS.clear();
		}
	};

	class MarginEstimator
	{
		vector<Instrument*> PORTFOLIO;
		vector<Instrument*> ORDERS;
		vector<InstrumentsContainer*> INSTRUMENTGROUPS;
		MarginCalculator* CALCULATOR;
		int Count;
		bool _IncludeDMC;
		bool _withGrouping;
	public:
		 MarginEstimator(MarginCalculator* Calculator, Portfolio* portfolio, bool withGrouping = false);
		 int Calculate(ESTIMATION EstimationType, Margin &Result, bool IncludeDMC = false);
		 ~MarginEstimator();
	private:
		double CalculateBrute(Margin& ResultMargin);
		double CalculatewithIteration(Margin& ResultMargin);
		double CalculatewithIteration(string StartingArray, int Length, bool LefttoRight, string& ResultArray, Margin& ResultMargin, bool Shuffle = false);
		
		// For Futures Only
		double CalculateBrutewithGrouping(Margin& ResultMargin);
		double CalculatewithIterationwithGrouping(Margin& ResultMargin);
		double CalculatewithIterationwithGrouping(string StartingArray, int Length, bool LefttoRight, string& ResultArray, Margin& ResultMargin, bool Shuffle = false);
		void GroupContract(Instrument* instrument, size_t index);
		void CleanGroups();


	};

#pragma endregion	
	}
