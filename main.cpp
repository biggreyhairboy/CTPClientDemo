#include <iostream>
#include <thread>
#include "ctpapi_linux64/ThostFtdcMdApi.h"
#include "MarketDataHandle.h"

using namespace std;

CThostFtdcMdApi *pUserApi;
//todo: move global config to .ini
char FRONT_ADDR_quote[] = "tcp://180.168.146.187:10011";
char FRONT_ADDR_trade[] = "tcp://180.168.146.187:10001";
TThostFtdcBrokerIDType brokerIDType = "9999";
TThostFtdcInvestorIDType investorIDType = "039395";
TThostFtdcPasswordType passwordType = "yjk19890412";
char *ppIntrumentID[] = {"rb1610", "rb1701"};
int iInstrumentID = 2;

void quoteThread()
{
    pUserApi = CThostFtdcMdApi::CreateFtdcMdApi();
    CThostFtdcMdSpi *pMarketDataHandle = new MarketDataHandle();
    pUserApi->RegisterSpi(pMarketDataHandle);
    pUserApi->RegisterFront(FRONT_ADDR_quote);
    pUserApi->Init();
    pUserApi->Join();
}

int main() {
    cout << "quote thread started .... " << endl;
    std::thread QuoteT(quoteThread);
    QuoteT.detach();
//todo: add trading logic
    getchar();
    return 0;
}