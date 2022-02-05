#ifndef FILEWIDGET_H
#define FILEWIDGET_H

#include <QPlainTextEdit>
#include <QMessageBox>
#include <QFileInfo>
#include <QPainter>

#include "highlighter.h"

class FileWidget;

class LineNumberArea : public QWidget
{
public:
    LineNumberArea(FileWidget *editor);

    QSize sizeHint() const override;

protected:
    void paintEvent(QPaintEvent *event) override;

private:
    FileWidget *filewidget;
};


class FileWidget : public QPlainTextEdit
{
    Q_OBJECT
public:
    FileWidget();
    FileWidget(const QString &filepath);

    void fromFileToWidget();
    void fromWidgetToFile();
    void fromWidgetToFile(const QString &filepath);

    bool isBinded();
    bool isModified();
    QString getCurrentFilePath();

    void lineNumberAreaPaintEvent(QPaintEvent *event);
    int lineNumberAreaWidth();

signals:
    void currentPathChanged(QString oldPath, QString newPath);

private slots:
    void updateLineNumberAreaWidth(int newBlockCount);
    void highlightCurrentLine();
    void updateLineNumberArea(const QRect &rect, int dy);

private:
    QString currfilepath;
    Highlighter* highlighter;

    void setupTextEdit();

    void resizeEvent(QResizeEvent *event) override;
    QWidget *lineNumberArea;
};

#endif // FILEWIDGET_H
