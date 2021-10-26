#ifndef MYSERVER_H
#define MYSERVER_H

#include <QObject>
#include <QDir>

class QTcpServer;
class QTextEdit;
class QTcpSocket;

// ===========================================
class MyServer : public QObject
{
    Q_OBJECT
private:
    QTcpServer* m_ptcpServer;
    quint16 m_nNextBlockSize;

private:
    void sendToClient(QTcpSocket* pSocket, const QString& str);

public:
    MyServer(int nPort, QObject* pobj = nullptr);

    void find(const QDir& dir, unsigned int& count, QString str);

public slots:
    virtual void slotNewConnection();
    void slotReadClient();
    void ipConfig(QTcpSocket* pClientSocket);
};

#endif // MYSERVER_H
