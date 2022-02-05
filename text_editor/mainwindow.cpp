#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    // Создание и настройка меню
    QMenu* menu = nullptr;
    QToolBar* toolbar = nullptr;
    menu = menuBar()->addMenu(tr("Файл")); toolbar = addToolBar(tr("Файл"));
    toolbar->addAction(menu->addAction(QIcon(":/images/filenew.png"),"Новый",this,&MainWindow::onNewFile,QKeySequence::New));
    toolbar->addAction(menu->addAction(QIcon(":/images/fileopen.png"),"Открыть",this,&MainWindow::onOpenFile,QKeySequence::Open));
    toolbar->addAction(menu->addAction(QIcon(":/images/filesave.png"),"Сохранить",this,&MainWindow::onSaveFile,QKeySequence::Save));
    toolbar->addAction(menu->addAction(QIcon(":/images/save_all.png"),"Сохранить все",this,&MainWindow::onSaveAllFiles,QKeySequence()));
    menu->addAction(QIcon(),tr("Сохранить как"),this,&MainWindow::onSaveFileAs,QKeySequence::SaveAs);
    menu->addAction(QIcon(),tr("Закрыть"),this,&MainWindow::onCloseFile,Qt::CTRL | Qt::Key_W);
    menu->addAction(QIcon(),tr("Закрыть все"),this,&MainWindow::onCloseAllFiles,QKeySequence());
    menu->addAction(QIcon(),tr("Выход"),this,&MainWindow::close,QKeySequence::Quit);

    menu = menuBar()->addMenu(tr("Правка"));
    menu->addAction(QIcon(),tr("Вырезать"),this,&MainWindow::onCut,QKeySequence::Cut);
    menu->addAction(QIcon(),tr("Копировать"),this,&MainWindow::onCopy,QKeySequence::Copy);
    menu->addAction(QIcon(),tr("Вставить"),this,&MainWindow::onPaste,QKeySequence::Paste);
    menu->addAction(QIcon(),tr("Удалить"),this,&MainWindow::onDelete,QKeySequence::Delete);
    menu->addAction(QIcon(),tr("Выделить все"),this,&MainWindow::onSelectAll,QKeySequence::SelectAll);

    menu = menuBar()->addMenu(tr("Вид"));
    menu->addAction(QIcon(),tr("Показать проводник"),this,&MainWindow::onShowExplorer,QKeySequence())->setCheckable(1);
    menu->addAction(QIcon(),tr("Показать обозреватель открытых документов"),this,&MainWindow::onShowViewer,QKeySequence())->setCheckable(1);
    for(int i=0; i<menu->actions().count(); i++) menu->actions().at(i)->setChecked(1);

    // Настройка проводника и обозревателя
    QFileSystemModel* fsmodel = new QFileSystemModel(this);
    fsmodel->setRootPath(QDir::rootPath());
    ui->treeView->setModel(fsmodel);
    ui->treeView->resizeColumnToContents(0);
    ui->treeView->hideColumn(1);    // Скрываем колонку с размерами файлов
    ui->treeView->hideColumn(2);    // Скрываем тип файла
    ui->treeView->hideColumn(3);    // Скрываем дату последнего изменения

    ui->listView->setModel(new QStringListModel(openedfiles));

    ui->fileExplorer->setWindowTitle(tr("Проводник"));
    ui->fileViewer->setWindowTitle(tr("Обозреватель открытых документов"));

    setWindowTitle(tr("Text editor"));
}

MainWindow::~MainWindow()
{
    delete ui;
}

int MainWindow::createFileTab(bool isnewfile, const QString &filepath)
{
    int index;
    FileWidget* file;
    if(isnewfile) {
        file = new FileWidget();
        index = ui->tabWidget->addTab(file,filepath);
    } else {
        file = new FileWidget(filepath);
        index = ui->tabWidget->addTab(file, QDir(filepath).dirName());
        onCurrentPathChanged("",filepath);
    }

    setupConnection(file,1);

    ui->tabWidget->setCurrentIndex(index);
    return index;
}

void MainWindow::saveFile(int index)
{
    FileWidget* file = (FileWidget*)ui->tabWidget->widget(index);
    if(!file->isModified() && file->isBinded()) return;

    ui->tabWidget->setCurrentIndex(index);
    if(file->isBinded()) { file->fromWidgetToFile(); }
    else { saveFileAs(index); }
}

void MainWindow::saveFileAs(int index)
{
    QString filename =
            QFileDialog::getSaveFileName(this,
                                         tr("Save file"),
                                         QDir::rootPath());
    if(filename.isNull()) return;

    FileWidget* file = (FileWidget*)ui->tabWidget->widget(index);
    file->fromWidgetToFile(filename);
    ui->tabWidget->setTabText(index, QDir(filename).dirName());             // Смена заголовка вкладки
}

void MainWindow::closeFileTab(int index)
{
    FileWidget* file = (FileWidget*)ui->tabWidget->widget(index);

    if(file->isModified()) {
        ui->tabWidget->setCurrentIndex(index);
        QMessageBox msgBox;
        msgBox.setText((file->isBinded()?"Файл ":"Файл Без названия") + file->getCurrentFilePath() + " изменен, но не сохранен.");
        msgBox.setInformativeText(tr("Сохранить изменения?"));
        msgBox.setStandardButtons(QMessageBox::Save | QMessageBox::Discard | QMessageBox::Cancel);
        msgBox.setDefaultButton(QMessageBox::Save);

        switch (msgBox.exec()) {
          case QMessageBox::Save:
              saveFile(index);
              break;
          case QMessageBox::Discard:
              break;
          case QMessageBox::Cancel:         // Отмена -> вкладка не закроется
              return;
              break;
        }
    }                     // Нужно спросить про изменения

    setupConnection(file,0);
    onCurrentPathChanged(file->getCurrentFilePath(),"");
    ui->tabWidget->removeTab(index);
}

void MainWindow::setupConnection(FileWidget *file, bool toconnect)
{
    if(toconnect) {
        connect(file,&FileWidget::currentPathChanged,this,&MainWindow::onCurrentPathChanged);
        connect(this,&MainWindow::onCut,file,&FileWidget::cut);
        connect(this,&MainWindow::onCopy,file,&FileWidget::copy);
        connect(this,&MainWindow::onPaste,file,&FileWidget::paste);
        connect(this,&MainWindow::onDelete,file,&FileWidget::clear);
        connect(this,&MainWindow::onSelectAll,file,&FileWidget::selectAll);
    } else {
        disconnect(file,&FileWidget::currentPathChanged,this,&MainWindow::onCurrentPathChanged);
        disconnect(this,&MainWindow::onCut,file,&FileWidget::cut);
        disconnect(this,&MainWindow::onCopy,file,&FileWidget::copy);
        disconnect(this,&MainWindow::onPaste,file,&FileWidget::paste);
        disconnect(this,&MainWindow::onDelete,file,&FileWidget::clear);
        disconnect(this,&MainWindow::onSelectAll,file,&FileWidget::selectAll);
    }
}

void MainWindow::closeEvent(QCloseEvent *event)
{
    FileWidget* file;
    QStringList unsavedfiles;

    for(int index=ui->tabWidget->count()-1; index >= 0; index--) {
        file = (FileWidget*)ui->tabWidget->widget(index);
        if(file->isModified()) {
            unsavedfiles << file->getCurrentFilePath();
        }
    }

    if(!unsavedfiles.isEmpty()) {
        ExitDialog dialog(unsavedfiles);
        switch(dialog.exec()) {
        case ExitDialog::YES :
            onCloseAllFiles();
            break;
        case ExitDialog::NO :
            break;
        case ExitDialog::CANCEL :
            event->ignore();
            return;
        default:
            event->ignore();
            return;
        }
    }
    event->accept();
}

void MainWindow::onCurrentPathChanged(QString oldpath, QString newpath)
{
    if(!oldpath.isEmpty() && !newpath.isEmpty()) {                              // Файл изменен
        openedfiles.replace(openedfiles.indexOf(oldpath), newpath);

        FileWidget* file;                                                       // Смена названия вкладки
        for(int index=ui->tabWidget->count()-1; index >= 0; index--) {
            file = (FileWidget*)ui->tabWidget->widget(index);
            if(file->getCurrentFilePath() == newpath) {
                ui->tabWidget->setTabText(index, QDir(newpath).dirName());
                break;
            }
        }
    }
    else if(oldpath.isEmpty() && !newpath.isEmpty()) {                                        // Файл добавлен
        openedfiles.append(newpath);
    }
    else if(!oldpath.isEmpty() && newpath.isEmpty()) {                                        // Файл закрыт
        openedfiles.removeOne(oldpath);
    }

    QStringListModel* model = (QStringListModel*)ui->listView->model();
    model->setStringList(openedfiles);
}




void MainWindow::onNewFile()
{
    createFileTab(1,"Без названия");
}

void MainWindow::onOpenFile()
{
    QString filename =
            QFileDialog::getOpenFileName(this,
                                         tr("Open file"),
                                         QDir::rootPath(),
                                         tr("Any file (*) ;; C++ file (*.h *.hpp *.cpp);; Python file (*.py)"));
    if(filename.isNull()) return;
    createFileTab(0,filename);
}

void MainWindow::onSaveFile()
{
    int index = ui->tabWidget->currentIndex();
    if(index >= 0) { saveFile(index); }
}

void MainWindow::onSaveFileAs()
{
    int index = ui->tabWidget->currentIndex();
    if(index >= 0) { saveFileAs(index); }
}

void MainWindow::onCloseFile()
{
    int index = ui->tabWidget->currentIndex();
    if(index >= 0) { closeFileTab(index); }
}

void MainWindow::onSaveAllFiles()
{
    for(int index=ui->tabWidget->count()-1; index >= 0; index--) {
        saveFile(index);
    }
}

void MainWindow::onCloseAllFiles()
{
    for(int index=ui->tabWidget->count()-1; index >= 0; index--) {      // Важно, чтобы цикл шел с конца, индексы относительные
        closeFileTab(index);
    }
}

void MainWindow::onQuit()
{
    FileWidget* file;
    QStringList unsavedfiles;

    for(int index=ui->tabWidget->count()-1; index >= 0; index--) {
        file = (FileWidget*)ui->tabWidget->widget(index);
        if(file->isModified()) {
            unsavedfiles << file->getCurrentFilePath();
        }
    }

    if(!unsavedfiles.isEmpty()) {
        ExitDialog dialog(unsavedfiles);
        switch(dialog.exec()) {
        case ExitDialog::YES :
            onCloseAllFiles();
            break;
        case ExitDialog::NO :
            break;
        case ExitDialog::CANCEL :
            return;
        default:
            return;
        }
    }
    QApplication::quit();
}

void MainWindow::onShowExplorer(bool checked)
{
    ui->fileExplorer->setVisible(checked);
}

void MainWindow::onShowViewer(bool checked)
{
    ui->fileViewer->setVisible(checked);
}




void MainWindow::on_treeView_doubleClicked(const QModelIndex &index)
{
    QFileSystemModel* fsmodel = (QFileSystemModel*)ui->treeView->model();
    if(!fsmodel->isDir(index)) {
        createFileTab(0,fsmodel->filePath(index));
    }
}


void MainWindow::on_listView_doubleClicked(const QModelIndex &index)
{
    QStringListModel* model = (QStringListModel*)ui->listView->model();
    QString filepath = model->data(index).toString();
    FileWidget* file;                                                       // Смена названия вкладки
    for(int index=ui->tabWidget->count()-1; index >= 0; index--) {
        file = (FileWidget*)ui->tabWidget->widget(index);
        if(file->getCurrentFilePath() == filepath) {
            ui->tabWidget->setCurrentIndex(index);
            break;
        }
    }
}


ExitDialog::ExitDialog(QStringList unsavedfiles)
{
    setModal(1);
    QGridLayout *layout = new QGridLayout;

    table = new QTableWidget;
    label = new QLabel(tr("Сохранить измененные документы?"));
    yes = new QPushButton(tr("Да"));
    no = new QPushButton(tr("Нет"));
    cancel = new QPushButton(tr("Отмена"));

    layout->addWidget(label, 0, 0, 1, 3);
    layout->addWidget(table, 1, 0, 1, 3);
    layout->addWidget(yes, 2, 0);
    layout->addWidget(no, 2, 1);
    layout->addWidget(cancel, 2, 2);
    setLayout(layout);

    table->setRowCount(unsavedfiles.count());
    table->setColumnCount(2);
    table->setHorizontalHeaderLabels(QStringList({"Файл","Путь"}));
    table->horizontalHeader()->setStretchLastSection(true);
    table->verticalHeader()->hide();
    
    for(int i=0; i<unsavedfiles.count(); i++) {
        QString path = unsavedfiles.at(i);
        table->setItem(i, 0, new QTableWidgetItem(path.isEmpty()? "Без названия" : QDir(path).dirName()));
        table->setItem(i, 1, new QTableWidgetItem(path.isEmpty()? "" : path));
    }

    connect(yes, &QPushButton::clicked, this, [&]() {done(YES);});
    connect(no, &QPushButton::clicked, this, [&]() {done(NO);});
    connect(cancel, &QPushButton::clicked, this, [&]() {done(CANCEL);});

    setWindowTitle(tr("Сохранить изменения?"));
    setFixedSize(500, 400); 
    setWindowFlags(windowFlags() & ~Qt::WindowCloseButtonHint);         // чтобы окно не закрывалось по кнопке
}

void ExitDialog::keyPressEvent(QKeyEvent *e)
{
    if(e->key() != Qt::Key_Escape) { QDialog::keyPressEvent(e); }
}
