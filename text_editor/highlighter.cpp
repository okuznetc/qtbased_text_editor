#include "highlighter.h"

Highlighter::Highlighter(QTextDocument *parent)
    : QSyntaxHighlighter(parent)
{
}

bool Highlighter::setSyntaxSettings(QString extention)
{
    QDomDocument doc(":/settings/text_editor.xml");            // Открываем XML и записываем его в QDOMDocument
    QFile file(":/settings/text_editor.xml");
    if (!file.open(QIODevice::ReadOnly)) {
//        qDebug() << "Open file error";
        return 0;
    }
//    QString errorMsg;
//    if (!doc.setContent(&file,1,&errorMsg)) {
//        qDebug() << "QDomDocument error" << errorMsg;
    if (!doc.setContent(&file)) {
        file.close();
        return 0;
    }
    file.close();

    // Пробежимся по XML и сохраним правила форматирования в highlightingRules
    highlightingRules.clear();

    QDomNode syntaxes = doc.documentElement().namedItem("syntaxes");
    QDomElement pattern, format;
    HighlightingRule hrule;
    QString nodename;

    for(auto syntax = syntaxes.firstChild(); !syntax.isNull(); syntax = syntax.nextSibling()) {     // цикл по синтаксисам (C++, python, ...)
        if( !syntax.toElement().attribute("ext_list").contains(extention) ) continue;

        for(auto rule = syntax.firstChild(); !rule.isNull(); rule = rule.nextSibling()) {           // цикл по нодам данного синтаксиса
            nodename = rule.toElement().tagName();
            if(nodename == "rule") {                                      // Ноды <rule>
                pattern = rule.namedItem("pattern").toElement();                            // Считываем атрибуты
                format = rule.namedItem("format").toElement();

                hrule.pattern = QRegularExpression(pattern.attribute("value"));             // Заполняем правило
                hrule.format.setForeground(QColor(format.attribute("foreground")));
                hrule.format.setFontWeight(format.attribute("font_weight").toInt());

                highlightingRules.append(hrule);                                            // Добавляем правило в вектор
            } else

            if(nodename == "startComment") {                              // Ноды <startComment>
                pattern = rule.namedItem("pattern").toElement();
                commentStartExpression = QRegularExpression(pattern.attribute("value"));
            }else

            if(nodename == "endComment") {                                // Ноды <endComment>
                pattern = rule.namedItem("pattern").toElement();
                commentEndExpression = QRegularExpression(pattern.attribute("value"));
            }else

            if(nodename == "multiLineComment") {                          // Ноды <multiLineComment>
                format = rule.namedItem("format").toElement();
                multiLineCommentFormat.setForeground(QColor(format.attribute("foreground")));
            }
        }
    }
    return 1;
}


void Highlighter::highlightBlock(const QString &text)
{
    if(highlightingRules.isEmpty()) return;

    for (const HighlightingRule &rule : qAsConst(highlightingRules)) {
        QRegularExpressionMatchIterator matchIterator = rule.pattern.globalMatch(text);
        while (matchIterator.hasNext()) {
            QRegularExpressionMatch match = matchIterator.next();
            setFormat(match.capturedStart(), match.capturedLength(), rule.format);      // применение форматирования для каждого паттерна
        }
    }

    setCurrentBlockState(0);

    int startIndex = 0;
    if (previousBlockState() != 1)                                                      // состояние 1 - комментарий
        startIndex = text.indexOf(commentStartExpression);                              // поиск начала комментария в данном блоке текста

    while (startIndex >= 0) {
        QRegularExpressionMatch match = commentEndExpression.match(text, startIndex);
        int endIndex = match.capturedStart();                                           // поиск символа конца комментария в данном блоке
        int commentLength = 0;
        if (endIndex == -1) {
          setCurrentBlockState(1);
          commentLength = text.length() - startIndex;
        } else {
          commentLength = endIndex - startIndex + match.capturedLength();
        }
        setFormat(startIndex, commentLength, multiLineCommentFormat);
        startIndex = text.indexOf(commentStartExpression, startIndex + commentLength);
    }
}
