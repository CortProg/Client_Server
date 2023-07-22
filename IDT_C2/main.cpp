#include "idt_c2.h"

#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    IDT_C2 w;
    w.show();
    return a.exec();
}
