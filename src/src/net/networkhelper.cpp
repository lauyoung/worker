#include "networkhelper.h"

static NetworkHelper *inst = NULL;

static QString m_URLGetUUid = "https://login.weixin.qq.com/jslogin?appid=wx782c26e4c19acffb";
static QString m_URLCheckLoginStatus = "https://login.wx.qq.com/cgi-bin/mmwebwx-bin/login?loginicon=true&uuid=";

NetworkHelper *NetworkHelper::instance(QObject *parent)
{
    if (inst == NULL) {
        inst = new NetworkHelper(parent);
    }

    return inst;
}

bool NetworkHelper::setCookie(const QByteArray &data, QMap<QString, QVariant> &map)
{
    QString webwx_data_ticket, webwx_auth_ticket;

    Utils::getValueByKey(data, "webwx_data_ticket", webwx_data_ticket);
    Utils::getValueByKey(data, "webwx_auth_ticket", webwx_auth_ticket);

    map.insert("webwx_data_ticket", webwx_data_ticket);
    map.insert("webwx_auth_ticket", webwx_auth_ticket);

    return true;
}
void NetworkHelper::getUUid(int clientKey)
{
    QNetworkRequest req(QUrl(m_URLGetUUid.toUtf8().data()));

    QNetworkAccessManager *access = new QNetworkAccessManager;
    QNetworkReply *reply = access->get(req);

    reply->setProperty("clientKey", clientKey);
    reply->setProperty("type", RequestQRCode);
    reply->setProperty("wxType", "login");
    connect(reply, SIGNAL(finished()), this, SLOT(onLoadHttpPostFinished()));
}

void NetworkHelper::onLoadHttpPostFinished()
{
    QNetworkReply *reply = dynamic_cast<QNetworkReply *>(sender());
    if (reply == NULL) {
        return ;
    }

    QNetworkAccessManager *access = reply->manager();
    reply->deleteLater();
    access->deleteLater();

    QByteArray data;
    if (reply->error() == QNetworkReply::NoError) {
        data = reply->readAll();
    }

    QString uin = reply->property("uin").toString();
    int type = reply->property("type").toInt();
    int clientKey = reply->property("clientKey").toInt();

    switch (type) {
    case RequestQRCode:
        emit getUUidFinished(clientKey, data);
        break;

    case RequestLoginStatus:
        emit checkLoginStatusFinished(clientKey, data);
        break;

    case RequestNewLoginPage:
        {
            qDebug() << "reply->rawHeader: " << reply->rawHeader("Set-Cookie");
            //getcookie uin dataticket
            QMap<QString, QVariant> map;
            setCookie(reply->rawHeader("Set-Cookie"), map);
            emit newLoginPageFinished(clientKey, data, map);
            break;
        }

	case RequestWebWxinit:
		emit webWxinitFinished(clientKey, data);
		break;
    }
}

void NetworkHelper::checkLoginStatus(int clientKey, const UserDataItem &item)
{
    QString url = m_URLCheckLoginStatus + item.m_UUid;

    QNetworkRequest req(QUrl(url.toUtf8().data()));

    QNetworkAccessManager *access = new QNetworkAccessManager;
    QNetworkReply *reply = access->get(req);
    reply->setProperty("type", RequestLoginStatus);
	reply->setProperty("clientKey", clientKey);
    connect(reply, SIGNAL(finished()), this, SLOT(onLoadHttpPostFinished()));
}

void NetworkHelper::newLoginPage(int clientKey, const UserDataItem &item)
{
    QString url = item.m_RedirectUrl + "&fun=new&version=v2";

    QNetworkRequest req(QUrl(url.toUtf8().data()));
    QNetworkAccessManager *access = new QNetworkAccessManager;
    QNetworkReply *reply = access->get(req);

    reply->setProperty("type", RequestNewLoginPage);
	reply->setProperty("clientKey", clientKey);
    connect(reply, SIGNAL(finished()), this, SLOT(onLoadHttpPostFinished()));
}

void NetworkHelper::webWxInit(int clientKey, const UserDataItem &item)
{
	qDebug() << "webWxinit";

	QString passTicket = item.m_PassTicket;
	QString sid		  = item.m_Sid;
	QString skey       = item.m_Skey;
	QString wxuin		  = item.m_Uin;
	QString host	      = item.m_Host;
	QString dataTicket  = item.m_DataTicket;
	QString authTicket  = item.m_AuthTicket;
	QString deviceID    = Utils::getRandomNumber();

	QString url = QString("https://") + host + 
		QString("/cgi-bin/mmwebwx-bin/webwxinit?") +
		QString("pass_ticket=") + passTicket;

	QJsonObject baseValue, baseRequest;

	baseValue.insert("Uin", wxuin);
	baseValue.insert("Sid", sid);
	baseValue.insert("Skey", skey);
	baseValue.insert("DeviceID", deviceID);

	baseRequest.insert("BaseRequest", baseValue);

	QJsonDocument jsDoc(baseRequest);
	QByteArray    data;
	QString      jStr;

	jStr = jsDoc.toJson(QJsonDocument::Compact);
	data.append(jStr);

	QString cookie = "wxuin="	+ wxuin + ";" + 
		"wxsid=" + sid + ";" +
		"webwx_data_ticket=" + dataTicket + ";" +
		"webwx_auth_ticket=" + authTicket + ";";

	QNetworkRequest req(QUrl(url.toUtf8().data()));
	req.setRawHeader("Cookie", cookie.toUtf8());

	QNetworkAccessManager *access = new QNetworkAccessManager;
	QNetworkReply *reply = access->post(req, data);
	reply->setProperty("type", RequestWebWxinit);
	reply->setProperty("clientKey", clientKey);

	connect(reply, SIGNAL(finished()), this, SLOT(onLoadHttpPostFinished()));
}

NetworkHelper::NetworkHelper(QObject *parent)
    : QObject(parent)
{

}
