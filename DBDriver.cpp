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
    //double cp = pDepthMarketData->ClosePrice;
    //double sp = pDepthMarketData->SettlementPrice;
    //todo: 多开 多平 双开 双平 空开
    string query = "insert into tick_data_futures values(" + std::string(pDepthMarketData->TradingDay)
                   + std::string(", \0") + "'" + string(pDepthMarketData->InstrumentID) + "'"
                   + std::string(", \0") + std::to_string(pDepthMarketData->LastPrice)
                   + std::string(", \0") + std::to_string(pDepthMarketData->PreSettlementPrice)
                   + std::string(", \0") + std::to_string(pDepthMarketData->PreClosePrice)
                   + std::string(", \0") + std::to_string(pDepthMarketData->PreOpenInterest)
                   + std::string(", \0") + std::to_string(pDepthMarketData->OpenPrice)
                   + std::string(", \0") + std::to_string(pDepthMarketData->HighestPrice)
                   + std::string(", \0") + std::to_string(pDepthMarketData->LowestPrice)
                   + std::string(", \0") + std::to_string(pDepthMarketData->Volume)
                   + std::string(", \0") + std::to_string(pDepthMarketData->Turnover)
                   + std::string(", \0") + std::to_string(pDepthMarketData->OpenInterest)
                   + std::string(", \0") + "0" //std::to_string(pDepthMarketData->ClosePrice)
                   + std::string(", \0") + "0"  //std::to_string(pDepthMarketData->SettlementPrice)
                   + std::string(", \0") + std::to_string(pDepthMarketData->UpperLimitPrice)
                   + std::string(", \0") + std::to_string(pDepthMarketData->LowerLimitPrice)
                   + std::string(", \0") + "'" + std::string(pDepthMarketData->UpdateTime) + "'"
                   + std::string(", \0") + std::to_string(pDepthMarketData->UpdateMillisec)
                   + std::string(", \0") + std::to_string(pDepthMarketData->BidPrice1)
                   + std::string(", \0") + std::to_string(pDepthMarketData->BidVolume1)
                   + std::string(", \0") + std::to_string(pDepthMarketData->AskPrice1)
                   + std::string(", \0") + std::to_string(pDepthMarketData->AskVolume1)
                   + std::string(", \0") + std::to_string(pDepthMarketData->AveragePrice)
                   + std::string(", \0") + std::string(pDepthMarketData->ActionDay) + ");";
    try {
        resultSet = statement->executeQuery(query);
    }catch (sql::SQLException ex)
    {
        //cerr << "sql error" << endl;
        //cerr << ex.what() << endl;
    }
    //resultSet = statement->executeQuery(query);
    //resultSet = statement->executeQuery("insert into one_day (High) values (2450)");
    //resultSet = statement->executeQuery("select * from one_day");
}

DBDriver::~DBDriver() {}
