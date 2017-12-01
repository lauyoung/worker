#ifndef NETWORKHELPER_H
#define NETWORKHELPER_H

#include <QObject>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QJsonObject>
#include <QJsonDocument>
#include <QByteArray>

#include "userdataitem.h"
#include "utils.h"

class NetworkHelper : public QObject
{
    Q_OBJECT

public:
    enum RequestType{
        RequestQRCode,
        RequestLoginStatus,
        RequestNewLoginPage,
		RequestWebWxinit,
    };

    static NetworkHelper *instance(QObject *parent = 0);

    void getUUid(int clientKey);
    void checkLoginStatus(int clientKey, const UserDataItem &item);
    void newLoginPage(int clientKey, const UserDataItem &item);
	void webWxInit(int clientKey, const UserDataItem &item);

    bool setCookie(const QByteArray &data, QMap<QString, QVariant> &map);

private:
    NetworkHelper(QObject *parent = 0);
    //~NetworkHelper();

signals:
    void getUUidFinished(int clientKey, const QByteArray &data);
    void checkLoginStatusFinished(int clientKey, const QByteArray &data);
    void newLoginPageFinished(int clientKey, const QByteArray &data, QMap<QString, QVariant> &map);
	void webWxinitFinished(int clientKey, const QByteArray &data);

public slots:
    void onLoadHttpPostFinished();

private:

};

#endif // NETWORKHELPER_H
