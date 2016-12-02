//
// Created by biggreyhairboy on 7/21/16.
//


#include "MarketDataHandle.h"
#include <cstring>
#include <iostream>
#include <iomanip>
#include "boost/format.hpp"
#include <math.h>
#include <string>
#include <iostream>
#include <vector>
using namespace std;
using boost::format;
MarketDataHandle::MarketDataHandle(CThostFtdcMdApi* iMdapi, char *front_address, TThostFtdcBrokerIDType brokerid,
                                   TThostFtdcInvestorIDType investorid, TThostFtdcPasswordType password,
                                   DBDriver *dbdriver, vector<string> ppinsturment, int insturmentid)
{
    int abc;
    pUserApi = iMdapi;
    strcpy(this->FRONT_ADDR_quote, front_address);
    strcpy(this->brokerIDType, brokerid);
    strcpy(this->investorIDType, investorid);
    strcpy(this->passwordType, password);
    this->iRequestID_quote = 0;
    strppInstrument = ppinsturment;
    InstrumentID = insturmentid;
    dbDriver = dbdriver;
    MarketTrend.insert({0, 0});
    MarketTrend.insert({1, 0});
    MarketTrend.insert({2, 0});
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
    for(size_t i = 0; i < strppInstrument.size(); i++)
    {
        instrumentarry[i] = new char[strppInstrument[i].size() + 1];
        std::strcpy(instrumentarry[i], strppInstrument[i].c_str());
    }
    SubscribeMarketData(instrumentarry, InstrumentID);
}

void MarketDataHandle::SubscribeMarketData(char* ppIntrumentID[], int iInstrumentID) {
    int iResult = pUserApi->SubscribeMarketData(ppIntrumentID, iInstrumentID);
    cerr << "--->>> request subscribe market data: " << ((iResult == 0) ? "success" : "fail") << endl;
    //SubscribeMarketData(ppIntrumentID,InstrumentID);
    //SubscribeForQuoteRsp(ppIntrumentID, InstrumentID);
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
    //怎么初始化价格的指针
    if (pPreDepthMarketData.LastPrice == 0)
    {
        //这里应该需要完全复制
        pPreDepthMarketData = *pDepthMarketData;
    }
    dbDriver->ExcuteQuery(pDepthMarketData);
    OpenInterestChange = pDepthMarketData->OpenInterest - pPreDepthMarketData.OpenInterest;
    VolumeChange = pDepthMarketData->Volume - pPreDepthMarketData.Volume;
    if (VolumeChange ==  abs(OpenInterestChange))
    {
        MarketTrend[0] = MarketTrend[0] + 1;
        if (OpenInterestChange > 0)
        {
            //双开
            cout << boost::format("%1%")%string(pDepthMarketData->UpdateTime) <<" 价格" << pDepthMarketData->LastPrice <<" 现手" << " 增仓" << OpenInterestChange<< VolumeChange << " 双开" << endl;
        }
        else{
            //双平
            cout << boost::format("%1%")%string(pDepthMarketData->UpdateTime) <<" 价格" << pDepthMarketData->LastPrice <<" 现手"  << std::setw(10) << std::left <<OpenInterestChange << " 增仓"<< VolumeChange << " 双平" << endl;
        }
    }
    else if (VolumeChange >0 && OpenInterestChange == 0)
    {
        //空换 or 多换
        MarketTrend[0] = MarketTrend[0] + 1;
        if(pDepthMarketData->LastPrice >= pPreDepthMarketData.AskPrice1)
        {
            //多换
            cout << boost::format("%1%")%string(pDepthMarketData->UpdateTime) <<" 价格" << pDepthMarketData->LastPrice <<" 现手" << VolumeChange << " 增仓" << OpenInterestChange << " 多换" << endl;
        }
        else
        {
            //空换
            cout << boost::format("%1%")%string(pDepthMarketData->UpdateTime) <<" 价格" << pDepthMarketData->LastPrice <<" 现手" << VolumeChange << " 增仓" << OpenInterestChange << " 空换" << endl;
        }
    }
    else if (abs(OpenInterestChange) > 0 && VolumeChange > abs(OpenInterestChange))
    {
        //todo: 多开与空平至少在某些情况下搞反了，能否用价格做为判断是个值得商榷的因素
        MarketTrend[1] = MarketTrend[1] + 1;
        if (pDepthMarketData->LastPrice  >= pPreDepthMarketData.AskPrice1)
        {
            //多开
            cout << boost::format("%1%")%string(pDepthMarketData->UpdateTime)<<" 价格" << pDepthMarketData->LastPrice <<" 现手" << VolumeChange << " 增仓" << OpenInterestChange << " 多开" << endl;

        }
        else {
            //空平
            cout << boost::format("%1%")%string(pDepthMarketData->UpdateTime)<<" 价格" << pDepthMarketData->LastPrice <<" 现手" << VolumeChange << " 增仓" << OpenInterestChange << " 空平" << endl;
        }

    } else if (OpenInterestChange > 0 && VolumeChange > (-OpenInterestChange))
    {
        MarketTrend[2] = MarketTrend[2] + 1;
        if (pDepthMarketData->LastPrice <= pPreDepthMarketData.BidPrice1)
        {
            //空开
            cout << boost::format("%1%")%string(pDepthMarketData->UpdateTime) <<" 价格" << pDepthMarketData->LastPrice <<" 现手" << VolumeChange << " 增仓" << OpenInterestChange << " 空开" << endl;
        } else {
            //买平
            cout << boost::format("%1%")%string(pDepthMarketData->UpdateTime) <<" 价格" << pDepthMarketData->LastPrice <<" 现手" << VolumeChange << " 增仓" << OpenInterestChange << " 买平" << endl;
        }
    } else{
        cout << boost::format("%1%")%string(pDepthMarketData->UpdateTime)<<" 价格" << pDepthMarketData->LastPrice <<" 现手" << VolumeChange << " 增仓" << OpenInterestChange << " 错误错误错误" << endl;
    }
    for (map<int, int>::iterator itermap = MarketTrend.begin(); itermap != MarketTrend.end(); itermap++)
    {
        if((*itermap).second >=5)
        {
            ///最后修改时间
            //TThostFtdcTimeType	UpdateTime;
            //todo: 清空并下单
            ///cout << "清空并下单" << endl;
        }
    }
    //todo: matain a price queue of last five minutes
    if (iRequestID_quote > 15)
    {
        return ;
    }
    //更新上一个最新tick
    pPreDepthMarketData = *pDepthMarketData;
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
    cerr << "error on responce "<<pRspInfo->ErrorID << "  " << pRspInfo->ErrorMsg << endl;
}


