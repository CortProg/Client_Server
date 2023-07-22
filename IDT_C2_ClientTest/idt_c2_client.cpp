#include "idt_c2_client.h"
#include "./ui_idt_c2_client.h"

IDT_C2_Client::IDT_C2_Client(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::IDT_C2_Client)
{
    ui->setupUi(this);
    c_Socket = new QTcpSocket(this);


    connect(this, &IDT_C2_Client::si_Messages, this, &IDT_C2_Client::sl_DisplayMessage);
    connect(c_Socket, &QTcpSocket::readyRead, this, &IDT_C2_Client::sl_ReadDataFromSocket);
    connect(c_Socket, &QTcpSocket::disconnected, this, &IDT_C2_Client::sl_DiscardSocket);
    connect(c_Socket, &QAbstractSocket::errorOccurred, this, &IDT_C2_Client::sl_DisplayError);

   // c_Socket->setSocketDescriptor(3131,);

    c_Socket->connectToHost(QHostAddress::LocalHost,1234);
    sl_DisplayMessage("Waiting  for Server");


    if(c_Socket->waitForConnected())
    {
         port = c_Socket->localPort();
       address=  c_Socket->localAddress();
         sl_DisplayMessage("Connected to Server");

    }
    else
    {
        QMessageBox::critical(this,"QTCPClient", QString("The following error occurred: %1.").arg(c_Socket->errorString()));
        exit(EXIT_FAILURE);
    }

}

IDT_C2_Client::~IDT_C2_Client()
{
    if(c_Socket->isOpen())
        c_Socket->close();
    delete ui;
}

void IDT_C2_Client::sl_DisplayMessage(const QString &str)
{
    ui->TBx_Messages->append(str);
}

void IDT_C2_Client::sl_ReadDataFromSocket()
{
    QByteArray buffer;

    QDataStream socketStream(c_Socket);
    socketStream.setFloatingPointPrecision(QDataStream::SinglePrecision);
    socketStream.setVersion(QDataStream::Qt_6_5);

    socketStream.startTransaction();
    socketStream >> buffer;

    if(!socketStream.commitTransaction())
    {
        QString message = QString("%1 :: Waiting for more data to come..").arg(c_Socket->socketDescriptor());
        emit si_Messages(message);
        return;
    }

    QString Header_start = buffer.sliced(0,3);
    QString Header_type = buffer.sliced(3,10);
    QString Header_FileName = buffer.sliced(13,60);
    Header_FileName = Header_FileName.split(" ")[0];
    QString Header_Time = buffer.sliced(73,19);

    QString header = buffer.mid(0,92);

    //QByteArray datasize = buffer.sliced(92,4).toHex();
    int sizela = buffer.sliced(92,4).toHex().toInt(nullptr,16) - 12;

    int alt_i ;
    alt_i = buffer.sliced(96,4).toHex().toInt(nullptr,16);
    //float* out = reinterpret_cast<float*>(&alt_i);
   // float alt_f = *out;
    float alt_f = *reinterpret_cast<float*>(&alt_i);


    int lat_i ;
    lat_i = buffer.sliced(100,4).toHex().toInt(nullptr,16);
    float* out2 = reinterpret_cast<float*>(&lat_i);
    float lat_f = *out2;

    int long_i ;
    long_i = buffer.sliced(104,4).toHex().toInt(nullptr,16);
    float* out3 = reinterpret_cast<float*>(&long_i);
    float long_f = *out3;


    //int sizela = datasize.toInt(nullptr,16);


    buffer = buffer.mid(108);

    if(Header_start=="IDT"){
      //  QString fileName = Header_FileName.split(" ")[0];
       // QString ext = Header_FileName.split(".")[1].split(" ")[0];
       // QString size = header.split(",")[2].split(":")[1].split(";")[0];

        if (QMessageBox::Yes == QMessageBox::question(this, "QTCPServer",
                                                      QString("You are receiving an attachment from sd:%1 of size: %2 bytes, called %3. Do you want to accept it?")
                                                          .arg(c_Socket->socketDescriptor()).arg(sizela).arg(Header_FileName)))
        {
            QString filePath = QFileDialog::getSaveFileName(this, tr("Save File"), QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation)+"/"+Header_FileName);

            QFile file(filePath);
            if(file.open(QIODevice::WriteOnly)){
                file.write(buffer);
                QString message = QString("INFO :: Attachment from sd:%1 successfully stored on disk under the path %2").arg(c_Socket->socketDescriptor()).arg(QString(filePath));
                emit si_Messages(message);
            }else
                QMessageBox::critical(this,"QTCPServer", "An error occurred while trying to write the attachment.");
        }else{
            QString message = QString("INFO :: Attachment from sd:%1 discarded").arg(c_Socket->socketDescriptor());
            emit si_Messages(message);
        }
    }

}

void IDT_C2_Client::sl_DiscardSocket()
{
    c_Socket->deleteLater();
    c_Socket=nullptr;

    ui->TBx_Messages->append("Disconnected!");
}

void IDT_C2_Client::sl_DisplayError(QAbstractSocket::SocketError socketError)
{
    switch (socketError) {
        case QAbstractSocket::RemoteHostClosedError:
        break;
        case QAbstractSocket::HostNotFoundError:
            QMessageBox::information(this, "QTCPClient", "The host was not found. Please check the host name and port settings.");
        break;
        case QAbstractSocket::ConnectionRefusedError:
            QMessageBox::information(this, "QTCPClient", "The connection was refused by the peer. Make sure QTCPServer is running, and check that the host name and port settings are correct.");
        break;
        default:
            QMessageBox::information(this, "QTCPClient", QString("The following error occurred: %1.").arg(c_Socket->errorString()));
        break;
    }
}

