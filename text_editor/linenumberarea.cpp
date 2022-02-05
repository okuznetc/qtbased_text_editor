#include "linenumberarea.h"


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
