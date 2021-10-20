#include "MyClient.h"
#include <QtWidgets>
#include <QTextCodec>

MyClient::MyClient(const QString& strHost,
                   int nPort,
                   QWidget* pwgt /*=nullptr*/
                   ) : QWidget(pwgt)
                     , m_nNextBlockSize(0)
{
    m_pTcpSocket = new QTcpSocket(this);

    m_pTcpSocket->connectToHost(strHost, nPort);
    connect(m_pTcpSocket, SIGNAL(connected()), SLOT(slotConnected()));
    connect(m_pTcpSocket, SIGNAL(readyRead()), SLOT(slotReadyRead()));
    connect(m_pTcpSocket, SIGNAL(errorOccurred(QAbstractSocket::SocketError)),
            this, SLOT(slotError(QAbstractSocket::SocketError))
            );

    m_ptxtInfo = new QTextEdit;
    m_ptxtInput = new QLineEdit;

    m_ptxtInfo->setReadOnly(true);

    QPushButton* pcmd1 = new QPushButton("ipconfig/ifconfig");
    connect(pcmd1, SIGNAL(clicked()), SLOT(slotSendIpConfig()));

    QPushButton* pcmd2 = new QPushButton("&Send");
    connect(pcmd2, SIGNAL(clicked()), SLOT(slotSendToServer()));
    connect(m_ptxtInput, SIGNAL(returnPressed()),
            this, SLOT(slotSendToServer())
            );

    //Layout setup
    QVBoxLayout* pvbxLayout = new QVBoxLayout;
    QHBoxLayout* phbxLayout = new QHBoxLayout;
    pvbxLayout->addWidget(new QLabel("<H1>Client</H1>"));
    pvbxLayout->addWidget(m_ptxtInfo);
    pvbxLayout->addWidget(m_ptxtInput);
    phbxLayout->addWidget(pcmd1);
    phbxLayout->addWidget(pcmd2);
    pvbxLayout->addLayout(phbxLayout);
    setLayout(pvbxLayout);
}

void MyClient::slotReadyRead()
{
    QDataStream in(m_pTcpSocket);
    in.setVersion(QDataStream::Qt_5_15);
    for(;;)
    {
        if(!m_nNextBlockSize)
        {
            if(m_pTcpSocket->bytesAvailable() < sizeof(quint16))
            {
                break;
            }
            in >> m_nNextBlockSize;
        }

        if(m_pTcpSocket->bytesAvailable() < m_nNextBlockSize)
        {
            break;
        }
        QTime time;
        QString str;
        in >> time >> str;

        m_ptxtInfo->append(time.toString() + " " + str);
        m_nNextBlockSize = 0;
    }
}

void MyClient::slotError(QAbstractSocket::SocketError err)
{
    QString strError =
            "Error: " + (err == QAbstractSocket::HostNotFoundError ?
                         "The host was not found." :
                         err == QAbstractSocket::RemoteHostClosedError ?
                         "The remote host is closed." :
                         err == QAbstractSocket::ConnectionRefusedError ?
                         "The connection was refused." :
                         QString(m_pTcpSocket->errorString())
                         );
    m_ptxtInfo->append(strError);
}

void MyClient::slotSendToServer()
{
    QByteArray arrBlock;
    QDataStream out(&arrBlock, QIODevice::WriteOnly);
    out.setVersion(QDataStream::Qt_5_15);
    out << quint16(0) << QTime::currentTime() << m_ptxtInput->text();

    out.device()->seek(0);
    out << quint16(arrBlock.size() - sizeof(quint16));

    m_pTcpSocket->write(arrBlock);
    m_ptxtInput->setText("");
}

void MyClient::slotConnected()
{
    m_ptxtInfo->append("Received the connected() signal");
}

void MyClient::slotSendIpConfig()
{
    QByteArray arrBlock;
    QString ip = "ipconfig";
    QDataStream out(&arrBlock, QIODevice::WriteOnly);
    out.setVersion(QDataStream::Qt_5_15);
    out << quint16(0) << QTime::currentTime() << ip;

    out.device()->seek(0);
    out << quint16(arrBlock.size() - sizeof(quint16));

    m_pTcpSocket->write(arrBlock);
}
