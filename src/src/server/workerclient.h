#ifndef WORKERSERVER_H
#define WORKERSERVER_H

#include <QObject>
#include <QLocalSocket>

class WorkerClient : public QObject
{
    Q_OBJECT
public:
    explicit WorkerClient(const QString &uin, QObject *parent = 0);
    ~WorkerClient();

    void writeMsg(const QByteArray &data);

private:
    int initServer(const QString &uin);
    void setupSignals();

signals:
    void recvMsg(const QString &str);

private slots:
    void onReadMsg();

private:
    QLocalSocket     *m_socket;
};

#endif // WORKERSERVER_H
