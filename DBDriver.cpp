//
// Created by biggreyhairboy on 7/29/16.
//

#include "DBDriver.h"

DBDriver::DBDriver(string host, string user, string password, string database) {
    this->host = host;
    this->user = user;
    this->password = password;
    this->database = database;
    Initialize();
}

void DBDriver::Initialize() {
    driver = get_driver_instance();
    connection = driver->connect("tcp://" + host + ":3306" , user, password);
    connection->setSchema(database);
    statement = connection->createStatement();
    //cout << resultSet->isFirst();
}
//
//DBDriver DBDriver::GetDBDriverInstance(){
//    if(*dbDriver == NULL)
//    {
//        DBDriver()
//    }
//}

bool DBDriver::ExcuteQuery(CThostFtdcDepthMarketDataField *pDepthMarketData){
    string query = "insert into tick_data_futures (" + *pDepthMarketData->TradingDay + \
                   *pDepthMarketData->InstrumentID + \
                   (string)(*pDepthMarketData->LastPrice)) + \
                   *pDepthMarketData->PreSettlementPrice + \
                   *pDepthMarketData->PreClosePrice +\
                   *pDepthMarketData->PreOpenInterest +\
                   *pDepthMarketData->OpenPrice +\
                   *pDepthMarketData->HighestPrice +\
                   *pDepthMarketData->LowestPrice +\
                   *pDepthMarketData->Volume +\
                   *pDepthMarketData->Turnover +\
                   *pDepthMarketData->OpenInterest +\
                   *pDepthMarketData->ClosePrice +\
                   *pDepthMarketData->SettlementPrice +\
                   *pDepthMarketData->UpperLimitPrice +\
                   *pDepthMarketData->LowerLimitPrice +\
                   *pDepthMarketData->UpdateTime +\
                   *pDepthMarketData->UpdateMillisec +\
                   *pDepthMarketData->BidPrice1 +\
                   *pDepthMarketData->BidVolume1 +\
                   *pDepthMarketData->AskPrice1 +\
                   *pDepthMarketData->AskVolume1 +\
                   *pDepthMarketData->AveragePrice +\
                   *pDepthMarketData->ActionDay + ");";
    resultSet = statement->executeQuery("");
    resultSet = statement->executeQuery("insert into one_day (High) values (2450)");
    resultSet = statement->executeQuery("select * from one_day");
}

DBDriver::~DBDriver() {}
