//#include "StdAfx.h"
#include "tinyxml.h"
#include "SpanXMLParser.h"
#include "Pricer.h"
using namespace MARGIN;
using namespace std;

//Physical::Physical(string Symbol, int ID)
//	:_ID(ID), _Symbol(Symbol)
//{
// 
//}

//Option::Option(string Symbol, int ID, int UnderlyingID, ExerciseType EType, string Maturity, double Strike, bool IsCall )
//	:_Symbol(Symbol),_ID(ID),_Strike(Strike),_IsCall(IsCall),_EType(EType)
//{
//
//}

//Futures::Futures(string Symbol, int ID, int UnderlyingID, string Maturity, double Strike, bool IsCall)
//	:_Symbol(Symbol),_ID(ID),_Strike(Strike),_IsCall(IsCall)
//{
//
//}

//Underlying* FindUnderlying(int ID, map<int, Underlying*>& Underlyings)
//{
//	map<int, Underlying*>::iterator it;
//	it = Underlyings.find(ID);
//	if (it != Underlyings.end())
//		return it->second;
//	return NULL;
//}

double ScenarioMultipliers[16][2] =
{
	{ 0,        1},
	{ 0,       -1},
	{ 1.0/3.0,  1},
	{ 1.0/3.0, -1},
	{-1.0/3.0,  1},
	{-1.0/3.0, -1},
	{ 2.0/3.0,  1},
	{ 2.0/3.0, -1},
	{-2.0/3.0,  1},
	{-2.0/3.0, -1},
	{1,			1},
	{1,        -1},
	{-1,        1},
	{-1,       -1},
	{3,			0},
	{-3,		0}
};

void Option::UpdateValues(double newUnderlyingPrice, double newVolatility, double newPrice, double newRiskFreeRate, vector<ScenarioPoint*> Scenarios)
{
	for (int i = 0; i < 16; i++)
	{
		raRT[i] = ((newPrice - BlackScholes(IsCall,newUnderlyingPrice + Scenarios[i]->PriceMultiplier*(PriceScan/cvf),Strike, TimetoMaturity -(1.0/252.0), newRiskFreeRate, newVolatility * (1 + Scenarios[i]->VolMultiplier*VolScan)))*Scenarios[i]->Weight)*cvf;
		//raRT[i] = ((BlackScholes(IsCall,newUnderlyingPrice + Scenarios[i]->PriceMultiplier*0,Strike, TimetoMaturity, newRiskFreeRate, newVolatility + Scenarios[i]->VolMultiplier*0) - newPrice)*Scenarios[i]->Weight)* (-1);
		//raRT[i] = ((callOptionPrice(newUnderlyingPrice + Scenarios[i]->PriceMultiplier*PriceScan,TimetoMaturity,Strike,newRiskFreeRate,newVolatility + Scenarios[i]->VolMultiplier*VolScan) - newPrice)*Scenarios[i]->Weight)*(-1);
	}
	PriceRT = newPrice;
	
}

void Option::ResetValues()
{
	for (int i = 0; i < 16; i++)
	{
		raRT[i] = ra[i];
		//raRT[i] = ((BlackScholes(IsCall,newUnderlyingPrice + Scenarios[i]->PriceMultiplier*0,Strike, TimetoMaturity, newRiskFreeRate, newVolatility + Scenarios[i]->VolMultiplier*0) - newPrice)*Scenarios[i]->Weight)* (-1);
		//raRT[i] = ((callOptionPrice(newUnderlyingPrice + Scenarios[i]->PriceMultiplier*PriceScan,TimetoMaturity,Strike,newRiskFreeRate,newVolatility + Scenarios[i]->VolMultiplier*VolScan) - newPrice)*Scenarios[i]->Weight)*(-1);
	}
	PriceRT = Price;

}

void Futures::UpdateValues(double newPrice, vector<ScenarioPoint*> Scenarios)
{
	for (int i = 0; i < 16; i++)
	{
		raRT[i] = ((newPrice + Scenarios[i]->PriceMultiplier*(PriceScan/cvf) - newPrice)*Scenarios[i]->Weight)*(-1);
	}
}

int FillScanPairs(TiXmlDocument* XMLDOC, vector<pair<int,int>>& ScanPointPairs, vector<ScenarioPoint*>& Scenarios)
{
	int i= 0;

	TiXmlHandle hDOC(XMLDOC);
	TiXmlElement *pointDef = hDOC.FirstChild("spanFile").FirstChild( "pointInTime" ).FirstChild( "clearingOrg" ).FirstChild( "pointDef" ).ToElement();

	TiXmlElement *scanPointDef = pointDef->FirstChildElement("scanPointDef");
	while(scanPointDef != NULL)
	{
		i++;
		TiXmlElement* point = scanPointDef->FirstChildElement("point");
		TiXmlNode* node = point->FirstChild();
		int pointID = atoi(node->Value())-1;

		TiXmlElement* pairedPoint = scanPointDef->FirstChildElement("pairedPoint");
		node = pairedPoint->FirstChild();
		int pairedpointID = atoi(node->Value())-1;

		ScanPointPairs.push_back(pair<int,int>(pointID,pairedpointID));


		double multPrice	= atof(scanPointDef->FirstChild("priceScanDef")->FirstChild("mult")->FirstChild()->Value());
		double multVol		= atof(scanPointDef->FirstChild("volScanDef")->FirstChild("mult")->FirstChild()->Value());
		double weight		= atof(scanPointDef->FirstChild("weight")->FirstChild()->Value());

		ScenarioPoint* scenarioPoint = new ScenarioPoint();
		scenarioPoint->PriceMultiplier = multPrice;
		scenarioPoint->VolMultiplier = multVol;
		scenarioPoint->Weight = weight;

		Scenarios.push_back(scenarioPoint);

		scanPointDef = scanPointDef->NextSiblingElement("scanPointDef");

	}

	return i;
}

int FillPhysicals(TiXmlDocument* XMLDOC,vector<Physical*>& Physicals)
{
	TiXmlHandle hDOC(XMLDOC);
	TiXmlElement *exchange = hDOC.FirstChild("spanFile").FirstChild( "pointInTime" ).FirstChild( "clearingOrg" ).FirstChild( "exchange" ).ToElement();

	TiXmlElement *phyPf = exchange->FirstChildElement("phyPf");
	
	if (phyPf == NULL)
		return 0;		// No Physicals
	
	int i = 0;
	while (phyPf != NULL)
	{
		i++;
		TiXmlElement* pfId = phyPf->FirstChildElement("pfId");
		TiXmlNode* node = pfId->FirstChild();
		int ID = atoi(node->Value()); 
		TiXmlElement* pfCode = phyPf->FirstChildElement("pfCode");
		node = pfCode->FirstChild();
		string Code = node->Value(); 
		TiXmlElement* pfName = phyPf->FirstChildElement("name");
		node = pfName->FirstChild();
		string Name = node->Value(); 
	//	cout << "Id: " << ID << ", Code: " << Code << ", Name: " << Name << endl;
		Physical* pPhy = new Physical();
		pPhy->ID = ID;
		pPhy->Code = Code;
		pPhy->Name = Name;
		Physicals.push_back(pPhy);

		phyPf = phyPf->NextSiblingElement("phyPf");

	}
	
	return i;
}


int FillOptionsON(string what,TiXmlDocument* XMLDOC,map<OptionKey, Option*>& OptionContracts)
{
	TiXmlHandle hDOC(XMLDOC);
	TiXmlElement *exchange = hDOC.FirstChild("spanFile").FirstChild( "pointInTime" ).FirstChild( "clearingOrg" ).FirstChild( "exchange" ).ToElement();

	TiXmlElement *oopPf = exchange->FirstChildElement(what.c_str());

	if (oopPf == NULL)
		return 0;		// No options

	int i = 0;
	while (oopPf != NULL)
	{
		i++;
		TiXmlElement* pfId = oopPf->FirstChildElement("pfId");
		TiXmlNode* node = pfId->FirstChild();
		int ID = atoi(node->Value()); 
		TiXmlElement* pfCode = oopPf->FirstChildElement("pfCode");
		node = pfCode->FirstChild();
		string Code = node->Value(); 
		//cout << "Id: " << ID << ", Code: " << Code << endl;
		TiXmlElement* pfName = oopPf->FirstChildElement("name");
		node = pfName->FirstChild();
		string Name = node->Value();

		TiXmlElement* pcvf = oopPf->FirstChildElement("cvf");
		node = pcvf->FirstChild();
		double cvf = atof(node->Value());

		TiXmlElement* undPfID = oopPf->FirstChild("undPf")->FirstChild("pfId")->ToElement();
		node = undPfID->FirstChild();
		int UnderlyingID = atoi(node->Value()); 
		TiXmlElement* exercise = oopPf->FirstChildElement("exercise");
		node = exercise->FirstChild();
		string EXERC = node->Value();

		TiXmlElement* series = oopPf->FirstChildElement("series");
		
		while (series != NULL)
		{
			string pe = series->FirstChildElement("pe")->FirstChild()->Value();
			double t = atof(series->FirstChildElement("t")->FirstChild()->Value());
			TiXmlElement* pcvf2 = series->FirstChildElement("cvf");
			node = pcvf2->FirstChild();
			double cvfR = atof(node->Value());
			
			
			double priceScan = atof(series->FirstChildElement("scanRate")->FirstChildElement("priceScan")->FirstChild()->Value());
			double volScanPct = atof(series->FirstChildElement("scanRate")->FirstChildElement("volScanPct")->FirstChild()->Value());

			
			TiXmlElement* opt = series->FirstChildElement("opt");
			while (opt != NULL)
			{
				int CCID = atoi(opt->FirstChildElement("cId")->FirstChild()->Value());
				string C = opt->FirstChildElement("o")->FirstChild()->Value();
				bool isCall = (C == "C");
				double Strike = atof(opt->FirstChildElement("k")->FirstChild()->Value());
				double Price = atof(opt->FirstChildElement("p")->FirstChild()->Value());
				TiXmlElement* a = opt->FirstChild("ra")->FirstChild("a")->ToElement();
				int aIndex = 0;
				Option* pOop = new Option();
				
				pOop->Code = Code;
				if (EXERC == "AMER")
				{
					pOop->EType = AMERICAN;
				}
				else 
					pOop->EType = EUROPIAN;
				pOop->ID = ID;
				pOop->IsCall = isCall;
				pOop->Maturity = pe;
				pOop->Name = Name;
				pOop->Strike = Strike;
				pOop->UnderlyingID = UnderlyingID;
				pOop->cvf = cvfR;
				pOop->Price = Price;
				pOop->PriceRT = Price;
				pOop->PriceScan = priceScan;
				pOop->VolScan = volScanPct;
				pOop->TimetoMaturity = t;
				while (a != NULL)
				{
					pOop->ra[aIndex] = atof(a->FirstChild()->Value());
					pOop->raRT[aIndex] = pOop->ra[aIndex];
					aIndex ++;
					a = a->NextSiblingElement();
				}

				pOop->delta = atof(opt->FirstChild("ra")->FirstChild("d")->ToElement()->FirstChild()->Value());
				pOop->deltaRT = pOop->delta;

				OptionContracts.insert(std::make_pair(OptionKey(Code, pe,isCall,pOop->EType,Strike), pOop));
				//OptionContracts.push_back(pOop);

				opt = opt->NextSiblingElement("opt");
			}
			series = series->NextSiblingElement("series");

		}

		oopPf = oopPf->NextSiblingElement(what.c_str());
	}

	return i;
}

int FillOptions(TiXmlDocument* XMLDOC,map<OptionKey, Option*>& OptionContracts)
{
	int Count= 	FillOptionsON("oofPf",XMLDOC,OptionContracts);
	Count += FillOptionsON("oopPf",XMLDOC,OptionContracts);
	Count += FillOptionsON("oocPf",XMLDOC,OptionContracts);
	return Count;
}


int FillFutures(TiXmlDocument* XMLDOC,map<pair<string,string>, Futures*>& FuturesContracts)
{
	TiXmlHandle hDOC(XMLDOC);
	TiXmlElement *exchange = hDOC.FirstChild("spanFile").FirstChild( "pointInTime" ).FirstChild( "clearingOrg" ).FirstChild( "exchange" ).ToElement();

	TiXmlElement *futPf = exchange->FirstChildElement("futPf");

	if (futPf == NULL)
		return 0;		// No Futures

	int i = 0;
	while (futPf != NULL)
	{
		i++;
		TiXmlElement* pfId = futPf->FirstChildElement("pfId");
		TiXmlNode* node = pfId->FirstChild();
		int ID = atoi(node->Value()); 
		TiXmlElement* pfCode = futPf->FirstChildElement("pfCode");
		node = pfCode->FirstChild();
		string Code = node->Value(); 
		//cout << "Id: " << ID << ", Code: " << Code << endl;
		TiXmlElement* pfName = futPf->FirstChildElement("name");
		node = pfName->FirstChild();
		string Name = node->Value();
		TiXmlElement* undPfID = futPf->FirstChild("undPf")->FirstChild("pfId")->ToElement();
		node = undPfID->FirstChild();
		int UnderlyingID = atoi(node->Value()); 

		TiXmlElement* fut = futPf->FirstChildElement("fut");
		while (fut!=NULL)
		{
			int CCID = atoi(fut->FirstChildElement("cId")->FirstChild()->Value());
			string pe = fut->FirstChildElement("pe")->FirstChild()->Value();
			double cvf = atof(fut->FirstChildElement("cvf")->FirstChild()->Value());

			double t = atof(fut->FirstChildElement("t")->FirstChild()->Value());
			double priceScan = atof(fut->FirstChildElement("scanRate")->FirstChildElement("priceScan")->FirstChild()->Value());
			double volScanPct = atof(fut->FirstChildElement("scanRate")->FirstChildElement("volScan")->FirstChild()->Value());




			TiXmlNode* ra = fut->FirstChild("ra");
			if (ra != NULL)
			{
				TiXmlElement* a = fut->FirstChild("ra")->FirstChild("a")->ToElement();
				int aIndex = 0;
				Futures* pFut = new Futures();
				pFut->ID = ID;
				pFut->Maturity = pe;
				pFut->Code = Code;
				pFut->Name = Name;
				pFut->UnderlyingID = UnderlyingID;
				pFut->cvf = cvf;
				pFut->TimetoMaturity = t;
				pFut->PriceScan = priceScan;
				pFut->VolScan = volScanPct;
				while (a != NULL)
				{
					
					pFut->ra[aIndex] = atof(a->FirstChild()->Value());
					pFut->raRT[aIndex] = pFut->ra[aIndex];
					aIndex ++;
					//cout<< "ra " << pFut->ra[aIndex] << " " << a->FirstChild()->Value() << endl;
					a = a->NextSiblingElement();
				}
				pFut->delta = atof(fut->FirstChild("ra")->FirstChild("d")->ToElement()->FirstChild()->Value());
				FuturesContracts.insert(std::make_pair(std::make_pair(Code, pe), pFut));
				//FuturesContracts.push_back(pFut);
			}
			fut = fut->NextSiblingElement("fut");
			
		}
		
		
		futPf = futPf->NextSiblingElement("futPf");
	}

	return i;
}

int AttachCC(XMLParser * Parser, int Link, CombinedCommodity* CC)
{
	int Count = 0;
	for(map<OptionKey, Option*>::iterator iter = Parser->OptionContracts.begin();iter != Parser->OptionContracts.end();iter++)
	{
		pair<OptionKey, Option*> temp = *iter;
		if (temp.second->ID == Link)
		{
			temp.second->CC = CC;
			Count++;
		}
	}

	for(map<pair<string,string>, Futures*>::iterator iter = Parser-> FuturesContracts.begin();iter != Parser->FuturesContracts.end();iter++)
	{
		pair<pair<string,string>, Futures*> temp = *iter;

		if (temp.second->ID == Link)
		{
			temp.second->CC = CC;
			Count++;
		}
	}
	return Count;
}

int FillCombinedCommodities(TiXmlDocument* XMLDOC, XMLParser * Parser, vector<CombinedCommodity*>& CCs)
{
	TiXmlHandle hDOC(XMLDOC);
	TiXmlElement *clearingOrg = hDOC.FirstChild("spanFile").FirstChild( "pointInTime" ).FirstChild( "clearingOrg" ).ToElement();

	TiXmlElement *ccDef = clearingOrg->FirstChildElement("ccDef");

	if (ccDef == NULL)
		return 0;		// No CCs

	int i = 0;
	while (ccDef != NULL)
	{
		i++;
		CombinedCommodity* pCC = new CombinedCommodity();
		TiXmlElement* CC = ccDef->FirstChildElement("cc");
		TiXmlNode* node = CC->FirstChild();
		pCC->cc = node->Value(); 
		TiXmlElement* Name = ccDef->FirstChildElement("name");
		node = Name->FirstChild();
		if (node!= NULL)
		{
			pCC->name = node->Value(); 

		}
		else pCC->name = "";
		//cout << "Id: " << pCC->cc << ", Code: " << pCC->name << endl;
		

		TiXmlElement* pfLink = ccDef->FirstChildElement("pfLink");
		while(pfLink != NULL)
		{
			string aaa = pfLink->FirstChildElement("pfId")->FirstChild()->Value();
			int LinkID = atoi(aaa.c_str());
			pCC->pfLinks.push_back(LinkID);
			AttachCC(Parser,LinkID,pCC);
			pfLink = pfLink->NextSiblingElement("pfLink");
		}
		TiXmlElement* intraTiers = ccDef->FirstChildElement("intraTiers");
		TiXmlElement* tier = intraTiers->FirstChildElement("tier"); 
		while(tier != NULL)
		{
			if (tier->FirstChildElement("sPe")!= NULL)
			{
				Tier* pTier = new Tier();
				pTier->tn = atoi(tier->FirstChildElement("tn")->FirstChild()->Value());
				pTier->StartDate = tier->FirstChild("sPe")->FirstChild()->Value();
				pTier->EndDate = tier->FirstChild("ePe")->FirstChild()->Value();
				pCC->intraTiers.insert(std::make_pair(pTier->tn, pTier));
				//pCC->intraTiers.push_back(pTier);
			}
			else
			{
				Tier* pTier = new Tier();
				pTier->tn = atoi(tier->FirstChildElement("tn")->FirstChild()->Value());
				pTier->StartDate = "";
				pTier->EndDate = "";
				pCC->intraTiers.insert(std::make_pair(pTier->tn, pTier));

				//pCC->intraTiers.push_back(pTier);
			}
			tier = tier->NextSiblingElement("tier");
		}
		tier = NULL;
		TiXmlElement* somTiers = ccDef->FirstChildElement("somTiers");
		tier = somTiers->FirstChildElement("tier"); 
		while(tier != NULL)
		{
			if (tier->FirstChildElement("tn")!= NULL)
			{
				SOMTier* pTier = new SOMTier();
				pTier->tn = atoi(tier->FirstChildElement("tn")->FirstChild()->Value());
				pTier->rate = atof(tier->FirstChildElement("rate")->FirstChildElement("val")->FirstChild()->Value());
				pCC->SOMTiers.push_back(pTier);
			}
			tier = tier->NextSiblingElement("tier");
		}

		TiXmlElement* dSpread = ccDef->FirstChildElement("dSpread");
		while(dSpread  != NULL)
		{
			DSpread* pdSpread = new DSpread();
			pdSpread->Priority = atoi(dSpread->FirstChildElement("spread")->FirstChild()->Value());
			pdSpread->Rate = atof(dSpread->FirstChildElement("rate")->FirstChildElement("val")->FirstChild()->Value());
			TiXmlElement* tLeg = dSpread->FirstChildElement("tLeg");
			TiXmlElement* pLeg = dSpread->FirstChildElement("pLeg");
			while (tLeg != NULL)
			{
				TLeg* pTLeg = new TLeg();
				pTLeg->cc = tLeg->FirstChildElement("cc")->FirstChild()->Value();
				pTLeg->i = atof(tLeg->FirstChildElement("i")->FirstChild()->Value());
				pTLeg->RS = tLeg->FirstChildElement("rs")->FirstChild()->Value()[0];
				pTLeg->tn = atoi(tLeg->FirstChildElement("tn")->FirstChild()->Value());
				Tier *pTier = pCC->FindTier(pTLeg->tn);
				if (pTier != NULL)
				{
					pTLeg->spe = pTier->StartDate;
					pTLeg->epe = pTier->EndDate;
				}
				pdSpread->TLegs.push_back(pTLeg);
				tLeg = tLeg->NextSiblingElement("tLeg");
			}
			while (pLeg != NULL)
			{
				PLeg* pPLeg = new PLeg();
				pPLeg->cc = tLeg->FirstChildElement("cc")->FirstChild()->Value();
				pPLeg->i = atof(tLeg->FirstChildElement("i")->FirstChild()->Value());
				pPLeg->RS = tLeg->FirstChildElement("rs")->FirstChild()->Value()[0];
				pPLeg->pe = tLeg->FirstChildElement("pe")->FirstChild()->Value();
				pdSpread->PLegs.push_back(pPLeg);
				pLeg = pLeg->NextSiblingElement("pLeg");
			}
			pCC->dSpreads.push_back(pdSpread);
			dSpread = dSpread->NextSiblingElement("dSpread");
		}

		TiXmlElement* spotRate = ccDef->FirstChildElement("spotRate");
		while(spotRate != NULL)
		{
			SpotRate* pspotRate = new SpotRate();
			pspotRate->r = atoi(spotRate->FirstChildElement("r")->FirstChild()->Value());
			pspotRate->pe = spotRate->FirstChildElement("pe")->FirstChild()->Value();
			pspotRate->outr = atof(spotRate->FirstChildElement("outr")->FirstChild()->Value());
			pspotRate->sprd = atof(spotRate->FirstChildElement("sprd")->FirstChild()->Value());

			pCC->SpotRates.push_back(pspotRate);
			spotRate = spotRate->NextSiblingElement("spotRate");
		}

		CCs.push_back(pCC);
		ccDef = ccDef->NextSiblingElement("ccDef");
	}

	return i;
}

int FillInterSpreads(TiXmlDocument* XMLDOC,vector<DSpread*>& InterSpreads)
{
	int i = 0;
	TiXmlHandle hDOC(XMLDOC);
	TiXmlElement *clearingOrg = hDOC.FirstChild("spanFile").FirstChild( "pointInTime" ).FirstChild( "clearingOrg" ).ToElement();

	TiXmlElement *interSpreads = clearingOrg->FirstChildElement("interSpreads");
	
	if (interSpreads == NULL)
		return -1;			// No interspreads
	
	TiXmlElement* dSpread = interSpreads->FirstChildElement("dSpread");
	while(dSpread != NULL)
	{
		DSpread* pdSpread = new DSpread();
		pdSpread->Priority = atoi(dSpread->FirstChildElement("spread")->FirstChild()->Value());
		pdSpread->Rate = atof(dSpread->FirstChildElement("rate")->FirstChildElement("val")->FirstChild()->Value());
		TiXmlElement* tLeg = dSpread->FirstChildElement("tLeg");
		while (tLeg != NULL)
		{
			TLeg* pTLeg = new TLeg();
			pTLeg->cc = tLeg->FirstChildElement("cc")->FirstChild()->Value();
			pTLeg->i = atof(tLeg->FirstChildElement("i")->FirstChild()->Value());
			pTLeg->RS = tLeg->FirstChildElement("rs")->FirstChild()->Value()[0];
			pTLeg->tn = atoi(tLeg->FirstChildElement("tn")->FirstChild()->Value());
			pdSpread->TLegs.push_back(pTLeg);
			tLeg = tLeg->NextSiblingElement("tLeg");
		}
		InterSpreads.push_back(pdSpread);
		dSpread = dSpread->NextSiblingElement("dSpread");
	}

	return i;
}



void XMLParser::PrintOptions()
{
	for(map<OptionKey, Option*>::iterator iter = OptionContracts.begin();iter != OptionContracts.end();iter++)
	{
		pair<OptionKey, Option*> temp = *iter;
		Option *opt = temp.second;
		outputFile << "Code: " << opt->Code << " C/P: " << opt->IsCall << " A/E: " << (int)(opt->EType) << " Maturity: "
			<< opt->Maturity << " Strike: " << opt->Strike << " Premium: " << opt->Price << endl ;
	}
	//int Size = OptionContracts.size();
	//for (int i = 0; i < Size; i++)
	//{
	//	Option *opt = OptionContracts[i];
	//	*outputFile << "Code: " << opt->Code << " C/P: " << opt->IsCall <<  " Maturity: " 
	//		<< opt->Maturity << " Strike: " << opt->Strike << endl;
	//	//*outputFile << "Code: " << opt->Code <<"ID: " <<  opt->ID << "C/P: " << opt->IsCall << "Maturity: " 
	//		//<< opt->Maturity << "Name: " << opt->Name << "Strike: " << opt->Strike << "Underlying: " << opt->UnderlyingID << endl;
	//}
}

void XMLParser::PrintFutures()
{
	for(map<pair<string,string>, Futures*>::iterator iter = FuturesContracts.begin();iter != FuturesContracts.end();iter++)
	{
		pair<pair<string,string>, Futures*> temp = *iter;
		Futures *fut = temp.second;
		outputFile << "Code: " << fut->Code << " Maturity: "
			<< fut->Maturity << endl;
	}
	//int Size = FuturesContracts.size();
	//for (int i = 0; i < Size; i++)
	//{
	//	Futures *fut = FuturesContracts[i];
	//	*outputFile << "Code: " << fut->Code << " Maturity: " 
	//		<< fut->Maturity << endl;
	//	//*outputFile << "Code: " << fut->Code <<"ID: " <<  fut->ID << "C/P: " << fut->IsCall << "Maturity: " 
	//	//	<< fut->Maturity << "Name: " << fut->Name << "Strike: " << fut->Strike << "Underlying: " << fut->UnderlyingID << endl;
	//}
}

void XMLParser::PrintCombinedCommodities()
{
	int Size = CCs.size();
	for (int i = 0; i < Size; i++)
	{
		CombinedCommodity *CC = CCs[i];
		cout << "cc: " << CC->cc <<" name: " <<  CC->name << endl;
		
		int iPfLinks = CC->pfLinks.size();
		for (int j = 0; j< iPfLinks ; j++ )
		{
			cout << "PfLink(" << (j+1) << "): " << CC->pfLinks[j]<< endl;
		}

		int iTiers = CC->intraTiers.size();
		for (int j = 0; j < iTiers; j++)
		{
			Tier* PTier = CC->intraTiers[j];
			cout << "Tier tn: " << PTier->tn << " Start: " << PTier->StartDate << " End: " << PTier->EndDate << endl;
		}
		
		int idSpreads = CC->dSpreads.size();
		for (int j = 0; j < idSpreads; j++)
		{
			DSpread* pdSpead = CC->dSpreads[j];
			cout << "dSpread priority: " << pdSpead->Priority  << " Rate: " << pdSpead->Rate << endl;
			int iPlegs = pdSpead->PLegs.size();
			int iTlegs = pdSpead->TLegs.size();
			for (int k = 0; k< iPlegs ;k++)
			{
				cout << "Pleg(" << (k+1) << ") cc: " 
					<< pdSpead->PLegs[k]->cc 
					<< " i: " << pdSpead->PLegs[k]->i 
					<< " pe: " << pdSpead->PLegs[k]->pe 
					<< " rs: " <<pdSpead->PLegs[k]->RS << endl;
			}
			for (int k = 0; k< iTlegs ; k++)
			{
				cout << "Tleg(" << (k+1) << ") cc: " 
					<< pdSpead->TLegs[k]->cc 
					<< " i: " << pdSpead->TLegs[k]->i 
					<< " tn: " << pdSpead->TLegs[k]->tn 
					<< " rs: " << pdSpead->TLegs[k]->RS << endl;
			}
		}

	}
}

int Parse(TiXmlDocument* XMLDOC, XMLParser * Parser)
{
	FillScanPairs(XMLDOC, Parser->ScanPointPairs, Parser->Scenarios);
	FillPhysicals(XMLDOC, Parser->Physicals);
	FillOptions(XMLDOC, Parser->OptionContracts);
	FillFutures(XMLDOC, Parser->FuturesContracts);
	FillCombinedCommodities(XMLDOC, Parser, Parser->CCs);
	FillInterSpreads(XMLDOC, Parser->InterSpreads);
	//Parser->PrintOptions();
	//Parser->PrintFutures();
	//PrintCombinedCommodities();
	//getchar();*/
	return 0;
}



XMLParser::XMLParser(string FileName)
{
	TiXmlDocument* XMLDOC;
	XMLDOC = NULL;
	//outputFile = new ofstream("Market.txt");
	XMLDOC = new TiXmlDocument(FileName.c_str());
	if (!XMLDOC->LoadFile())
	{
		XMLDOC = NULL;
	}
	Parse(XMLDOC,this);
	if (XMLDOC != NULL)
	{
		delete XMLDOC;
		XMLDOC = NULL;
	}
}

XMLParser::~XMLParser()
{	//delete outputFile;
	//if (XMLDOC != NULL)
	//{
	//	delete XMLDOC;
	//	XMLDOC = NULL;
	//}

	for (size_t i = 0; i < Physicals.size(); i++)
	{
		delete Physicals[i];
		Physicals[i] = NULL;
	}

	/*for (int i = 0; i < OptionContracts.size(); i++)
	{	
		delete OptionContracts[i];
		OptionContracts[i] = NULL;
	}*/

	/*for (int i = 0; i< FuturesContracts.size(); i++)
	{
		delete FuturesContracts[i];
		FuturesContracts[i] = NULL;
	}*/

	for(map<OptionKey, Option*>::iterator iter = OptionContracts.begin();iter != OptionContracts.end();iter++)
	{
		pair<OptionKey, Option*> temp = *iter;

		delete temp.second;
		temp.second = NULL;
	}
	OptionContracts.clear();

	for(map<pair<string,string>, Futures*>::iterator iter = FuturesContracts.begin();iter != FuturesContracts.end();iter++)
	{
		pair<pair<string,string>, Futures*> temp = *iter;

		delete temp.second;
		temp.second = NULL;
	}
	FuturesContracts.clear();
	for (size_t i = 0; i < CCs.size(); i++ )
	{
		/*for (int j = 0; j < CCs[i]->pfLinks.size(); j++)
		//{
		//	delete (CCs[i]->pfLinks[j]);
		//	CCs[i]->pfLinks[j] = NULL;
		//}*/
		//for (int j = 0; j < CCs[i]->dSpreads.size(); j++)
		//{
		//	for (int k = 0 ; k< CCs[i]->dSpreads[j]->PLegs.size(); k++ )
		//	{
		//		delete CCs[i]->dSpreads[j]->PLegs[k];
		//		CCs[i]->dSpreads[j]->PLegs[k]= NULL;
		//	}
		//	for (int k = 0 ; k< CCs[i]->dSpreads[j]->TLegs.size(); k++ )
		//	{
		//		delete CCs[i]->dSpreads[j]->TLegs[k];
		//		CCs[i]->dSpreads[j]->TLegs[k]= NULL;
		//	}

		//	delete CCs[i]->dSpreads[j];
		//	CCs[i]->dSpreads[j] = NULL;
		//}
		//for (int j= 0; j < CCs[i]->intraTiers.size(); j ++ )
		//{
		//	delete CCs[i]->intraTiers[j];
		//	CCs[i]->intraTiers[j] = NULL;
		//}

		delete CCs[i];
		CCs[i] = NULL;
	}

	for (size_t i = 0; i< InterSpreads.size(); i++)
	{
		delete InterSpreads[i];
		InterSpreads[i] = NULL;
	}

	for(vector<ScenarioPoint*>::const_iterator it = Scenarios.begin(); it != Scenarios.end(); it++)
	{
		delete *it;
	} 
	Scenarios.clear();

}

string XMLParser::GetCC(int ID)
{
	string CC = "";
	for (size_t i = 0; i < CCs.size(); i++)
	{
		for (size_t j= 0; j< CCs[i]->pfLinks.size(); j++)
		{
			if (CCs[i]->pfLinks[j] == ID)
			{
				return CCs[i]->cc;
			}
		}
	}
	return CC;
}



//Tier* FindTier(int tn);