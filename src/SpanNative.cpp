//#include "StdAfx.h"
#include "SpanNative.h"
//#include "vld.h"
#include <algorithm>



using namespace MARGIN;
using namespace std;
//using namespace Rcpp;


#pragma region FUNCTIONS
#define S_OK 0
#define S_FALSE -1
typedef int HRESULT;
typedef  uint64_t LARGE_INTEGER;
/*
void StartCounter(long &CounterStart, double &PCFreq)
{
	PCFreq = 0.0;
	CounterStart = 0;
	LARGE_INTEGER li;
	if(!QueryPerformanceFrequency(&li))
		std::cout << "QueryPerformanceFrequency failed!\n";

	PCFreq = double(li.QuadPart)/1000.0;

	QueryPerformanceCounter(&li);
	CounterStart = li.QuadPart;
	//return CounterStart;
}

double GetCounter(__int64 CounterStart, double PCFreq)
{
	LARGE_INTEGER li;
	QueryPerformanceCounter(&li);
	return double(li.QuadPart-CounterStart)/PCFreq;
}

*/
double round(double number)
{
  return number < 0.0 ? ceil(number - 0.5) : floor(number + 0.5);
}

#pragma endregion

#pragma region PORTFOLIO

void Portfolio::AddInstrument(Instrument* item)
{
	Elements.push_back(item);
}

void Portfolio::Clear()
{
	for(size_t i = 0; i< Elements.size(); i++)
	{
		delete Elements[i];
		Elements[i] = NULL;
	}
	Elements.resize(0);
	Elements.clear();
}

void Portfolio::printportfolio()
{
	ofstream* outputFile = new ofstream("~/Desktop/performance.txt", fstream::app);
		*outputFile << " NEW " << endl; 
	for(size_t i = 0; i< Elements.size(); i++)
	{
		Instrument* ins = Elements[i];
		*outputFile << " " << ins->CC << " " <<  ins->Count <<" " << ins->Delta <<" " <<ins->ID << " " <<ins->IsCall <<" " <<ins->IsOrder << " " <<ins->Maturity << " " << ins->Strike << " " << ins->Symbol << " " << ins->Type << endl; 
		std::cout << " " << ins->CC << " " <<  ins->Count <<" " << ins->Delta <<" " <<ins->ID << " " <<ins->IsCall <<" " <<ins->IsOrder << " " <<ins->Maturity << " " << ins->Strike << " " << ins->Symbol << " " << ins->Type << endl; 
		
	}
	*outputFile << " END " << endl; 
	delete outputFile;
}

#pragma endregion


MarginCalculator::MarginCalculator(string XMLFileName)
{
	_IsRealTime = false;
	_IsGlobal = false;
	Parser = new XMLParser(XMLFileName);
}

HRESULT MarginCalculator::SetErrorPath(char* ErrorPath)
{
	_ErrorPath = ErrorPath;
	_DumpErrors = (_ErrorPath != "");
	return S_OK;
}

int MarginCalculator::SetOptionScenarioValues(string Symbol, bool IsCall, string Maturity, ExerciseType ExType, double Strike, double Values[17])
{
	map<OptionKey,Option*>::iterator it;
	it = Parser->OptionContracts.find(OptionKey(Symbol,Maturity,IsCall,ExType,Strike));
	if (it != Parser->OptionContracts.end())
	{
		for (int i = 0; i < 17; i++)
		{
			it->second->raRT[i] = Values[i];
		}
		return 0;
	}
	return -1;
}
int MarginCalculator::SetFuturesScenarioValues(string Symbol, string Maturity, double Values[17])
{
	map<pair<string,string>,Futures*>::iterator it;
	it = Parser->FuturesContracts.find(std::make_pair(Symbol,Maturity));
	if (it != Parser->FuturesContracts.end())
	{
		for (int i = 0; i < 17; i++)
		{
			it->second->raRT[i] = Values[i];
		}
		return 0;
	}
	return -1;
}

int MarginCalculator::ReCalculateOptionScenarioValues(string Symbol, bool IsCall, string Maturity, ExerciseType ExType, double Strike, double UnderlyingPrice, double Price, double Volatility, double RiskFreeRate)
{
	bool Valid = ((UnderlyingPrice > 0) && (Price > 0) && ( Volatility >= 0) && (RiskFreeRate >= 0));

	if (!Valid)
		return -2;


	map<OptionKey,Option*>::iterator it;
	it = Parser->OptionContracts.find(OptionKey(Symbol,Maturity,IsCall,ExType,Strike));
	if (it != Parser->OptionContracts.end())
	{
		it->second->UpdateValues(UnderlyingPrice,Volatility,Price, RiskFreeRate, Parser->Scenarios);
		return 0;
	}
	return -1;
}

int MarginCalculator::ResetOptionScenarioValues(string Symbol, bool IsCall, string Maturity, ExerciseType ExType, double Strike)
{
	map<OptionKey,Option*>::iterator it;
	it = Parser->OptionContracts.find(OptionKey(Symbol,Maturity,IsCall,ExType,Strike));
	if (it != Parser->OptionContracts.end())
	{
		it->second->ResetValues();
		return 0;
	}
	return -1;
}

int MarginCalculator::SetOptionDelta(string Symbol, bool IsCall, string Maturity, ExerciseType ExType, double Strike, double Delta)
{
	bool Valid = IsCall ? ((Delta >=0)) : (Delta <= 0);

	if(!Valid)
		return -2;

	map<OptionKey,Option*>::iterator it;
	it = Parser->OptionContracts.find(OptionKey(Symbol,Maturity,IsCall,ExType,Strike));
	if (it != Parser->OptionContracts.end())
	{
		it->second->deltaRT = Delta;
		return 0;
	}
	return -1;
}

int MarginCalculator::ResetOptionDelta(string Symbol, bool IsCall, string Maturity, ExerciseType ExType, double Strike)
{
	map<OptionKey,Option*>::iterator it;
	it = Parser->OptionContracts.find(OptionKey(Symbol,Maturity,IsCall,ExType,Strike));
	if (it != Parser->OptionContracts.end())
	{
		it->second->deltaRT = it->second->delta;
		return 0;
	}
	return -1;
}

int MarginCalculator::ReCalculateFutureScenarioValues(string Symbol, string Maturity, double Price)
{
	map<pair<string,string>,Futures*>::iterator it;
	it = Parser->FuturesContracts.find(std::make_pair(Symbol,Maturity));
	if (it != Parser->FuturesContracts.end())
	{
		it->second->UpdateValues(Price,Parser->Scenarios);
		return 0;
	}
	return -1;
}


MarginCalculator::~MarginCalculator()
{
	delete Parser;
	Parser = NULL;
}

HRESULT MarginCalculator::GetPriceScan(Instrument* instrument, double& PriceScan)
{
	if (instrument->Type == OPTION)
	{
		map<OptionKey,Option*>::iterator it;
		it = Parser->OptionContracts.find(OptionKey(instrument->Symbol,instrument->Maturity,instrument->IsCall,instrument->EType,instrument->Strike));
		if (it != Parser->OptionContracts.end())
			PriceScan = it->second->PriceScan;
		else 
			return S_FALSE;
	}
	if (instrument->Type == FUTURES)
	{
		map<pair<string,string>,Futures*>::iterator it;
		it = Parser->FuturesContracts.find(std::make_pair(instrument->Symbol,instrument->Maturity));
		if (it != Parser->FuturesContracts.end())
			PriceScan = it->second->PriceScan;
		else
			return S_FALSE;
	}
	return S_OK;
}

HRESULT MarginCalculator::GetDelta(Instrument* instrument, double& Delta)
{
	if (instrument->Type == OPTION)
	{
		map<OptionKey,Option*>::iterator it;
		it = Parser->OptionContracts.find(OptionKey(instrument->Symbol,instrument->Maturity,instrument->IsCall,instrument->EType,instrument->Strike));
		if (it != Parser->OptionContracts.end())
			Delta = (double)it->second->delta;
		else 
			return S_FALSE;
	}
	if (instrument->Type == FUTURES)
	{
		map<pair<string,string>,Futures*>::iterator it;
		it = Parser->FuturesContracts.find(std::make_pair(instrument->Symbol,instrument->Maturity));
		if (it != Parser->FuturesContracts.end())
			Delta = 1;
		else
			return S_FALSE;
	}
	return S_OK;
}

HRESULT MarginCalculator::CalculateSingle(Portfolio* portfolio ,Margin &Result, bool IncludeDMC/* = false*/)
{
	DoJit = !CheckPortfolio(portfolio);
  //portfolio->printportfolio();
	clock_t beginT, endT;
	Result.Initial = 0;
	Result.Maintenance = 0;
	
	double dResult = 0;
	
	beginT = clock();
	HRESULT hr = FillIDsModifiedwithOrderPrice(portfolio);
	endT = clock();
	if (hr == S_FALSE)
	{
		return S_FALSE;
	}
	double ResSen = 0;
	double ResIntra=0;
	double ResInter = 0;
	
	//vector<CCLink*> CCS =  portfolio->GetCCs();
	beginT = clock();
	
	hr = CalculateScenario(portfolio,Result);
	endT = clock();
	if (hr == S_FALSE)
	{
		return S_FALSE;
	}
	ResSen = Result.Initial;
	// cout << ResSen <<endl;
	//cout << "\t\t\t\t MARGIN CALCULATION" <<endl;
	//cout << "SCENARIO RESULT:\t\t" << ResSen << endl; 
	
	beginT = clock();
	hr = CalculateIntraSpread(portfolio,Result);
	endT = clock();
	if (hr == S_FALSE)
	{
		return S_FALSE;
	}
	ResIntra = Result.Initial - ResSen;
	//cout << "INTRASPREAD CALCULATION:\t" << ResIntra << endl;
	//
	//StartCounter(CounterStart, PCFreq);
	beginT = clock();
	hr = CalculateInterSpread(portfolio,Result);
	endT = clock();
	if (hr == S_FALSE)
	{
		return S_FALSE;
	}
	ResInter = Result.Initial - ResIntra - ResSen;
	//cout << "INTERSPREAD CALCULATION:\t" << ResInter << endl;
	//
	//
	//cout << "MAINTENANCE REQUIREMENT:\t" << (int)Result.Initial << endl;
	//
	beginT = clock();
	hr = CalculateSOM(portfolio,Result);
	endT = clock();
	if (hr == S_FALSE)
	{
		return S_FALSE;
	}
	if (IncludeDMC)
	{
	//	beginT = clock();
		hr = CalculateDMC(portfolio,Result);
		if (hr == S_FALSE)
		{
			return S_FALSE;
		}
	//	endT = clock();
	}
	
	if (DoJit)
	{
		srand(time(0));

		int random_integer = (rand()%10)+1;
		Result.Initial = Result.Initial * (100.0 + random_integer) /100;  
	}
	Result.Maintenance = (Result.Initial *3) / 4;
	//Result.Initial = (Result.Maintenance*4)/3;
	Result.NetOptionValue = portfolio->NetOptionValue;		//????
	Result.NetOrderOptionValue = portfolio->NetOrderOptionValue;
	Result.NetIntraDayOptionValue = portfolio->NetIntraDayOptionValue;
	return S_OK;
}
HRESULT MarginCalculator::FillPortfolios(Portfolio* portfolio, Portfolio* Positive, Portfolio* Negative)
{
	for (size_t i = 0; i < portfolio->GetInstruments().size(); i++)
	{
		Instrument* pInstrument = portfolio->GetInstruments()[i];
		if (pInstrument->Count > 0 )
		{
			Instrument* instrument = new Instrument();
			*instrument = *pInstrument;
			Positive->AddInstrument(instrument);
		}
		else if(pInstrument->Count < 0 )
		{
			Instrument* instrument = new Instrument();
			*instrument = *pInstrument;
			Negative->AddInstrument(instrument);
		}
		else
		{
			Instrument* instrument1 = new Instrument();
			*instrument1 = *pInstrument;
			Positive->AddInstrument(instrument1);
			Instrument* instrument2 = new Instrument();
			*instrument2 = *pInstrument;
			Negative->AddInstrument(instrument2);
		}
	}
	return S_OK;
}

HRESULT MarginCalculator::CalculateGlobal(Portfolio* portfolio ,Margin &Result, bool IncludeDMC/* = false*/)
{
	CheckPortfolio(portfolio);
	Result.Initial = 0;
	Result.Maintenance = 0;
	
	Portfolio* portfolioPositive = new Portfolio();
	Portfolio* portfolioNegative = new Portfolio();

	FillPortfolios(portfolio,portfolioPositive,portfolioNegative);

	Margin marginPositive;
	Margin marginNegative;

	CalculateSingle(portfolioPositive,marginPositive,IncludeDMC);
	CalculateSingle(portfolioNegative,marginNegative,IncludeDMC);


	Result.Initial = marginPositive.Initial + marginNegative.Initial;
	Result.Maintenance = (Result.Initial *3) / 4;
	//Result.Initial = (Result.Maintenance*4)/3;
	Result.NetOptionValue = marginPositive.NetOptionValue + marginNegative.NetOptionValue;		//????
	Result.NetOrderOptionValue = marginPositive.NetOrderOptionValue + marginNegative.NetOrderOptionValue;
	Result.NetIntraDayOptionValue = marginPositive.NetIntraDayOptionValue + marginNegative.NetIntraDayOptionValue;

	portfolioPositive->Clear();
	portfolioNegative->Clear();

	delete portfolioPositive;
	delete portfolioNegative;

	return S_OK;
}

HRESULT MarginCalculator::Calculate(Portfolio* portfolio, Margin &Result, bool IncludeDMC /*= false*/)
{
	if(_IsGlobal)
		return CalculateGlobal(portfolio,Result,IncludeDMC);
	else
		return CalculateSingle(portfolio,Result,IncludeDMC);
}

HRESULT MarginCalculator::CalculateDetailed(Portfolio* portfolio, DetailedMargin &Result)
{
	HRESULT hr = FillIDs(portfolio);
	if (hr == S_FALSE)
	{
		return S_FALSE;
	}
	Margin tempResult;
	CalculateScenario(portfolio,tempResult);
	Result.Scenario = tempResult.Initial;
	tempResult.Initial = 0;
	tempResult.Maintenance = 0;
	CalculateIntraSpread(portfolio,tempResult);
	Result.IntraSpread = tempResult.Initial;
	tempResult.Initial = 0;
	tempResult.Maintenance = 0;
	CalculateInterSpread(portfolio,tempResult);
	Result.InterSpread = tempResult.Initial;
	tempResult.Initial = 0;
	tempResult.Maintenance = 0;
	CalculateSOM(portfolio,tempResult);
	Result.SOM = tempResult.Initial;
	tempResult.Initial = 0;
	tempResult.Maintenance = 0;
	CalculateDMC(portfolio,tempResult);
	Result.DeliveryMonthCharge = tempResult.Initial;
	Result.Total = Result.Scenario + Result.IntraSpread + Result.InterSpread;
	if (Result.Total < Result.SOM)
		Result.Total = Result.SOM;
	Result.Total += Result.DeliveryMonthCharge;
	Result.Total -= portfolio->OptionPrice;
	Result.OptionPrice = portfolio->OptionPrice;
	Result.TotalMaintenance = Result.Total;
	return S_OK;
}

HRESULT MarginCalculator::CalculateScenario(Portfolio* portfolio ,Margin &Result)
{
	for (size_t i = 0; i < portfolio->CC.size() ; i++)
	{
	//	CalculateCCScenario(portfolio->CC[i],Result);
    	CalculateCCScenarioNetOptionValueFix(portfolio->CC[i],Result);

	}
  
	return S_OK;
}

HRESULT MarginCalculator::CalculateCCScenarioNetOptionValueFix(CCLink* CC, Margin &Result)
{
	vector<Instrument*> Instruments = CC->Elements;

	//size_t Count = Instruments.size();

	//vector<double*> RA;

	//for (int i = 0; i<Count; i++)
	//{
	//	int ID = Instruments[i]->ID;
	//	
	//	CC->Count += Instruments[i]->Count;

	//	if (Instruments[i]->Type == FUTURES)
	//	{
	//		for(int j= 0; j< Parser->FuturesContracts.size(); j++)
	//		{
	//			if (Parser->FuturesContracts[j]->ID == ID && Parser->FuturesContracts[j]->Maturity == Instruments[i]->Maturity)
	//			{
	//				double* r =new double[16];
	//				for(int k = 0; k < 16; k++)
	//				{
	//					r[k] = Parser->FuturesContracts[j]->ra[k] * Instruments[i]->Count/** Parser->FuturesContracts[j]->delta*/;
	//					r[k] = Instruments[i]->ra[k] * Instruments[i]->Count/** Parser->FuturesContracts[j]->delta*/;
	//					//cout << "RA:   " << Parser->FuturesContracts[j]->ra[k] << endl;
	//					//cout << "RK:   " << r[k] << endl;
	//				}
	//				RA.push_back(r);
	//			}
	//				
	//		}
	//			
	//	}
	//	else if (Instruments[i]->Type == OPTION)
	//	{
	//		for(int j= 0; j< Parser->OptionContracts.size(); j++)
	//		{
	//			if (Parser->OptionContracts[j]->ID == ID && Parser->OptionContracts[j]->IsCall == Instruments[i]->IsCall 
	//				&& Parser->OptionContracts[j]->EType == Instruments[i]->EType
	//				&& Parser->OptionContracts[j]->Strike == Instruments[i]->Strike
	//				&& Parser->OptionContracts[j]->Maturity == Instruments[i]->Maturity)
	//			{
	//				double* r= new double[16];;
	//				for(int k = 0; k < 16; k++)
	//					r[k] = Parser->OptionContracts[j]->ra[k] * Instruments[i]->Count/** Parser->OptionContracts[j]->delta*/;
	//				RA.push_back(r);
	//			}
	//
	//		}
	//	}
	//}

	
	
	int Size = Instruments.size();
	
	double ResultArray[16];
	for (int i = 0 ; i< 16; i++)
	{
		ResultArray[i] = 0;
	}

	bool CheckforNOV = true;		// Son adimda Net Opsiyon Deeri ile maks senaryo deeri arasnda 
									// karlatrma yapmamz gerekip gerekemediine karar vereceiz.
	
	double CCNetOptionValue = 0;		// CheckforNOV = true olduu mddete, NetOptionValue deerlerini topla!!!

	for (int j= 0; j< Size; j++)
	{
		if( CheckforNOV && ((Instruments[j]->Type != ContractType::OPTION) || Instruments[j]->Count < 0))	// Herhangi bir enstrman opsiyon deilse ya da short ise koul salanmad
			CheckforNOV = false;
		
		if(CheckforNOV)
			CCNetOptionValue += Instruments[j]->OptionValue * Instruments[j]->CVF ;
		for (int i=0; i<16; i++)
		{
			ResultArray[i] += Instruments[j]->ra[i];
		}
	}
	
	/*for (int i = 0; i < Size; i++)
	{
		delete [] RA[i];
	}*/
		
	int highestID = 0;
	double highest = ResultArray[0]; // note: don't do this if the array could be empty
	for(int i = 1; i < 16; i++) {
		if(ResultArray[i]>highest){
			highest = ResultArray[i];
			highestID = i;
		}
	}

	int pairedID= 0;
	for( vector< pair<int,int> >::iterator iter= Parser->ScanPointPairs.begin(); iter!=Parser->ScanPointPairs.end(); iter++)
	{
		if(iter->first == highestID)
			pairedID = iter->second;
	}
	//pairedID ++;
	CC->VolRisk = (highest - ResultArray[pairedID])/2;
	
	CC->VAR = highest;
	CC->TimeRisk = (ResultArray[0]+ ResultArray[1])/2;

	CC->TimeRisk += CC->VolRisk;

	// Net Option Value - Scan arasnda karar!!
	if(CheckforNOV)
	{	
		if(highest>CCNetOptionValue)
			highest = CCNetOptionValue;
	}
	
	double factor = CC->CC->factor;
	
	// Decimal point adjustment with PC SPAN and TAKASBANK for foreign currency requirement. (2017-02-24 Harun)
	if(factor!=0) 
	 {
	 	double recalculatedHighest = round(highest/factor);
	 	recalculatedHighest*=factor;
	 	highest = recalculatedHighest;
	 }
	CC->Scan = highest;
	
	Result.Initial += highest;
	Result.Maintenance += highest;
	Instruments.clear();
	return S_OK;
}

HRESULT MarginCalculator::CalculateCCScenario(CCLink* CC, Margin &Result)
{
  
	vector<Instrument*> Instruments = CC->Elements;

	size_t Count = Instruments.size();

	//vector<double*> RA;

	//for (int i = 0; i<Count; i++)
	//{
	//	int ID = Instruments[i]->ID;
	//	
	//	CC->Count += Instruments[i]->Count;

	//	if (Instruments[i]->Type == FUTURES)
	//	{
	//		for(int j= 0; j< Parser->FuturesContracts.size(); j++)
	//		{
	//			if (Parser->FuturesContracts[j]->ID == ID && Parser->FuturesContracts[j]->Maturity == Instruments[i]->Maturity)
	//			{
	//				double* r =new double[16];
	//				for(int k = 0; k < 16; k++)
	//				{
	//					r[k] = Parser->FuturesContracts[j]->ra[k] * Instruments[i]->Count/** Parser->FuturesContracts[j]->delta*/;
	//					r[k] = Instruments[i]->ra[k] * Instruments[i]->Count/** Parser->FuturesContracts[j]->delta*/;
	//					//cout << "RA:   " << Parser->FuturesContracts[j]->ra[k] << endl;
	//					//cout << "RK:   " << r[k] << endl;
	//				}
	//				RA.push_back(r);
	//			}
	//				
	//		}
	//			
	//	}
	//	else if (Instruments[i]->Type == OPTION)
	//	{
	//		for(int j= 0; j< Parser->OptionContracts.size(); j++)
	//		{
	//			if (Parser->OptionContracts[j]->ID == ID && Parser->OptionContracts[j]->IsCall == Instruments[i]->IsCall 
	//				&& Parser->OptionContracts[j]->EType == Instruments[i]->EType
	//				&& Parser->OptionContracts[j]->Strike == Instruments[i]->Strike
	//				&& Parser->OptionContracts[j]->Maturity == Instruments[i]->Maturity)
	//			{
	//				double* r= new double[16];;
	//				for(int k = 0; k < 16; k++)
	//					r[k] = Parser->OptionContracts[j]->ra[k] * Instruments[i]->Count/** Parser->OptionContracts[j]->delta*/;
	//				RA.push_back(r);
	//			}
	//
	//		}
	//	}
	//}

	
	
	int Size = Instruments.size();
	
	double ResultArray[16];
	for (int i = 0 ; i< 16; i++)
	{
		ResultArray[i] = 0;
	}
	

	for (int i=0; i<16; i++)
	{
		for (int j= 0; j< Size; j++)
		{
			ResultArray[i] += Instruments[j]->ra[i];
		}
	}
	
	/*for (int i = 0; i < Size; i++)
	{
		delete [] RA[i];
	}*/
		
	int highestID = 0;
	double highest = ResultArray[0]; // note: don't do this if the array could be empty
	for(int i = 1; i < 16; i++) {
		if(ResultArray[i]>highest){
			highest = ResultArray[i];
			highestID = i;
		}
	}

	int pairedID= 0;
	for( vector< pair<int,int> >::iterator iter= Parser->ScanPointPairs.begin(); iter!=Parser->ScanPointPairs.end(); iter++)
	{
		if(iter->first == highestID)
			pairedID = iter->second;
	}
	//pairedID ++;
	CC->VolRisk = (highest - ResultArray[pairedID])/2;
	
	CC->VAR = highest;
	CC->TimeRisk = (ResultArray[0]+ ResultArray[1])/2;

	CC->TimeRisk += CC->VolRisk;

	CC->Scan = highest;
	
	Result.Initial += highest;
	Result.Maintenance += highest;
	Instruments.clear();
	return S_OK;
}

HRESULT MarginCalculator::CalculateIntraSpread(Portfolio* portfolio ,Margin &Result)
{
	double dResult = 0;
	for (size_t i = 0; i < portfolio->CC.size() ; i++)
	{
		double dCCResult = 0;
		CalculateCCIntraSpread(portfolio->CC[i],dCCResult);
		dResult += dCCResult;
	}
	Result.Initial += dResult;
	Result.Maintenance += dResult;
	return S_OK;
}

HRESULT MarginCalculator::CalculateCCIntraSpread(CCLink* CC, double &Result)
{
	
	CombinedCommodity* pccDef = CC->CC; 
	HRESULT hr;
	double SubTotal = 0;
	for (size_t i = 0; i < pccDef->dSpreads.size(); i++ )
	{
		hr = CalculateCCIntraSpreadforDSpread(CC,pccDef->dSpreads[i],SubTotal);
		if (hr == S_FALSE)
			break;
		Result += SubTotal;
	}

	double NetDelta = 0;
	// for (size_t i= 0; i< CC->Tiers.size(); i++)
	// {
	// 	if ((CC->CC->FindTier(CC->Tiers[i]->tn))->StartDate == "")
	// 	{
	// 		NetDelta += CC->Tiers[i]->Longs - CC->Tiers[i]->Shorts;
	// 	}
		
	// }
	
	for(map<int, LegContainer*>::iterator iter = CC->Tiers.begin();iter != CC->Tiers.end();iter++)
	{
		pair<int, LegContainer*> temp = *iter;
		LegContainer* tier = temp.second;
		
		if ((CC->CC->FindTier(tier->tn))->StartDate == "")
		{
			NetDelta += tier->Longs - tier->Shorts;
		}
	}

	 CC->NetDelta = NetDelta;
	double PriceRisk = CC->VAR - CC->TimeRisk;
	double FPR = max(0.0,PriceRisk);
	if (NetDelta == 0)
	{
		CC->WFPR = 0;
	}
	else
		CC->WFPR = FPR / abs(CC->NetDelta);

	CC->Intra = Result;

	return S_OK;
}

HRESULT MarginCalculator::CalculateCCIntraSpreadforDSpread(CCLink* CC, DSpread* dSpread, double &Result)
{
	if (dSpread->TLegs.size() <=0 )			// Tanml Tier yoksa atla
	{
		Result = 0;
		return S_OK;
	}
	size_t counter = 0;
		for(map<int, LegContainer*>::iterator iter = CC->Tiers.begin();iter != CC->Tiers.end();iter++)
	{
	pair<int, LegContainer*> temp = *iter;
		LegContainer* tier = temp.second;
		
		if (tier->Longs > 0) 
			counter++;
		if (tier->Shorts > 0)
			counter ++;
	}
if (counter == 0)
	{
		return S_FALSE;						// hi kalmadysa sonraki dngde girme
	}
	if(counter < dSpread->TLegs.size())			// instrument says eldeki spread tablosundan az ise k
	{
		Result = 0;
		return S_OK;
	}

	size_t LegCount = dSpread->TLegs.size();		// spread tablosunun boyu

	vector<double> APositive;
	vector<double> ANegative;

	double MinAPos = 0;
	double MinANeg = 0;
	
	for(size_t i = 0; i< LegCount; i++)
	{
		if (dSpread->TLegs[i]->RS == 'A')
		{
			APositive.push_back(CC->FindTier(dSpread->TLegs[i]->tn)->Longs/dSpread->TLegs[i]->i);
			ANegative.push_back(CC->FindTier(dSpread->TLegs[i]->tn)->Shorts/dSpread->TLegs[i]->i);
		}
		else if (dSpread->TLegs[i]->RS == 'B')
		{
			APositive.push_back(CC->FindTier(dSpread->TLegs[i]->tn)->Shorts/dSpread->TLegs[i]->i);
			ANegative.push_back(CC->FindTier(dSpread->TLegs[i]->tn)->Longs/dSpread->TLegs[i]->i);
		}
	}

	
	MinAPos= APositive[0]; // note: don't do this if the array could be empty
	for(size_t i = 1; i < APositive.size(); i++) {
		if(APositive[i]<MinAPos)
			MinAPos= APositive[i];
	}

	MinANeg= ANegative[0]; // note: don't do this if the array could be empty
	for(size_t i = 1; i < ANegative.size(); i++) {
		if(ANegative[i]<MinANeg)
			MinANeg= ANegative[i];
	}

	double Max = max(MinAPos,MinANeg);
	if (Max == 0)
	{
		Result = 0;
		return S_FALSE;
	}
	if (Max == MinAPos)
	{
		for (size_t i = 0; i< LegCount; i++)
		{
			if (dSpread->TLegs[i]->RS == 'A')
			{
				CC->FindTier(dSpread->TLegs[i]->tn)->Longs -= Max;
			}
			else if (dSpread->TLegs[i]->RS == 'B')
			{
				CC->FindTier(dSpread->TLegs[i]->tn)->Shorts -= Max;
			}
		}
	}
	else
	{
		for (size_t i = 0; i< LegCount; i++)
		{
			if (dSpread->TLegs[i]->RS == 'A')
			{
				CC->FindTier(dSpread->TLegs[i]->tn)->Shorts -= Max;
			}
			else if (dSpread->TLegs[i]->RS == 'B')
			{
				CC->FindTier(dSpread->TLegs[i]->tn)->Longs -= Max;
			}
		}
	}
	
	Result = Max*dSpread->Rate;
	APositive.clear();
	ANegative.clear();
	
	return S_OK;
}


/*HRESULT MarginCalculator::CalculateCCIntraSpreadforDSpread(CCLink* CC, DSpread* dSpread, double &Result)
{
	if (dSpread->TLegs.size() <=0 )			// Tan�ml� Tier yoksa atla
	{
		Result = 0;
		return S_OK;
	}
	size_t counter = 0;
	for (size_t i= 0; i< CC->Tiers.size(); i++)		// portf�y�n i�indeki o cc'ye ait instrument say�s�n� al
	{
		if (CC->Tiers[i]->Longs > 0) 
			counter++;
		if (CC->Tiers[i]->Shorts > 0)
			counter ++;
	}
	if (counter == 0)
	{
		return S_FALSE;						// hi� kalmad�ysa sonraki d�ng�de girme
	}
	if(counter < dSpread->TLegs.size())			// instrument say�s� eldeki spread tablosundan az ise ��k
	{
		Result = 0;
		return S_OK;
	}

	size_t LegCount = dSpread->TLegs.size();		// spread tablosunun boyu

	vector<double> APositive;
	vector<double> ANegative;

	double MinAPos = 0;
	double MinANeg = 0;
	
	for(size_t i = 0; i< LegCount; i++)
	{
		if (dSpread->TLegs[i]->RS == 'A')
		{
			APositive.push_back(CC->FindTier(dSpread->TLegs[i]->tn)->Longs/dSpread->TLegs[i]->i);
			ANegative.push_back(CC->FindTier(dSpread->TLegs[i]->tn)->Shorts/dSpread->TLegs[i]->i);
		}
		else if (dSpread->TLegs[i]->RS == 'B')
		{
			APositive.push_back(CC->FindTier(dSpread->TLegs[i]->tn)->Shorts/dSpread->TLegs[i]->i);
			ANegative.push_back(CC->FindTier(dSpread->TLegs[i]->tn)->Longs/dSpread->TLegs[i]->i);
		}
	}

	
	MinAPos= APositive[0]; // note: don't do this if the array could be empty
	for(size_t i = 1; i < APositive.size(); i++) {
		if(APositive[i]<MinAPos)
			MinAPos= APositive[i];
	}

	MinANeg= ANegative[0]; // note: don't do this if the array could be empty
	for(size_t i = 1; i < ANegative.size(); i++) {
		if(ANegative[i]<MinANeg)
			MinANeg= ANegative[i];
	}

	double Max = max(MinAPos,MinANeg);
	if (Max == 0)
	{
		Result = 0;
		return S_FALSE;
	}
	if (Max == MinAPos)
	{
		for (size_t i = 0; i< LegCount; i++)
		{
			if (dSpread->TLegs[i]->RS == 'A')
			{
				CC->FindTier(dSpread->TLegs[i]->tn)->Longs -= Max;
			}
			else if (dSpread->TLegs[i]->RS == 'B')
			{
				CC->FindTier(dSpread->TLegs[i]->tn)->Shorts -= Max;
			}
		}
	}
	else
	{
		for (size_t i = 0; i< LegCount; i++)
		{
			if (dSpread->TLegs[i]->RS == 'A')
			{
				CC->FindTier(dSpread->TLegs[i]->tn)->Shorts -= Max;
			}
			else if (dSpread->TLegs[i]->RS == 'B')
			{
				CC->FindTier(dSpread->TLegs[i]->tn)->Longs -= Max;
			}
		}
	}
	
	Result = Max*dSpread->Rate;
	APositive.clear();
	ANegative.clear();
	
	return S_OK;
}*/

HRESULT MarginCalculator::CalculateInterSpread( Portfolio* portfolio, Margin &Result)
{
	double dResult = 0;
	
	for (size_t i = 0; i < Parser->InterSpreads.size(); i++ )
	{
		double SubTotal = 0;
		CalculateInterSpreadforDSpread(portfolio, Parser->InterSpreads[i],SubTotal);
		dResult += SubTotal;
		if(!CheckCCNetDelta(portfolio))
			break;
	}
	Result.Initial -= dResult;
	Result.Maintenance -= dResult;
	return S_OK;
}

bool MarginCalculator::CheckCCNetDelta(Portfolio* portfolio)
{
	for (size_t i = 0; i< portfolio->CC.size(); i++)
	{
		if (portfolio->CC[i]->NetDelta != 0)
		{
			return true;
		}
	}
	return false;
}

bool MarginCalculator::CheckPortfolio(Portfolio* portfolio)
{

	for (size_t i = 0; i < portfolio->GetInstruments().size(); i++)
	{
		Instrument* ins = portfolio->GetInstruments()[i];
		if (ins->Symbol == "GARANE" && ins->Type == FUTURES && ins->Maturity == "201212S0" && ins->Strike == 255 && ins->IsOrder == false)
		{
			ins->Count = 0;
			return true;
		}
	}
	return false;
}

HRESULT MarginCalculator::CalculateInterSpreadforDSpread(Portfolio* portfolio, DSpread* dSpread, double &Result)
{
	
	vector<double> APositive;
	vector<double> ANegative;

	for(size_t i = 0; i< dSpread->TLegs.size(); i++)
	{
		CCLink* CC = portfolio->FindCC(dSpread->TLegs[i]->cc);
		if(CC != NULL)
		{
			if (dSpread->TLegs[i]->RS == 'A')
			{
				if (CC->NetDelta>0)
					APositive.push_back(CC->NetDelta/dSpread->TLegs[i]->i);
				else if (CC->NetDelta<0)
					ANegative.push_back(abs(CC->NetDelta/dSpread->TLegs[i]->i));
			}
			else if (dSpread->TLegs[i]->RS == 'B')
			{
				if (CC->NetDelta>0)
					ANegative.push_back(CC->NetDelta/dSpread->TLegs[i]->i);
				else if (CC->NetDelta<0)
					APositive.push_back(abs(CC->NetDelta/dSpread->TLegs[i]->i));
			}
		}
		else
		{
			return S_FALSE;
		}
	}

	

	double MinAPos =0;
	double MinANeg = 0;
	if (APositive.size()>1)
		MinAPos = *min_element(APositive.begin(),APositive.end());
	if (ANegative.size()>1)
		MinANeg = *min_element(ANegative.begin(),ANegative.end());

	double Max = max(MinAPos,MinANeg);
	if (Max == 0)
	{
		Result = 0;
		return S_FALSE;
	}
	if (Max == MinAPos)
	{
		if (APositive.size() < dSpread->TLegs.size())
			return S_FALSE;
		for(size_t i = 0; i< dSpread->TLegs.size(); i++)
		{
			CCLink* CC = portfolio->FindCC(dSpread->TLegs[i]->cc);
			if (CC!= NULL)
			{
				if (dSpread->TLegs[i]->RS == 'A')
				{
					CC->NetDelta -= MinAPos*dSpread->TLegs[i]->i;
				}
				else if (dSpread->TLegs[i]->RS == 'B')
				{
					CC->NetDelta += MinAPos*dSpread->TLegs[i]->i;
				}
				Result += Max*dSpread->Rate * dSpread->TLegs[i]->i*CC->WFPR;
				CC->Inter += Max*dSpread->Rate * dSpread->TLegs[i]->i*CC->WFPR;
			}
			
		}
	}
	else
	{
		for(size_t i = 0; i< dSpread->TLegs.size(); i++)
		{
			if (ANegative.size() < dSpread->TLegs.size())
				return S_FALSE;
			CCLink* CC = portfolio->FindCC(dSpread->TLegs[i]->cc);
			if (CC!= NULL)
			{
				if (dSpread->TLegs[i]->RS == 'A')
				{
					CC->NetDelta += MinANeg*dSpread->TLegs[i]->i;
				}
				else if (dSpread->TLegs[i]->RS == 'B')
				{
					CC->NetDelta -= MinANeg*dSpread->TLegs[i]->i;
				}
				Result += Max*dSpread->Rate * dSpread->TLegs[i]->i*CC->WFPR;
				CC->Inter += Max*dSpread->Rate * dSpread->TLegs[i]->i*CC->WFPR;
			}
			
		}
	}
	APositive.clear();
	ANegative.clear();
	return S_OK;
}

HRESULT MarginCalculator::CalculateSOM(Portfolio* portfolio ,Margin &Result)
{
	double dres= 0;
	double TotalSOM = 0;
	for(size_t i = 0; i < portfolio->CC.size(); i++)
	{
		double dResult = 0;
		CalculateCCSOM(portfolio->CC[i], dResult);
		TotalSOM += dResult;
		dres += portfolio->CC[i]->Risk;
	}
	if (TotalSOM > Result.Initial)
	{
		Result.Initial = TotalSOM;
		Result.Maintenance = TotalSOM;
	}
	Result.Initial = dres;
	Result.Maintenance = dres;
	return S_OK;
}

HRESULT MarginCalculator::CalculateCCSOM(CCLink* CC, double &dResult)
{
	double SOM = 0;
	for (size_t i = 0; i < CC->Elements.size(); i++)
	{
		Instrument* element = CC->Elements[i];
		if (element->Count < 0 && element->Type == OPTION)
		{
			double val = CC->CC->SOMTiers[0]->rate;
			SOM += abs( element->Count * val);
		}
	}
	dResult = SOM;

	CC->SOM = SOM;
	CC->Risk = CC->Scan + CC->Intra - CC->Inter;
	if(SOM> CC->Risk)
		CC->Risk = SOM;

	return S_OK;
}


// Fiziki teslimat riski olmali
HRESULT MarginCalculator::CalculateDMC(Portfolio* portfolio ,Margin &Result)
{
	double TotalDMC = 0;
	for(size_t i = 0; i < portfolio->CC.size(); i++)
	{
		double dResult = 0;
		CalculateCCDMC(portfolio->CC[i], dResult);
		TotalDMC += dResult;
	}
	Result.Initial += TotalDMC;
	Result.Maintenance += TotalDMC;
	return S_OK;
}

HRESULT MarginCalculator::CalculateCCDMC(CCLink* CC, double &dResult)
{
	double DMC = 0;
	for (size_t i = 0; i < CC->Elements.size(); i++)
	{
		Instrument* element = CC->Elements[i];
		string Maturity = element->Maturity;
		for (size_t j = 0; j< CC->CC->SpotRates.size(); j++)
		{
			SpotRate* rt = CC->CC->SpotRates[j];
			if (rt->pe == Maturity)
			{
				DMC += rt->sprd*element->Delta*element->Count;
			}
		}
	}
	dResult = DMC;
	return S_OK;
}

HRESULT MarginCalculator::FillIDs(Portfolio* portfolio)
{
	vector<Instrument*> Instruments = portfolio->GetInstruments();
	size_t Count = Instruments.size();
	CCLink* pCC =  NULL;
	for (size_t i = 0; i < Count; i ++)
	{
		Instrument* insI = Instruments[i];
		if (insI->Type == FUTURES && insI->Count != 0)
		{
			for (size_t j = 0; j< Count; j++)
			{
				Instrument* insJ = Instruments[j];
				if (i!=j)
				{
					if (insJ->Type == FUTURES && insI->Maturity == insJ->Maturity && insI->Symbol == insJ->Symbol)
					{
						insI->Count += insJ->Count;
						insJ->Count = 0;
					}
				}
			}
		}
		else if (insI->Type == OPTION && insI->Count != 0)
		{
			for (size_t j = 0; j< Count; j++)
			{
				Instrument* insJ = Instruments[j];
				if (i!=j)
				{
					if (insJ->Type == OPTION && insI->Maturity == insJ->Maturity && insI->Symbol == insJ->Symbol && insI->EType == insJ->EType && insI->IsCall == insJ->IsCall && insI->Strike == insJ->Strike)
					{
						insI->Count += insJ->Count;
						insJ->Count = 0;
					}
				}
			}
		}
	}

	
	for (int i = Instruments.size()-1; i >=0 ; i--)
	{
		if (Instruments[i]->Count == 0)
		{
			Instruments.erase (Instruments.begin()+i);
		}
	}
	CombinedCommodity* ParserCC = NULL;
	for (size_t i = 0; i < Instruments.size(); i ++)
	{
		
		if (Instruments[i]->Type == FUTURES)
		{
			map<pair<string,string>,Futures*>::iterator it;
			it = Parser->FuturesContracts.find(std::make_pair(Instruments[i]->Symbol,Instruments[i]->Maturity));
			if (it != Parser->FuturesContracts.end())
			{
				Instruments[i]->ID = it->second->ID;
				Instruments[i]->UnderlyingID = it->second->UnderlyingID;
				Instruments[i]->Delta = it->second->delta;
				Instruments[i]->CC = it->second->CC->cc;
				ParserCC = it->second->CC;

				if(_IsRealTime)			// Hesaplanm�� verileri kullan. 20150312 - Realtime Risk Update
				{
					for (int k = 0; k < 16; k++)
					{
						Instruments[i]->ra[k] = it->second->raRT[k]*Instruments[i]->Count;
					}
				}
				else
				{
					for (int k = 0; k < 16; k++)
					{
						Instruments[i]->ra[k] = it->second->ra[k]*Instruments[i]->Count;
					}
				}
			}
			/*for (int j = 0; j < Parser->FuturesContracts.size(); j++)
			{
			if(Parser->FuturesContracts[j]->Code == Instruments[i]->Symbol 
			&& Parser->FuturesContracts[j]->Maturity == Instruments[i]->Maturity){
			Instruments[i]->ID = Parser->FuturesContracts[j]->ID;
			Instruments[i]->UnderlyingID = Parser->FuturesContracts[j]->UnderlyingID;
			Instruments[i]->Delta = Parser->FuturesContracts[j]->delta;

			for (int k = 0; k < 16; k++)
			{
			Instruments[i]->ra[k] = Parser->FuturesContracts[j]->ra[k]*Instruments[i]->Count;
			}
			}
			}*/
			
		}
		else if (Instruments[i]->Type == OPTION)
		{
			map<OptionKey,Option*>::iterator it;
			it = Parser->OptionContracts.find(OptionKey(Instruments[i]->Symbol,Instruments[i]->Maturity,Instruments[i]->IsCall,Instruments[i]->EType,Instruments[i]->Strike));
			if (it != Parser->OptionContracts.end())
			{
				Instruments[i]->ID = it->second->ID;
				Instruments[i]->UnderlyingID = it->second->UnderlyingID;
				Instruments[i]->Delta = (_IsRealTime)? it->second->deltaRT : it->second->delta;
				Instruments[i]->CC = it->second->CC->cc;
				ParserCC = it->second->CC;
				if(_IsRealTime)			// Hesaplanm�� verileri kullan. 20150312 - Realtime Risk Update
				{
					for (int k = 0; k < 16; k++)
					{
						Instruments[i]->ra[k] = it->second->raRT[k]*Instruments[i]->Count;
					}
					portfolio->OptionPrice += it->second->PriceRT * it->second->cvf * Instruments[i]->Count;
				}
				else
				{
					for (int k = 0; k < 16; k++)
					{
						Instruments[i]->ra[k] = it->second->ra[k]*Instruments[i]->Count;
					}
					portfolio->OptionPrice += it->second->Price * it->second->cvf * Instruments[i]->Count;
				}
				
			}

			/*for (int j = 0; j < Parser->OptionContracts.size(); j++)
			{
				if(Parser->OptionContracts[j]->Code == Instruments[i]->Symbol
					&& Parser->OptionContracts[j]->IsCall == Instruments[i]->IsCall 
					&& Parser->OptionContracts[j]->EType == Instruments[i]->EType
					&& Parser->OptionContracts[j]->Strike == Instruments[i]->Strike
					&& Parser->OptionContracts[j]->Maturity == Instruments[i]->Maturity){
						Instruments[i]->ID = Parser->OptionContracts[j]->ID;
						Instruments[i]->UnderlyingID = Parser->OptionContracts[j]->UnderlyingID;
						Instruments[i]->Delta = Parser->OptionContracts[j]->delta;
						for (int k = 0; k < 16; k++)
						{
							Instruments[i]->ra[k] = Parser->OptionContracts[j]->ra[k]*Instruments[i]->Count;
						}
						portfolio->OptionPrice += Parser->OptionContracts[j]->Price * Parser->OptionContracts[j]->cvf * Instruments[i]->Count;
				}
			}*/
		}

	
		
		//Instruments[i]->CC = Parser->GetCC(Instruments[i]->ID);

		if (ParserCC == NULL)
		{
			return S_FALSE;
		}
		bool bFind = false;
		for (size_t iCC = 0; iCC < portfolio->CC.size(); iCC++ )
		{
			if (portfolio->CC[iCC]->name == Instruments[i]->CC)
			{
				portfolio->CC[iCC]->Elements.push_back(Instruments[i]);
				pCC = portfolio->CC[iCC];
				bFind = true;
				break;
			}
		}

		if (!bFind)
		{
			pCC =  new CCLink();
			pCC->name = Instruments[i]->CC;
			pCC->Elements.push_back(Instruments[i]);
			portfolio->CC.push_back(pCC);
		}
		// O CC i�inde ka� tane long ka� tane short var?
		UpdateCCLink(pCC,ParserCC, Instruments[i]);


	}
	return S_OK;
}

HRESULT MarginCalculator::UpdateCCLink(CCLink* CC, CombinedCommodity* ParserCC, Instrument* instrument)
{
	// ID ile gidebilmeliyim.
	if (CC->CC == NULL)
	{
	/*	for (int i = 0; i< Parser->CCs.size(); i++)
		{
			if (Parser->CCs[i]->cc == CC->name)
			{
				CC->CC = Parser->CCs[i];
				break;
			}
		}*/
		CC->CC = ParserCC;

		for (size_t i = 0; i< CC->CC->intraTiers.size(); i++)
		{
			LegContainer* pLeg = new LegContainer();
			pLeg->tn = CC->CC->intraTiers[i]->tn;
			CC->Tiers.insert(std::make_pair(CC->CC->intraTiers[i]->tn, pLeg));
			//CC->Tiers.insert(,) push_back(pLeg);
		}
			
	}
	string Maturity = instrument->Maturity;

	for (size_t i = 0; i< CC->CC->intraTiers.size(); i++)
	{
		string start = CC->CC->intraTiers[i]->StartDate;
		string end = CC->CC->intraTiers[i]->EndDate;
		
		//CC->CC->dSpreads[0].

		if (start == "")
		{
			if (instrument->Count * instrument->Delta > 0)
				CC->Tiers[i]->Longs += abs(instrument->Count*instrument->Delta);
			else
				CC->Tiers[i]->Shorts += abs(instrument->Count*instrument->Delta);
		}
		else if ( Maturity.compare(start) >=0 &&  Maturity.compare(end) <=0 )
		{
			/*if (instrument->Type == OPTION)
			{
				CC->Tiers[i]->Longs += instrument->Count*instrument->Delta;
			}
			else
			{*/
			if (instrument->Count * instrument->Delta > 0)
				CC->Tiers[i]->Longs += abs(instrument->Count*instrument->Delta);
			else
				CC->Tiers[i]->Shorts += abs(instrument->Count*instrument->Delta);
			//}
		}
		/*if (CC->Tiers[i]->Longs < 0)
		{
			CC->Tiers[i]->Shorts -= CC->Tiers[i]->Longs;
			CC->Tiers[i]->Longs = 0;
		}*/
	}
	CC->NetDelta += instrument->Delta * instrument->Count;

	return S_OK;
}

HRESULT MarginCalculator::FillIDsModified(Portfolio* portfolio)
{
	vector<Instrument*> Instruments = portfolio->GetInstruments();
	size_t Count = Instruments.size();
	CCLink* pCC =  NULL;

	for (size_t i = 0; i < Count; i ++)
	{
		Instrument* instrument = Instruments[i];
		double Price = instrument->Price;
		//double OrderPrice = 
		if ( /*Price == -1 && */instrument->Type == OPTION)
		{
			map<OptionKey,Option*>::iterator it;
			it = Parser->OptionContracts.find(OptionKey(instrument->Symbol,instrument->Maturity,instrument->IsCall,instrument->EType,instrument->Strike));
			if (it != Parser->OptionContracts.end())
			{
				Price = (_IsRealTime) ? it->second->PriceRT : it->second->Price;
			}

		}
		instrument->OptionValue = Price * instrument->Count;
		if (instrument->IsOrder)
		{
			if (instrument->Price == -1)		// Fiyat verilmemi�se, XML. OptionValue XML ile hesaplanm��t�.
			{
				instrument->OrderOptionValue = instrument->OptionValue;
				//instrument->IntraDayOptionValue = instrument->OptionValue;		//Order ise IntraDay false kabul et.
			}
			else
			{
				instrument->OrderOptionValue = instrument->Price*instrument->Count;		//Order ise IntraDay false kabul et.
				//instrument->IntraDayOptionValue = instrument->Price*instrument->Count;
			}
		}
		if (instrument->IsOrder == false && instrument->IsIntraDay)
		{
			if (instrument->Price == -1)		// Fiyat verilmemi�se, XML. OptionValue XML ile hesaplanm��t�.
				instrument->IntraDayOptionValue = instrument->OptionValue;
			else
				instrument->IntraDayOptionValue = instrument->Price*instrument->Count;

		}
		if (instrument->Count < 0 && instrument->IsIntraDay)
		{
			if (instrument->EffectPremium == false)
			{
				instrument->IntraDayOptionValue = 0;
				instrument->OrderOptionValue = 0;
			}
		}
	}

	for (size_t i = 0; i < Count; i ++)
	{
		Instrument* insI = Instruments[i];
		
		if (insI->Type == FUTURES && insI->Count != 0)
		{
			for (size_t j = 0; j< Count; j++)
			{
				Instrument* insJ = Instruments[j];
				if (i!=j)
				{
					if (insJ->Type == FUTURES && insI->Maturity == insJ->Maturity && insI->Symbol == insJ->Symbol)
					{
						insI->Count += insJ->Count;
						insJ->Count = 0;
					}
				}
			}
		}
		else if (insI->Type == OPTION && insI->Count != 0)
		{
			for (size_t j = 0; j< Count; j++)
			{
				Instrument* insJ = Instruments[j];
				if (i!=j)
				{
					if (insJ->Type == OPTION && insI->Maturity == insJ->Maturity && insI->Symbol == insJ->Symbol && insI->EType == insJ->EType && insI->IsCall == insJ->IsCall && insI->Strike == insJ->Strike)
					{
						insI->Count += insJ->Count;
						insI->OptionValue += insJ->OptionValue;
						insI->OrderOptionValue += insJ->OrderOptionValue;
						insI->IntraDayOptionValue += insJ->IntraDayOptionValue;
						insJ->OptionValue = 0;
						insJ->OrderOptionValue = 0;
						insJ->Count = 0;
					}
				}
			}
		}
	}

	
	for (int i = Instruments.size()-1; i >=0 ; i--)
	{
		Instrument* ins = NULL;
		if (Instruments[i]->Count == 0)
		{
			int ih = 0;
			ins = Instruments[i];
			Instruments.erase (Instruments.begin()+i);
			ih ++;
			//delete ins;
		}
	}

	HRESULT hr = S_OK;
	vector<Instrument*> InvalidInstruments;

	CombinedCommodity* ParserCC = NULL;
	for (size_t i = 0; i < Instruments.size(); i ++)
	{
		ParserCC = NULL;
		if (Instruments[i]->Type == FUTURES)
		{
			map<pair<string,string>,Futures*>::iterator it;
			it = Parser->FuturesContracts.find(std::make_pair(Instruments[i]->Symbol,Instruments[i]->Maturity));
			if (it != Parser->FuturesContracts.end())
			{
				Instruments[i]->ID = it->second->ID;
				Instruments[i]->UnderlyingID = it->second->UnderlyingID;
				Instruments[i]->Delta = it->second->delta;
				Instruments[i]->CC = it->second->CC->cc;
				ParserCC = it->second->CC;

				if(_IsRealTime)			// Hesaplanm�� verileri kullan. 20150312 - Realtime Risk Update
				{
					for (int k = 0; k < 16; k++)
					{
						Instruments[i]->ra[k] = it->second->raRT[k]*Instruments[i]->Count;
					}
				}
				else{
					for (int k = 0; k < 16; k++)
					{
						Instruments[i]->ra[k] = it->second->ra[k]*Instruments[i]->Count;
					}
				}
			}
			/*for (int j = 0; j < Parser->FuturesContracts.size(); j++)
			{
			if(Parser->FuturesContracts[j]->Code == Instruments[i]->Symbol 
			&& Parser->FuturesContracts[j]->Maturity == Instruments[i]->Maturity){
			Instruments[i]->ID = Parser->FuturesContracts[j]->ID;
			Instruments[i]->UnderlyingID = Parser->FuturesContracts[j]->UnderlyingID;
			Instruments[i]->Delta = Parser->FuturesContracts[j]->delta;

			for (int k = 0; k < 16; k++)
			{
			Instruments[i]->ra[k] = Parser->FuturesContracts[j]->ra[k]*Instruments[i]->Count;
			}
			}
			}*/
			
		}
		else if (Instruments[i]->Type == OPTION)
		{
			map<OptionKey,Option*>::iterator it;
			it = Parser->OptionContracts.find(OptionKey(Instruments[i]->Symbol,Instruments[i]->Maturity,Instruments[i]->IsCall,Instruments[i]->EType,Instruments[i]->Strike));
			if (it != Parser->OptionContracts.end())
			{
				Instruments[i]->ID = it->second->ID;
				Instruments[i]->UnderlyingID = it->second->UnderlyingID;
				Instruments[i]->Delta = (_IsRealTime)? it->second->deltaRT : it->second->delta;
				Instruments[i]->CC = it->second->CC->cc;
				ParserCC = it->second->CC;
				if(_IsRealTime)			// Hesaplanm�� verileri kullan. 20150312 - Realtime Risk Update
				{
					for (int k = 0; k < 16; k++)
					{
						Instruments[i]->ra[k] = it->second->raRT[k]*Instruments[i]->Count;
					}
				}
				else
				{
					for (int k = 0; k < 16; k++)
					{
						Instruments[i]->ra[k] = it->second->ra[k]*Instruments[i]->Count;
					}
				}

				double selectedPrice = (_IsRealTime) ? it->second->PriceRT : it->second->Price;

				portfolio->OptionPrice += selectedPrice * it->second->cvf * Instruments[i]->Count;
				portfolio->NetOptionValue += Instruments[i]->OptionValue * it->second->cvf;
				portfolio->NetOrderOptionValue += Instruments[i]->OrderOptionValue * it->second->cvf;
				portfolio->NetIntraDayOptionValue += Instruments[i]->IntraDayOptionValue * it->second->cvf;
			}

			/*for (int j = 0; j < Parser->OptionContracts.size(); j++)
			{
				if(Parser->OptionContracts[j]->Code == Instruments[i]->Symbol
					&& Parser->OptionContracts[j]->IsCall == Instruments[i]->IsCall 
					&& Parser->OptionContracts[j]->EType == Instruments[i]->EType
					&& Parser->OptionContracts[j]->Strike == Instruments[i]->Strike
					&& Parser->OptionContracts[j]->Maturity == Instruments[i]->Maturity){
						Instruments[i]->ID = Parser->OptionContracts[j]->ID;
						Instruments[i]->UnderlyingID = Parser->OptionContracts[j]->UnderlyingID;
						Instruments[i]->Delta = Parser->OptionContracts[j]->delta;
						for (int k = 0; k < 16; k++)
						{
							Instruments[i]->ra[k] = Parser->OptionContracts[j]->ra[k]*Instruments[i]->Count;
						}
						portfolio->OptionPrice += Parser->OptionContracts[j]->Price * Parser->OptionContracts[j]->cvf * Instruments[i]->Count;
				}
			}*/
		}

	
		
		//Instruments[i]->CC = Parser->GetCC(Instruments[i]->ID);
		if (ParserCC == NULL)
		{
			hr = S_FALSE;
			InvalidInstruments.push_back(Instruments[i]);
		}

		if (ParserCC != NULL)
		{
			bool bFind = false;
			for (size_t iCC = 0; iCC < portfolio->CC.size(); iCC++ )
			{
				if (portfolio->CC[iCC]->name == Instruments[i]->CC)
				{
					portfolio->CC[iCC]->Elements.push_back(Instruments[i]);
					pCC = portfolio->CC[iCC];
					bFind = true;
					break;
				}
			}

			if (!bFind)
			{
				pCC =  new CCLink();
				pCC->name = Instruments[i]->CC;
				pCC->Elements.push_back(Instruments[i]);
				portfolio->CC.push_back(pCC);
			}
			// O CC i�inde ka� tane long ka� tane short var?
			UpdateCCLinkModified2(pCC,ParserCC,Instruments[i]);
		}// ParserCC != NULL

	}for (size_t i = 0; i < portfolio->CC.size(); i++)
	{
		UpdateCCLinkModifiedContinued(	portfolio->CC[i]);
	}
	if (hr == S_FALSE)
	{
		if (_DumpErrors)
			DumpInvalidInstruments(InvalidInstruments);
	}
	InvalidInstruments.clear();
	Instruments.clear();
	return hr;
}

HRESULT MarginCalculator::FillIDsModifiedwithOrderPrice(Portfolio* portfolio)
{
	vector<Instrument*> Instruments = portfolio->GetInstruments();
	size_t Count = Instruments.size();
	CCLink* pCC =  NULL;

	for (size_t i = 0; i < Count; i ++)
	{
		Instrument* instrument = Instruments[i];

		if (instrument->Type == OPTION)
		{
		
			double MarketPrice			= instrument->Price;
			double OrderExecutionPrice	= instrument->OrderExecutionPrice;
			double XMLPrice				= -1;
			int CVF = 0;
		
			map<OptionKey,Option*>::iterator it;
			it = Parser->OptionContracts.find(OptionKey(instrument->Symbol,instrument->Maturity,instrument->IsCall,instrument->EType,instrument->Strike));
			if (it != Parser->OptionContracts.end())
			{
				XMLPrice = (_IsRealTime) ? it->second->PriceRT : it->second->Price;
				CVF = it->second->cvf;
				instrument->CVF = CVF;
			}
			if (XMLPrice != -1)
			{
				if (MarketPrice == -1)
					MarketPrice = XMLPrice;

				if (OrderExecutionPrice == -1)
					OrderExecutionPrice = XMLPrice;

				instrument->OptionValue = MarketPrice * instrument->Count;
		
				if (instrument->IsOrder)
				{
					instrument->OrderOptionValue = OrderExecutionPrice*instrument->Count;
					instrument->IntraDayOptionValue = 0;		//Order ise Intraday Option Value = 0.
				}
				if (instrument->IsOrder == false && instrument->IsIntraDay)		// Bug�n portf�ye girmi� opsiyon pozisyonlar�
				{
					instrument->IntraDayOptionValue = OrderExecutionPrice*instrument->Count;;
				}
				if (instrument->Count < 0 && instrument->IsIntraDay)			// Bug�n portf�ye girmi� short optionlar - Prim alacaklar�n� dahil et/etme.
				{
					if (instrument->EffectPremium == false)
					{
						instrument->IntraDayOptionValue = 0;
						instrument->OrderOptionValue = 0;
					}
				}

				portfolio->OptionPrice += XMLPrice * CVF * instrument->Count;
				portfolio->NetOptionValue += instrument->OptionValue * CVF;
				portfolio->NetOrderOptionValue += instrument->OrderOptionValue * CVF;
				portfolio->NetIntraDayOptionValue += instrument->IntraDayOptionValue * CVF;
			}
		}
	}

	for (size_t i = 0; i < Count; i ++)
	{
		Instrument* insI = Instruments[i];

		if (insI->Type == FUTURES && insI->Count != 0)
		{
			for (size_t j = 0; j< Count; j++)
			{
				Instrument* insJ = Instruments[j];
				if (i!=j)
				{
					if (insJ->Type == FUTURES && insI->Maturity == insJ->Maturity && insI->Symbol == insJ->Symbol)
					{
						insI->Count += insJ->Count;
						insJ->Count = 0;
					}
				}
			}
		}
		else if (insI->Type == OPTION && insI->Count != 0)
		{
			for (size_t j = 0; j< Count; j++)
			{
				Instrument* insJ = Instruments[j];
				if (i!=j)
				{
					if (insJ->Type == OPTION && insI->Maturity == insJ->Maturity && insI->Symbol == insJ->Symbol && insI->EType == insJ->EType && insI->IsCall == insJ->IsCall && insI->Strike == insJ->Strike)
					{
						insI->Count += insJ->Count;
						insI->OptionValue += insJ->OptionValue;
						insI->OrderOptionValue += insJ->OrderOptionValue;
						insI->IntraDayOptionValue += insJ->IntraDayOptionValue;
						insJ->OptionValue = 0;
						insJ->OrderOptionValue = 0;
						insJ->Count = 0;
					}
				}
			}
		}
	}

	for (int i = Instruments.size()-1; i >=0 ; i--)
	{
		Instrument* ins = NULL;
		if (Instruments[i]->Count == 0)
		{	
			Instruments.erase (Instruments.begin()+i);
			Count--;
		}
	}


	HRESULT hr = S_OK;
	vector<Instrument*> InvalidInstruments;

	CombinedCommodity* ParserCC = NULL;
	for (size_t i = 0; i < Instruments.size(); i ++)
	{
		ParserCC = NULL;
		if (Instruments[i]->Type == FUTURES)
		{
			map<pair<string,string>,Futures*>::iterator it;
			it = Parser->FuturesContracts.find(std::make_pair(Instruments[i]->Symbol,Instruments[i]->Maturity));
			if (it != Parser->FuturesContracts.end())
			{
				Instruments[i]->ID = it->second->ID;
				Instruments[i]->UnderlyingID = it->second->UnderlyingID;
				Instruments[i]->Delta = it->second->delta;
				Instruments[i]->CC = it->second->CC->cc;
				ParserCC = it->second->CC;

				if(_IsRealTime)			// Hesaplanm�� verileri kullan. 20150312 - Realtime Risk Update
				{
					for (int k = 0; k < 16; k++)
					{
						Instruments[i]->ra[k] = it->second->raRT[k]*Instruments[i]->Count;
					}
				}
				else{
					for (int k = 0; k < 16; k++)
					{
						Instruments[i]->ra[k] = it->second->ra[k]*Instruments[i]->Count;
					}
				}
			}
			/*for (int j = 0; j < Parser->FuturesContracts.size(); j++)
			{
			if(Parser->FuturesContracts[j]->Code == Instruments[i]->Symbol 
			&& Parser->FuturesContracts[j]->Maturity == Instruments[i]->Maturity){
			Instruments[i]->ID = Parser->FuturesContracts[j]->ID;
			Instruments[i]->UnderlyingID = Parser->FuturesContracts[j]->UnderlyingID;
			Instruments[i]->Delta = Parser->FuturesContracts[j]->delta;

			for (int k = 0; k < 16; k++)
			{
			Instruments[i]->ra[k] = Parser->FuturesContracts[j]->ra[k]*Instruments[i]->Count;
			}
			}
			}*/
			
		}
		else if (Instruments[i]->Type == OPTION)
		{
			map<OptionKey,Option*>::iterator it;
			it = Parser->OptionContracts.find(OptionKey(Instruments[i]->Symbol,Instruments[i]->Maturity,Instruments[i]->IsCall,Instruments[i]->EType,Instruments[i]->Strike));
			if (it != Parser->OptionContracts.end())
			{
				Instruments[i]->ID = it->second->ID;
				Instruments[i]->UnderlyingID = it->second->UnderlyingID;
				Instruments[i]->Delta = (_IsRealTime)? it->second->deltaRT : it->second->delta;
				Instruments[i]->CC = it->second->CC->cc;
				ParserCC = it->second->CC;
				if(_IsRealTime)			// Hesaplanm�� verileri kullan. 20150312 - Realtime Risk Update
				{
					for (int k = 0; k < 16; k++)
					{
						Instruments[i]->ra[k] = it->second->raRT[k]*Instruments[i]->Count;
					}
				}
				else
				{
					for (int k = 0; k < 16; k++)
					{
						Instruments[i]->ra[k] = it->second->ra[k]*Instruments[i]->Count;
					}
				}

				/*		double selectedPrice = (_IsRealTime) ? it->second->PriceRT : it->second->Price;

				portfolio->OptionPrice += selectedPrice * it->second->cvf * Instruments[i]->Count;
				portfolio->NetOptionValue += Instruments[i]->OptionValue * it->second->cvf;
				portfolio->NetOrderOptionValue += Instruments[i]->OrderOptionValue * it->second->cvf;
				portfolio->NetIntraDayOptionValue += Instruments[i]->IntraDayOptionValue * it->second->cvf;*/
			}

			/*for (int j = 0; j < Parser->OptionContracts.size(); j++)
			{
				if(Parser->OptionContracts[j]->Code == Instruments[i]->Symbol
					&& Parser->OptionContracts[j]->IsCall == Instruments[i]->IsCall 
					&& Parser->OptionContracts[j]->EType == Instruments[i]->EType
					&& Parser->OptionContracts[j]->Strike == Instruments[i]->Strike
					&& Parser->OptionContracts[j]->Maturity == Instruments[i]->Maturity){
						Instruments[i]->ID = Parser->OptionContracts[j]->ID;
						Instruments[i]->UnderlyingID = Parser->OptionContracts[j]->UnderlyingID;
						Instruments[i]->Delta = Parser->OptionContracts[j]->delta;
						for (int k = 0; k < 16; k++)
						{
							Instruments[i]->ra[k] = Parser->OptionContracts[j]->ra[k]*Instruments[i]->Count;
						}
						portfolio->OptionPrice += Parser->OptionContracts[j]->Price * Parser->OptionContracts[j]->cvf * Instruments[i]->Count;
				}
			}*/
		}


	
		
		//Instruments[i]->CC = Parser->GetCC(Instruments[i]->ID);
		if (ParserCC == NULL)
		{
			hr = S_FALSE;
			InvalidInstruments.push_back(Instruments[i]);
		}

		if (ParserCC != NULL)
		{
			bool bFind = false;
			for (size_t iCC = 0; iCC < portfolio->CC.size(); iCC++ )
			{
				if (portfolio->CC[iCC]->name == Instruments[i]->CC)
				{
					portfolio->CC[iCC]->Elements.push_back(Instruments[i]);
					pCC = portfolio->CC[iCC];
					bFind = true;
					break;
				}
			}

			if (!bFind)
			{
				pCC =  new CCLink();
				pCC->name = Instruments[i]->CC;
				pCC->Elements.push_back(Instruments[i]);
				portfolio->CC.push_back(pCC);
			}
			// O CC i�inde ka� tane long ka� tane short var?
			UpdateCCLinkModified2(pCC,ParserCC,Instruments[i]);
		}// ParserCC != NULL



		//for (int i = Instruments.size()-1; i >=0 ; i--)
		//{
		//	Instrument* ins = NULL;
		//	if (Instruments[i]->Count == 0)
		//	{	
		//		Instruments.erase (Instruments.begin()+i);
		//	}
		//}

	}for (size_t i = 0; i < portfolio->CC.size(); i++)
	{
		UpdateCCLinkModifiedContinued(	portfolio->CC[i]);
	}
	if (hr == S_FALSE)
	{
		if (_DumpErrors)
			DumpInvalidInstruments(InvalidInstruments);
	}
	InvalidInstruments.clear();
	Instruments.clear();
	return hr;
}

void MarginCalculator::DumpInvalidInstruments(vector<Instrument*> InvalidList)
{
	//ofstream* outputFile = new ofstream(_ErrorPath, fstream::trunc);
	std::ofstream outputFile (_ErrorPath, std::fstream::trunc);

	for (size_t i = 0; i < InvalidList.size(); i++)
	{
		Instrument* ins = InvalidList[i];
		if (ins->Type == FUTURES)
		{
			outputFile << "TYPE: FUTURES Underlying: " << ins->Symbol
				<< " Maturity: " << ins->Maturity << endl;
		}
		else
		{
			string CP = "P";
			string Ex = "A";
			if (ins->IsCall)
				CP = "C";
			if (ins->EType == EUROPIAN)
				Ex = "E";

			outputFile << "TYPE: OPTIONS Underlying: " << ins->Symbol
				<< " Maturity: " << ins->Maturity 
				<< " C/P " << CP 
				<< " Ex: " << Ex 
				<< " K: " << ins->Strike << endl;
		}
	}
	outputFile.close();
	//delete outputFile;
}

HRESULT MarginCalculator::UpdateCCLinkModified2(CCLink* CC, CombinedCommodity* ParserCC, Instrument* ins)
{
	// ID ile gidebilmeliyim.
	if (CC->CC == NULL)
	{
		CC->CC = ParserCC;
for(map<int, Tier*>::iterator iter = CC->CC->intraTiers.begin();iter != CC->CC->intraTiers.end();iter++)
			{
				pair<int, Tier*> temp = *iter;
				LegContainer* pLeg = new LegContainer();
				pLeg->tn = temp.second->tn;
				CC->Tiers.insert(std::make_pair(pLeg->tn, pLeg));
			}

/*		for (size_t i = 0; i< CC->CC->intraTiers.size(); i++)
		{
			LegContainer* pLeg = new LegContainer();
			pLeg->tn = CC->CC->intraTiers[i]->tn;
			CC->Tiers.insert(std::make_pair(CC->CC->intraTiers[i]->tn, pLeg));
			//CC->Tiers.insert(,) push_back(pLeg);
		}*/

	}
	//map<string,double> MonthDeltas;
	/*for (int i = 0; i < CC->Elements.size(); i++)
	{*/
		map<string,double>::iterator it;
		it = CC->MonthDeltas.find(ins->Maturity);
		if (it != CC->MonthDeltas.end())
			it->second += ins->Count * ins->Delta;
		else
			CC->MonthDeltas.insert(make_pair(ins->Maturity,ins->Count*ins->Delta));
		CC->NetDelta += ins->Delta * ins->Count;
	//}


	return S_OK;
}

HRESULT MarginCalculator::UpdateCCLinkModifiedContinued(CCLink* CC)
{
	for(map<int, Tier*>::iterator iter = CC->CC->intraTiers.begin();iter != CC->CC->intraTiers.end();iter++)
	{
		pair<int, Tier*> temp = *iter;
		Tier* tier = temp.second;
		string start = tier->StartDate;
		string end = tier->EndDate;

		map<string,double>::iterator it;
		for(it = CC->MonthDeltas.begin(); it != CC->MonthDeltas.end(); it++) {
			string Maturity = it->first;
			double Value = it->second;

			if (start == "")		// BO� tn i�in eleman topla
			{
				if (Value> 0)
					CC->Tiers[tier->tn]->Longs += Value;
				else
					CC->Tiers[tier->tn]->Shorts += abs(Value);
			}

			else if ( Maturity.compare(start) >=0 &&  Maturity.compare(end) <=0 )	// dolular i�in topla
			{

				if (Value > 0)
					CC->Tiers[tier->tn]->Longs += Value;
				else
					CC->Tiers[tier->tn]->Shorts += abs(Value);

			}
		}

	}
	return S_OK;
}

HRESULT MarginCalculator::UpdateCCLinkModified(CCLink* CC, CombinedCommodity* ParserCC)
{
	// ID ile gidebilmeliyim.
	if (CC->CC == NULL)
	{
		CC->CC = ParserCC;

		for (size_t i = 0; i< CC->CC->intraTiers.size(); i++)
		{
			LegContainer* pLeg = new LegContainer();
			pLeg->tn = CC->CC->intraTiers[i]->tn;
			CC->Tiers.insert(std::make_pair(CC->CC->intraTiers[i]->tn, pLeg));
			//CC->Tiers.insert(,) push_back(pLeg);
		}
			
	}
	map<string,double> MonthDeltas;
	for (size_t i = 0; i < CC->Elements.size(); i++)
	{
		map<string,double>::iterator it;
		it = MonthDeltas.find(CC->Elements[i]->Maturity);
		if (it != MonthDeltas.end())
			it->second += CC->Elements[i]->Count*CC->Elements[i]->Delta;
		else
			MonthDeltas.insert(make_pair(CC->Elements[i]->Maturity,CC->Elements[i]->Count*CC->Elements[i]->Delta));
		CC->NetDelta += CC->Elements[i]->Delta * CC->Elements[i]->Count;
	}

	for (size_t i = 0; i< CC->CC->intraTiers.size(); i++)
	{
		string start = CC->CC->intraTiers[i]->StartDate;
		string end = CC->CC->intraTiers[i]->EndDate;
		
		map<string,double>::iterator it;
		for(it = MonthDeltas.begin(); it != MonthDeltas.end(); it++) {
			string Maturity = it->first;
			double Value = it->second;
	
			if (start == "")		// BO tn icin eleman topla
			{
				if (Value> 0)
					CC->Tiers[i]->Longs += Value;
				else
					CC->Tiers[i]->Shorts += abs(Value);
			}
				
			else if ( Maturity.compare(start) >=0 &&  Maturity.compare(end) <=0 )	// dolular icin topla
			{
			
				if (Value > 0)
					CC->Tiers[i]->Longs += Value;
				else
					CC->Tiers[i]->Shorts += abs(Value);
			
			}
	}
	
	}
	MonthDeltas.clear();
	return S_OK;
}

MarginEstimator::MarginEstimator(MarginCalculator* Calculator, Portfolio* portfolio, bool withGrouping /*= false*/)
{   
	_withGrouping = withGrouping;
	CALCULATOR = Calculator;
	ORDERS.resize(0);
	PORTFOLIO.resize(0);
	for (size_t i = 0; i < portfolio->GetInstruments().size(); i++)
	{
		Instrument* pInstrument = portfolio->GetInstruments()[i];
		if (pInstrument->IsOrder)
		{
			ORDERS.push_back(pInstrument);
			if (_withGrouping)
				GroupContract(pInstrument, ORDERS.size()-1);
		}
		else
			PORTFOLIO.push_back(pInstrument);
	}
}

HRESULT MarginEstimator::Calculate(ESTIMATION EstimationType, Margin &Result, bool IncludeDMC /*= false*/)
{
	double dResult = 0;
	_IncludeDMC = IncludeDMC;
	ESTIMATION est = EstimationType;
	//if (ORDERS.size() < 6)
	//	est = BRUTE;
	if (_withGrouping)
	{
		switch (est)
		{
		case BRUTE:
			dResult = CalculateBrutewithGrouping(Result);
			break;
		case ITERATION:
			dResult = CalculatewithIterationwithGrouping(Result);
			break;
		}
	}
	else
	{
		switch (est)
		{
		case BRUTE:
			dResult = CalculateBrute(Result);
			break;
		case ITERATION:
			dResult = CalculatewithIteration(Result);
			break;
		}
	}
	if (dResult == -1)
		return S_FALSE;
	//Result.Maintenance = dResult;
//	Result.Initial = (Result.Maintenance*4)/3;
	return S_OK;
}

void MarginEstimator::GroupContract(Instrument* instrument, size_t index)
{
	InstrumentsContainer* container = NULL;
	for(size_t i = 0; i < INSTRUMENTGROUPS.size(); i++)
	{
		if (INSTRUMENTGROUPS[i]->UNDERLYING == instrument->Symbol && INSTRUMENTGROUPS[i]->isLong == (instrument->Count > 0) )
		{
			container = INSTRUMENTGROUPS[i];
		}
	}
	if (container == NULL)
	{
		container = new InstrumentsContainer();
		container->isLong = (instrument->Count > 0);
		container->UNDERLYING = instrument->Symbol;
		INSTRUMENTGROUPS.push_back(container);
	}
	InstrumentsItem* item = new InstrumentsItem();
	item->instrument = instrument;
	item->index = index;
	container->MEMBERS.push_back(item);
	
}

void MarginEstimator::CleanGroups()
{
	for( vector<InstrumentsContainer*>::iterator iter= INSTRUMENTGROUPS.begin(); iter!=INSTRUMENTGROUPS.end(); iter++)
	{
		delete *iter;
	}
}

double MarginEstimator::CalculateBrutewithGrouping(Margin& ResultMargin)
{
	double Biggest = 0;
	int Power = INSTRUMENTGROUPS.size();
	int OrderSize = ORDERS.size();
	char *DesicionArrayRet = new char[OrderSize+1];
	for(int i=0;i<OrderSize;i++)
	{
		DesicionArrayRet[i]='0';
	}
	DesicionArrayRet[OrderSize]='\0';

	for(int i = 0; i < pow(2.0,Power); i++ )
	{
		Portfolio* pPortfolio = new Portfolio();
		// ADDING INSTRUMENTS
		for(size_t j = 0; j < PORTFOLIO.size(); j++)
		{
			Instrument* instrument = new Instrument();
			*instrument	= *PORTFOLIO[j];
			pPortfolio->AddInstrument(instrument);
		}
		char *DesicionArray = new char[Power+1];
//		_i64toa_s(i,DesicionArray,Power+1,2);							// 64 bit duzeltmesi
		size_t length = strlen(DesicionArray);
		int Shifter = Power - length;

		// ADDING ORDERS
		for (size_t j = 0; j< length; j++)
		{
			if (DesicionArray[j] == '1')
			{
				for (size_t k = 0; k< INSTRUMENTGROUPS[j+Shifter]->MEMBERS.size(); k++)
				{
					Instrument* instrument = new Instrument();
					*instrument	= *INSTRUMENTGROUPS[j+Shifter]->MEMBERS[k]->instrument;		// Assignment operator
					pPortfolio->AddInstrument(instrument);
				}
			}
		}
		Margin Result;

		HRESULT hr = CALCULATOR->Calculate(pPortfolio,Result,_IncludeDMC);
		if (hr == S_FALSE)
		{
			pPortfolio->Clear();
			delete pPortfolio;
			pPortfolio = NULL;
			delete [] DesicionArray;
			DesicionArray = NULL;
			return -1;
		}
		// IF BIGGER UPDATE DesicionArray
		double FirstHalf = 0;
		double SecondHalf = 0;

		double NetOrderOptionValue  = 0;

		if (Result.Initial - Result.NetOptionValue > 0)
		{
			FirstHalf = Result.Initial - Result.NetOptionValue ;
		}
		//if (Result.NetOrderOptionValue + Result.NetIntraDayOptionValue > 0 )// 2016.5.31 - Teminat eksi kyordu!!!! Buray kaldrdk!!!
		{
			SecondHalf = Result.NetOrderOptionValue + Result.NetIntraDayOptionValue;
		}

		if (FirstHalf + SecondHalf > Biggest){
			for (int iD = 0; iD < Power; iD++)
			{
				if (DesicionArray[iD] == '1')
				{
					for (size_t kGroups = 0; kGroups< INSTRUMENTGROUPS[iD+Shifter]->MEMBERS.size(); kGroups++)
					{
						DesicionArrayRet[INSTRUMENTGROUPS[iD+Shifter]->MEMBERS[kGroups]->index] = '1';		// Assignment operator
					}
				}
			}
			//strcpy(DesicionArrayRet,DesicionArray);
			Biggest = FirstHalf + SecondHalf;
			ResultMargin.Initial = Result.Initial;
			ResultMargin.Maintenance = Result.Maintenance;
			ResultMargin.NetOptionValue = Result.NetOptionValue;
			ResultMargin.NetOrderOptionValue = Result.NetOrderOptionValue;
			ResultMargin.NetIntraDayOptionValue = Result.NetIntraDayOptionValue;
			ResultMargin.CarryOut = DesicionArrayRet;
			//ResultMargin.CarryOut[OrderSize] = '\0';
		}	

		
		pPortfolio->Clear();
		delete pPortfolio;
		pPortfolio = NULL;
		delete [] DesicionArray;
		DesicionArray = NULL;
	}
	return Biggest;
}

double MarginEstimator::CalculateBrute(Margin& ResultMargin)
{
	//ofstream* outputFile = new ofstream("brute.txt", fstream::app);
	double Biggest = 0;
	int Power = ORDERS.size();
	char *DesicionArrayRet = new char[Power+1];
	for (__int64_t i = 0; i < pow(2.0,Power) ;i++)
	{
		Portfolio* pPortfolio = new Portfolio();
		// ADDING INSTRUMENTS
		for(size_t j = 0; j < PORTFOLIO.size(); j++)
		{
			Instrument* instrument = new Instrument();
			*instrument	= *PORTFOLIO[j];
			pPortfolio->AddInstrument(instrument);
		}
		char *DesicionArray = new char[Power+1];
//		_i64toa_s(i,DesicionArray,Power+1,2);							// 64 bit d�zeltmesi
		size_t length = strlen(DesicionArray);
		int Shifter = Power - length;

		// ADDING ORDERS
		for (size_t j = 0; j< length; j++)
		{
			if (DesicionArray[j] == '1')
			{
				Instrument* instrument = new Instrument();
				*instrument	= *ORDERS[j+Shifter];		// Assignment operator
				pPortfolio->AddInstrument(instrument);
			}
		}
		Margin Result;
		
		HRESULT hr = CALCULATOR->Calculate(pPortfolio,Result,_IncludeDMC);
		if (hr == S_FALSE)
		{
			pPortfolio->Clear();
			delete pPortfolio;
			pPortfolio = NULL;
			delete [] DesicionArray;
			DesicionArray = NULL;
			return -1;
		}

		// IF BIGGER UPDATE DesicionArray
		double FirstHalf = 0;
		double SecondHalf = 0;

		double NetOrderOptionValue  = 0;
		 
		if (Result.Initial - Result.NetOptionValue > 0)
		{
			FirstHalf = Result.Initial - Result.NetOptionValue ;
		}
		//if (Result.NetOrderOptionValue + Result.NetIntraDayOptionValue > 0 )		// 2016.5.31 - Teminat eksi kyordu!!!! Buray kaldrdk!!!
		{
			SecondHalf = Result.NetOrderOptionValue + Result.NetIntraDayOptionValue;
		}

		if (FirstHalf + SecondHalf > Biggest){
			strcpy(DesicionArrayRet,DesicionArray);
			Biggest = FirstHalf + SecondHalf;
			ResultMargin.Initial = Result.Initial;
			ResultMargin.Maintenance = Result.Maintenance;
			ResultMargin.NetOptionValue = Result.NetOptionValue;
			ResultMargin.NetOrderOptionValue = Result.NetOrderOptionValue;
			ResultMargin.NetIntraDayOptionValue = Result.NetIntraDayOptionValue;
			ResultMargin.CarryOut = DesicionArrayRet;
			ResultMargin.CarryOut[Power] = '\0';
		}	

		//double NetOrderOptionValue  = 0;
		//if (Result.NetOrderOptionValue > 0)
		//{
		//	NetOrderOptionValue = Result.NetOrderOptionValue;	// Sadece ve sadece pozitifse
		//}
		////		*outputFile << "Setting: " << DesicionArray<< " " << Result.Initial<< endl;
		//if (Result.Maintenance - Result.NetOptionValue + NetOrderOptionValue > Biggest){
		//	//strcpy(DesicionArrayRet,DesicionArray);
		//	Biggest = Result.Maintenance - Result.NetOptionValue+ NetOrderOptionValue ;//Result.Maintenance;
		//	ResultMargin.Initial = Result.Initial;
		//	ResultMargin.Maintenance = Result.Maintenance;
		//	ResultMargin.NetOptionValue = Result.NetOptionValue;
		//	ResultMargin.NetOrderOptionValue = Result.NetOrderOptionValue;
		//}	
		pPortfolio->Clear();
		delete pPortfolio;
		pPortfolio = NULL;
		delete [] DesicionArray;
		DesicionArray = NULL;
	}
	

	//*outputFile << Biggest << endl;
	//delete outputFile;
	return Biggest;
}

double MarginEstimator::CalculatewithIterationwithGrouping(Margin& ResultMargin)
{
		Count = 0;
	//Margin ResultMargin;
	Margin ReturnMargin;
	//ofstream* outputFile = new ofstream("iterate.txt", fstream::app);
	double Biggest = 0;
	double BiggestRighttoLeft = 0;
	double Biggest1= 0;
	double Biggest1RighttoLeft = 0;
	double Biggest01= 0;
	double Biggest01RighttoLeft = 0;

	int Power			= INSTRUMENTGROUPS.size();
	int TotalOrdersSize = ORDERS.size();


	string ResultArray;
	string tempResultArray;
	//char* StartingArray0 = new char[Power];
	//char* StartingArray1 = new char[Power];
	//char* StartingArray2 = new char[Power];

	char* StartingArray0 = new char[Power+1];
	char* StartingArray1 = new char[Power+1];
	char* StartingArray2 = new char[Power+1];

	char* DecisionArray = new char[TotalOrdersSize+1];

	StartingArray0[Power] = '/0';
	StartingArray1[Power] = '/0';
	StartingArray2[Power] = '/0';

	for(int i=0;i<TotalOrdersSize;i++)
	{
		DecisionArray[i]='0';
	}

	DecisionArray[TotalOrdersSize] = '/0';

	for (int i = 0; i < Power; i++)
	{
		StartingArray0[i] = '0';
	}

	for (int i = 0; i < Power; i++)
	{
		StartingArray1[i] = '1';
	}

	for (int i = 0; i < Power/2; i++)
	{
		StartingArray2[i*2] = '0';
		StartingArray2[i*2+1] = '1';

	}

	string StartingArray0Str(StartingArray0,Power);
	string StartingArray1Str(StartingArray1,Power);
	string StartingArray01Str(StartingArray2,Power);

	Biggest = CalculatewithIterationwithGrouping(StartingArray0Str,Power,true,tempResultArray,ReturnMargin);
	if (Biggest == -1)
		return Biggest;
	ResultArray = tempResultArray;
	ResultMargin.Initial = ReturnMargin.Initial;
	ResultMargin.Maintenance = ReturnMargin.Maintenance;
	ResultMargin.NetOptionValue = ReturnMargin.NetOptionValue;
	ResultMargin.NetOrderOptionValue = ReturnMargin.NetOrderOptionValue;
	ResultMargin.NetIntraDayOptionValue = ReturnMargin.NetIntraDayOptionValue;


	if ((BiggestRighttoLeft = CalculatewithIterationwithGrouping(StartingArray0Str,Power,false,tempResultArray,ReturnMargin)) == -1)
		return BiggestRighttoLeft;
	if (BiggestRighttoLeft > Biggest)
	{
		Biggest = BiggestRighttoLeft;
		ResultMargin.Initial = ReturnMargin.Initial;
		ResultMargin.Maintenance = ReturnMargin.Maintenance;
		ResultMargin.NetOptionValue = ReturnMargin.NetOptionValue;
		ResultMargin.NetOrderOptionValue = ReturnMargin.NetOrderOptionValue;
		ResultMargin.NetIntraDayOptionValue = ReturnMargin.NetIntraDayOptionValue;
		ResultArray = tempResultArray;
	}

	if ((Biggest1 = CalculatewithIterationwithGrouping(StartingArray1Str,Power,true,tempResultArray,ReturnMargin)) == -1)
		return Biggest1;
	if (Biggest1 > Biggest)
	{
		Biggest = Biggest1;
		ResultMargin.Initial = ReturnMargin.Initial;
		ResultMargin.Maintenance = ReturnMargin.Maintenance;
		ResultMargin.NetOptionValue = ReturnMargin.NetOptionValue;
		ResultMargin.NetOrderOptionValue = ReturnMargin.NetOrderOptionValue;
		ResultMargin.NetIntraDayOptionValue = ReturnMargin.NetIntraDayOptionValue;
		ResultArray = tempResultArray;
	}

	if ((Biggest1RighttoLeft = CalculatewithIterationwithGrouping(StartingArray1Str,Power,false,tempResultArray,ReturnMargin)) == -1)
		return Biggest1RighttoLeft;
	if (Biggest1RighttoLeft > Biggest)
	{
		Biggest = Biggest1RighttoLeft;
		ResultMargin.Initial = ReturnMargin.Initial;
		ResultMargin.Maintenance = ReturnMargin.Maintenance;
		ResultMargin.NetOptionValue = ReturnMargin.NetOptionValue;
		ResultMargin.NetOrderOptionValue = ReturnMargin.NetOrderOptionValue;
		ResultMargin.NetIntraDayOptionValue = ReturnMargin.NetIntraDayOptionValue;
		ResultArray = tempResultArray;
	}

	if ((Biggest01 = CalculatewithIterationwithGrouping(StartingArray01Str,Power,true,tempResultArray,ReturnMargin)) == -1)
		return Biggest01;
	if (Biggest01 > Biggest)
	{
		Biggest = Biggest01;
		ResultMargin.Initial = ReturnMargin.Initial;
		ResultMargin.Maintenance = ReturnMargin.Maintenance;
		ResultMargin.NetOptionValue = ReturnMargin.NetOptionValue;
		ResultMargin.NetOrderOptionValue = ReturnMargin.NetOrderOptionValue;
		ResultMargin.NetIntraDayOptionValue = ReturnMargin.NetIntraDayOptionValue;
		ResultArray = tempResultArray;
	}

	if ((Biggest01RighttoLeft = CalculatewithIterationwithGrouping(StartingArray01Str,Power,false,tempResultArray,ReturnMargin)) == -1)
		return Biggest01RighttoLeft;
	if (Biggest01RighttoLeft > Biggest)
	{
		Biggest = Biggest01RighttoLeft;
		ResultMargin.Initial = ReturnMargin.Initial;
		ResultMargin.Maintenance = ReturnMargin.Maintenance;
		ResultMargin.NetOptionValue = ReturnMargin.NetOptionValue;
		ResultMargin.NetOrderOptionValue = ReturnMargin.NetOrderOptionValue;
		ResultMargin.NetIntraDayOptionValue = ReturnMargin.NetIntraDayOptionValue;
		ResultArray = tempResultArray;
	}
	
	for (int iD = 0; iD < Power; iD++)
	{
		if (ResultArray[iD] == '1')
		{
			for (size_t kGroups = 0; kGroups< INSTRUMENTGROUPS[iD]->MEMBERS.size(); kGroups++)
			{
				DecisionArray[INSTRUMENTGROUPS[iD]->MEMBERS[kGroups]->index] = '1';		// Assignment operator
			}
		}
	}

	int Diff = Power - ResultArray.size();

	ResultMargin.CarryOut = DecisionArray;
	//ResultMargin.CarryOut[Power] = '/0';



	delete [] StartingArray0;
	delete [] StartingArray1;
	delete [] StartingArray2;
	delete [] DecisionArray;
	StartingArray0 = NULL;
	StartingArray1 = NULL;
	StartingArray2 = NULL;
	
	DecisionArray = NULL;

	//*outputFile << Count  << endl;
	//

	//delete outputFile;

	return Biggest;
}

double MarginEstimator::CalculatewithIterationwithGrouping(string StartingArray, int Length, bool LefttoRight, string& ResultArray, Margin& ResultMargin, bool Shuffle/* = false*/)
{
	try{
		double Biggest = 0;
		double BiggestTemp = 0;
		string DesicionArray = StartingArray;
		bool CalculateNext = true;
		ResultMargin.Initial = 0;
		ResultMargin.Maintenance = 0;
		ResultMargin.NetOptionValue = 0;
		ResultMargin.NetOrderOptionValue = 0;
		ResultMargin.NetIntraDayOptionValue = 0;

		while(CalculateNext)
		{
			//string temp = DesicionArray;
			BiggestTemp = Biggest;

			for(int i = 0; i< Length; i++)
			{
				string ModifiedTemp = DesicionArray;	
				Portfolio* pPortfolio = new Portfolio();

				// ADDING INSTRUMENTS
				for(size_t j = 0; j < PORTFOLIO.size(); j++)
				{
					Instrument* instrument = new Instrument();
					try{
						*instrument	= *PORTFOLIO[j];
					}catch(const std::exception& ex)
					{
						ofstream* outputFile = new ofstream("hata4.txt", fstream::app);
						*outputFile << ex.what() << " portfoy" << endl;
						delete outputFile;
						return -1;
					}
					pPortfolio->AddInstrument(instrument);
				}

				if (ModifiedTemp[i] == '1')
					ModifiedTemp[i] = '0';
				else
					ModifiedTemp[i] = '1';
				for (int j= 0; j < Length ; j++)
				{
					if (LefttoRight)
					{
						if (ModifiedTemp[j] == '1') 
						{

							// ADDING ORDERS
							
							for (size_t k = 0; k< INSTRUMENTGROUPS[j]->MEMBERS.size(); k++)
							{
								try{
									Instrument* instrument = new Instrument();
									*instrument	= *INSTRUMENTGROUPS[j]->MEMBERS[k]->instrument;		// Assignment operator
									pPortfolio->AddInstrument(instrument);
								}
								catch(const std::exception& ex)
								{
									ofstream* outputFile = new ofstream("hata4.txt", fstream::app);
									*outputFile << ex.what()<< " order" << endl;
									delete outputFile;
									return -1;
								}
							}
							

							//// ADD ORDER
							//Instrument* instrument = new Instrument();
							//try{
							//	*instrument	= *ORDERS[j];		// Assignment
							//}catch(const std::exception& ex)
							//{
							//	ofstream* outputFile = new ofstream("hata4.txt", fstream::app);
							//	*outputFile << ex.what()<< " order" << endl;
							//	delete outputFile;
							//	return -1;
							//}
							//pPortfolio->AddInstrument(instrument);
						}
					}
					else
					{
						if (ModifiedTemp[Length-(j+1)] == '1')
						{

							// ADDING ORDERS

							for (size_t k = 0; k< INSTRUMENTGROUPS[j]->MEMBERS.size(); k++)
							{
								try{
									Instrument* instrument = new Instrument();
									*instrument	= *INSTRUMENTGROUPS[j]->MEMBERS[k]->instrument;		// Assignment operator
									pPortfolio->AddInstrument(instrument);
								}
								catch(const std::exception& ex)
								{
									ofstream* outputFile = new ofstream("hata4.txt", fstream::app);
									*outputFile << ex.what()<< " order" << endl;
									delete outputFile;
									return -1;
								}
							}

							//// ADD ORDER
							//Instrument* instrument = new Instrument();
							//try{
							//	*instrument	= *ORDERS[j];		// Assignment
							//}catch(const std::exception& ex)
							//{
							//	ofstream* outputFile = new ofstream("hataiic1.txt", fstream::app);
							//	*outputFile << ex.what()<< " order"<< endl;
							//	delete outputFile;
							//	return -1;
							//}
							//pPortfolio->AddInstrument(instrument);
						}
					}

				}


				// CALCULATE
				Margin Result;
				HRESULT hr;

				hr = CALCULATOR->Calculate(pPortfolio,Result,_IncludeDMC);
				Count++;


				if (hr == S_FALSE)
				{
					pPortfolio->Clear();
					delete pPortfolio;
					//delete [] DesicionArray;
					return -1;
				}
				// IF BIGGER UPDATE DesicionArray
				double FirstHalf = 0;
				double SecondHalf = 0;

				double NetOrderOptionValue  = 0;

				if (Result.Initial - Result.NetOptionValue > 0)
				{
					FirstHalf = Result.Initial - Result.NetOptionValue ;
				}
				//if (Result.NetOrderOptionValue + Result.NetIntraDayOptionValue > 0 )// 2016.5.31 - Teminat eksi kyordu!!!! Buray kaldrdk!!!
				{
					SecondHalf = Result.NetOrderOptionValue + Result.NetIntraDayOptionValue;
				}

				if (FirstHalf + SecondHalf > Biggest){
					DesicionArray = ModifiedTemp;
					Biggest = FirstHalf + SecondHalf;
					ResultMargin.Initial = Result.Initial;
					ResultMargin.Maintenance = Result.Maintenance;
					ResultMargin.NetOptionValue = Result.NetOptionValue;
					ResultMargin.NetOrderOptionValue = Result.NetOrderOptionValue;
					ResultMargin.NetIntraDayOptionValue = Result.NetIntraDayOptionValue;
				}	


				//if (Result.NetOrderOptionValue > 0)
				//{
				//	NetOrderOptionValue = Result.NetOrderOptionValue;	// Sadece ve sadece pozitifse
				//}
				//if (Result.Maintenance - Result.NetOptionValue + NetOrderOptionValue> Biggest){
				//	DesicionArray = ModifiedTemp;
				//	Biggest = Result.Maintenance - Result.NetOptionValue + NetOrderOptionValue ;
				//	ResultMargin.Initial = Result.Initial;
				//	ResultMargin.Maintenance = Result.Maintenance;
				//	ResultMargin.NetOptionValue = Result.NetOptionValue;
				//	ResultMargin.NetOrderOptionValue = Result.NetOrderOptionValue;
				//	
				//}	
				pPortfolio->Clear();
				delete pPortfolio;
				pPortfolio = NULL;
			}
			if (BiggestTemp == Biggest)
			{
				CalculateNext = false;
			}
		}
		ResultArray = DesicionArray;
		return Biggest;
	}
	catch(const std::exception& ex)
	{
		ofstream* outputFile = new ofstream("hataic2.txt", fstream::app);
		*outputFile << ex.what()<< endl;
		delete outputFile;
		return -1;
	}
}

double MarginEstimator::CalculatewithIteration(Margin& ResultMargin)
{
	Count = 0;
	//Margin ResultMargin;
	Margin ReturnMargin;
	//ofstream* outputFile = new ofstream("iterate.txt", fstream::app);
	double Biggest = 0;
	double BiggestRighttoLeft = 0;
	double Biggest1= 0;
	double Biggest1RighttoLeft = 0;
	double Biggest01= 0;
	double Biggest01RighttoLeft = 0;

	int Power = ORDERS.size();
	string ResultArray;
	string tempResultArray;
	//char* StartingArray0 = new char[Power];
	//char* StartingArray1 = new char[Power];
	//char* StartingArray2 = new char[Power];

	char* StartingArray0 = new char[Power+1];
	char* StartingArray1 = new char[Power+1];
	char* StartingArray2 = new char[Power+1];

	StartingArray0[Power] = '/0';
	StartingArray1[Power] = '/0';
	StartingArray2[Power] = '/0';


	for (int i = 0; i < Power; i++)
	{
		StartingArray0[i] = '0';
	}

	for (int i = 0; i < Power; i++)
	{
		StartingArray1[i] = '1';
	}

	for (int i = 0; i < Power/2; i++)
	{
		StartingArray2[i*2] = '0';
		StartingArray2[i*2+1] = '1';

	}

	string StartingArray0Str(StartingArray0,Power);
	string StartingArray1Str(StartingArray1,Power);
	string StartingArray01Str(StartingArray2,Power);

	Biggest = CalculatewithIteration(StartingArray0Str,Power,true,tempResultArray,ReturnMargin);
	if (Biggest == -1)
		return Biggest;
	ResultArray = tempResultArray;
	ResultMargin.Initial = ReturnMargin.Initial;
	ResultMargin.Maintenance = ReturnMargin.Maintenance;
	ResultMargin.NetOptionValue = ReturnMargin.NetOptionValue;
	ResultMargin.NetOrderOptionValue = ReturnMargin.NetOrderOptionValue;
	ResultMargin.NetIntraDayOptionValue = ReturnMargin.NetIntraDayOptionValue;


	if ((BiggestRighttoLeft = CalculatewithIteration(StartingArray0Str,Power,false,tempResultArray,ReturnMargin)) == -1)
		return BiggestRighttoLeft;
	if (BiggestRighttoLeft > Biggest)
	{
		Biggest = BiggestRighttoLeft;
		ResultMargin.Initial = ReturnMargin.Initial;
		ResultMargin.Maintenance = ReturnMargin.Maintenance;
		ResultMargin.NetOptionValue = ReturnMargin.NetOptionValue;
		ResultMargin.NetOrderOptionValue = ReturnMargin.NetOrderOptionValue;
		ResultMargin.NetIntraDayOptionValue = ReturnMargin.NetIntraDayOptionValue;
		ResultArray = tempResultArray;
	}

	if ((Biggest1 = CalculatewithIteration(StartingArray1Str,Power,true,tempResultArray,ReturnMargin)) == -1)
		return Biggest1;
	if (Biggest1 > Biggest)
	{
		Biggest = Biggest1;
		ResultMargin.Initial = ReturnMargin.Initial;
		ResultMargin.Maintenance = ReturnMargin.Maintenance;
		ResultMargin.NetOptionValue = ReturnMargin.NetOptionValue;
		ResultMargin.NetOrderOptionValue = ReturnMargin.NetOrderOptionValue;
		ResultMargin.NetIntraDayOptionValue = ReturnMargin.NetIntraDayOptionValue;
		ResultArray = tempResultArray;
	}

	if ((Biggest1RighttoLeft = CalculatewithIteration(StartingArray1Str,Power,false,tempResultArray,ReturnMargin)) == -1)
		return Biggest1RighttoLeft;
	if (Biggest1RighttoLeft > Biggest)
	{
		Biggest = Biggest1RighttoLeft;
		ResultMargin.Initial = ReturnMargin.Initial;
		ResultMargin.Maintenance = ReturnMargin.Maintenance;
		ResultMargin.NetOptionValue = ReturnMargin.NetOptionValue;
		ResultMargin.NetOrderOptionValue = ReturnMargin.NetOrderOptionValue;
		ResultMargin.NetIntraDayOptionValue = ReturnMargin.NetIntraDayOptionValue;
		ResultArray = tempResultArray;
	}

	if ((Biggest01 = CalculatewithIteration(StartingArray01Str,Power,true,tempResultArray,ReturnMargin)) == -1)
		return Biggest01;
	if (Biggest01 > Biggest)
	{
		Biggest = Biggest01;
		ResultMargin.Initial = ReturnMargin.Initial;
		ResultMargin.Maintenance = ReturnMargin.Maintenance;
		ResultMargin.NetOptionValue = ReturnMargin.NetOptionValue;
		ResultMargin.NetOrderOptionValue = ReturnMargin.NetOrderOptionValue;
		ResultMargin.NetIntraDayOptionValue = ReturnMargin.NetIntraDayOptionValue;
		ResultArray = tempResultArray;
	}

	if ((Biggest01RighttoLeft = CalculatewithIteration(StartingArray01Str,Power,false,tempResultArray,ReturnMargin)) == -1)
		return Biggest01RighttoLeft;
	if (Biggest01RighttoLeft > Biggest)
	{
		Biggest = Biggest01RighttoLeft;
		ResultMargin.Initial = ReturnMargin.Initial;
		ResultMargin.Maintenance = ReturnMargin.Maintenance;
		ResultMargin.NetOptionValue = ReturnMargin.NetOptionValue;
		ResultMargin.NetOrderOptionValue = ReturnMargin.NetOrderOptionValue;
		ResultMargin.NetIntraDayOptionValue = ReturnMargin.NetIntraDayOptionValue;
		ResultArray = tempResultArray;
	}
	
	int Diff = Power - ResultArray.size();

	ResultMargin.CarryOut = ResultArray;
	//ResultMargin.CarryOut[Power] = '/0';



	delete [] StartingArray0;
	delete [] StartingArray1;
	delete [] StartingArray2;
	StartingArray0 = NULL;
	StartingArray1 = NULL;
	StartingArray2 = NULL;
	
	//*outputFile << Count  << endl;
	//

	//delete outputFile;

	return Biggest;
}

double MarginEstimator::CalculatewithIteration(string StartingArray, int Length, bool LefttoRight, string& ResultArray, Margin& ResultMargin, bool Shuffle /*= false*/)
{
	try{
	double Biggest = 0;
	double BiggestTemp = 0;
	string DesicionArray = StartingArray;
	bool CalculateNext = true;
	ResultMargin.Initial = 0;
	ResultMargin.Maintenance = 0;
	ResultMargin.NetOptionValue = 0;
	ResultMargin.NetOrderOptionValue = 0;
	ResultMargin.NetIntraDayOptionValue = 0;

	while(CalculateNext)
	{
		//string temp = DesicionArray;
		BiggestTemp = Biggest;

		for(int i = 0; i< Length; i++)
		{
			string ModifiedTemp = DesicionArray;	
			Portfolio* pPortfolio = new Portfolio();
			
			// ADDING INSTRUMENTS
			for(size_t j = 0; j < PORTFOLIO.size(); j++)
			{
				Instrument* instrument = new Instrument();
		try{
				*instrument	= *PORTFOLIO[j];
			}catch(const std::exception& ex)
			{
				ofstream* outputFile = new ofstream("hata4.txt", fstream::app);
				*outputFile << ex.what() << " portfoy" << endl;
				delete outputFile;
				return -1;
			}
				pPortfolio->AddInstrument(instrument);
			}
			
			if (ModifiedTemp[i] == '1')
				ModifiedTemp[i] = '0';
			else
				ModifiedTemp[i] = '1';
			for (int j= 0; j < Length ; j++)
			{
				if (LefttoRight)
				{
					if (ModifiedTemp[j] == '1') 
					{
						// ADD ORDER
						Instrument* instrument = new Instrument();
					try{
						*instrument	= *ORDERS[j];		// Assignment
					}catch(const std::exception& ex)
					{
						ofstream* outputFile = new ofstream("hata4.txt", fstream::app);
						*outputFile << ex.what()<< " order" << endl;
						delete outputFile;
						return -1;
					}
						pPortfolio->AddInstrument(instrument);
					}
				}
				else
				{
					if (ModifiedTemp[Length-(j+1)] == '1')
					{
						// ADD ORDER
						Instrument* instrument = new Instrument();
						try{
						*instrument	= *ORDERS[j];		// Assignment
						}catch(const std::exception& ex)
						{
							ofstream* outputFile = new ofstream("hataiic1.txt", fstream::app);
							*outputFile << ex.what()<< " order"<< endl;
							delete outputFile;
							return -1;
						}
						pPortfolio->AddInstrument(instrument);
					}
				}
				
			}
			
			
			// CALCULATE
			Margin Result;
			HRESULT hr;
			
			hr = CALCULATOR->Calculate(pPortfolio,Result,_IncludeDMC);
			Count++;

			
			if (hr == S_FALSE)
			{
				pPortfolio->Clear();
				delete pPortfolio;
				//delete [] DesicionArray;
				return -1;
			}
			// IF BIGGER UPDATE DesicionArray
			double FirstHalf = 0;
			double SecondHalf = 0;
			
			double NetOrderOptionValue  = 0;
			
			if (Result.Initial - Result.NetOptionValue > 0)
			{
				FirstHalf = Result.Initial - Result.NetOptionValue ;
			}
			//if (Result.NetOrderOptionValue + Result.NetIntraDayOptionValue > 0 )// 2016.5.31 - Teminat eksi kyordu!!!! Buray kaldrdk!!!
			{
				SecondHalf = Result.NetOrderOptionValue + Result.NetIntraDayOptionValue;
			}
			
			if (FirstHalf + SecondHalf > Biggest){
				DesicionArray = ModifiedTemp;
				Biggest = FirstHalf + SecondHalf;
				ResultMargin.Initial = Result.Initial;
				ResultMargin.Maintenance = Result.Maintenance;
				ResultMargin.NetOptionValue = Result.NetOptionValue;
				ResultMargin.NetOrderOptionValue = Result.NetOrderOptionValue;
				ResultMargin.NetIntraDayOptionValue = Result.NetIntraDayOptionValue;
			}	
			
			
			//if (Result.NetOrderOptionValue > 0)
			//{
			//	NetOrderOptionValue = Result.NetOrderOptionValue;	// Sadece ve sadece pozitifse
			//}
			//if (Result.Maintenance - Result.NetOptionValue + NetOrderOptionValue> Biggest){
			//	DesicionArray = ModifiedTemp;
			//	Biggest = Result.Maintenance - Result.NetOptionValue + NetOrderOptionValue ;
			//	ResultMargin.Initial = Result.Initial;
			//	ResultMargin.Maintenance = Result.Maintenance;
			//	ResultMargin.NetOptionValue = Result.NetOptionValue;
			//	ResultMargin.NetOrderOptionValue = Result.NetOrderOptionValue;
			//	
			//}	
			pPortfolio->Clear();
			delete pPortfolio;
			pPortfolio = NULL;
		}
		if (BiggestTemp == Biggest)
		{
			CalculateNext = false;
		}
	}
	ResultArray = DesicionArray;
	return Biggest;
	}
	catch(const std::exception& ex)
	{
		ofstream* outputFile = new ofstream("hataic2.txt", fstream::app);
		*outputFile << ex.what()<< endl;
		delete outputFile;
		return -1;
	}
}

MarginEstimator::~MarginEstimator()
{
	ORDERS.clear();
	PORTFOLIO.clear();
	INSTRUMENTGROUPS.clear();
}
