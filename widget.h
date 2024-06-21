#ifndef WIDGET_H
#define WIDGET_H

#include <QWidget>
#include <QStandardItemModel>
#include <QTableView>
#include <QDir>
#include <QStandardPaths>
// #include <QDebug>
#include <sqlcipher/sqlite3.h>
#include <QMenu>
#include <QAction>
#include <QContextMenuEvent>
#include <QTimer>
class Worker;
QT_BEGIN_NAMESPACE
namespace Ui {
class Widget;
}
QT_END_NAMESPACE

class Widget : public QWidget
{
    Q_OBJECT

public:
    Widget(QString passwordinput, QWidget *parent = nullptr);
    ~Widget();

public slots:
    void showAndActivate();
private slots:
    void onItemChanged(QStandardItem *item);
    void on_searchbtn_clicked();

    void on_savebtn_clicked();
    void onDeleteRow();

    void  onCustomContextMenuRequested(const QPoint &pos);
    void adjustColumnWidths();
private:
    QString dbpath;
    QString dirpath;
    Ui::Widget *ui;
    void checkError(int result, QString errorMsg, QString successMsg);
    void setStatusBar(QString status, QColor color);
    QStandardItemModel* model;
    sqlite3* db;
    void fetchData();
    void fetchSpecify(QString query);
    void setupContextMenu();

protected:
    void contextMenuEvent(QContextMenuEvent *event) override;
    void resizeEvent(QResizeEvent *event) override ;
};
#endif // WIDGET_H
