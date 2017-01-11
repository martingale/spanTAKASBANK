makePortfolio<- function(pTable)
{
# sapply(pTable,class)
if(!hasArg(pTable)){return(FALSE)}
  
head(pTable)
isins<-pTable$Sozlesme
substr(isins,nchar(isins)-1,nchar(isins))
begin<-unlist(gregexpr(pattern="[0-1][0-9]16",isins,ignore.case = T))


IsOption<- as.numeric(substr(isins,1,1)=="O")
IsEquity<- as.numeric(!grepl("XU030",isins)) & (!grepl("TRYUSD",isins) )
UnderlyingSymbol<-substr(isins, 3, begin - 2 + !IsOption )
UnderlyingSymbol<- ifelse(IsEquity, paste0(UnderlyingSymbol,"E"), UnderlyingSymbol)

Amount<-pTable$Net.Pozisyon
IsOrder<- pTable$IsOrder
IsIntraday<- pTable$IsIntraDay
MarketPrice<- -1
IsOption<- as.numeric(substr(isins,1,1)=="O")
IsAmerican<-0
IsCall<- as.numeric( substr(isins,begin+4,begin+4)=="C" )

Maturity<-paste0(
  "20",
  substr(isins,start = begin+2,begin+3),
  substr(isins,start = begin,begin+1),
  substr(isins,nchar(isins)-1,nchar(isins))
)

StrikePrice<-as.numeric(substr(isins,begin+5,nchar(isins)-2))
OExPrice<- (-1)

df<-data.frame(UnderlyingSymbol,	Amount,	
               IsOrder	,IsIntraday,	MarketPrice,	IsOption,	IsAmerican,	IsCall,	Maturity,	StrikePrice	,OExPrice)
.clearPortfolio()
# print(df)
cat("# of items:", nrow(df))
for(i in c(1:nrow(df))){
   
  addItem(ticker=UnderlyingSymbol[i],
          isOption = as.logical(IsOption[i]),
          isCall = as.logical(IsCall[i]),
          strike = StrikePrice[i],
          cMaturity = Maturity[i],
          quantity = Amount[i],
          price = MarketPrice,
          oExPrice = OExPrice,
          isOrder = as.logical(IsOrder[i]),
          isIntraDay = as.logical(IsIntraday[i]),
          EffectPremium = TRUE)
  
  
}
return(TRUE)
#write.table(df,file ="~/desktop/outEx.csv" , quote = F,row.names = F,na = "0", sep=";")
}

margin <- function(pTable, onlyPositions=TRUE){
  if(onlyPositions){
    pTable<-pTable[!pTable$IsOrder,,drop=F]
  }
  if(!.isCalculatorLoaded()) {cat("Please load the Margin Calculator first.\n"); return(NULL)}
  if(makePortfolio(pTable)) return(.margin(1)) else return(NULL)
  
}

margin <- function(pTable, onlyPositions=TRUE){
  if(onlyPositions){
    pTable<-pTable[!pTable$IsOrder,,drop=F]
  }
  if(!isCalculatorLoaded()) {cat("Please load the Margin Calculator first.\n"); return(NULL)}
  if(makePortfolio(pTable)) return(.margin(1)) else return(NULL)
  
}

# dmargin <- function(pTable, onlyPositions=TRUE){
#   if(onlyPositions){
#     pTable<-pTable[!pTable$IsOrder,,drop=F]
#   }
#   if(!isCalculatorLoaded()) {cat("Please load the Margin Calculator first.\n"); return(NULL)}
#   if(makePortfolio(pTable)) {
#     return(.dmargin(1))
#     } else return(NULL)
#   
# }

marginwc <- function(pTable){
  if(!isCalculatorLoaded()) {cat("Please load the Margin Calculator first.\n"); return(NULL)}
  if(makePortfolio(pTable)) res<- .marginwc(1) else return(NULL)
  res[[6]]<-as.integer(unlist(strsplit(res[[6]],NULL)))
  res$worstCaseOrders<- pTable[as.logical(pTable$IsOrder),][as.logical(res[[6]]),1:2]
  return(res)
}

