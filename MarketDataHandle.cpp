//
// Created by biggreyhairboy on 7/21/16.
//


#include "MarketDataHandle.h"
#include <iostream>
#include <cstring>

using namespace std;

extern CThostFtdcMdApi* pUserApi;
extern char FRONT_ADDR_quote[];
extern TThostFtdcBrokerIDType brokerIDType;
extern TThostFtdcInvestorIDType investorIDType;
extern TThostFtdcPasswordType passwordType;

extern char* ppIntrumentID[];
extern int iInstrumentID;
extern int iRequestID_quote = 0;
double OpenPrice = 0;



void MarketDataHandle::OnRspError(CThostFtdcRspInfoField *pRspInfo, int nRquestID, bool bIsLast) {
    cerr << "--->>> " << "OnrspError" << endl;
    IsErrorRspInfo(pRspInfo);
}

void MarketDataHandle::OnFrontDisconnected(int nReason){
    cerr << "---->>> " << "OnFrontDisconnected" << endl;
    cerr << "--->>> Reason = " << nReason << endl;
}

void MarketDataHandle::OnHeartBeatWarning(int nTimeLapse){
    cerr << "--->>> " << "OnHeartBeatWarning" <<endl;
    cerr << "--->>> nTimerLapse = " << nTimeLapse << endl;
}

void MarketDataHandle::OnFrontConnected()
{
    cerr << "--->>> " << "OnFrontConnected" <<endl;
    ReqUserLogin();
}


void MarketDataHandle::ReqUserLogin() {
    CThostFtdcReqUserLoginField req;
    memset(&req, 0, sizeof(req));
    strcpy(req.BrokerID, brokerIDType);
    strcpy(req.UserID, investorIDType);
    strcpy(req.Password, passwordType);
    int iResult = pUserApi->ReqUserLogin(&req, ++iRequestID_quote);
    cerr << "--->>> sending user login request: " << ((iResult == 0) ? "success" : "fail" )<< endl;
}

void MarketDataHandle::OnRspUserLogin(CThostFtdcRspUserLoginField *pRspUserLogin, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast) {
    cerr << "--->>> " << "OnRspUserLogin" << endl;
    if (bIsLast && !IsErrorRspInfo(pRspInfo))
    {
        cerr << "--->>> current trading date = " << pUserApi->GetTradingDay() << endl;
        SubscribeMarketData();
        SubscribeForQuoteRsp();
    }
}

void MarketDataHandle::SubscribeMarketData() {
    int iResult = pUserApi->SubscribeMarketData(ppIntrumentID, iInstrumentID);
    cerr << "--->>> request subscribe market data: " << ((iResult == 0) ? "success" : "fail") << endl;
}
void MarketDataHandle::SubscribeForQuoteRsp() {
    int iResult = pUserApi->SubscribeForQuoteRsp(ppIntrumentID, iInstrumentID);
    cerr << "--->>> request subscribe quoting: " << ((iResult == 0) ? "success" : "fail") << endl;
    cout << "--->>> symbol " << ppIntrumentID[0] << endl;
}

void MarketDataHandle::OnRspSubMarketData(CThostFtdcSpecificInstrumentField *pSpecificInstrument, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast){
    cerr << "OnRspSubmarketData" << endl;
}

void MarketDataHandle::OnRspSubForQuoteRsp(CThostFtdcSpecificInstrumentField *pSpecificInstrument, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast){
    cerr << "OnRspSubForQuoteRsp" << endl;
}

void MarketDataHandle::OnRspUnSubForQuoteRsp(CThostFtdcSpecificInstrumentField *pSpecificInstrument, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast){
    cerr << "OnRspUnSubForQuoteRsp" << endl;
}

void MarketDataHandle::OnRtnDepthMarketData(CThostFtdcDepthMarketDataField *pDepthMarketData){
    //cerr << "DailyChangeRatio: " << (pDepthMarketData->OpenPrice - pDepthMarketData->AskPrice1) / pDepthMarketData->OpenPrice << endl;
    //cerr << "OnRtnDepthMarketData: askprice" << pDepthMarketData->AskPrice1 << endl;
    if (iRequestID_quote > 15)
    {
        return ;
    }
}

void MarketDataHandle::OnRtnForQuoteRsp(CThostFtdcForQuoteRspField *pForQuoteRsp){
    cerr << "OnRtnForQuoteRsp" << endl;
}

bool MarketDataHandle::IsErrorRspInfo(CThostFtdcRspInfoField *pRspInfo) {
    bool bResult = ((pRspInfo) && (pRspInfo->ErrorID != 0));
    if(bResult)
    {
        cerr << "--->>> ErrorID = " << pRspInfo->ErrorID << ", ErrorMsg= " << pRspInfo->ErrorMsg << endl;
    }
    return bResult;
}


