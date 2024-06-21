#include "widget.h"
#include "./ui_widget.h"

void Widget::fetchData()
{
    model->removeRows(0,model->rowCount());
    const char* selectSQL = "SELECT col1, col2 FROM sec;";
    sqlite3_stmt* stmt;
    int result = sqlite3_prepare_v2(db, selectSQL, -1, &stmt, 0);
    checkError(result, "Cannot prepare select statement", "fetch all preparation success");
    while (sqlite3_step(stmt) == SQLITE_ROW) {
        QString col1 = QString::fromUtf8(reinterpret_cast<const unsigned char*>(sqlite3_column_text(stmt, 0)));
        QString col2 = QString::fromUtf8(reinterpret_cast<const unsigned char*>(sqlite3_column_text(stmt, 1)));
        QList<QStandardItem*> rowItems;
        QStandardItem* item =new QStandardItem(col1);
        item->setEditable(false);
        rowItems.append(item);
        rowItems.append(new QStandardItem(col2));
        model->appendRow(rowItems);
    }
    sqlite3_finalize(stmt);
    //setStatusBar(QString::number(model->rowCount()) + " items loaded");

}

Widget::Widget(QString passwordinput, QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::Widget)
    , db()
{
    ui->setupUi(this);
    dbpath = QStandardPaths::writableLocation(QStandardPaths::AppConfigLocation) + QDir::separator() + "poneglyphs.db";
    dirpath = QStandardPaths::writableLocation(QStandardPaths::AppConfigLocation);
    QDir dir;
    if(!dir.exists(dirpath)){
        dir.mkdir(dirpath);
    }

    int resultOpen;
    QByteArray path = dbpath.toLocal8Bit();
    const char* pathc = path.constData();
    resultOpen = sqlite3_open(pathc, &db);
    checkError(resultOpen, "Cannot open database", "open db success ~");

    QByteArray byteArray = passwordinput.toLocal8Bit();
    const char* key = byteArray.constData();

    int resultKey = sqlite3_key(db, key, strlen(key));
    //checkError(resultKey, "Cannot set encryption key", "password set success !!!");

    if(resultKey != SQLITE_OK){
        qApp->quit();
    }

    const char* createTableSQL = "CREATE TABLE IF NOT EXISTS sec (col1 TEXT PRIMARY KEY, col2 TEXT);";
    int resultCre = sqlite3_exec(db, createTableSQL, 0, 0, 0);
    checkError(resultCre, "Cannot create table", "create table (IF NOT EXISTS) success !!!");

    model =  new QStandardItemModel(0,2,this);
    model->setHeaderData(0, Qt::Horizontal, "col1");
    model->setHeaderData(1, Qt::Horizontal, "col2");
    ui->tableView->setModel(model);
    ui->tableView->verticalHeader()->setVisible(false);
    fetchData();
    connect(model, &QStandardItemModel::itemChanged, this, &Widget::onItemChanged);
    setupContextMenu();

    QHeaderView *header = ui->tableView->horizontalHeader();
    header->setSectionResizeMode(QHeaderView::Interactive);
    QTimer::singleShot(0, this, &Widget::adjustColumnWidths);
}

Widget::~Widget()
{
    sqlite3_close(db);
    delete ui;

    delete model;
}

void Widget::showAndActivate()
{
    this->show();
    this->activateWindow();
}

void Widget::onItemChanged(QStandardItem *item)
{
    QString col1 = model->item(item->row(),0)->text();

    QString updateSQL = "update sec set col2 = '" + item->text().trimmed() + "' where col1 = '" + col1 + "'";
    QByteArray byteArray = updateSQL.toLocal8Bit();
    const char* updateSQLc = byteArray.constData();
    int result = sqlite3_exec(db, updateSQLc, 0, 0, 0);
    checkError(result, "Cannot delete record", "update success !!!");


}

void Widget::checkError(int result, QString errorMsg, QString successMsg)
{
    if (result != SQLITE_OK) {
        sqlite3_close(db);
        setStatusBar("error: " + errorMsg, Qt::red);
    }else {
        setStatusBar("success: " + successMsg, Qt::green);
    }
}

void Widget::setStatusBar(QString status, QColor color)
{
    QPalette palette;
    palette.setColor(QPalette::WindowText, color);
    ui->statusbar->setPalette(palette);
    ui->statusbar->setText(status);
}

void Widget::fetchSpecify(QString query)
{
    model->removeRows(0,model->rowCount());
    QString selsql = "SELECT col1, col2 FROM sec where col1 like '%" + query + "%';";
    QByteArray byteArray  = selsql.toLocal8Bit();
    const char* selectSQL = byteArray.constData();
    sqlite3_stmt* stmt1;
    int result = sqlite3_prepare_v2(db, selectSQL, -1, &stmt1, 0);
    checkError(result, "Cannot prepare select statement", "select specific row preparation success");
    while (sqlite3_step(stmt1) == SQLITE_ROW) {
        QString col1 = QString::fromUtf8(reinterpret_cast<const unsigned char*>(sqlite3_column_text(stmt1, 0)));
        QString col2 = QString::fromUtf8(reinterpret_cast<const unsigned char*>(sqlite3_column_text(stmt1, 1)));
        QList<QStandardItem*> rowItems;
        QStandardItem* item =new QStandardItem(col1);
        item->setEditable(false);
        rowItems.append(item);
        rowItems.append(new QStandardItem(col2));
        model->appendRow(rowItems);
    }
    sqlite3_finalize(stmt1);
}

void Widget::setupContextMenu()
{
    ui->tableView->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(ui->tableView, &QTableView::customContextMenuRequested, this, &Widget::onCustomContextMenuRequested);
}

void Widget::contextMenuEvent(QContextMenuEvent *event)
{
    QMenu contextMenu(this);
    QAction deleteAction("Delete Row", this);

    connect(&deleteAction, &QAction::triggered, this, &Widget::onDeleteRow);
    contextMenu.addAction(&deleteAction);

    contextMenu.exec(event->globalPos());
}

void Widget::resizeEvent(QResizeEvent *event)
{
    QWidget::resizeEvent(event);
    adjustColumnWidths();
}

void Widget::on_searchbtn_clicked()
{

    if(!ui->searchinput->text().isEmpty()){
        fetchSpecify(ui->searchinput->text());
        ui->searchinput->clear();
    }else {
        fetchData();
    }
}


void Widget::on_savebtn_clicked()
{
    if((!ui->colinput1->text().isEmpty()) && (!ui->colinput2->text().isEmpty())){
        QString inssql = "INSERT INTO sec (col1, col2) VALUES ('" + ui->colinput1->text() + "', '" + ui->colinput2->text() + "');";
        QByteArray byteArray  = inssql.toLocal8Bit();
        const char* insertSQL = byteArray.constData();
        int result = sqlite3_exec(db, insertSQL, 0, 0, 0);
        checkError(result, "Cannot insert record", "save success !!!");
        fetchData();
        ui->colinput1->clear();
        ui->colinput2->clear();
    }
}

void Widget::onDeleteRow()
{
    QModelIndexList selectedIndexes = ui->tableView->selectionModel()->selectedIndexes();
    if (!selectedIndexes.isEmpty()) {
        int row = selectedIndexes.first().row();
        QString todel = model->item(row)->text();
        model->removeRow(row);
        QString insertSQL = "delete from sec where col1 = '" + todel + "'";
        QByteArray byteArray  = insertSQL.toLocal8Bit();
        const char* insertSQLc = byteArray.constData();
        int result = sqlite3_exec(db, insertSQLc, 0, 0, 0);
        checkError(result, "Cannot delete record", "delete row success !!!");

    }
}

void Widget::onCustomContextMenuRequested(const QPoint &pos)
{
    QModelIndex index = ui->tableView->indexAt(pos);
    if (index.isValid()) {
        QPoint globalPos = ui->tableView->viewport()->mapToGlobal(pos);
        QContextMenuEvent event(QContextMenuEvent::Mouse, pos, globalPos);
        contextMenuEvent(&event);
    }
}

void Widget::adjustColumnWidths()
{
    int totalWidth = ui->tableView->viewport()->width();
    int column0Width = totalWidth / 4; // 1/4 of the total width
    int column1Width = 3 * totalWidth / 4; // 3/4 of the total width
    ui->tableView->setColumnWidth(0, column0Width);
    ui->tableView->setColumnWidth(1, column1Width);
}

