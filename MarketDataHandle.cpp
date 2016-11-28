//
// Created by biggreyhairboy on 7/21/16.
//


#include "MarketDataHandle.h"
#include <cstring>
#include <string>
#include <iostream>
#include <vector>
using namespace std;
MarketDataHandle::MarketDataHandle(CThostFtdcMdApi* iMdapi, char *front_address, TThostFtdcBrokerIDType brokerid,
                                   TThostFtdcInvestorIDType investorid, TThostFtdcPasswordType password,
                                   DBDriver *dbdriver, vector<string> ppinsturment, int insturmentid)
{
    pUserApi = iMdapi;
    strcpy(this->FRONT_ADDR_quote, front_address);
    strcpy(this->brokerIDType, brokerid);
    strcpy(this->investorIDType, investorid);
    strcpy(this->passwordType, password);
    this->iRequestID_quote = 0;
    strppInstrument = ppinsturment;
    InstrumentID = insturmentid;
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
    }

    char** instrumentarry= new char*[strppInstrument.size()];
//    int n = 0;
//    //可以使用std::copy
//    for(vector<string>::iterator iter = strppInstrument.begin(); iter != strppInstrument.end(); iter++)
//    {
//        //将vector数组转换成char* 数组
//        strcpy(instrumentarry[n], (*iter).c_str());
//        n++;
//    }
    for(size_t i = 0; i < strppInstrument.size(); ++i)
    {
        instrumentarry[i] = new char[strppInstrument[i].size() + 1];
        std::strcpy(instrumentarry[i], strppInstrument[i].c_str());
    }
    SubscribeMarketData(instrumentarry, InstrumentID);
}

void MarketDataHandle::SubscribeMarketData(char* ppIntrumentID[], int iInstrumentID) {
    int iResult = pUserApi->SubscribeMarketData(ppIntrumentID, iInstrumentID);
    cerr << "--->>> request subscribe market data: " << ((iResult == 0) ? "success" : "fail") << endl;
    //SubscribeMarketData(ppIntrumentID,t InstrumentID);
    SubscribeForQuoteRsp(ppIntrumentID, InstrumentID);
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
    if (ConsecutiveTime == 0)
    {
        PreviousPrice == pDepthMarketData->PreClosePrice;
    }
    //todo: matain a price queue of last five minutes

    CThostFtdcDepthMarketDataField* LastTick;
    int OpenInterestChange = 0;
//    TThostFtdcRatioType	;

    ///最后修改时间
    TThostFtdcTimeType	UpdateTime;

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

void MarketDataHandle::OnRspError(CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{
    cerr << "error on responce"<< endl;
}


