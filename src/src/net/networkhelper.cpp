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
        emit checkLoginStatusFinished(data);
        break;

    case RequestNewLoginPage:
        {
            qDebug() << "reply->rawHeader: " << reply->rawHeader("Set-Cookie");
            //getcookie uin dataticket
            QMap<QString, QVariant> map;
            setCookie(reply->rawHeader("Set-Cookie"), map);
            emit newLoginPageFinished(data, map);
            break;
        }
    }
}

void NetworkHelper::checkLoginStatus(const UserDataItem &item)
{
    QString url = m_URLCheckLoginStatus + item.m_UUid;

    QNetworkRequest req(QUrl(url.toUtf8().data()));

    QNetworkAccessManager *access = new QNetworkAccessManager;
    QNetworkReply *reply = access->get(req);
    reply->setProperty("type", RequestLoginStatus);
    connect(reply, SIGNAL(finished()), this, SLOT(onLoadHttpPostFinished()));
}

void NetworkHelper::newLoginPage(const UserDataItem &item)
{
    QString url = item.m_RedirectUrl + "&fun=new&version=v2";

    QNetworkRequest req(QUrl(url.toUtf8().data()));
    QNetworkAccessManager *access = new QNetworkAccessManager;
    QNetworkReply *reply = access->get(req);

    reply->setProperty("type", RequestNewLoginPage);
    connect(reply, SIGNAL(finished()), this, SLOT(onLoadHttpPostFinished()));
}

NetworkHelper::NetworkHelper(QObject *parent)
    : QObject(parent)
{

}
