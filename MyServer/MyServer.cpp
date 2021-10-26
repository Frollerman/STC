#include <QTime>
#include <QCoreApplication>
#include <QTcpServer>
#include <QTcpSocket>
#include <QProcess>
#include <QTextCodec>
#include <QFileInfo>
#include "MyServer.h"

MyServer::MyServer(int nPort, QObject* pobj /*=nullptr*/) : QObject(pobj)
                                                          , m_nNextBlockSize(0)
{
    m_ptcpServer = new QTcpServer(this);
    if(!m_ptcpServer->listen(QHostAddress::Any, nPort))
    {
        qDebug() << "Server Error: Unable to start the server";
        m_ptcpServer->close();
        return;
    }
    connect(m_ptcpServer, SIGNAL(newConnection()),
            this, SLOT(slotNewConnection())
            );
    qDebug() << "Server successfully started" << Qt::endl;
}

/*virtual*/ void MyServer::slotNewConnection()
{
    QTcpSocket* pClientSocket = m_ptcpServer->nextPendingConnection();
    connect(pClientSocket, SIGNAL(disconnected()),
            pClientSocket, SLOT(deleteLater())
            );
    connect(pClientSocket, SIGNAL(readyRead()),
            this, SLOT(slotReadClient())
            );

    sendToClient(pClientSocket, "Server Response: Connected!");
}

void MyServer::slotReadClient()
{
    QTcpSocket* pClientSocket = (QTcpSocket*)sender();
    QDataStream in(pClientSocket);
    in.setVersion(QDataStream::Qt_5_15);
    for(;;)
    {
        if(!m_nNextBlockSize)
        {
            if(pClientSocket->bytesAvailable() < sizeof(quint16))
            {
                break;
            }
            in >> m_nNextBlockSize;
        }

        if(pClientSocket->bytesAvailable() < m_nNextBlockSize)
        {
            break;
        }
        QTime time;
        QString str;
        in >> time >> str;

        QString strMessage =
                time.toString() + " " + "Client has sent - " + str;
        qDebug() << strMessage;

        m_nNextBlockSize = 0;

        sendToClient(pClientSocket,
                     "Server Response: Received \"" + str + "\""
                     );

        if(str == "command_ipconfig")
        {
            ipConfig(pClientSocket);
        }
        else
        {
            QString dir = QCoreApplication::applicationDirPath();
            unsigned int file_count = 0;

            find(QDir(dir), file_count, str);
            QString result = QString::number(file_count) + " file(s) has (have) been found in " + dir;
            sendToClient(pClientSocket, result);
        }
    }
}

void MyServer::sendToClient(QTcpSocket *pSocket, const QString &str)
{
    QByteArray arrBlock;
    QDataStream out(&arrBlock, QIODevice::WriteOnly);
    out.setVersion(QDataStream::Qt_5_15);
    out << quint16(0) << QTime::currentTime() << str;

    out.device()->seek(0);
    out << quint16(arrBlock.size() - sizeof(quint16));

    pSocket->write(arrBlock);
}

void MyServer::ipConfig(QTcpSocket* pClientSocket)
{
    QByteArray output;

#ifdef Q_OS_WIN
    QProcess ipcfg;
    ipcfg.start("ipconfig");
    ipcfg.waitForFinished();
    output = ipcfg.readAllStandardOutput();
    qDebug() << "Finished IpConfig" << Qt::endl;
#else
    QProcess ifcfg;
    ifcfg.start("ifconfig");
    ifcfg.waitForFinished();
    output = ifcfg.readAllStandardOutput();
    qDebug() << "Finished IfConfig" << Qt::endl;
#endif

    QTextCodec *codec = QTextCodec::codecForName("IBM 866");
    QString res = codec->toUnicode(output);

    m_nNextBlockSize = 0;

    sendToClient(pClientSocket, res);
}

void MyServer::find(const QDir& dir, unsigned int& count, QString str)
{
    QCoreApplication::processEvents();

    QStringList mask;
    mask << str;
    QStringList listFiles = dir.entryList(mask, QDir::Files);

    count = listFiles.size();
}
