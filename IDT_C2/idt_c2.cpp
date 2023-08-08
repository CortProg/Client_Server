#include "idt_c2.h"
#include "./ui_idt_c2.h"

IDT_C2::IDT_C2(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::IDT_C2)
{
    ui->setupUi(this);
    connect(ui->B_SendScreenShot,&QPushButton::clicked,this,&IDT_C2::sl_SendScreenShotToClient);

    c_server = new QTcpServer();
   // QHostAddress Adress;
    //quint16 port;


    if(c_server->listen(QHostAddress::Any,1234))
    {
        connect(this,&IDT_C2::si_Messages,this,&IDT_C2::sl_DisplayMessage);
        connect(c_server,&QTcpServer::newConnection,this,&IDT_C2::sl_NewConnection); // sender,signal,receiver,slot
        ui->TBx_Messages->append("Waiting For Client Connection. Any IP, 1234 Port is Listening");
       // Adress=c_server->serverAddress();
        //port = c_server->serverPort();
    }
    else
    {
         QMessageBox::critical(this,"QTCPServer",QString("Unable to start the server: %1.").arg(c_server->errorString()));
         exit(EXIT_FAILURE);
    }

}

IDT_C2::~IDT_C2()
{
    foreach (QTcpSocket* socket, connection_set)
    {
        socket->close();
        socket->deleteLater();
    }

    c_server->close();
    c_server->deleteLater();

    delete ui;
}

void IDT_C2::sl_NewConnection()
{
    while(c_server->hasPendingConnections())
    {
        sl_AppendToSocketList(c_server->nextPendingConnection());
    }

}

void IDT_C2::sl_AppendToSocketList(QTcpSocket *socket)
{
    connection_set.insert(socket);
    connect(socket, &QTcpSocket::readyRead,this,&IDT_C2::sl_ReadDataFromSocket);
    connect(socket, &QTcpSocket::disconnected, this, &IDT_C2::sl_DiscardSocket);
    connect(socket, &QAbstractSocket::errorOccurred, this, &IDT_C2::sl_DisplayError);
    ui->CB_Receivers->addItem(QString::number(socket->peerPort()));
    port = socket->peerPort();
    sl_DisplayMessage(QString(" Client with socked:%1 has just connected").arg(socket->peerPort()));
}

void IDT_C2::sl_ReadDataFromSocket()
{

}

void IDT_C2::sl_DiscardSocket()
{
    QTcpSocket* socket = reinterpret_cast<QTcpSocket*>(sender());
    QSet<QTcpSocket*>::iterator it = connection_set.find(socket);
    if (it != connection_set.end()){
        sl_DisplayMessage(QString(" Client with socket:%1 has just disconnected").arg(socket->peerPort()));
        connection_set.remove(*it);
    }
 //   refreshComboBox(); LAter

    socket->deleteLater();
}

void IDT_C2::sl_DisplayError(QAbstractSocket::SocketError socketError)
{
    switch (socketError) {
        case QAbstractSocket::RemoteHostClosedError:
        break;
        case QAbstractSocket::HostNotFoundError:
            QMessageBox::information(this, "QTCPServer", "The host was not found. Please check the host name and port settings.");
        break;
        case QAbstractSocket::ConnectionRefusedError:
            QMessageBox::information(this, "QTCPServer", "The connection was refused by the peer. Make sure QTCPServer is running, and check that the host name and port settings are correct.");
        break;
        default:
            QTcpSocket* socket = qobject_cast<QTcpSocket*>(sender());
            QMessageBox::information(this, "QTCPServer", QString("The following error occurred: %1.").arg(socket->errorString()));
        break;
    }
}

void IDT_C2::sl_SendScreenShotToClient()
{
    QString receiver = ui->CB_Receivers->currentText();

    QString filePath = QFileDialog::getOpenFileName(this, ("Select a Secreenshot"), QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation), ("File (*.json *.txt *.png *.jpg *.jpeg)"));

    if(filePath.isEmpty()){
        QMessageBox::critical(this,"QTCPClient","You haven't selected any attachment!");
        return;
    }

    // Some actions have to be taken

    if(receiver=="Broadcast")
    {
        foreach (QTcpSocket* socket,connection_set)
        {
            sl_PackAndSend(socket, filePath);
        }
    }
    else
    {
        foreach (QTcpSocket* socket,connection_set)
        {
            if(socket->peerPort() == receiver.toLongLong())
            {
                sl_PackAndSend(socket, filePath);
                break;
            }
        }
    }
   // ui->lineEdit_message->clear();

}

void IDT_C2::sl_PackAndSend(QTcpSocket *socket, QString filePath)
{
    if(socket)
    {
        if(socket->isOpen())
        {
            QFile m_file(filePath);
            if(m_file.open(QIODevice::ReadOnly)){

                QFileInfo fileInfo(m_file.fileName());
                QString fileName(fileInfo.fileName());
                qsizetype filenamesize = fileName.size();
                QDateTime DateTime = QDateTime::currentDateTime();
                QString DateTimeString = DateTime.toString("dd.MM.yyyy_hh:mm:ss");


                QDataStream socketStream(socket);
                socketStream.setVersion(QDataStream::Qt_6_5);

                QByteArray header;
                header.append(QString("IDTscreenshot").toUtf8());
                header.resize(13,0x20);
                header.append(QString("%1").arg(fileName).toUtf8());
                header.resize(73,0x20);
                header.append((DateTimeString).toUtf8());
                qsizetype headersize = header.size();

                QByteArray DataSectionArray;
                QDataStream DataSectionStream(&DataSectionArray,QIODevice::ReadWrite);

                float altitude = 120.45;
                float latitude = 33.45;
                float longitude = 88.45;
                quint32 DataSectionSize = (uint32_t)(m_file.size()) + 12;

                DataSectionStream.setFloatingPointPrecision(QDataStream::SinglePrecision);
                DataSectionStream << DataSectionSize;
                DataSectionStream << altitude;
                DataSectionStream << latitude;
                DataSectionStream << longitude;

                DataSectionArray.append(m_file.readAll());
                DataSectionArray.append(QString("AEC").toUtf8());

               // QByteArray FileArray = m_file.readAll();
                //byteArray.prepend(header);
                //FileArray.append(QString("AEC").toUtf8());

                QSaveFile file("C:/Users/Poncikzade/Desktop/test.txt");
                file.open(QIODevice::WriteOnly);
                file.write(header);
                file.write(DataSectionArray);             
                file.commit();

                QFile file_read("C:/Users/Poncikzade/Desktop/test.txt");
                if (!file_read.open(QIODevice::ReadOnly | QIODevice::Text))
                    return;
                QByteArray readed2;
                while (!file_read.atEnd()) {
                    QByteArray readed = file_read.readLine();
                    readed2.append(readed);
                }

                socketStream<<header + DataSectionArray;

            }else
                QMessageBox::critical(this,"QTCPClient","Couldn't open the attachment!");
        }
        else
            QMessageBox::critical(this,"QTCPServer","Socket doesn't seem to be opened");
    }
    else
        QMessageBox::critical(this,"QTCPServer","Not connected");
}


void IDT_C2::sl_DisplayMessage(const QString &str)
{
    ui->TBx_Messages->append(str);
}



