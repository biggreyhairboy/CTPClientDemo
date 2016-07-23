#include <iostream>
#include <thread>
#include "ctpapi_linux64/ThostFtdcMdApi.h"
#include "MarketDataHandle.h"

using namespace std;

CThostFtdcMdApi *pUserApi;

char FRONT_ADDR_quote[] = "tcp://180.168.146.187.10031";
char FRONT_ADDR_trade[] = "tcp://180.168.146.187.10030";
TThostFtdcBrokerIDType BROKER_ID = "9999";
TThostFtdcInvestorIDType INVESTOR_ID = "039395";
TThostFtdcPasswordType PASSWORD = "yjk19890412";
char *ppInstrumentID[] = {"rb1610", "rb1701"};
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

    getchar();
    return 0;
}