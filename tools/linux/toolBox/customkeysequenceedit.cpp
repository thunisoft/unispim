#include "customkeysequenceedit.h"

#include <QKeyEvent>

CustomKeySequenceEdit::CustomKeySequenceEdit(QWidget *parent) : QKeySequenceEdit(parent) {
}

CustomKeySequenceEdit::~CustomKeySequenceEdit() { }

void CustomKeySequenceEdit::keyPressEvent(QKeyEvent *pEvent)
{
    if((pEvent->key() == Qt::Key_Escape) || (pEvent->key() == Qt::Key_Return))
    {
        pEvent->ignore();
        return;
    }
    QKeySequenceEdit::keyPressEvent(pEvent);
    QKeySequence seq(QKeySequence::fromString(keySequence().toString().split(", ").first()));
    setKeySequence(seq);
}
