//
// Created by patrick on 1/3/17.
//

#include "initialize.h"
#include "GVAR.h"
#include <QDebug>
#include <QFile>
#include <QTextStream>
#include <QSettings>
#include <iostream>

using namespace std;

//读取文档找到前置机地址
void iniFrontAdress(){
    QFile iniFile("/home/patrick/ClionProjects/CTPClientDemo/ini/front.ini");
    if (!iniFile.open(QIODevice::ReadOnly|QIODevice::Text)){
        qDebug() << "cannot find front.ini";
        cerr << "错误,找不到front.ini" << endl;
        abort();
    }
    QTextStream in(&iniFile);
    FRONT_ADDRESS = in.readLine();
    iniFile.close();
}

////读取文档初始化合约信息
//void iniInstrument(){
//    QFile iniFile("ini/instrumentInfo.ini");
//    if (!iniFile.open(QIODevice::ReadOnly | QIODevice::Text)){
//        qDebug() << "cannot find instrumentInfo.ini";
//        cerr << "错误, 找不到instrumentInfo.ini" << endl;
//        abort();
//    }
//    QTextStream in(&iniFile);
//    QString line;
//    while (!in.atEnd()){
//        line = in.readLine();
//        QStringList list = line.split("|");
//        QString code = list.at(0).trimmed();
//        QString name = list.at(1).trimmed();
//        instruList.insert(make_pair(code, new Instrument(code, name)));
//    }
//    iniFile.close();
//}

//读取文件初始化数据库信息
void iniDB(){
    QSettings setting("/home/patrick/ClionProjects/CTPClientDemo/ini/db.ini", QSettings::IniFormat);
    DB_DRIVER_NAME = setting.value("DB_DRIVER_NAME").toString();
    DB_HOST_NAME = setting.value("DB_HOST_NAME").toString();
    DATABASE_NAME = setting.value("DATABASE_NAME").toString();
    USER_NAME = setting.value("USER_NAME").toString();
    PASSWORD = setting.value("PASSWORD").toString();
}