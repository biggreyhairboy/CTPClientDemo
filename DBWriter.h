//
// Created by patrick on 1/1/17.
//

#ifndef SAVEFUTURESTICK_DBWRITER_H
#define SAVEFUTURESTICK_DBWRITER_H
#include <QSqlQuery>
#include <QSqlDatabase>
#include <QTextStream>
#include "GVAR.h"
#include "ctpapi_linux64/ThostFtdcUserApiStruct.h"
#include <string>

#include <QString>

#include <set>

class DBWriter {
public:
    static DBWriter *getInstance();
//    ~DBWriter();
    void InsertTickData(QString tableName, CThostFtdcDepthMarketDataField *pDepthMarketData, std::string tickdata);
    void InsertOrUpdateAccountPostion(QString tableName, CThostFtdcInvestorPositionField *positionField);
    void DBClose();
private:
    static DBWriter *writer;
    DBWriter();
    QSqlDatabase db;
};


#endif //SAVEFUTURESTICK_DBWRITER_H
