#include "idt_c2_client.h"

#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    IDT_C2_Client w;
    w.show();
    return a.exec();
}
