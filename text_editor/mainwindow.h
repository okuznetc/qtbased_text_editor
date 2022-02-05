#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QDebug>
#include <QToolBar>
#include <QFileDialog>
#include <QFileSystemModel>
#include <QStringListModel>
#include <QStringListModel>
#include <QMessageBox>
#include <QLabel>
#include <QTableWidget>
#include <QPushButton>

#include "filewidget.h"

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE


class ExitDialog : public QDialog
{
    Q_OBJECT
public:
    ExitDialog(QStringList unsavedfiles);

    QTableWidget *table;
    QLabel *label;
    QPushButton *yes;
    QPushButton *no;
    QPushButton *cancel;

    enum status{YES,NO,CANCEL};

protected:
    void keyPressEvent(QKeyEvent *e) override;      // Чтобы окно не закрывалось на esc
};

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private:
    Ui::MainWindow *ui;

    int createFileTab(bool isnewfile, const QString &filepath);
    void saveFile(int index);
    void saveFileAs(int index);
    void closeFileTab(int index);
    void setupConnection(FileWidget* file, bool toconnect);             // toconnect=1 -> connect; toconnect=0 -> disconnect

    QStringList openedfiles;


public slots:
    void onCurrentPathChanged(QString oldpath, QString newpath);

private slots:
    void on_treeView_doubleClicked(const QModelIndex &index);
    void on_listView_doubleClicked(const QModelIndex &index);

    void onNewFile();                                   // Слоты для меню "Файл"
    void onOpenFile();
    void onSaveFile();
    void onSaveFileAs();
    void onSaveAllFiles();
    void onCloseFile();
    void onCloseAllFiles();
    void onQuit();

    void onShowExplorer(bool checked);                  // Слоты для меню "Вид"
    void onShowViewer(bool checked);

signals:
    void onCut();                                       // Слоты для меню "Правка"
    void onCopy();
    void onPaste();
    void onDelete();
    void onSelectAll();

protected:
    void closeEvent(QCloseEvent *event) override;

};

#endif // MAINWINDOW_H
