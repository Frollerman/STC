#ifndef MYSERVER_H
#define MYSERVER_H

#include <QObject>

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

public slots:
    virtual void slotNewConnection();
    void slotReadClient();
};

#endif // MYSERVER_H
