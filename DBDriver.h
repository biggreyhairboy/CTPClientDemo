//
// Created by biggreyhairboy on 7/29/16.
//

#ifndef CTPCLIENTDEMO_DBDRIVER_H
#define CTPCLIENTDEMO_DBDRIVER_H

#include <string>
#include <istream>
#include <exception>
#include "mysql_connection.h"
#include <cppconn/driver.h>
#include "cppconn/statement.h"
#include "cppconn/resultset.h"
#include "cppconn/exception.h"
#include "ctpapi_linux64/ThostFtdcUserApiStruct.h"

using namespace std;

class DBDriver {
public:
    //DBDriver *dbDriver;
    std::string host;
    std::string user;
    std::string password;
    std::string database;
    sql::Driver * driver;
    sql::Connection *connection;
    sql::Statement *statement;
    sql::ResultSet *resultSet;

    //static DBDriver GetDBDriverInstance();
    DBDriver(std::string host, std::string user, std::string password, std::string database);
    bool ExcuteQuery(CThostFtdcDepthMarketDataField *pDepthMarketData);
    ~DBDriver();
private:
    void Initialize();
};


#endif //CTPCLIENTDEMO_DBDRIVER_H
