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
    pUserApi->Init();
    pUserApi->Join();
}

void tradeThread(char* FRONT_ADDR_trade, TThostFtdcBrokerIDType brokerid, TThostFtdcInvestorIDType investorid,
                 TThostFtdcPasswordType password, DBDriver* dbdriver, TThostFtdcInstrumentIDType INSTRUMENT_ID,
                 TThostFtdcPriceType LIMIT_PRICE, int quantity, TThostFtdcDirectionType DIRECTION)
{
    CThostFtdcTraderApi* pTraderApi = CThostFtdcTraderApi::CreateFtdcTraderApi();
    TradingHandle *pTradingHandle = new TradingHandle(pTraderApi, FRONT_ADDR_trade, brokerid, investorid, password, dbdriver,
                                                      INSTRUMENT_ID, LIMIT_PRICE, quantity,  DIRECTION);
    pTraderApi->RegisterSpi((CThostFtdcTraderSpi*) pTradingHandle);
    pTraderApi->SubscribePublicTopic(THOST_TERT_QUICK);
    pTraderApi->SubscribePrivateTopic(THOST_TERT_QUICK);
    pTraderApi->RegisterFront(FRONT_ADDR_trade);
    pTraderApi->Init();
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
    strcpy(FRONT_ADDR_quote, TF.c_str());

    BOOST_LOG_TRIVIAL(info)<<"quote thread started ...";
    //cout << "quote thread started .... " << endl;
    std::thread QuoteT(quoteThread, FRONT_ADDR_quote, brokerIDType, investorIDType, passwordType,
                       &dbDriver, ppIntrumentID,iInstrumentID);
    QuoteT.detach();

//    BOOST_LOG_TRIVIAL(info)<<"trade thread started ...";
//    std::thread TradingT(tradeThread, FRONT_ADDR_quote, brokerIDType, investorIDType, passwordType,
//                       &dbDriver, tinstrumemt, price, quantity, direction);
//    TradingT.detach();

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