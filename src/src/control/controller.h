#ifndef CONTROLLER_H
#define CONTROLLER_H

#include <QObject>

#include "userdataitem.h"
#include "workerclient.h"

class Controller;
typedef void (Controller::*reqFunc)(int, const QJsonObject&);

class Controller : public QObject
{
    Q_OBJECT

public:
	Controller(const QString &uin, QObject *parent = 0);
	~Controller();

private:
	void setupSignals();

    void netGetUUid(int clientKey, const QJsonObject &obj);
    void netCheckLoginStatus(int clientKey, const QJsonObject &obj);
    void netNewLoginPage(int clientKey);
	void netWebWxInit(int clientKey, const QJsonObject &obj);

    void initReqFuncMap();
    void processMsg(int clientKey, const QString &type, const QJsonObject &obj);
    void sendMsg(int clientKey, const QString &type, const QJsonObject &obj);

signals:

public slots:
    void onGetUUidFinished(int clientKey, const QByteArray &data);
    void onCheckLoginStatusFinished(int clientKey, const QByteArray &data);
    void onNewLoginPageFinished(int clientKey, const QByteArray &data, QMap<QString, QVariant> &map);
	void onWebWxinitFinished(int clientKey, const QByteArray &data);

private slots:
    void onRecvMsg(const QString &str);

private:
    WorkerClient      *m_workerClient;
    UserDataItem       m_UserData;

    QMap<QString, reqFunc> m_typeFuncMap;

};

#endif // CONTROLLER_H
