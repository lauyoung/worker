#include "workerclient.h"

#include <QDebug>
#include <QTextStream>
#include <QJsonObject>
#include <QJsonDocument>

const QString SERVERNAME = "/tmp/master";


WorkerClient::WorkerClient(const QString &uin, QObject *parent)
    : QObject(parent)
{
    int ret = initServer(uin);
    if (-1 == ret) {
        exit(-1);
    }

    setupSignals();
}

WorkerClient::~WorkerClient()
{
    if (m_socket) {
        m_socket->deleteLater();
    }
}

/**
 * @brief WorkerClient::initServer
 * @return 0:success, -1:failure
 */
int WorkerClient::initServer(const QString &uin)
{
    m_socket = new QLocalSocket(this);
    m_socket->connectToServer(SERVERNAME, QIODevice::ReadWrite);

	QJsonObject jsObj;
	jsObj.insert("WxType", "workinit");
	jsObj.insert("Uin", uin);

	QJsonDocument jsDoc(jsObj);
	QByteArray data = jsDoc.toJson(QJsonDocument::Compact);

    if (m_socket->waitForConnected(5000)) {
        qDebug() << "connected";
		writeMsg(data);
        return 0;
    }
    return -1;
}

/**
 * @brief WorkerClient::setupSignals
 * connect signals with slots
 */
void WorkerClient::setupSignals()
{
    connect(m_socket, SIGNAL(readyRead()), this, SLOT(onReadMsg()));
}

/**
 * @brief WorkerClient::writeMsg
 * @param str
 * send msg to server
 */
void WorkerClient::writeMsg(const QByteArray &data)
{
    QTextStream dataStream(m_socket);
    dataStream << data;
    m_socket->flush();
    m_socket->waitForBytesWritten(5000);
    qDebug() << "write msg to server: " << data;
}

/**
 * @brief WorkerClient::onReadMsg
 * read msg from server
 */
void WorkerClient::onReadMsg()
{
    QTextStream ts(m_socket);
    QString str = ts.readAll();
    qDebug() << "receive from server : " << str;

    emit recvMsg(str);
}

