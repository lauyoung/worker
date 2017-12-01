#include "controller.h"
#include "networkhelper.h"
#include "utils.h"

#include <QDomDocument>
#include <QJsonDocument>
#include <QJsonObject>

Controller::Controller(const QString &uin, QObject *parent)
	:QObject(parent)
{
	m_UserData.m_Uin = uin;
	m_UserData.m_Status = 0;

    m_workerClient = new WorkerClient(uin, this);
    setupSignals();

	//初始化map
//	netGetUUid();
}

Controller::~Controller()
{

}

void Controller::netGetUUid(int clientKey, const QJsonObject &obj)
{
    NetworkHelper *inst = NetworkHelper::instance();
    inst->getUUid(clientKey);
}

void Controller::netCheckLoginStatus()
{
    if (m_UserData.m_Status == 200) {
        //LocalSocket 返回用户已登录
        return ;
    }
    NetworkHelper *inst =NetworkHelper::instance();
    inst->checkLoginStatus(m_UserData);
}

void Controller::netNewLoginPage()
{
    NetworkHelper *inst =NetworkHelper::instance();
    inst->newLoginPage(m_UserData);
}

void Controller::initReqFunc()
{
//    reqFunc getUid = &Controller::netGetUUid;
//    m_typeFuncMap["login"] = getUid;
}

/**
 * @brief Controller::processMsg
 * @param clientKey
 * @param type
 * @param obj
 */
void Controller::processMsg(int clientKey, const QString &type, const QJsonObject &obj)
{
//    QMap<QString, reqFunc>::const_iterator it = m_typeFuncMap.find(type);
//    if (m_typeFuncMap.cend() != it) {
//        const reqFunc func = it.value();
//        (*func)(clientKey, obj);
//    }
    if (type == "login") {
        netGetUUid(clientKey, obj);
    }
}

void Controller::sendMsg(const QJsonObject &obj)
{
    QJsonDocument jdoc(obj);
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

    connect(inst, SIGNAL(checkLoginStatusFinished(const QByteArray &)),
            this, SLOT(onCheckLoginStatusFinished(const QByteArray &)));

    connect(inst, SIGNAL(newLoginPageFinished(const QByteArray &, QMap<QString, QVariant> &)),
        this, SLOT(onNewLoginPageFinished(const QByteArray &, QMap<QString, QVariant> &)));

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
    QJsonObject content, obj;
    content.insert("qrcodeurl", retUrl);
    obj.insert("ClientKey", clientKey);
    obj.insert("WxType", "login");
    obj.insert("Uin", m_UserData.m_Uin);
    obj.insert("Data", content);

    sendMsg(obj);

    //开启监测登录状态
    netCheckLoginStatus();
}

void Controller::onCheckLoginStatusFinished(const QByteArray &data)
{
    QString result, redirectUrl, host;
    Utils::getMatchStr(data, "window.code=[0-9]{3}", result);
    int code = result.mid(result.size()-3).toInt();

    switch (code)
    {
    case 201:
        //localSocket不需要返回 返回扫码成功 继续检测
        qDebug() << "need confirm";

        netCheckLoginStatus();
        break;
    case 200:
        //localSocket 登录成功
        qDebug() << "login success";

        Utils::getValueByKey(data, "window.redirect_uri", redirectUrl);
        Utils::getMatchStr(redirectUrl, "[a-z0-9.]*.com", host);

        m_UserData.m_Status = 200;
        m_UserData.m_Host = host;
        m_UserData.m_RedirectUrl = redirectUrl;

        netNewLoginPage();
        break;
    case 408:
        //localSocket 25s超时 继续检测-----这里localsocket好像不需要返回
        qDebug() << "timeout";

        netCheckLoginStatus();

        break;
    case 400:
        //localSocket 二维码失效---是否需要返回新的uuid
        qDebug() << "QR code expired";
        break;
    default:
        break;
    }
}


void Controller::onNewLoginPageFinished(const QByteArray &data, QMap<QString, QVariant> &map)
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
