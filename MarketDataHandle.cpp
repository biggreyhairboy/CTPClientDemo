//
// Created by biggreyhairboy on 7/21/16.
//


#include "MarketDataHandle.h"
#include <cstring>
#include <iostream>
#include <iomanip>
#include <mutex>
#include "boost/format.hpp"
#include <math.h>
#include <string>
#include <iostream>
#include <vector>
using namespace std;
using boost::format;



extern double lastorderprice;
mutex lastorderpricemutex;
void UpdateLastPrice(double price)
{
    lock_guard<mutex> guard(lastorderpricemutex);
    lastorderprice = price;
//    cout << "last mutex price is " << lastorderprice << endl;
}
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

void ColorfulConsolePrint(TThostFtdcTimeType time, double lastprice, double volumechange, double openinterestchange, string ticktype, int color)
{

    switch(color)
    {
        //无色
        case 0:
            cout << boost::format("%1%")%string(time) <<setw(10)<<  setprecision(2) << lastprice << setw(10)
                 << boost::format("\033[;34m%1%\033[0m")%to_string((long)volumechange) << setw(10)
                 <<boost::format("\033[;34m%1%\033[0m")%to_string((long)volumechange) << setw(10)  << fixed << right << boost::format("\033[;34m%1%\033[0m")%ticktype<< endl;
            break;
            //"\033[;36msome text\033[0m"; 蓝绿色
        case 2:
            cout << boost::format("%1%")%string(time) <<setw(10) << setprecision(2) <<lastprice << setw(10)
                 << boost::format("\033[;32m%1%\033[0m")%to_string((long)volumechange) << setw(10)
                 <<boost::format("\033[;32m%1%\033[0m")%to_string((long)openinterestchange)<< setw(10) << boost::format("\033[;32m%1%\033[0m")%ticktype << endl;
            break;
            //"\033[;31msome text\033[0m";
        case 1:
            cout << boost::format("%1%")%string(time) <<setw(10) << setprecision(2) << lastprice << setw(10)
                 <<  boost::format("\033[;31m%1%\033[0m")%to_string((long)volumechange)  << setw(10)
                 << boost::format("\033[;31m%1%\033[0m")%to_string((long)openinterestchange) << setw(10)  << boost::format("\033[;31m%1%\033[0m")%ticktype << endl;
            break;
        default:
            cout << "error " << endl;
    }


}

void MarketDataHandle::OnRtnDepthMarketData(CThostFtdcDepthMarketDataField *pDepthMarketData){
    UpdateLastPrice(pDepthMarketData->LastPrice);
    //怎么初始化价格的指针
    if (pPreDepthMarketData.LastPrice == 0)
    {
        //这里应该需要完全复制
        pPreDepthMarketData = *pDepthMarketData;
    }
    dbDriver->ExcuteQuery(pDepthMarketData);
    OpenInterestChange = pDepthMarketData->OpenInterest - pPreDepthMarketData.OpenInterest;
    VolumeChange = pDepthMarketData->Volume - pPreDepthMarketData.Volume;

    if (OpenInterestChange > 0)
    {
        if(VolumeChange ==  abs(OpenInterestChange)) {
            ColorfulConsolePrint(pDepthMarketData->UpdateTime, pDepthMarketData->LastPrice, VolumeChange, OpenInterestChange,"双开", 0);
        }
        else {
            if(pDepthMarketData->LastPrice >= pPreDepthMarketData.LastPrice) {
                ColorfulConsolePrint(pDepthMarketData->UpdateTime, pDepthMarketData->LastPrice, VolumeChange, OpenInterestChange,"多开", 1);
            }
            else{
            ColorfulConsolePrint(pDepthMarketData->UpdateTime, pDepthMarketData->LastPrice, VolumeChange, OpenInterestChange,"空开", 2);
            }
        }
    }
    else if (OpenInterestChange < 0)
    {
        if(VolumeChange ==  abs(OpenInterestChange)) {
            ColorfulConsolePrint(pDepthMarketData->UpdateTime, pDepthMarketData->LastPrice, VolumeChange, OpenInterestChange,"双平", 0);
        }
        else {
            if(pDepthMarketData->LastPrice >= pPreDepthMarketData.LastPrice) {
                ColorfulConsolePrint(pDepthMarketData->UpdateTime, pDepthMarketData->LastPrice, VolumeChange, OpenInterestChange,"空平", 1);
            }
            else{
                ColorfulConsolePrint(pDepthMarketData->UpdateTime, pDepthMarketData->LastPrice, VolumeChange, OpenInterestChange,"多平", 2);
            }
        }
    }
    else if (OpenInterestChange == 0) {
        if(pDepthMarketData->LastPrice >= pPreDepthMarketData.LastPrice) {
            ColorfulConsolePrint(pDepthMarketData->UpdateTime, pDepthMarketData->LastPrice, VolumeChange, OpenInterestChange,"多换", 0);
        }
        else{
            ColorfulConsolePrint(pDepthMarketData->UpdateTime, pDepthMarketData->LastPrice, VolumeChange, OpenInterestChange,"空换", 0);
        }
    }
    else {
        ColorfulConsolePrint(pDepthMarketData->UpdateTime, pDepthMarketData->LastPrice, VolumeChange, OpenInterestChange,"其他类型", 0);
    }

//
//    MarketTrend[0] = MarketTrend[0] + 1;
//    if (VolumeChange ==  abs(OpenInterestChange))
//    {
//        //双开
//        ColorfulConsolePrint(pDepthMarketData->UpdateTime, pDepthMarketData->LastPrice, VolumeChange, OpenInterestChange,"双开", 0);
//    }
//    else{
//        //双平
//        ColorfulConsolePrint(pDepthMarketData->UpdateTime, pDepthMarketData->LastPrice, VolumeChange, OpenInterestChange,"双平", 0);
//    }
//    else if (VolumeChange >0 && OpenInterestChange == 0)
//    {
//        //空换 or 多换
//        MarketTrend[0] = MarketTrend[0] + 1;
//        if(pDepthMarketData->LastPrice >= pPreDepthMarketData.AskPrice1)
//        {
//            //多换
//            ColorfulConsolePrint(pDepthMarketData->UpdateTime, pDepthMarketData->LastPrice, VolumeChange, OpenInterestChange,"多换", 0);
//        }
//        else
//        {
//            //空换
//            ColorfulConsolePrint(pDepthMarketData->UpdateTime, pDepthMarketData->LastPrice, VolumeChange, OpenInterestChange,"空换", 0);
//        }
//    }
//    else if (abs(OpenInterestChange) > 0 && VolumeChange > abs(OpenInterestChange))
//    {
//        //todo: 多开与空平至少在某些情况下搞反了，能否用价格做为判断是个值得商榷的因素
//        MarketTrend[1] = MarketTrend[1] + 1;
//        if (pDepthMarketData->LastPrice  >= pPreDepthMarketData.AskPrice1)
//        {
//            //多开
//            ColorfulConsolePrint(pDepthMarketData->UpdateTime, pDepthMarketData->LastPrice, VolumeChange, OpenInterestChange,"多开", 1);
//
//        }
//        else {
//            //空平
//            ColorfulConsolePrint(pDepthMarketData->UpdateTime, pDepthMarketData->LastPrice, VolumeChange, OpenInterestChange,"空平", 1);
//        }
//
//    } else if (OpenInterestChange > 0 && VolumeChange > (-OpenInterestChange))
//    {
//        MarketTrend[2] = MarketTrend[2] + 1;
//        if (pDepthMarketData->LastPrice <= pPreDepthMarketData.BidPrice1)
//        {
//            //空开
//            ColorfulConsolePrint(pDepthMarketData->UpdateTime, pDepthMarketData->LastPrice, VolumeChange, OpenInterestChange,"空开", 2);
//        } else {
//            //买平
//            ColorfulConsolePrint(pDepthMarketData->UpdateTime, pDepthMarketData->LastPrice, VolumeChange, OpenInterestChange,"买平", 2);
//        }
//    } else{
//        ColorfulConsolePrint(pDepthMarketData->UpdateTime, pDepthMarketData->LastPrice, VolumeChange, OpenInterestChange,"双平", 0);
//    }
//    for (map<int, int>::iterator itermap = MarketTrend.begin(); itermap != MarketTrend.end(); itermap++)
//    {
//        if((*itermap).second >=5)
//        {
//            ///最后修改时间
//            //TThostFtdcTimeType	UpdateTime;
//            //todo: 清空并下单
//            ///cout << "清空并下单" << endl;
//        }
//    }
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


