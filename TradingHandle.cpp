//
// Created by biggreyhairboy on 7/27/16.
//

#include <cstring>
#include <iostream>
#include <thread>
#include "TradingHandle.h"
#include "ctpapi_linux64/ThostFtdcTraderApi.h"
using namespace std;

TradingHandle::TradingHandle(CThostFtdcTraderApi* iTraderApi, char* front_address, TThostFtdcBrokerIDType brokerid,
                             TThostFtdcInvestorIDType investorid, TThostFtdcPasswordType password, DBDriver *dbdriver,
                             TThostFtdcInstrumentIDType INSTRUMENT_ID,
                             TThostFtdcPriceType LIMIT_PRICE, int  quantity, TThostFtdcDirectionType DIRECTION)
{
    this->pTraderApi = iTraderApi;
    this->dbDrvier = dbdriver;
    strcpy(this->FRONT_ADDR_trade, front_address);
    strcpy(this->brokerIDType, brokerid);
    strcpy(this->investorIDType, investorid);
    strcpy(this->passwordType, password);

    strcpy(this->INSTRUMENT_ID, INSTRUMENT_ID);
    this->LIMIT_PRICE = LIMIT_PRICE;
    this->DIRECTION = DIRECTION;
    this->quantity = quantity;
    int iRequestID_trade = 0;
}
// 流控判断
bool IsFlowControl(int iResult)
{
    return ((iResult == -2) || (iResult == -3));
}

void TradingHandle::OnFrontConnected()
{
    cerr << "trade---->>> " << "OnFrontConnected" << endl;
    ///用户登录请求
    ReqUserLogin();
}

void TradingHandle::ReqUserLogin()
{
    CThostFtdcReqUserLoginField req;
    memset(&req, 0, sizeof(req));
    strcpy(req.BrokerID, brokerIDType);
    strcpy(req.UserID, investorIDType);
    strcpy(req.Password, passwordType);
    int iResult = pTraderApi->ReqUserLogin(&req, ++iRequestID_trade);
    cerr << "trade---->>> 发送用户登录请求: " << iResult << ((iResult == 0) ? ", 成功" : ", 失败") << endl;
}

void TradingHandle::OnRspUserLogin(CThostFtdcRspUserLoginField *pRspUserLogin,
                                CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{
    cerr << "trade---->>> " << "OnRspUserLogin" << endl;
    if (bIsLast && !IsErrorRspInfo(pRspInfo))
    {
        // 保存会话参数
        FRONT_ID = pRspUserLogin->FrontID;
        SESSION_ID = pRspUserLogin->SessionID;
        int iNextOrderRef = atoi(pRspUserLogin->MaxOrderRef);
        iNextOrderRef++;
        sprintf(ORDER_REF, "%d", iNextOrderRef);
        sprintf(EXECORDER_REF, "%d", 1);
        sprintf(FORQUOTE_REF, "%d", 1);
        sprintf(QUOTE_REF, "%d", 1);
        ///获取当前交易日
        cerr << "trade---->>> 获取当前交易日 = " << pTraderApi->GetTradingDay() << endl;
        ///投资者结算结果确认
        ReqSettlementInfoConfirm();
    }
}

void TradingHandle::ReqSettlementInfoConfirm()
{
    CThostFtdcSettlementInfoConfirmField req;
    memset(&req, 0, sizeof(req));
    strcpy(req.BrokerID, brokerIDType);
    strcpy(req.InvestorID, investorIDType);
    int iResult = pTraderApi->ReqSettlementInfoConfirm(&req, ++iRequestID_trade);
    cerr << "trade---->>> 投资者结算结果确认: " << iResult << ((iResult == 0) ? ", 成功" : ", 失败") << endl;
    ReqOrderInsert();
}

void TradingHandle::OnRspSettlementInfoConfirm(CThostFtdcSettlementInfoConfirmField *pSettlementInfoConfirm, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{
    cerr << "trade---->>> " << "OnRspSettlementInfoConfirm" << endl;
    if (bIsLast && !IsErrorRspInfo(pRspInfo))
    {
        ///请求查询合约
        ReqQryInstrument();
    }

}

void TradingHandle::ReqQryInstrument()
{
    CThostFtdcQryInstrumentField req;
    memset(&req, 0, sizeof(req));
    strcpy(req.InstrumentID, INSTRUMENT_ID);
    while (true)
    {
        int iResult = pTraderApi->ReqQryInstrument(&req, ++iRequestID_trade);
        if (!IsFlowControl(iResult))
        {
            cerr << "trade---->>> 请求查询合约: " << iResult << ((iResult == 0) ? ", 成功" : ", 失败") << endl;
            break;
        }
        else
        {
            cerr << "trade---->>> 请求查询合约: " << iResult << ", 受到流控" << endl;
            //Sleep(1000);
            std::this_thread::sleep_for(std::chrono::microseconds(1000));
        }
    } // while
}

void TradingHandle::OnRspQryInstrument(CThostFtdcInstrumentField *pInstrument, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{
    cerr << "trade---->>> " << "OnRspQryInstrument" << endl;
    if (bIsLast && !IsErrorRspInfo(pRspInfo))
    {
        ///请求查询合约
        ReqQryTradingAccount();
    }
}

void TradingHandle::ReqQryTradingAccount()
{
    CThostFtdcQryTradingAccountField req;
    memset(&req, 0, sizeof(req));
    strcpy(req.BrokerID, brokerIDType);
    strcpy(req.InvestorID, investorIDType);
    while (true)
    {
        int iResult = pTraderApi->ReqQryTradingAccount(&req, ++iRequestID_trade);
        if (!IsFlowControl(iResult))
        {
            //cerr << "trade---->>> 请求查询资金账户: " << iResult << ((iResult == 0) ? ", 成功" : ", 失败") << endl;
            break;
        }
        else
        {
            //cerr << "trade---->>> 请求查询资金账户: " << iResult << ", 受到流控" << endl;
            //Sleep(1000);
            std::this_thread::sleep_for(std::chrono::microseconds(1000));
        }
    } // while
}

void TradingHandle::OnRspQryTradingAccount(CThostFtdcTradingAccountField *pTradingAccount, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{
    cerr << "trade---->>> " << "OnRspQryTradingAccount" << endl;
    if (bIsLast && !IsErrorRspInfo(pRspInfo))
    {
        ///请求查询投资者持仓
        ReqQryInvestorPosition();
    }
}

void TradingHandle::ReqQryInvestorPosition()
{
    CThostFtdcQryInvestorPositionField req;
    memset(&req, 0, sizeof(req));
    strcpy(req.BrokerID, brokerIDType);
    strcpy(req.InvestorID, investorIDType);
    strcpy(req.InstrumentID, INSTRUMENT_ID);
    while (true)
    {
        int iResult = pTraderApi->ReqQryInvestorPosition(&req, ++iRequestID_trade);
        if (!IsFlowControl(iResult))
        {
//            cerr << "trade---->>> 请求查询投资者持仓: " << iResult << ((iResult == 0) ? ", 成功" : ", 失败") << endl;
            break;
        }
        else
        {
//            cerr << "trade---->>> 请求查询投资者持仓: " << iResult << ", 受到流控" << endl;
            std::this_thread::sleep_for(std::chrono::microseconds(1000));
        }
    }
}

void TradingHandle::OnRspQryInvestorPosition(CThostFtdcInvestorPositionField *pInvestorPosition, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{
    cerr << "trade---->>> " << "OnRspQryInvestorPosition" << endl;
    if (bIsLast && !IsErrorRspInfo(pRspInfo))
    {
        ///报单录入请求
        ReqOrderInsert();
        //执行宣告录入请求
        ReqExecOrderInsert();
        //询价录入
        ReqForQuoteInsert();
        //做市商报价录入
        ReqQuoteInsert();
    }
}

void TradingHandle::ReqOrderInsert()
{
    CThostFtdcInputOrderField req;
    memset(&req, 0, sizeof(req));
    ///经纪公司代码
    strcpy(req.BrokerID, brokerIDType);
    ///投资者代码
    strcpy(req.InvestorID, investorIDType);
    ///合约代码
    strcpy(req.InstrumentID, INSTRUMENT_ID);
    ///报单引用
    strcpy(req.OrderRef, ORDER_REF);
    ///用户代码
    //	TThostFtdcUserIDType	UserID;
    ///报单价格条件: 限价
    req.OrderPriceType = THOST_FTDC_OPT_LimitPrice;
    ///买卖方向: 
    req.Direction = DIRECTION;
    ///组合开平标志: 开仓
    req.CombOffsetFlag[0] = THOST_FTDC_OF_Open;
    ///组合投机套保标志
    req.CombHedgeFlag[0] = THOST_FTDC_HF_Speculation;
    ///价格
    req.LimitPrice = LIMIT_PRICE;
    ///数量: 1
    req.VolumeTotalOriginal = quantity;
    ///有效期类型: 当日有效
    req.TimeCondition = THOST_FTDC_TC_GFD;
    ///GTD日期
    //	TThostFtdcDateType	GTDDate;
    ///成交量类型: 任何数量
    req.VolumeCondition = THOST_FTDC_VC_AV;
    ///最小成交量: 1
    req.MinVolume = 1;
    ///触发条件: 立即
    req.ContingentCondition = THOST_FTDC_CC_Immediately;
    ///止损价
    //	TThostFtdcPriceType	StopPrice;
    ///强平原因: 非强平
    req.ForceCloseReason = THOST_FTDC_FCC_NotForceClose;
    ///自动挂起标志: 否
    req.IsAutoSuspend = 0;
    ///业务单元
    //	TThostFtdcBusinessUnitType	BusinessUnit;
    ///请求编号
    //	TThostFtdcRequestIDType	RequestID;
    ///用户强评标志: 否
    req.UserForceClose = 0;

    int iResult = pTraderApi->ReqOrderInsert(&req, ++iRequestID_trade);
    cerr << "trade---->>> 报单录入请求: " << iResult << ((iResult == 0) ? ", 成功" : ", 失败") << endl;
}


////获得下单汇报
//void TradingHandle::OnRtnOrder(CThostFtdcOrderField *pOrder)
//{
//
//}

//执行宣告录入请求
void TradingHandle::ReqExecOrderInsert()
{
    CThostFtdcInputExecOrderField req;
    memset(&req, 0, sizeof(req));
    ///经纪公司代码
    strcpy(req.BrokerID, brokerIDType);
    ///投资者代码
    strcpy(req.InvestorID, investorIDType);
    ///合约代码
    strcpy(req.InstrumentID, INSTRUMENT_ID);
    ///报单引用
    strcpy(req.ExecOrderRef, EXECORDER_REF);
    ///用户代码
    //	TThostFtdcUserIDType	UserID;
    ///数量
    req.Volume = 1;
    ///请求编号
    //TThostFtdcRequestIDType	RequestID;
    ///业务单元
    //TThostFtdcBusinessUnitType	BusinessUnit;
    ///开平标志
    req.OffsetFlag = THOST_FTDC_OF_Close;//如果是上期所，需要填平今或平昨
    ///投机套保标志
    req.HedgeFlag = THOST_FTDC_HF_Speculation;
    ///执行类型
    req.ActionType = THOST_FTDC_ACTP_Exec;//如果放弃执行则填THOST_FTDC_ACTP_Abandon
    ///保留头寸申请的持仓方向
    req.PosiDirection = THOST_FTDC_PD_Long;
    ///期权行权后是否保留期货头寸的标记
    req.ReservePositionFlag = THOST_FTDC_EOPF_UnReserve;//这是中金所的填法，大商所郑商所填THOST_FTDC_EOPF_Reserve
    ///期权行权后生成的头寸是否自动平仓
    req.CloseFlag = THOST_FTDC_EOCF_AutoClose;//这是中金所的填法，大商所郑商所填THOST_FTDC_EOCF_NotToClose

    int iResult = pTraderApi->ReqExecOrderInsert(&req, ++iRequestID_trade);
    cerr << "trade---->>> 执行宣告录入请求: " << iResult << ((iResult == 0) ? ", 成功" : ", 失败") << endl;
}

//询价录入请求
void TradingHandle::ReqForQuoteInsert()
{
    CThostFtdcInputForQuoteField req;
    memset(&req, 0, sizeof(req));
    ///经纪公司代码
    strcpy(req.BrokerID, brokerIDType);
    ///投资者代码
    strcpy(req.InvestorID, investorIDType);
    ///合约代码
    strcpy(req.InstrumentID, INSTRUMENT_ID);
    ///报单引用
    strcpy(req.ForQuoteRef, EXECORDER_REF);
    ///用户代码
    //	TThostFtdcUserIDType	UserID;

    int iResult = pTraderApi->ReqForQuoteInsert(&req, ++iRequestID_trade);
    cerr << "trade---->>> 询价录入请求: " << iResult << ((iResult == 0) ? ", 成功" : ", 失败") << endl;
}
//报价录入请求
void TradingHandle::ReqQuoteInsert()
{
    CThostFtdcInputQuoteField req;
    memset(&req, 0, sizeof(req));
    ///经纪公司代码
    strcpy(req.BrokerID, brokerIDType);
    ///投资者代码
    strcpy(req.InvestorID, investorIDType);
    ///合约代码
    strcpy(req.InstrumentID, INSTRUMENT_ID);
    ///报单引用
    strcpy(req.QuoteRef, QUOTE_REF);
    ///卖价格
    req.AskPrice = LIMIT_PRICE;
    ///买价格
    req.BidPrice = LIMIT_PRICE - 1.0;
    ///卖数量
    req.AskVolume = 1;
    ///买数量
    req.BidVolume = 1;
    ///请求编号
    //TThostFtdcRequestIDType	RequestID;
    ///业务单元
    //TThostFtdcBusinessUnitType	BusinessUnit;
    ///卖开平标志
    req.AskOffsetFlag = THOST_FTDC_OF_Open;
    ///买开平标志
    req.BidOffsetFlag = THOST_FTDC_OF_Open;
    ///卖投机套保标志
    req.AskHedgeFlag = THOST_FTDC_HF_Speculation;
    ///买投机套保标志
    req.BidHedgeFlag = THOST_FTDC_HF_Speculation;

    int iResult = pTraderApi->ReqQuoteInsert(&req, ++iRequestID_trade);
    cerr << "trade---->>> 报价录入请求: " << iResult << ((iResult == 0) ? ", 成功" : ", 失败") << endl;
}

void TradingHandle::OnRspOrderInsert(CThostFtdcInputOrderField *pInputOrder, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{
    cerr << "trade---->>> " << "OnRspOrderInsert" << endl;
    IsErrorRspInfo(pRspInfo);
}

void TradingHandle::OnRspExecOrderInsert(CThostFtdcInputExecOrderField *pInputExecOrder, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{
    //如果执行宣告正确，则不会进入该回调
    cerr << "trade---->>> " << "OnRspExecOrderInsert" << endl;
    IsErrorRspInfo(pRspInfo);
}

void TradingHandle::OnRspForQuoteInsert(CThostFtdcInputForQuoteField *pInputForQuote, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{
    //如果询价正确，则不会进入该回调
    cerr << "trade---->>> " << "OnRspForQuoteInsert" << endl;
    IsErrorRspInfo(pRspInfo);
}

void TradingHandle::OnRspQuoteInsert(CThostFtdcInputQuoteField *pInputQuote, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{
    //如果报价正确，则不会进入该回调
    cerr << "trade---->>> " << "OnRspQuoteInsert" << endl;
    IsErrorRspInfo(pRspInfo);
}

void TradingHandle::ReqOrderAction(CThostFtdcOrderField *pOrder)
{
    static bool ORDER_ACTION_SENT = false;		//是否发送了报单
    if (ORDER_ACTION_SENT)
        return;

    CThostFtdcInputOrderActionField req;
    memset(&req, 0, sizeof(req));
    ///经纪公司代码
    strcpy(req.BrokerID, pOrder->BrokerID);
    ///投资者代码
    strcpy(req.InvestorID, pOrder->InvestorID);
    ///报单操作引用
    //	TThostFtdcOrderActionRefType	OrderActionRef;
    ///报单引用
    strcpy(req.OrderRef, pOrder->OrderRef);
    ///请求编号
    //	TThostFtdcRequestIDType	RequestID;
    ///前置编号
    req.FrontID = FRONT_ID;
    ///会话编号
    req.SessionID = SESSION_ID;
    ///交易所代码
    //	TThostFtdcExchangeIDType	ExchangeID;
    ///报单编号
    //	TThostFtdcOrderSysIDType	OrderSysID;
    ///操作标志
    req.ActionFlag = THOST_FTDC_AF_Delete;
    ///价格
    //	TThostFtdcPriceType	LimitPrice;
    ///数量变化
    //	TThostFtdcVolumeType	VolumeChange;
    ///用户代码
    //	TThostFtdcUserIDType	UserID;
    ///合约代码
    strcpy(req.InstrumentID, pOrder->InstrumentID);

    int iResult = pTraderApi->ReqOrderAction(&req, ++iRequestID_trade);
    cerr << "trade---->>> 报单操作请求: " << iResult << ((iResult == 0) ? ", 成功" : ", 失败") << endl;
    ORDER_ACTION_SENT = true;
}

void TradingHandle::ReqExecOrderAction(CThostFtdcExecOrderField *pExecOrder)
{
    static bool EXECORDER_ACTION_SENT = false;		//是否发送了报单
    if (EXECORDER_ACTION_SENT)
        return;

    CThostFtdcInputExecOrderActionField req;
    memset(&req, 0, sizeof(req));

    ///经纪公司代码
    strcpy(req.BrokerID, pExecOrder->BrokerID);
    ///投资者代码
    strcpy(req.InvestorID, pExecOrder->InvestorID);
    ///执行宣告操作引用
    //TThostFtdcOrderActionRefType	ExecOrderActionRef;
    ///执行宣告引用
    strcpy(req.ExecOrderRef, pExecOrder->ExecOrderRef);
    ///请求编号
    //TThostFtdcRequestIDType	RequestID;
    ///前置编号
    req.FrontID = FRONT_ID;
    ///会话编号
    req.SessionID = SESSION_ID;
    ///交易所代码
    //TThostFtdcExchangeIDType	ExchangeID;
    ///执行宣告操作编号
    //TThostFtdcExecOrderSysIDType	ExecOrderSysID;
    ///操作标志
    req.ActionFlag = THOST_FTDC_AF_Delete;
    ///用户代码
    //TThostFtdcUserIDType	UserID;
    ///合约代码
    strcpy(req.InstrumentID, pExecOrder->InstrumentID);

    int iResult = pTraderApi->ReqExecOrderAction(&req, ++iRequestID_trade);
    cerr << "trade---->>> 执行宣告操作请求: " << iResult << ((iResult == 0) ? ", 成功" : ", 失败") << endl;
    EXECORDER_ACTION_SENT = true;
}

void TradingHandle::ReqQuoteAction(CThostFtdcQuoteField *pQuote)
{
    static bool QUOTE_ACTION_SENT = false;		//是否发送了报单
    if (QUOTE_ACTION_SENT)
        return;

    CThostFtdcInputQuoteActionField req;
    memset(&req, 0, sizeof(req));
    ///经纪公司代码
    strcpy(req.BrokerID, pQuote->BrokerID);
    ///投资者代码
    strcpy(req.InvestorID, pQuote->InvestorID);
    ///报价操作引用
    //TThostFtdcOrderActionRefType	QuoteActionRef;
    ///报价引用
    strcpy(req.QuoteRef, pQuote->QuoteRef);
    ///请求编号
    //TThostFtdcRequestIDType	RequestID;
    ///前置编号
    req.FrontID = FRONT_ID;
    ///会话编号
    req.SessionID = SESSION_ID;
    ///交易所代码
    //TThostFtdcExchangeIDType	ExchangeID;
    ///报价操作编号
    //TThostFtdcOrderSysIDType	QuoteSysID;
    ///操作标志
    req.ActionFlag = THOST_FTDC_AF_Delete;
    ///用户代码
    //TThostFtdcUserIDType	UserID;
    ///合约代码
    strcpy(req.InstrumentID, pQuote->InstrumentID);

    int iResult = pTraderApi->ReqQuoteAction(&req, ++iRequestID_trade);
    cerr << "trade---->>> 报价操作请求: " << iResult << ((iResult == 0) ? ", 成功" : ", 失败") << endl;
    QUOTE_ACTION_SENT = true;
}

void TradingHandle::OnRspOrderAction(CThostFtdcInputOrderActionField *pInputOrderAction, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{
    cerr << "trade---->>> " << "OnRspOrderAction" << endl;
    IsErrorRspInfo(pRspInfo);
}

void TradingHandle::OnRspExecOrderAction(CThostFtdcInputExecOrderActionField *pInpuExectOrderAction, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{
    //正确的撤单操作，不会进入该回调
    cerr << "trade---->>> " << "OnRspExecOrderAction" << endl;
    IsErrorRspInfo(pRspInfo);
}

void TradingHandle::OnRspQuoteAction(CThostFtdcInputQuoteActionField *pInpuQuoteAction, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{
    //正确的撤单操作，不会进入该回调
    cerr << "trade---->>> " << "OnRspQuoteAction" << endl;
    IsErrorRspInfo(pRspInfo);
}

///报单通知
void TradingHandle::OnRtnOrder(CThostFtdcOrderField *pOrder)
{
    cerr << "trade---->>> " << "OnRtnOrder" << endl;
    if (IsMyOrder(pOrder))
    {
        if (IsTradingOrder(pOrder))
            ReqOrderAction(pOrder);
        else if (pOrder->OrderStatus == THOST_FTDC_OST_Canceled)
            cout << "trade---->>> 撤单成功" << endl;
    }
}

//执行宣告通知
void TradingHandle::OnRtnExecOrder(CThostFtdcExecOrderField *pExecOrder)
{
    cerr << "trade---->>> " << "OnRtnExecOrder" << endl;
    if (IsMyExecOrder(pExecOrder))
    {
        if (IsTradingExecOrder(pExecOrder))
            ReqExecOrderAction(pExecOrder);
        else if (pExecOrder->ExecResult == THOST_FTDC_OER_Canceled)
            cout << "trade---->>> 执行宣告撤单成功" << endl;
    }
}

//询价通知
void TradingHandle::OnRtnForQuoteRsp(CThostFtdcForQuoteRspField *pForQuoteRsp)
{
    //上期所中金所询价通知通过该接口返回；只有做市商客户可以收到该通知
    cerr << "trade---->>> " << "OnRtnForQuoteRsp" << endl;
}

//报价通知
void TradingHandle::OnRtnQuote(CThostFtdcQuoteField *pQuote)
{
    cerr << "trade---->>> " << "OnRtnQuote" << endl;
    if (IsMyQuote(pQuote))
    {
        if (IsTradingQuote(pQuote))
            ReqQuoteAction(pQuote);
        else if (pQuote->QuoteStatus == THOST_FTDC_OST_Canceled)
            cout << "trade---->>> 报价撤单成功" << endl;
    }
}

///成交通知
void TradingHandle::OnRtnTrade(CThostFtdcTradeField *pTrade)
{
    cerr << "trade---->>> " << "OnRtnTrade" << endl;
}

void TradingHandle::OnFrontDisconnected(int nReason)
{
    cerr << "trade---->>> " << "OnFrontDisconnected" << endl;
    cerr << "trade---->>> Reason = " << nReason << endl;
}

void TradingHandle::OnHeartBeatWarning(int nTimeLapse)
{
    cerr << "trade---->>> " << "OnHeartBeatWarning" << endl;
    cerr << "trade---->>> nTimerLapse = " << nTimeLapse << endl;
}

void TradingHandle::OnRspError(CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{
    cerr << "trade---->>> " << "OnRspError" << endl;
    IsErrorRspInfo(pRspInfo);
}

bool TradingHandle::IsErrorRspInfo(CThostFtdcRspInfoField *pRspInfo)
{
    // 如果ErrorID != 0, 说明收到了错误的响应
    bool bResult = ((pRspInfo) && (pRspInfo->ErrorID != 0));
    if (bResult)
        cerr << "trade---->>> ErrorID=" << pRspInfo->ErrorID << ", ErrorMsg=" << pRspInfo->ErrorMsg << endl;
    return bResult;
}

bool TradingHandle::IsMyOrder(CThostFtdcOrderField *pOrder)
{
    return ((pOrder->FrontID == FRONT_ID) &&
            (pOrder->SessionID == SESSION_ID) &&
            (strcmp(pOrder->OrderRef, ORDER_REF) == 0));
}

bool TradingHandle::IsMyExecOrder(CThostFtdcExecOrderField *pExecOrder)
{
    return ((pExecOrder->FrontID == FRONT_ID) &&
            (pExecOrder->SessionID == SESSION_ID) &&
            (strcmp(pExecOrder->ExecOrderRef, EXECORDER_REF) == 0));
}

bool TradingHandle::IsMyQuote(CThostFtdcQuoteField *pQuote)
{
    return ((pQuote->FrontID == FRONT_ID) &&
            (pQuote->SessionID == SESSION_ID) &&
            (strcmp(pQuote->QuoteRef, QUOTE_REF) == 0));
}

bool TradingHandle::IsTradingOrder(CThostFtdcOrderField *pOrder)
{
    return ((pOrder->OrderStatus != THOST_FTDC_OST_PartTradedNotQueueing) &&
            (pOrder->OrderStatus != THOST_FTDC_OST_Canceled) &&
            (pOrder->OrderStatus != THOST_FTDC_OST_AllTraded));
}

bool TradingHandle::IsTradingExecOrder(CThostFtdcExecOrderField *pExecOrder)
{
    return (pExecOrder->ExecResult != THOST_FTDC_OER_Canceled);
}

bool TradingHandle::IsTradingQuote(CThostFtdcQuoteField *pQuote)
{
    return (pQuote->QuoteStatus != THOST_FTDC_OST_Canceled);
}
