#ifndef LINENUMBERAREA_H
#define LINENUMBERAREA_H

#include <QWidget>
#include "filewidget.h"


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

#endif // LINENUMBERAREA_H
