#ifndef NETWORKHELPER_H
#define NETWORKHELPER_H

#include <QObject>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>

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
    };

    static NetworkHelper *instance(QObject *parent = 0);

    void getUUid(int clientKey);
    void checkLoginStatus(const UserDataItem &item);
    void newLoginPage(const UserDataItem &item);



    bool setCookie(const QByteArray &data, QMap<QString, QVariant> &map);

private:
    NetworkHelper(QObject *parent = 0);
    //~NetworkHelper();

signals:
    void getUUidFinished(int clientKey, const QByteArray &data);
    void checkLoginStatusFinished(const QByteArray &data);
    void newLoginPageFinished(const QByteArray &data, QMap<QString, QVariant> &map);

public slots:
    void onLoadHttpPostFinished();

private:

};

#endif // NETWORKHELPER_H
