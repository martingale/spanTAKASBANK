#include <Rcpp.h>
#include "FastSpanCalculator.h"
#include <iostream>
// #include <dlfcn.h>
#include <cstdlib>
#include <vector>
//#include <alloca.h>
#include <string>
#include <cstring>
#include <R.h>
#include <Rinternals.h>
#include <Rdefines.h> 

// #include "minicsv.h"


using namespace std;
using namespace Rcpp;
// [[Rcpp::export(name=".loadSPN")]]
RcppExport SEXP  loadSPN (SEXP xmlFile)
{
  /*	  void* handle = dlopen("libSpanNative.so", RTLD_LAZY);
  typedef void (*hello_t)();
  hello_t hello = (hello_t) dlsym(handle, "InitMarginCalculator");
  hello();
  */	  
  TerminateMarginCalculator();
  ClearPortfolio();
  
  // string to char
  std::string sxmlfileName = Rcpp::as<std::string>(xmlFile); 
  char *cxmlFileName = &sxmlfileName[0u];  
  // end string to char
  InitMarginCalculator(cxmlFileName);
  AddtoPortfolio("GARANE",false,false,false,255,"201212S0",100); //dummy; for purpose of check
  return( Rcpp::wrap(true));
  
}

// [[Rcpp::export(name="isCalculatorLoaded")]]
RcppExport SEXP  isCalculatorLoaded()
{
  
  bool isLoaded=  isMarginCalculatorLoaded();
  
  return( Rcpp::wrap(isLoaded));
  
}
// [[Rcpp::export(name="unLoadCalculator")]]
void  unLoadCalculator()
{
  TerminateMarginCalculator();
  
 // return(Rcpp::wrap(true));
  
}


// [[Rcpp::export(name=".clearPortfolio")]]
RcppExport SEXP  clearPortfolio ()
{
  /*	  void* handle = dlopen("libSpanNative.so", RTLD_LAZY);
  typedef void (*hello_t)();
  hello_t hello = (hello_t) dlsym(handle, "InitMarginCalculator");
  hello();
  */	  
  
  ClearPortfolio();
  AddtoPortfolio("GARANE",false,false,false,255,"201212S0",100); //dummy; for purpose of check
  
  return( Rcpp::wrap(true));
  
}
/*int  AddtoPortfolio(char* cSymbol, bool IsOption, bool IsCall, 
                      bool IsAmerican, double Strike, char* cMaturity, 
double Count, double Price/* = 0, bool IsOrder, bool IsIntraDay, bool Effectpremium /*= true)
*/

//bool IsOrder /*= false*/, bool IsIntraDay /*= true*/, bool Effectpremium /*= true*/
// [[Rcpp::export]]
RcppExport SEXP  addItem (std::string ticker, SEXP isOption, SEXP isCall, SEXP strike, std::string cMaturity,
                          SEXP quantity, SEXP price, SEXP oExPrice, SEXP isOrder, SEXP isIntraDay,
                          SEXP EffectPremium)
{
  
//   std::string str = "string";
  //const char *cticker = ticker.c_str();
  
  bool bIsOption = Rcpp::as< bool >(isOption);
  bool bIsCall = Rcpp::as< bool >(isCall);
  double dStrike = Rcpp::as< double >(strike);
  int iquantity = Rcpp::as< int >(quantity);
  double dprice = Rcpp::as< double >(price);
  double doExPrice = Rcpp::as< double >(oExPrice);
  bool bIsOrder = Rcpp::as< bool >(isOrder);
  bool bIsIntraday = Rcpp::as< bool >(isIntraDay);
  bool bEffectPremium = Rcpp::as< bool >(EffectPremium);
  
  
  char *cticker, *sMaturity;
  cticker = (char *)malloc(ticker.size() + 1);
  memcpy(cticker, ticker.c_str(), ticker.size() + 1);
  
  sMaturity = (char *)malloc(cMaturity.size() + 1);
  memcpy(sMaturity, cMaturity.c_str(), cMaturity.size() + 1);
  
  
//  AddtoPortfolio("GARANE",false,false,false,255,"201212S0",100); //dummy; for purpose of check
AddtoPortfoliowithOrderPrice(cticker,bIsOption,bIsCall,false,dStrike,sMaturity,
                             iquantity,dprice,doExPrice,bIsOrder,bIsIntraday,bEffectPremium); 
//Rcpp::Rcout<< cticker<<std::endl;
  
  return( Rcpp::wrap(true));
  
}




// [[Rcpp::export(name=".margin")]]
RcppExport SEXP  margin (SEXP arg1)
{
//  ClearPortfolio();
  //loadPortfolio();
//  InitMarginCalculator("takas.spn");
    double Initial, Maintenance, NetOptionValue, NetOrderOptionValue,NetIntraDayOptionValue;
  // char *Carry;
  
  CalculateMargin(Initial,Maintenance,NetOptionValue,NetOrderOptionValue,NetIntraDayOptionValue,false);
 // Rcpp::Rcout<<Initial<<" / "<<Maintenance<< "/" <<NetOptionValue <<"/" << NetIntraDayOptionValue<< std::endl;
  Rcpp::Rcout<< "Total balance requirement: "<<Initial - NetOptionValue-NetIntraDayOptionValue-NetOrderOptionValue<< " -TRY"<<std::endl; 
  //ClearPortfolio();
  //TerminateMarginCalculator();
  // return( Rcpp::wrap(Initial));
  //std::vector<double> x = Rcpp::as<std::vector<double> >(arg1);
  
  // return(Rcpp::wrap(arg1));
  double total = Initial-NetOptionValue;
  total = total>0. ? total : 0.;
  return Rcpp::List::create(Rcpp::Named("Initial")=Rcpp::wrap(Initial),
                            Rcpp::Named("NetOption")=Rcpp::wrap(NetOptionValue),
                            Rcpp::Named("NetIntraDayOptionValue")=Rcpp::wrap(NetIntraDayOptionValue),
                            Rcpp::Named("NetOrderOptionValue")=Rcpp::wrap(NetOrderOptionValue),
                            Rcpp::Named("Total")=Rcpp::wrap(total)
  );
}

// [[Rcpp::export(name=".marginwc")]]
RcppExport SEXP  marginwc (SEXP arg1)
{
  
  double Initial, Maintenance, NetOptionValue, NetOrderOptionValue,NetIntraDayOptionValue;
  char* Carry;
  int j;
  
  CalculateWorstCase(Initial,Maintenance,NetOptionValue,NetOrderOptionValue,NetIntraDayOptionValue,false ,&Carry, false,false);
  // int   CalculateWorstCase(double& Initial,double& Maintenance, double& NetOptionValue, double& NetOrderOptionValue, double& NetIntraDayOptionValue, bool DoBrute,char** CarryOut, bool IncludeDMC /*= false*/, bool withGrouping /*= false*/)

  Carry[strlen(Carry) - 1] = '\0';
  
  string str(Carry);
  //std::fprintf(stderr,"%f\n",NetIntraDayOptionValue);
  // std::cout<<Initial<<" / "<< "/" <<NetOptionValue <<"/" << NetOrderOptionValue<<". The worst order sequence" <<
  // (unsigned)strlen(Carry) <<
  // ": "<<Carry<< std::endl;
  Rcpp::Rcout<< "Total balance requirement: "<<Initial - NetOptionValue-NetIntraDayOptionValue-NetOrderOptionValue<< " -TRY"<<std::endl; 

   double total = Initial-NetOptionValue;
  total = total>0. ? total : 0.;
  return Rcpp::List::create(Rcpp::Named("Initial")=Rcpp::wrap(Initial),
                      Rcpp::Named("NetOptionValue")=Rcpp::wrap(NetOptionValue),
                      Rcpp::Named("NetOrderOptionValue")=Rcpp::wrap(NetOrderOptionValue),
                      Rcpp::Named("NetIntraDayOptionValue")=Rcpp::wrap(NetIntraDayOptionValue),
                      Rcpp::Named("Total")=Rcpp::wrap(total),
                      Rcpp::Named("WorstCaseOrderSequence")=Rcpp::wrap(str)
                      );
}

/*
// [[Rcpp::export(name=".dmargin")]]
RcppExport SEXP  dmargin (SEXP arg1)
{

  double Scenario, IntraSpread, InterSpread, SOM, DMC, Total, TotalMaintenance;
  
  CalculateDetailedMargin(Scenario, IntraSpread, InterSpread, SOM, DMC, Total, TotalMaintenance);
  Rcout<<Total<<" / "<<Scenario<< "/" <<IntraSpread <<"/" << InterSpread<< std::endl;
  Rcout<< "Total balance requirement: "<<Total<< " -TRY"<<std::endl; 
  return List::create(Rcpp::Named("Scenario")=wrap(Scenario),
                      Rcpp::Named("IntraSpread")=wrap(IntraSpread),
                      Rcpp::Named("InterSpread")=wrap(InterSpread),
                      Rcpp::Named("SOM")=wrap(SOM),
                      Rcpp::Named("DMC")=wrap(DMC),
                      Rcpp::Named("TotalMaintenance")=wrap(TotalMaintenance),
                      Rcpp::Named("Total")=wrap(Total)
                      
  );
  
  // return( Rcpp::wrap(Total));

}
*/




//g++ -fPIC -shared -w -Wall   -I/usr/share/R/include -I./SpanNative/ -I/usr/local/lib/R/site-library/Rcpp/include  ./libleminat.cpp  -o teminat.so -lSpanNative
