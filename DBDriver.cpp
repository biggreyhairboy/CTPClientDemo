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
    sql::Driver * driver;
    sql::Connection *connection;
    sql::Statement *statement;
    sql::ResultSet *resultSet;

    driver = get_driver_instance();
    connection = driver->connect("tcp://" + host + ":3306" , user, password);
    connection->setSchema(database);
    statement = connection->createStatement();
    resultSet = statement->executeQuery("select * from one_day");
    cout << resultSet->isFirst();
}

DBDriver::~DBDriver() {}
