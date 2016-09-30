//
// Created by biggreyhairboy on 7/21/16.
//


#include "MarketDataHandle.h"
#include <cstring>

#include <string>
#include <iostream>

using namespace std;

MarketDataHandle::MarketDataHandle(char front_address[], TThostFtdcBrokerIDType brokerid, TThostFtdcInvestorIDType investorid, TThostFtdcPasswordType password, DBDriver* dbdriver, char* ppinsturment[], int insturmentid)
{
    strcpy(FRONT_ADDR_quote, front_address);
    strcpy(brokerIDType, brokerid);
    strcpy(investorIDType, investorid);
    strcpy(passwordType, password);
    strcpy(*ppIntrumentID, *ppinsturment);
    //ppIntrumentID = ppinsturment;
    InstrumentID = insturmentid;
//
//    FRONT_ADDR_quote = front_address;
//    brokerIDType = brokerid;
//    investorIDType = investorid;
//    passwordType = password;
    dbDriver = dbdriver;
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
        SubscribeMarketData(ppIntrumentID, InstrumentID);
        SubscribeForQuoteRsp(ppIntrumentID, InstrumentID);
    }
}

void MarketDataHandle::SubscribeMarketData(char* ppIntrumentID[], int iInstrumentID) {
    int iResult = pUserApi->SubscribeMarketData(ppIntrumentID, iInstrumentID);
    cerr << "--->>> request subscribe market data: " << ((iResult == 0) ? "success" : "fail") << endl;
}
void MarketDataHandle::SubscribeForQuoteRsp(char* ppIntrumentID[], int iInstrumentID) {
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
    dbDriver->ExcuteQuery(pDepthMarketData);

    cerr << "DailyChangeRatio: " << (pDepthMarketData->OpenPrice - pDepthMarketData->AskPrice1) / pDepthMarketData->OpenPrice << endl;
    cerr << "OnRtnDepthMarketData: askprice" << pDepthMarketData->AskPrice1 << endl;

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


