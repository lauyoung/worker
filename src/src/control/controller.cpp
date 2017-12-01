#include "controller.h"
#include "networkhelper.h"
#include "utils.h"

#include <QDomDocument>
#include <QJsonDocument>
#include <QJsonObject>

const QString NETTYPE_LOGIN      = "login";
const QString NETTYPE_CHECKSTATUS = "checkstatus";
const QString NETTYPE_RECENTCONTACT = "getrecentcontact";

Controller::Controller(const QString &uin, QObject *parent)
	:QObject(parent)
{
	m_UserData.m_Uin = uin;
	m_UserData.m_Status = 0;

	m_workerClient = new WorkerClient(uin, this);
	initReqFuncMap();
    setupSignals();

	//初始化map
}

Controller::~Controller()
{

}

void Controller::netGetUUid(int clientKey, const QJsonObject &obj)
{
	Q_UNUSED(obj);

    NetworkHelper *inst = NetworkHelper::instance();
    inst->getUUid(clientKey);
}

void Controller::netCheckLoginStatus(int clientKey, const QJsonObject &obj)
{
	Q_UNUSED(obj);

    NetworkHelper *inst =NetworkHelper::instance();
    inst->checkLoginStatus(clientKey, m_UserData);
}

void Controller::netNewLoginPage(int clientKey)
{
    NetworkHelper *inst =NetworkHelper::instance();
    inst->newLoginPage(clientKey, m_UserData);
}

void Controller::netWebWxInit(int clientKey, const QJsonObject &obj)
{
	Q_UNUSED(obj);

	NetworkHelper *inst =NetworkHelper::instance();
	inst->webWxInit(clientKey, m_UserData);

}
void Controller::initReqFuncMap()
{
    m_typeFuncMap.insert(NETTYPE_LOGIN, &Controller::netGetUUid);
	m_typeFuncMap.insert(NETTYPE_CHECKSTATUS, &Controller::netCheckLoginStatus);
	m_typeFuncMap.insert(NETTYPE_RECENTCONTACT, &Controller::netWebWxInit);
}

/**
 * @brief Controller::processMsg
 * @param clientKey
 * @param type
 * @param obj
 */
void Controller::processMsg(int clientKey, const QString &type, const QJsonObject &obj)
{
    QMap<QString, reqFunc>::const_iterator it = m_typeFuncMap.find(type);
	if (m_typeFuncMap.cend() != it) {
		const reqFunc func = it.value();
		(this->*func)(clientKey, obj);
	}
}

void Controller::sendMsg(int clientKey, const QString &type, const QJsonObject &obj)
{
	QJsonObject jsMsg;
	jsMsg.insert("ClientKey", clientKey);
	jsMsg.insert("WxType", type);
	jsMsg.insert("Uin", m_UserData.m_Uin.toInt());
	jsMsg.insert("Data", obj);

    QJsonDocument jdoc(jsMsg);
    QByteArray data = jdoc.toJson(QJsonDocument::Compact);
    m_workerClient->writeMsg(data);
}

/**
 * @brief Controller::setupSignals
 */
void Controller::setupSignals()
{
	connect(m_workerClient, SIGNAL(recvMsg(QString)),
		this, SLOT(onRecvMsg(QString)));

    NetworkHelper *inst = NetworkHelper::instance();

    connect(inst, SIGNAL(getUUidFinished(int, const QByteArray &)),
            this, SLOT(onGetUUidFinished(int, const QByteArray &)));

    connect(inst, SIGNAL(checkLoginStatusFinished(int, const QByteArray &)),
            this, SLOT(onCheckLoginStatusFinished(int, const QByteArray &)));

    connect(inst, SIGNAL(newLoginPageFinished(int, const QByteArray &, QMap<QString, QVariant> &)),
        this, SLOT(onNewLoginPageFinished(int, const QByteArray &, QMap<QString, QVariant> &)));

	connect(inst, SIGNAL(webWxinitFinished(int, const QByteArray &)), 
		this, SLOT(onWebWxinitFinished(int, const QByteArray &)));
}

void Controller::onGetUUidFinished(int clientKey, const QByteArray &data)
{ 
    QString result;
    if (!Utils::getMatchStr(data, "[0-9a-zA-Z_\-]{10}==", result)) {
        qDebug() << "GetUUid Failed";
        return ;
    }
	m_UserData.m_UUid = result;
    QString retUrl = QString("https://login.weixin.qq.com/qrcode/") + m_UserData.m_UUid;

    //LocalSocket 写入数据
    QJsonObject obj;
    obj.insert("qrcodeurl", retUrl);

    sendMsg(clientKey, NETTYPE_LOGIN, obj);
}

void Controller::onCheckLoginStatusFinished(int clientKey, const QByteArray &data)
{
    QString result, redirectUrl, host;
    Utils::getMatchStr(data, "window.code=[0-9]{3}", result);
    int code = result.mid(result.size()-3).toInt();
	QJsonObject jsObj;

    switch (code)
    {
    case 201:
			//localSocket不需要返回 返回扫码成功 继续检测
			qDebug() << "need confirm";
			netCheckLoginStatus(clientKey, jsObj);
			break;

    case 200:
        //localSocket 登录成功
        qDebug() << "login success";

        Utils::getValueByKey(data, "window.redirect_uri", redirectUrl);
        Utils::getMatchStr(redirectUrl, "[a-z0-9.]*.com", host);

        m_UserData.m_Status = 200;
        m_UserData.m_Host = host;
        m_UserData.m_RedirectUrl = redirectUrl;

        netNewLoginPage(clientKey);
        break;
    case 408:
        //localSocket 25s超时 继续检测-----这里localsocket好像不需要返回
        qDebug() << "timeout";
        netCheckLoginStatus(clientKey, jsObj);
        break;

    case 400:
		//localSocket 二维码失效
		jsObj.insert("LoginStatus", 400);
		sendMsg(clientKey, NETTYPE_CHECKSTATUS, jsObj);
        break;
		
    default:
        break;
    }
}


void Controller::onNewLoginPageFinished(int clientKey, const QByteArray &data, QMap<QString, QVariant> &map)
{
    qDebug() << "onNewLoginPageFinished";

    QDomDocument doc;
    doc.setContent(data);

    QDomElement docElem = doc.documentElement();
    QDomNode node = docElem.firstChild();
    while (!node.isNull()) {
        QDomElement element = node.toElement();
        qDebug() << "Name : " << element.tagName() << "  text: " << element.text();

        QString tagName = element.tagName();
        if (tagName.compare("skey") == 0) {
            m_UserData.m_Skey = element.text();
        } else if (tagName.compare("wxsid") == 0) {
            m_UserData.m_Sid = element.text();
        } else if (tagName.compare("wxuin") == 0) {
            m_UserData.m_Uin = element.text();
        } else if (tagName.compare("pass_ticket") == 0) {
            m_UserData.m_PassTicket = element.text();
        }

        node = node.nextSibling();
    }

    m_UserData.m_AuthTicket = map.value("webwx_auth_ticket").toString();
    m_UserData.m_DataTicket = map.value("webwx_data_ticket").toString();

    //初始化完成
	QJsonObject obj, cookies;
	obj.insert("LoginStatus", m_UserData.m_Status);
	obj.insert("Host", m_UserData.m_Host);

	cookies.insert("wxsid", m_UserData.m_Sid);
	cookies.insert("wxuin", m_UserData.m_Uin.toInt());
	cookies.insert("skey", m_UserData.m_Skey);
	cookies.insert("webwx_data_ticket", m_UserData.m_DataTicket);
	cookies.insert("webwx_auth_ticket", m_UserData.m_AuthTicket);
	cookies.insert("webwx_pass_ticket", m_UserData.m_PassTicket);
	
	obj.insert("Cookies", cookies);

	sendMsg(clientKey, NETTYPE_CHECKSTATUS, obj);
}

void Controller::onWebWxinitFinished(int clientKey, const QByteArray &data)
{
	qDebug() << "onWebWxinitFinished";

	//localSocket 返回最近联系人信息
	QJsonParseError errStr;
	QJsonDocument jsDoc = QJsonDocument::fromJson(data, &errStr);
	QJsonObject jsonData = jsDoc.object();

	QJsonObject syncString = jsonData.value("SyncKey").toObject();
	m_UserData.setData(syncString);
}

/**
 * @brief Controller::onRecvMsg
 * @param str
 */
void Controller::onRecvMsg(const QString &str)
{
    QJsonDocument jdoc = QJsonDocument::fromJson(str.toLocal8Bit());
    if (jdoc.isNull()) {
        qDebug() << str <<" is not json";
        return ;
    }

    QJsonObject jobj = jdoc.object();
    int clientKey = jobj.value("ClientKey").toVariant().toInt(0);
    QString type = jobj.value("WxType").toString("");
    QJsonObject jData = jobj.value("Data").toObject();

    processMsg(clientKey, type, jData);
}
