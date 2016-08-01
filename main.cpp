#include <iostream>
#include <thread>
#include "ctpapi_linux64/ThostFtdcMdApi.h"
#include "ctpapi_linux64/ThostFtdcTraderApi.h"
#include "MarketDataHandle.h"
#include "TradingHandle.h"
#include "DBDriver.h"

using namespace std;

CThostFtdcMdApi *pUserApi;
CThostFtdcTraderApi * pTraderApi;
//todo: move global config to .ini
char FRONT_ADDR_quote[] = "tcp://180.168.146.187:10011";
char FRONT_ADDR_trade[] = "tcp://180.168.146.187:10001";
TThostFtdcBrokerIDType brokerIDType = "9999";
TThostFtdcInvestorIDType investorIDType = "039395";
TThostFtdcPasswordType passwordType = "yjk19890412";
char *ppIntrumentID[] = {"rb1610", "rb1701"};
int iInstrumentID = 2;

TThostFtdcInstrumentIDType INSTRUMENT_ID = "rb1610";
TThostFtdcDirectionType DIRECTION = THOST_FTDC_D_Sell;
TThostFtdcPriceType LIMIT_PRICE = 2430;
int iRequestID_trade = 0;

void quoteThread()
{
    pUserApi = CThostFtdcMdApi::CreateFtdcMdApi();
    CThostFtdcMdSpi *pMarketDataHandle = new MarketDataHandle();
    pUserApi->RegisterSpi(pMarketDataHandle);
    pUserApi->RegisterFront(FRONT_ADDR_quote);
    pUserApi->Init();
    pUserApi->Join();
}

void tradeThread()
{
    // 初始化UserApi
    pTraderApi = CThostFtdcTraderApi::CreateFtdcTraderApi();			// 创建UserApi
    TradingHandle *pTradingHandle = new TradingHandle();
    pTraderApi->RegisterSpi((CThostFtdcTraderSpi*) pTradingHandle);			// 注册事件类
    pTraderApi->SubscribePublicTopic(THOST_TERT_QUICK);				// 注册公有流
    pTraderApi->SubscribePrivateTopic(THOST_TERT_QUICK);				// 注册私有流
    pTraderApi->RegisterFront(FRONT_ADDR_trade);							// connect
    pTraderApi->Init();
    pTraderApi->Join();
    pTraderApi->Release();
}

int main() {
    //DBDriver dbDriver("localhost", "root", "223223", "talk_is_cheap");
    cout << "quote thread started .... " << endl;
    std::thread QuoteT(quoteThread);
    QuoteT.detach();

    std::thread TradingT(tradeThread);
    TradingT.detach();

//todo: add trading logic
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