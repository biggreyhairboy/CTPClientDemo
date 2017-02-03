/***********************************
 * Author: Patrick Yang
 * Encoding: GB10830
 * Date: 20160927
 * Function: Extract data from CTP interface and store to MySQL
 */
// todo: using valgrind to profile
//https://startupnextdoor.com/how-to-run-valgrind-in-clion-for-c-and-c-programs/

#include <iostream>
#include <chrono>
#include <sstream>
#include <vector>
#include <thread>
#include <QtCore>
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
#include "initialize.h"

double lastorderprice = 0;
using namespace std;
namespace  logging = boost::log;
//CThostFtdcTraderApi* pTraderApi;

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

void quoteThread(CThostFtdcTraderApi* ptraderapi, char* FRONT_ADDR_quote, TThostFtdcBrokerIDType brokerid, TThostFtdcInvestorIDType investorid,
                 TThostFtdcPasswordType password, DBDriver* dbdriver, vector<string> ppinsturment, int instrument )
{
    CThostFtdcMdApi* pUserApi = CThostFtdcMdApi::CreateFtdcMdApi();
    CThostFtdcMdSpi *pMarketDataHandle = new MarketDataHandle(pUserApi, ptraderapi, FRONT_ADDR_quote, brokerid, investorid,
                                                              password, dbdriver, ppinsturment, instrument);
    pUserApi->RegisterSpi(pMarketDataHandle);
    pUserApi->RegisterFront(FRONT_ADDR_quote);
    pUserApi->Init();
    pUserApi->Join();
}

void tradeThread(TradingHandle *pTradingHandle,CThostFtdcTraderApi  *pTraderApi, char *FRONT_ADDR_trade)
{
    pTraderApi->RegisterSpi((CThostFtdcTraderSpi*) pTradingHandle);
    pTraderApi->SubscribePublicTopic(THOST_TERT_QUICK);
    pTraderApi->SubscribePrivateTopic(THOST_TERT_QUICK);
    pTraderApi->RegisterFront(FRONT_ADDR_trade);
    pTraderApi->Init();
}

int main() {
    cout << "新加入部分" << endl;
    iniFrontAdress();
    iniDB();

    cout << "开始吧" <<endl;
    //配置文件加上default值，防止exception
    //read config
    boost::property_tree::ptree pt;
    boost::property_tree::ini_parser::read_ini("/home/patrick/ClionProjects/CTPClientDemo/ini/CTPClientDemo.ini", pt);
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

    string SubscribeSymbolList = pt.get<std::string>("MarketData.SubscribeSymbolList");
    vector<string> ppIntrumentID(split(SubscribeSymbolList, ','));
    int iInstrumentID = ppIntrumentID.size();
    logging::add_file_log(pt.get<std::string>("CTPClientDemo.LogPath"));
    //trading
    string tradeinstrument = pt.get<std::string>("Trading.tradeinstrument");
    char tinstrumemt[chararraylength];
    strcpy(tinstrumemt, tradeinstrument.c_str());
    int quantity = pt.get<int>("Trading.quantity");
    double price = pt.get<double>("Trading.price");
    string strdirection = pt.get<std::string>("Trading.direction");
    char direction = strdirection.at(0);
//读配置完毕

    CThostFtdcTraderApi* pTraderApi = CThostFtdcTraderApi::CreateFtdcTraderApi();

    BOOST_LOG_TRIVIAL(info)<<"quote thread started ...";
    std::thread QuoteT(quoteThread, pTraderApi, FRONT_ADDR_quote, brokerIDType, investorIDType, passwordType,
                       &dbDriver, ppIntrumentID,iInstrumentID);
    QuoteT.detach();
    TradingHandle *pTradingHandle = new TradingHandle(pTraderApi,FRONT_ADDR_trade, brokerIDType, investorIDType, passwordType, &dbDriver,
                                                      tinstrumemt, price, quantity,  direction);
    pTraderApi->RegisterSpi((CThostFtdcTraderSpi*) pTradingHandle);
    pTraderApi->SubscribePublicTopic(THOST_TERT_QUICK);
    pTraderApi->SubscribePrivateTopic(THOST_TERT_QUICK);
    pTraderApi->RegisterFront(FRONT_ADDR_trade);
    pTraderApi->Init();
    this_thread::sleep_for(chrono::seconds(2));
    BOOST_LOG_TRIVIAL(info)<<"spi thread started ...";


//
//    //std::thread TradingT(tradeThread, pTradingHandle,  FRONT_ADDR_trade);
////    CThostFtdcTraderApi* orderTradeapi = CThostFtdcTraderApi::CreateFtdcTraderApi();
////    orderTradeapi->RegisterS;pi((CThostFtdcTraderSpi*) pTradingHandle);
////    orderTradeapi->SubscribePublicTopic(THOST_TERT_QUICK);
////    orderTradeapi->SubscribePrivateTopic(THOST_TERT_QUICK);
////    orderTradeapi->RegisterFront(FRONT_ADDR_trade);
//
//    CThostFtdcInputOrderField req;
//    memset(&req, 0, sizeof(req));
//    ///经纪公司代码
//    strcpy(req.BrokerID, brokerIDType);
//    ///投资者代码
//    strcpy(req.InvestorID, investorIDType);
//    ///合约代码
//    strcpy(req.InstrumentID, tinstrumemt);
//    ///报单引用
//    //orderref
//    strcpy(req.OrderRef, new char[3]);
//    ///用户代码
//    //	TThostFtdcUserIDType	UserID;
//    ///报单价格条件: 限价
//    req.OrderPriceType = THOST_FTDC_OPT_LimitPrice;
//    ///买卖方向:
//    req.Direction = direction;
//    ///组合开平标志: 开仓
//    req.CombOffsetFlag[0] = THOST_FTDC_OF_Open;
//    ///组合投机套保标志
//    req.CombHedgeFlag[0] = THOST_FTDC_HF_Speculation;
//    ///价格
//    //先用22900试试
//    req.LimitPrice = 2900;
//    ///数量: 1
//    req.VolumeTotalOriginal = quantity;
//    ///有效期类型: 当日有效
//    req.TimeCondition = THOST_FTDC_TC_GFD;
//    ///GTD日期
//    //	TThostFtdcDateType	GTDDate;
//    ///成交量类型: 任何数量
//    req.VolumeCondition = THOST_FTDC_VC_AV;
//    ///最小成交量: 1
//    req.MinVolume = 1;
//    ///触发条件: 立即
//    req.ContingentCondition = THOST_FTDC_CC_Immediately;
//    ///止损价
//    //	TThostFtdcPriceType	StopPrice;
//    ///强平原因: 非强平
//    req.ForceCloseReason = THOST_FTDC_FCC_NotForceClose;
//    ///自动挂起标志: 否
//    req.IsAutoSuspend = 0;
//    ///业务单元
//    //	TThostFtdcBusinessUnitType	BusinessUnit;
//    ///请求编号
//    //	TThostFtdcRequestIDType	RequestID;
//    ///用户强评标志: 否
//    req.UserForceClose = 0;
//    int iRequestID_trade = 2;
//    int iResult = pTraderApi->ReqOrderInsert(&req, ++iRequestID_trade);
//
//    //pTraderApi->Join();
////    TradingT.join();
//
//    //cerr << "trade---->>> 报单录入请求: " << iResult << ((iResult == 0) ? ", 成功" : ", 失败") << endl;
////    pTraderApi->Init();
//    //pTraderApi->Join();
//    //orderTradeapi->Release();
//

    getchar();
    return 0;
}