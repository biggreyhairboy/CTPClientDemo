/***********************************
 * Author: Patrick Yang
 * Encoding: GB10830
 * Date: 20160927
 * Function: Extract data from CTP interface and store to MySQL
 */

#include <iostream>
#include <sstream>
#include <vector>
#include <thread>
#include <unistd.h>
#include <boost/log/core.hpp>
#include <boost/log/trivial.hpp>
#include <boost/log/expressions.hpp>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/ini_parser.hpp>
#include <boost/log/utility/setup/file.hpp>
#include "ctpapi_linux64/ThostFtdcMdApi.h"
#include "ctpapi_linux64/ThostFtdcTraderApi.h"
#include "MarketDataHandle.h"
#include "TradingHandle.h"
#include "DBDriver.h"

double lastorderprice = 0;
using namespace std;
namespace  logging = boost::log;

void split(const string &s, char delim, vector<string> &elems);
vector<string> split(const string &s, char delim);

void split(const string &s, char delim, vector<string> &elems){
    stringstream ss;
    ss.str(s);
    string item;
    while(getline(ss, item, delim)){
        elems.push_back(item);
    }
}

vector<string> split(const string &s, char delim){
    vector<string> elems;
    split(s, delim, elems);
    return elems;
}

void quoteThread(char* FRONT_ADDR_quote, TThostFtdcBrokerIDType brokerid, TThostFtdcInvestorIDType investorid,
                 TThostFtdcPasswordType password, DBDriver* dbdriver, vector<string> ppinsturment, int instrument )
{
    CThostFtdcMdApi* pUserApi = CThostFtdcMdApi::CreateFtdcMdApi();
    CThostFtdcMdSpi *pMarketDataHandle = new MarketDataHandle(pUserApi, FRONT_ADDR_quote, brokerid, investorid,
                                                              password, dbdriver, ppinsturment, instrument);
    pUserApi->RegisterSpi(pMarketDataHandle);
    pUserApi->RegisterFront(FRONT_ADDR_quote);
//    pMarketDataHandle->OnRspSubMarketData()
//    CThostFtdcDepthMarketDataField *pDepthMarketData;
//    pMarketDataHandle->OnRtnDepthMarketData(pDepthMarketData);
//    cout << "can i get you out " << pDepthMarketData->LastPrice;
    pUserApi->Init();
    pUserApi->Join();
}

void tradeThread(char* FRONT_ADDR_trade, TThostFtdcBrokerIDType brokerid, TThostFtdcInvestorIDType investorid,
                 TThostFtdcPasswordType password, DBDriver* dbdriver, TThostFtdcInstrumentIDType INSTRUMENT_ID,
                 TThostFtdcPriceType LIMIT_PRICE, int quantity, TThostFtdcDirectionType DIRECTION)
{
//    cout << "last order price print from trade thread " << lastorderprice << endl;
    CThostFtdcTraderApi* pTraderApi = CThostFtdcTraderApi::CreateFtdcTraderApi();
    TradingHandle *pTradingHandle = new TradingHandle(pTraderApi, FRONT_ADDR_trade, brokerid, investorid, password, dbdriver,
                                                      INSTRUMENT_ID, LIMIT_PRICE, quantity,  DIRECTION);
    pTraderApi->RegisterSpi((CThostFtdcTraderSpi*) pTradingHandle);
    pTraderApi->SubscribePublicTopic(THOST_TERT_QUICK);
    pTraderApi->SubscribePrivateTopic(THOST_TERT_QUICK);
    pTraderApi->RegisterFront(FRONT_ADDR_trade);
    pTraderApi->Init();


//    pTraderApi->Release();

    cout << "开始下单" << endl;
    CThostFtdcInputOrderField req;
    memset(&req, 0, sizeof(req));
    ///经纪公司代码
    strcpy(req.BrokerID, pTradingHandle->brokerIDType);
    ///投资者代码
    strcpy(req.InvestorID, pTradingHandle->investorIDType);
    ///合约代码
    strcpy(req.InstrumentID, INSTRUMENT_ID);
    ///报单引用
    strcpy(req.OrderRef, pTradingHandle->ORDER_REF);
    ///用户代码
    //	TThostFtdcUserIDType	UserID;
    ///报单价格条件: 限价
    req.OrderPriceType = THOST_FTDC_OPT_LimitPrice;
    ///买卖方向:
    req.Direction = DIRECTION;
    ///组合开平标志: 开仓
    req.CombOffsetFlag[0] = THOST_FTDC_OF_Open;
    ///组合投机套保标志
    req.CombHedgeFlag[0] = THOST_FTDC_HF_Speculation;
    ///价格
    req.LimitPrice = 23000;
    ///数量: 1
    req.VolumeTotalOriginal = quantity;
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
    int iRequestID_trade = 0;
    int iResult = pTraderApi->ReqOrderInsert(&req, ++iRequestID_trade);
    cerr << "trade---->>> 报单录入请求: " << iResult << ((iResult == 0) ? ", 成功" : ", 失败") << endl;
    cout << "下单完成" << endl;
    pTraderApi->Join();
    pTraderApi->Release();
}

int main() {
    cout << "开始吧" <<endl;
    //配置文件加上default值，防止exception
    //read config
    boost::property_tree::ptree pt;
    boost::property_tree::ini_parser::read_ini("/home/patrick/ClionProjects/CTPClientDemo/CTPClientDemo.ini", pt);
    //server
    string MF= pt.get<std::string>("Server_IP.MarketFront");
    string TF = pt.get<std::string>("Server_IP.TradeFront");
    const int chararraylength = 50;
    char FRONT_ADDR_quote[chararraylength];
    char FRONT_ADDR_trade[chararraylength];
    strcpy(FRONT_ADDR_quote, MF.c_str());
    strcpy(FRONT_ADDR_trade, TF.c_str());
    //account
    TThostFtdcBrokerIDType brokerIDType;
    strcpy(brokerIDType, pt.get<std::string>("Account.BrokerID").c_str());
    TThostFtdcInvestorIDType investorIDType;
    strcpy(investorIDType, pt.get<std::string>("Account.InvestorID").c_str());
    TThostFtdcPasswordType passwordType;
    strcpy(passwordType, pt.get<std::string>("Account.Password").c_str());
    //database
    string Server = pt.get<std::string>("Database.Server");
    string User = pt.get<std::string>("Database.User");
    string Password = pt.get<std::string>("Database.Password");
    string Scheme = pt.get<std::string>("Database.Scheme");
    DBDriver dbDriver(Server, User, Password, Scheme);
    //market
    int iInstrumentID = 2;
    string SubscribeSymbolList = pt.get<std::string>("MarketData.SubscribeSymbolList");
    vector<string> ppIntrumentID(split(SubscribeSymbolList, ','));
    logging::add_file_log(pt.get<std::string>("CTPClientDemo.LogPath"));
    //trading
    string tradeinstrument = pt.get<std::string>("Trading.tradeinstrument");
    char tinstrumemt[chararraylength];
    strcpy(tinstrumemt, tradeinstrument.c_str());
    int quantity = pt.get<int>("Trading.quantity");
    double price = pt.get<double>("Trading.price");
    string strdirection = pt.get<std::string>("Trading.direction");
    char direction = strdirection.at(0);


    BOOST_LOG_TRIVIAL(info)<<"quote thread started ...";
    //cout << "quote thread started .... " << endl;
    std::thread QuoteT(quoteThread, FRONT_ADDR_quote, brokerIDType, investorIDType, passwordType,
                       &dbDriver, ppIntrumentID,iInstrumentID);
    QuoteT.detach();

    usleep(1000);
    BOOST_LOG_TRIVIAL(info)<<"spi thread started ...";
    std::thread TradingT(tradeThread, FRONT_ADDR_trade, brokerIDType, investorIDType, passwordType,
                       &dbDriver, tinstrumemt, price, quantity, direction);
    TradingT.detach();

    getchar();
    return 0;
}

/*
cmake_minimum_required(VERSION 3.5)
project(CTPClientDemo)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
#set(CMAKE_VERBOSE_MAKEFILE ON)
#add_subdirectory(ctpapi_linux64)
set(SOURCE_FILES main.cpp MarketDataHandle.cpp MarketDataHandle.h)
#add_library(TCPClientDemo ${SOURCE_FILES})
#include_directories(${PROJECT_SOURCE_DIR})
#include_directories(${PROJECT_SOURCE_DIR}}/ctpapi_linux64)
#link_directories(${PROJECT_SOURCE_DIR}/ctpapi_linux64/)
#find_library(/home/biggreyhairboy/ClionProjects/CTPClientDemo/ctpapi_linux64 thostmduserapi.so )
add_library(thostmduserapi SHARED IMPORTED)
SET_TARGET_PROPERTIES(thostmduserapi PROPERTIES IMPORTED_LOCATION ${PROJECT_SOURCE_DIR}/ctpapi_linux64/thostmduserapi.so)
add_executable(CTPClientDemo ${SOURCE_FILES})
target_link_libraries( CTPClientDemo thostmduserapi)

 */