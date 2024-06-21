#include "widget.h"
#include <QApplication>
#include <QInputDialog>
#include <QSharedMemory>
#include <QMessageBox>
int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    QSharedMemory sharedMemory("poneglyphs", &a);
    if(!sharedMemory.attach()){
        if(sharedMemory.create(1)){
            QObject::connect(&a, &QApplication::aboutToQuit, &a, [&sharedMemory]() {
                sharedMemory.detach();
            });
            bool ok;
            QString passwordinput = QInputDialog::getText(nullptr, "auth",
                                                          "Enter your password:", QLineEdit::Password,
                                                          "", &ok);
            if (ok && !passwordinput.isEmpty()) {
                Widget w(passwordinput);
                w.show();
                return a.exec();
            } else {
                a.quit();
            }
        }else {
            QMessageBox::critical(nullptr, "Error", "cann't create a single instance");
            a.quit();
            return 1;
        }
    }else {
        sharedMemory.detach();
        QMessageBox::critical(nullptr, "warning", "there is one dereshi player is running");
        a.quit();
        return 0;
    }



}


