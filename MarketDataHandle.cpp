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
MarketDataHandle::MarketDataHandle(CThostFtdcMdApi* iMdapi, CThostFtdcTraderApi *traderapi, char *front_address, TThostFtdcBrokerIDType brokerid,
                                   TThostFtdcInvestorIDType investorid, TThostFtdcPasswordType password,
                                   DBDriver *dbdriver, vector<string> ppinsturment, int insturmentid)
{
    int abc;
    pTraderApi = traderapi;
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
                 << boost::format("\033[;m%1%\033[0m")%to_string((long)volumechange) << setw(10)
                 <<boost::format("\033[;m%1%\033[0m")%to_string((long)volumechange) << setw(10)  << fixed << right << boost::format("\033[;m%1%\033[0m")%ticktype<< endl;
            break;
            //"\033[;36msome text\033[0m"; 蓝绿色
        case 1:
            cout << boost::format("%1%")%string(time) <<setw(10) << setprecision(2) << lastprice << setw(10)
                 <<  boost::format("\033[;31m%1%\033[0m")%to_string((long)volumechange)  << setw(10)
                 << boost::format("\033[;31m%1%\033[0m")%to_string((long)openinterestchange) << setw(10)  << boost::format("\033[;31m%1%\033[0m")%ticktype << endl;
            break;
        case 2:
            cout << boost::format("%1%")%string(time) <<setw(10) << setprecision(2) <<lastprice << setw(10)
                 << boost::format("\033[;32m%1%\033[0m")%to_string((long)volumechange) << setw(10)
                 <<boost::format("\033[;32m%1%\033[0m")%to_string((long)openinterestchange)<< setw(10) << boost::format("\033[;32m%1%\033[0m")%ticktype << endl;
            break;
            //"\033[;31msome text\033[0m";

        default:
            cout << "error " << endl;
    }
}

void MarketDataHandle::OnRtnDepthMarketData(CThostFtdcDepthMarketDataField *pDepthMarketData){
    //todo:写日志，方便后续分析
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
            ticktype = "双开";
            ColorfulConsolePrint(pDepthMarketData->UpdateTime, pDepthMarketData->LastPrice, VolumeChange, OpenInterestChange,"双开", 0);
            MarketTrend[1] = MarketTrend[1] + 1;
        }
        else {
            if(pDepthMarketData->LastPrice > pPreDepthMarketData.LastPrice) {
                ticktype = "多开";
                ColorfulConsolePrint(pDepthMarketData->UpdateTime, pDepthMarketData->LastPrice, VolumeChange, OpenInterestChange,"多开", 1);
                MarketTrend[1] = MarketTrend[1] + 1;
            }
            else if (pDepthMarketData->LastPrice > pPreDepthMarketData.LastPrice){
                ticktype = "空开";
                ColorfulConsolePrint(pDepthMarketData->UpdateTime, pDepthMarketData->LastPrice, VolumeChange, OpenInterestChange,"空开", 2);
                MarketTrend[2] = MarketTrend[2] + 1;
            } else {
                if (pDepthMarketData->LastPrice >= pPreDepthMarketData.AskPrice1){
                    ticktype = "多开";
                    ColorfulConsolePrint(pDepthMarketData->UpdateTime, pDepthMarketData->LastPrice, VolumeChange, OpenInterestChange,"多开", 1);
                    MarketTrend[1] = MarketTrend[1] + 1;
                } else if (pDepthMarketData->LastPrice <= pPreDepthMarketData.BidPrice1){
                    ticktype = "空开";
                    ColorfulConsolePrint(pDepthMarketData->UpdateTime, pDepthMarketData->LastPrice, VolumeChange, OpenInterestChange,"空开", 2);
                    MarketTrend[2] = MarketTrend[2] + 1;
                } else {
                    ticktype = "开仓类型待定";
                    ColorfulConsolePrint(pDepthMarketData->UpdateTime, pDepthMarketData->LastPrice, VolumeChange, OpenInterestChange,"开仓类型待定", 2);
                    //无法归类的tick直接drop
                    // MarketTrend[2] = MarketTrend[2] + 1;
                }
            }
        }
    }
    else if (OpenInterestChange < 0)
    {
        if(VolumeChange ==  abs(OpenInterestChange)) {
            ticktype = "双平";
            ColorfulConsolePrint(pDepthMarketData->UpdateTime, pDepthMarketData->LastPrice, VolumeChange, OpenInterestChange,"双平", 0);
            MarketTrend[2] = MarketTrend[2] + 1;
        }

        else {
            if(pDepthMarketData->LastPrice > pPreDepthMarketData.LastPrice) {
                ticktype = "空平";
                ColorfulConsolePrint(pDepthMarketData->UpdateTime, pDepthMarketData->LastPrice, VolumeChange, OpenInterestChange,"空平", 1);
                MarketTrend[1] = MarketTrend[1] + 1;
            }
            else if (pDepthMarketData->LastPrice < pPreDepthMarketData.LastPrice){
                ticktype = "多平";
                ColorfulConsolePrint(pDepthMarketData->UpdateTime, pDepthMarketData->LastPrice, VolumeChange, OpenInterestChange,"多平", 2);
                MarketTrend[2] = MarketTrend[2] + 1;
            } else {
                if (pDepthMarketData->LastPrice >= pPreDepthMarketData.AskPrice1){
                    ticktype = "多平";
                    ColorfulConsolePrint(pDepthMarketData->UpdateTime, pDepthMarketData->LastPrice, VolumeChange, OpenInterestChange,"多平", 2);
                    MarketTrend[2] = MarketTrend[2] + 1;
                } else if (pDepthMarketData->LastPrice <= pPreDepthMarketData.BidPrice1){
                    ticktype = "空平";
                    ColorfulConsolePrint(pDepthMarketData->UpdateTime, pDepthMarketData->LastPrice, VolumeChange, OpenInterestChange,"空平", 1);
                    MarketTrend[1] = MarketTrend[1] + 1;
                } else {
                    ticktype = "平仓类型待定";
                    ColorfulConsolePrint(pDepthMarketData->UpdateTime, pDepthMarketData->LastPrice, VolumeChange, OpenInterestChange,"平仓类型待定", 1);
                    //无法归类的tick直接drop
                    //MarketTrend[1] = MarketTrend[1] + 1;
                }
            }
        }
    }
    else if (OpenInterestChange == 0) {
        MarketTrend[0] = MarketTrend[0] + 1;
        if(pDepthMarketData->LastPrice >= pPreDepthMarketData.LastPrice) {
            ticktype = "多换";
            ColorfulConsolePrint(pDepthMarketData->UpdateTime, pDepthMarketData->LastPrice, VolumeChange, OpenInterestChange,"多换", 1);
        }
        else{
            ticktype = "空换";
            ColorfulConsolePrint(pDepthMarketData->UpdateTime, pDepthMarketData->LastPrice, VolumeChange, OpenInterestChange,"空换", 2);
        }
    }
    else {
        ticktype = "其他类型";
        ColorfulConsolePrint(pDepthMarketData->UpdateTime, pDepthMarketData->LastPrice, VolumeChange, OpenInterestChange,"其他类型", 0);
    }
    //todo: 线程detach之后是否可以调用回调函数
    for(map<int, int>::iterator mapiter = MarketTrend.begin(); mapiter != MarketTrend.end(); mapiter++)
    {
        if(mapiter->second == 5){
            //order
            if(pTraderApi->GetApiVersion() != NULL)
            {
                CThostFtdcInputOrderField req;
                memset(&req, 0, sizeof(req));
                ///经纪公司代码
                strcpy(req.BrokerID, brokerIDType);
                ///投资者代码
                strcpy(req.InvestorID, investorIDType);
                ///合约代码
                char abc[] = "rb1705";
                strcpy(req.InstrumentID, abc);
                ///报单引用
                //orderref
                strcpy(req.OrderRef, new char[3]);
                ///用户代码
                //	TThostFtdcUserIDType	UserID;
                ///报单价格条件: 限价
                req.OrderPriceType = THOST_FTDC_OPT_LimitPrice;
                ///买卖方向:
                req.Direction = '0';
                ///组合开平标志: 开仓
                req.CombOffsetFlag[0] = THOST_FTDC_OF_Open;
                ///组合投机套保标志
                req.CombHedgeFlag[0] = THOST_FTDC_HF_Speculation;
                ///价格
                //先用22900试试
                req.LimitPrice = 22300;
                ///数量: 1
                req.VolumeTotalOriginal = 1;
                ///有效期类型: 当日有效
                req.TimeCondition = THOST_FTDC_TC_GFD;
                ///GTD日期
                //	TThostFtdcDateType	GTDDate;
                ///成交量类型: 任何数量
                req.VolumeCondition = THOST_FTDC_VC_AV;
                ///最小成交量: 1
                req.MinVolume = 1;
                ///触发条件: 立即
                req.ContingentCondition = THOST_FTDC_CC_Immediately;
                ///止损价
                //	TThostFtdcPriceType	StopPrice;
                ///强平原因: 非强平
                req.ForceCloseReason = THOST_FTDC_FCC_NotForceClose;
                ///自动挂起标志: 否
                req.IsAutoSuspend = 0;
                ///业务单元
                //	TThostFtdcBusinessUnitType	BusinessUnit;
                ///请求编号
                //	TThostFtdcRequestIDType	RequestID;
                ///用户强评标志: 否
                req.UserForceClose = 0;
                int iRequestID_trade = 2;
                int iResult = pTraderApi->ReqOrderInsert(&req, ++iRequestID_trade);
            }
            MarketTrend.clear();
            break;
        }

    }




//    //todo: matain a price queue of last five minutes
//    if (iRequestID_quote > 15)
//    {
//        return ;
//    }
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


