#pragma once
//#include "Stdafx.h"
#include <fstream>
#include <string>
#include <cstring>
//#include "Markup.h"
#include <vector>
#include <map>
#include <iostream>


using namespace std;
#ifdef SPANNATIVE_EXPORTS
#define SPANNATIVEDLL_API __declspec(dllexport) 
#else 
#define SPANNATIVEDLL_API __attribute__((dllimport))
#endif 
namespace MARGIN
{
#pragma region ENUMS
	enum ContractType {OPTION = 0 , FUTURES = 1};
	enum ExerciseType{ AMERICAN = 0 , EUROPIAN = 1};
#pragma endregion

#pragma region KEYS
	class FuturesKey
	{
		std::string _Symbol;
		std::string _Maturity;
	public:
		FuturesKey(std::string Symbol, std::string Maturity): _Symbol(Symbol),_Maturity(Maturity){}
		bool operator<(const FuturesKey &right) const {
			return (_Symbol < right._Symbol && _Maturity < right._Maturity);
		}
		bool operator==(const FuturesKey &right) const {
			return (_Symbol == right._Symbol && _Maturity == right._Maturity);
		}
	};

	class OptionKey
	{
		std::string _Symbol;
		std::string _Maturity;
		bool _IsCall;
		ExerciseType _EType;
		double _Strike;
	public:
		OptionKey(std::string Symbol, std::string Maturity, bool IsCall, ExerciseType EType, double Strike)
			: _Symbol(Symbol),_Maturity(Maturity), _IsCall(IsCall), _EType(EType), _Strike(Strike){}
		bool operator<(const OptionKey &right) const {
			if (this->_Symbol < right._Symbol)
			{
				return true;
			}
			else if (this->_Symbol == right._Symbol)
			{
				if (this->_Maturity < right._Maturity)
				{
					return true;
				}
				else if (this->_Maturity == right._Maturity)
				{
					if (this->_IsCall < right._IsCall)
					{
						return true;
					}
					else if(this->_IsCall == right._IsCall)
					{
						if (this->_EType < right._EType)
						{
							return true;
						}
						else if(this->_EType == right._EType)
						{
							if (this->_Strike < right._Strike)
							{
								return true;
							}
							else 
								return false;
						}
						else 
							return false;
					}
					else
						return false;
				}
				else 
					return false;
			}
			else
				return false;
			//return (this->_Symbol < right._Symbol && this->_Maturity < right._Maturity && this->_EType);
		}
		bool operator==(const OptionKey &right) const {
			return (this->_Symbol == right._Symbol 
				&& this->_Maturity == right._Maturity 
				&& this->_EType == right._EType 
				&& this->_IsCall == right._IsCall 
				&& this->_Strike == right._Strike);
		}
	};

#pragma endregion

#pragma region STRUCTS

	//struct Underlying
	//{
	//	string Name;
	//	int ID;
	//	map<OptionKey, Option*> OptionContracts;
	//	map<pair<string,string>, Futures*> FuturesContracts;
	//	vector<CombinedCommodity*> CCs;
	//	vector<DSpread*> InterSpreads;
	//	vector<pair<int,int>> ScanPointPairs;
	//	Underlying & operator=(const Underlying &rhs)
	//	{
	//		Name = rhs.Name;
	//		ID = rhs.ID;
	//		return *this;
	//	}
	//	~Underlying()
	//	{

	//		for(map<OptionKey, Option*>::iterator iter = OptionContracts.begin();iter != OptionContracts.end();iter++)
	//		{
	//			pair<OptionKey, Option*> temp = *iter;

	//			delete temp.second;
	//			temp.second = NULL;
	//		}
	//		OptionContracts.clear();

	//		for(map<pair<string,string>, Futures*>::iterator iter = FuturesContracts.begin();iter != FuturesContracts.end();iter++)
	//		{
	//			pair<pair<string,string>, Futures*> temp = *iter;

	//			delete temp.second;
	//			temp.second = NULL;
	//		}
	//		FuturesContracts.clear();
	//		for (int i = 0; i < CCs.size(); i++ )
	//		{

	//			delete CCs[i];
	//			CCs[i] = NULL;
	//		}

	//		for (int i = 0; i< InterSpreads.size(); i++)
	//		{
	//			delete InterSpreads[i];
	//			InterSpreads[i] = NULL;
	//		}
	//		CCs.clear();
	//		InterSpreads.clear();
	//	}
	//};

	struct Tier
	{
		int tn;
		std::string StartDate;
		std::string EndDate;
	};

	struct SOMTier
	{
		int tn;
		double rate;
	};

	struct TLeg
	{
		std::string cc;
		int tn;
		std::string spe;
		std::string epe;
		char RS;
		double i;
	};

	struct PLeg
	{
		std::string cc;
		std::string pe;
		char RS;
		double i;
	};

	struct DSpread
	{
		int Priority;
		double Rate;
		vector<PLeg*> PLegs;
		vector<TLeg*> TLegs;
		~DSpread()
		{
			for(size_t i = 0; i < PLegs.size(); i++)
			{
				delete PLegs[i];
				PLegs[i] = NULL;
			}	
			for(size_t i = 0; i < TLegs.size(); i++)
			{
				delete TLegs[i];
				TLegs[i] = NULL;
			}	
		};
	};

	struct SpotRate
	{
		int r;
		std::string pe;
		double sprd;
		double outr;
	};

	struct CombinedCommodity
	{
		std::string cc;
		std::string name;
		vector<int> pfLinks;
		map<int, Tier*> intraTiers;
		//vector<Tier*> intraTiers;
		vector<SOMTier*> SOMTiers;
		vector<DSpread*> dSpreads;
		vector<SpotRate*> SpotRates;
		Tier* FindTier(int tn)
		{
			map<int,Tier*>::iterator it;
			it = intraTiers.find(tn);
			if (it != intraTiers.end())
				return it->second;
			return NULL;
			//for(int i = 0; i < intraTiers.size(); i++)
			//{
			//	if (intraTiers[i]->tn == tn)
			//	{
			//		return intraTiers[i];
			//	}
			//}
			//return NULL;
		};
		~CombinedCommodity()
		{
			for(map<int, Tier*>::iterator iter = intraTiers.begin();iter != intraTiers.end();iter++)
			{
				pair<int, Tier*> temp = *iter;

				delete temp.second;
				temp.second = NULL;
			}
			/*for(int i = 0; i < intraTiers.size(); i++)
			{
				delete intraTiers[i];
			}*/
			for(size_t i = 0; i < dSpreads.size(); i++)
			{
				delete dSpreads[i];
				dSpreads[i] = NULL;
			}
			for(size_t i = 0; i < SOMTiers.size(); i++)
			{
				delete SOMTiers[i];
				SOMTiers[i] = NULL;
			}
			for(size_t i = 0; i < SpotRates.size(); i++)
			{
				delete SpotRates[i];
				SpotRates[i] = NULL;
			}
			intraTiers.clear();
			dSpreads.clear();
			SOMTiers.clear();
			SpotRates.clear();
			
		};
	};

	struct ScenarioPoint 
	{
		double PriceMultiplier;
		double VolMultiplier;
		double Weight;
	};

	struct Physical
	{

		std::string Code;
		std::string Name;
		int ID;
	/*public:
		Physical(string Code, string Name, int ID);
		string GetCode(){return _Code;};
		string GetName(){return _Name;};
		int GetID(){return _ID;};*/
	};

	struct Option
	{
		Option(): CC(NULL), PriceScan(0){}
		std::string Code;
		std::string Name;
		int ID;
		int UnderlyingID;
		std::string Maturity;
		double Strike;
		bool IsCall;
		double cvf;
		double Price;
		double PriceRT;
		
		ExerciseType EType;
		double delta;
		double deltaRT;

		double ra[17];
		double raRT[17];		// Real Time scenario values
		double PriceScan;
		double VolScan;
		double TimetoMaturity;
		CombinedCommodity* CC;
		void UpdateValues(double newUnderlyingPrice, double newVolatility, double newPrice, double newRiskFreeRate, vector<ScenarioPoint*> Scenarios);
		void ResetValues();
	};

	struct Futures
	{
		Futures(): CC(NULL), PriceScan(0){} 
		std::string Code;
		std::string Name;
		int ID;
		int UnderlyingID;
		std::string Maturity;
		double Strike;
		bool IsCall;
		double ra[17];
		double raRT[17];		// Real Time scenario values
		double delta;
		double cvf;
		double PriceScan;
		double VolScan;
		double TimetoMaturity;
		CombinedCommodity* CC;
		void UpdateValues(double newPrice, vector<ScenarioPoint*> Scenarios);
	};



#pragma endregion

#pragma region KEYS	
	class XMLParser
	{
		
	//	TiXmlDocument* XMLDOC;
		std::ofstream outputFile;
	public:
		SPANNATIVEDLL_API XMLParser(std::string FileName);

		SPANNATIVEDLL_API virtual ~XMLParser();
		//int Parse();

		int ReCalculateOptionScenarios(std::string Symbol, bool IsCall, std::string Maturity, ExerciseType ExType, double Strike, double UnderlyingPrice, double Volatility, double RiskFreeRate);
		int ReCalculateFutureScenarios(std::string Symbol, std::string Maturity, double Price);

		std::string GetCC(int ID);
		
		vector<Physical*> Physicals;
		//vector<Option*> OptionContracts;
		map<OptionKey, Option*> OptionContracts;

		//vector<Futures*> FuturesContracts;
		map<pair<std::string,std::string>, Futures*> FuturesContracts;
		vector<CombinedCommodity*> CCs;
		vector<DSpread*> InterSpreads;
		vector<pair<int, int> > ScanPointPairs;
		vector<ScenarioPoint*> Scenarios;

		void PrintOptions();
		void PrintFutures();
	private:
		//int FillScanPairs();
		//int FillPhysicals();
		//int FillOptionsON(string what);
		//int FillOptions();
		//int FillFutures();
		//int FillCombinedCommodities();
		//int FillInterSpreads();
		//int AttachCC(int Link, CombinedCommodity* CC);
		

		void PrintCombinedCommodities();
		
	};
#pragma endregion
}
