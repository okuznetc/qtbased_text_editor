#include "filewidget.h"

FileWidget::FileWidget()
{
    currfilepath = "";
    setupTextEdit();
}

FileWidget::FileWidget(const QString &filepath)
{
    currfilepath = filepath;
    fromFileToWidget();
    setupTextEdit();
}

void FileWidget::fromFileToWidget()
{
    QFile file(currfilepath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) return;
    QByteArray content = file.readAll();
    file.close();
    document()->setPlainText(content);

//    // Может быть куча проблем с кодировкой файла. Есть неидеальное, но решение отсюда:
//    // https://stackoverflow.com/questions/18226858/detect-text-file-encoding
//    QTextCodec::ConverterState state;
//    QTextCodec *codec = QTextCodec::codecForName("UTF-8");
//    if (!codec) {
//        QMessageBox::warning(this, "Ошибка", "Проблема с кодировкой при чтении файла.");
//        return;
//    }
//    QString text = codec->toUnicode(content.constData(), content.size(), &state);
//    if (state.invalidChars > 0) {
//        // Not a UTF-8 text - using system default locale
//        QTextCodec * codec = QTextCodec::codecForLocale();
//        if (!codec) {
//            QMessageBox::warning(this, "Ошибка", "Проблема с кодировкой при чтении файла.");
//            return;
//        }
//        document()->setPlainText(codec->toUnicode(content));
//    }
//    else {
//        document()->setPlainText(text);
//    }

    document()->setModified(0);
}

void FileWidget::fromWidgetToFile()
{
    QFile file(currfilepath);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) return;
    QString content = document()->toPlainText();
    file.write(content.toUtf8());
    file.close();
    document()->setModified(0);
}

void FileWidget::fromWidgetToFile(const QString &filepath)
{
    if(!isBinded()) { highlighter = new Highlighter(document()); }
    QString oldPath = currfilepath;
    currfilepath = filepath;

    if(!highlighter->setSyntaxSettings(QFileInfo(currfilepath).suffix())) {
        QMessageBox::warning(this, "Ошибка", "Проблема с XML файлом настроек.");
    }
    fromWidgetToFile();
    emit currentPathChanged(oldPath, currfilepath);
}

bool FileWidget::isBinded()
{
    return !currfilepath.isEmpty();
}

bool FileWidget::isModified()
{
    return document()->isModified();
}

QString FileWidget::getCurrentFilePath()
{
    return currfilepath;
}

void FileWidget::lineNumberAreaPaintEvent(QPaintEvent *event)
{
    QPainter painter(lineNumberArea);
    painter.fillRect(event->rect(), Qt::lightGray);
    QTextBlock block = firstVisibleBlock();
    int blockNumber = block.blockNumber();
    int top = qRound(blockBoundingGeometry(block).translated(contentOffset()).top());
    int bottom = top + qRound(blockBoundingRect(block).height());
    while (block.isValid() && top <= event->rect().bottom()) {
        if (block.isVisible() && bottom >= event->rect().top()) {
            QString number = QString::number(blockNumber + 1);
            painter.setPen(Qt::black);
            painter.drawText(0, top, lineNumberArea->width(), fontMetrics().height(),
                             Qt::AlignRight, number);
        }

        block = block.next();
        top = bottom;
        bottom = top + qRound(blockBoundingRect(block).height());
        ++blockNumber;
    }
}

int FileWidget::lineNumberAreaWidth()
{
    int digits = 1;
    int max = qMax(1, blockCount());
    while (max >= 10) {
        max /= 10;
        ++digits;
    }

    int space = 3 + fontMetrics().horizontalAdvance(QLatin1Char('9')) * digits;

    return space;
}

void FileWidget::updateLineNumberAreaWidth(int newBlockCount)
{
    Q_UNUSED(newBlockCount);
    setViewportMargins(lineNumberAreaWidth(), 0, 0, 0);
}

void FileWidget::highlightCurrentLine()
{
    QList<QTextEdit::ExtraSelection> extraSelections;

    if (!isReadOnly()) {
        QTextEdit::ExtraSelection selection;

        QColor lineColor = QColor(Qt::yellow).lighter(160);

        selection.format.setBackground(lineColor);
        selection.format.setProperty(QTextFormat::FullWidthSelection, true);
        selection.cursor = textCursor();
        selection.cursor.clearSelection();
        extraSelections.append(selection);
    }

    setExtraSelections(extraSelections);
}

void FileWidget::updateLineNumberArea(const QRect &rect, int dy)
{
    if (dy)
        lineNumberArea->scroll(0, dy);
    else
        lineNumberArea->update(0, rect.y(), lineNumberArea->width(), rect.height());

    if (rect.contains(viewport()->rect()))
        updateLineNumberAreaWidth(0);
}

void FileWidget::setupTextEdit()
{
    QFont font;
    font.setFamily("Courier");
    font.setFixedPitch(true);
    font.setPointSize(15);
    setFont(font);

    if(isBinded()) {
        highlighter = new Highlighter(document());
        if(!highlighter->setSyntaxSettings(QFileInfo(currfilepath).suffix())) {
            QMessageBox::warning(this, "Ошибка", "Проблема с XML файлом настроек.");
        }
    }

    lineNumberArea = new LineNumberArea(this);

    connect(this, &FileWidget::blockCountChanged, this, &FileWidget::updateLineNumberAreaWidth);
    connect(this, &FileWidget::updateRequest, this, &FileWidget::updateLineNumberArea);
    connect(this, &FileWidget::cursorPositionChanged, this, &FileWidget::highlightCurrentLine);

    updateLineNumberAreaWidth(0);
    highlightCurrentLine();
}

void FileWidget::resizeEvent(QResizeEvent *event)
{
    QPlainTextEdit::resizeEvent(event);

    QRect cr = contentsRect();
    lineNumberArea->setGeometry(QRect(cr.left(), cr.top(), lineNumberAreaWidth(), cr.height()));
}



LineNumberArea::LineNumberArea(FileWidget *editor) : QWidget(editor), filewidget(editor)
{}

QSize LineNumberArea::sizeHint() const
{
    return QSize(filewidget->lineNumberAreaWidth(), 0);
}

void LineNumberArea::paintEvent(QPaintEvent *event)
{
    filewidget->lineNumberAreaPaintEvent(event);
}


