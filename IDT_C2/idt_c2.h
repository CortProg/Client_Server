#ifndef IDT_C2_H
#define IDT_C2_H

#include <QWidget>
#include <QDebug>
#include <QFile>
#include <QFileDialog>
#include <QMessageBox>
#include <QMetaType>
#include <QSet>
#include <QStandardPaths>
#include <QTcpServer>
#include <QTcpSocket>
#include <QDateTime>
#include <QSaveFile>
#include <QFile>


QT_BEGIN_NAMESPACE
namespace Ui { class IDT_C2; }
QT_END_NAMESPACE

class IDT_C2 : public QWidget
{
    Q_OBJECT

public:
    IDT_C2(QWidget *parent = nullptr);
    ~IDT_C2();
    qint16 port;

private:
    Ui::IDT_C2 *ui;
    QTcpServer* c_server;
    QSet<QTcpSocket *> connection_set;

signals:
    void si_Messages(QString);
private slots:
    void sl_NewConnection(void);
    void sl_DisplayMessage(const QString& str);
    void sl_AppendToSocketList(QTcpSocket * socket);
    void sl_ReadDataFromSocket();
    void sl_DiscardSocket();
    void sl_DisplayError(QAbstractSocket::SocketError socketError);
    void sl_SendScreenShotToClient(void);
    void sl_PackAndSend(QTcpSocket* socket, QString filePath);

};
#endif // IDT_C2_H
