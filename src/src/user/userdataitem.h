#ifndef USERDATAITEM_H
#define USERDATAITEM_H

#include <QObject>
#include <QString>
#include <QJsonObject>

struct UserDataItem
{
    QString m_Uin;
    QString m_UUid;
    QString m_Sid;
    QString m_Skey;
    QString m_PassTicket;
    QString m_DataTicket;
    QString m_AuthTicket;
    QString m_SyncCheckKey;
    QString m_SyncKey;
    QString m_Host;
	QString m_RedirectUrl;
    int     m_Status;

    void setData(const QJsonObject &obj);
};

#endif // USERDATAITEM_H
