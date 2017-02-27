//
// Created by patrick on 1/1/17.
//


#include <iostream>
#include "DBWriter.h"
#include "GVAR.h"

using namespace::std;

DBWriter* DBWriter::writer = nullptr;

//单例模式
DBWriter *DBWriter::getInstance(){
    if (writer == nullptr){
        writer = new DBWriter();
        return writer;
    }
    else
    {
        return writer;
    }
}

DBWriter::DBWriter() {
    db = QSqlDatabase::addDatabase(DB_DRIVER_NAME);
    db.setHostName(DB_HOST_NAME);
    db.setDatabaseName(DATABASE_NAME);
    db.setUserName(USER_NAME);
    db.setPassword(PASSWORD);
    if (!db.open()) {
        cout << "database error, cannot connect to database";
        abort();
    }
}

void DBWriter::InsertTickData(QString tableName, CThostFtdcDepthMarketDataField *pDepthMarketData, string ticktype) {
    QSqlQuery insert;
    //24列
    insert.prepare("insert into " + tableName + " values (?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?)");
    //绑定查询
    QString tdate                 = pDepthMarketData->TradingDay;
    QString InstrumentID         = pDepthMarketData->InstrumentID;
    double LastPrice            = pDepthMarketData->LastPrice;
    double PreSetttlementPrice  = pDepthMarketData->PreSettlementPrice;
    double PreClosePrice        = pDepthMarketData->PreClosePrice;
    double PreOpenInterest      = pDepthMarketData->PreOpenInterest;
    double OpenPrice            = pDepthMarketData->OpenPrice;
    double HighestPrice         = pDepthMarketData->HighestPrice;
    double LowestPrice          = pDepthMarketData->LowestPrice;
    int    Volume               = pDepthMarketData->Volume;
    double Turnover             = pDepthMarketData->Turnover;
    double OpenInterest         = pDepthMarketData->OpenInterest;
    double ClosePrice           = 0;//pDepthMarketData->ClosePrice;
    double SettlementPrie       = 0;// pDepthMarketData->SettlementPrice;
    double UpperLimitPrice      = pDepthMarketData->UpperLimitPrice;
    double LowerLimitPirce      = pDepthMarketData->LowerLimitPrice;
    QString UpdateTime          = pDepthMarketData->UpdateTime;
    int    UpdateMillisec       = pDepthMarketData->UpdateMillisec;
    double BidPrice1            = pDepthMarketData->BidPrice1;
    int    BidVolume1           = pDepthMarketData->BidVolume1;
    double AskPrice1            = pDepthMarketData->AskPrice1;
    int    AskVolume1           = pDepthMarketData->AskVolume1;
    double AveragePrice         = pDepthMarketData->AveragePrice;
    QString ActionDay           = pDepthMarketData->ActionDay;
    QString TickType             = QString::fromStdString(ticktype);


    insert.bindValue(0, tdate);
    insert.bindValue(1, InstrumentID);
    insert.bindValue(2, LastPrice);
    insert.bindValue(3, PreSetttlementPrice);
    insert.bindValue(4, PreClosePrice);
    insert.bindValue(5, OpenPrice);
    insert.bindValue(6, PreOpenInterest);
    insert.bindValue(7, HighestPrice);
    insert.bindValue(8, LowestPrice);
    insert.bindValue(9, Volume);
    insert.bindValue(10, Turnover);
    insert.bindValue(11, OpenInterest);
    insert.bindValue(12, ClosePrice);
    insert.bindValue(13, SettlementPrie);
    insert.bindValue(14, UpperLimitPrice);
    insert.bindValue(15, LowerLimitPirce);
    insert.bindValue(16, UpdateTime);
    insert.bindValue(17, UpdateMillisec);
    insert.bindValue(18, BidPrice1);
    insert.bindValue(19, BidVolume1);
    insert.bindValue(20, AskPrice1);
    insert.bindValue(21, AskVolume1);
    insert.bindValue(22, AveragePrice);
    insert.bindValue(23, ActionDay);
    insert.bindValue(24, TickType);

    bool result = insert.exec();
//    qDebug() << result;

}

void DBWriter::InsertOrUpdateAccountPostion(QString tableName, CThostFtdcInvestorPositionField *positionInfo) {
    //先检查是否存在该账户的持仓纪录
    QSqlQuery exist(this->db);
    exist.prepare("select investor_id from account_position where investor_id=:id and instrument_id=:instru ");

    exist.bindValue(":id", positionInfo->InvestorID);
    exist.bindValue(":instru", positionInfo->InstrumentID);
    exist.exec();
    cout << "handle instrument " << positionInfo->InstrumentID << endl;
    //qDebug() << "000:" << exist.lastError().databaseText();
    if (exist.next()) {
        //如果存在就更新
        //exist.record();
        QSqlQuery update(this->db);
        if (positionInfo->PosiDirection == THOST_FTDC_PD_Long) {    //多头持仓
            update.prepare("update account_position set long_position=:long_position , long_profit=:long_profit, "
                                   " long_margin=:long_margin,query_date=:date where investor_id=:id and instrument_id=:instru ");
            update.bindValue(":long_position", positionInfo->Position);
            update.bindValue(":long_profit", positionInfo->PositionProfit);
            update.bindValue("long_margin", positionInfo->UseMargin);
            update.bindValue(":date", positionInfo->TradingDay);
            update.bindValue(":id", positionInfo->InvestorID);
            update.bindValue(":instru", positionInfo->InstrumentID);
            cout << "update long position " << update.lastQuery().toStdString() << endl;
            bool update_result = update.exec();
            if(update_result)
            {
                cout << "update long success" << positionInfo->InstrumentID <<endl;
            }
            //qDebug() << "111:"<<update.lastError().databaseText();
        }
        if (positionInfo->PosiDirection == THOST_FTDC_PD_Short) { //空头持仓
            update.prepare(" update account_position set short_position=:short_position,short_profit=:short_profit, "
                                   " short_margin=:short_margin,query_date=:date where investor_id=:id and instrument_id=:instru ");
            update.bindValue(":short_position", positionInfo->Position);
            update.bindValue(":short_profit", positionInfo->PositionProfit);
            update.bindValue(":short_margin", positionInfo->UseMargin);
            update.bindValue(":date", positionInfo->TradingDay);
            update.bindValue(":id", positionInfo->InvestorID);
            update.bindValue(":instru", positionInfo->InstrumentID);
            bool update_result = update.exec();
            cout << "update short position " << update.lastQuery().toStdString() << endl;
            if(update_result)
            {
                cout << "update short success"<< positionInfo->InstrumentID << endl;
            }

            //qDebug() << "222:" << update.lastError().databaseText();
        }
    } else {
        //否则就插入
        QSqlQuery insert(this->db);
        if (positionInfo->PosiDirection == THOST_FTDC_PD_Long) {    //多头持仓
            insert.prepare("insert into account_position (investor_id,instrument_id,long_position,long_profit, long_margin,query_date) values (:id,:instru,:long_position,:long_profit,:long_margin,:date) ");
            insert.bindValue(":id", positionInfo->InvestorID);
            insert.bindValue(":instru", positionInfo->InstrumentID);
            insert.bindValue(":long_position", positionInfo->Position);
            insert.bindValue(":long_profit", positionInfo->PositionProfit);
            insert.bindValue(":long_margin", positionInfo->UseMargin);
            insert.bindValue(":date", positionInfo->TradingDay);
            cout << "insert long position " << insert.lastQuery().toStdString() << endl;
            bool insert_result = insert.exec();
            if(insert_result)
            {
                cout << "insert success" <<endl;
            }

            //qDebug() << "333:" << insert.lastError().databaseText();
        }
        if (positionInfo->PosiDirection == THOST_FTDC_PD_Short) { //空头持仓
            insert.prepare("insert into account_position (investor_id,instrument_id,short_position,short_profit, "
                                   " short_margin,query_date) values (:id,:instru,:short_position,:short_profit,:short_margin,:date) ");
            insert.bindValue(":id", positionInfo->InvestorID);
            insert.bindValue(":instru", positionInfo->InstrumentID);
            insert.bindValue(":short_position", positionInfo->Position);
            insert.bindValue(":short_profit", positionInfo->PositionProfit);
            insert.bindValue(":short_margin", positionInfo->UseMargin);
            insert.bindValue(":date", positionInfo->TradingDay);
            insert.exec();
            cout << "insert long position " << insert.lastQuery().toStdString() << endl;
            //qDebug() << "444:" << insert.lastError().databaseText();
        }
    }
}

void DBWriter::DBClose(){
    QString connection;
    connection = db.connectionName();
    db.close();
    db = QSqlDatabase();
    db.removeDatabase(connection);

}