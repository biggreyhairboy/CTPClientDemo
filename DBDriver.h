//
// Created by biggreyhairboy on 7/29/16.
//

#ifndef CTPCLIENTDEMO_DBDRIVER_H
#define CTPCLIENTDEMO_DBDRIVER_H

#include <string>
#include <istream>
#include "mysql_connection.h"
#include <cppconn/driver.h>
#include "cppconn/statement.h"
#include "cppconn/resultset.h"
#include "cppconn/exception.h"

using namespace std;

class DBDriver {
public:
    string host;
    string user;
    string password;
    string database;
    DBDriver GetDBDriverInstance();
    DBDriver(string host, string user, string password, string database);
    ~DBDriver();
private:
    void Initialize();
};


#endif //CTPCLIENTDEMO_DBDRIVER_H
