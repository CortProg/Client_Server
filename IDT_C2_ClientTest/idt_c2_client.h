#ifndef IDT_C2_CLIENT_H
#define IDT_C2_CLIENT_H

#include <QWidget>
#include <QAbstractSocket>
#include <QDebug>
#include <QFile>
#include <QFileDialog>
#include <QHostAddress>
#include <QMessageBox>
#include <QMetaType>
#include <QString>
#include <QStandardPaths>
#include <QTcpSocket>

QT_BEGIN_NAMESPACE
namespace Ui { class IDT_C2_Client; }
QT_END_NAMESPACE

class IDT_C2_Client : public QWidget
{
    Q_OBJECT

public:
    IDT_C2_Client(QWidget *parent = nullptr);
    ~IDT_C2_Client();
    quint16 port;
    QHostAddress address;

signals:
   void si_Messages(QString);

private slots:
     void sl_DisplayMessage(const QString& str);
     void sl_ReadDataFromSocket();
     void sl_DiscardSocket();
     void sl_DisplayError(QAbstractSocket::SocketError socketError);

private:
    Ui::IDT_C2_Client *ui;
    QTcpSocket* c_Socket;
};
#endif // IDT_C2_CLIENT_H
