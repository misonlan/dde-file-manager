#include <QTextEdit>
#include <QTextBlock>

#include <anchors.h>

#include "fileitem.h"
#include "../app/global.h"

DWIDGET_USE_NAMESPACE

FileIconItem::FileIconItem(QWidget *parent) :
    QFrame(parent)
{
    icon = new QLabel(this);
    edit = new QTextEdit(this);

    icon->setAlignment(Qt::AlignCenter);
    icon->setFrameShape(QFrame::NoFrame);
    icon->installEventFilter(this);

    edit->setWordWrapMode(QTextOption::WrapAtWordBoundaryOrAnywhere);
    edit->setAlignment(Qt::AlignHCenter);
    edit->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    edit->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    edit->setFrameShape(QFrame::NoFrame);
    edit->setContextMenuPolicy(Qt::NoContextMenu);
    edit->installEventFilter(this);

    AnchorsBase::setAnchor(icon, Qt::AnchorHorizontalCenter, this, Qt::AnchorHorizontalCenter);
    AnchorsBase::setAnchor(edit, Qt::AnchorTop, icon, Qt::AnchorBottom);
    AnchorsBase::setAnchor(edit, Qt::AnchorHorizontalCenter, icon, Qt::AnchorHorizontalCenter);
    AnchorsBase::getAnchorBaseByWidget(edit)->setTopMargin(-3);

    setFrameShape(QFrame::NoFrame);
    setFocusProxy(edit);

    connect(edit, &QTextEdit::textChanged, this, [this] {
        QSignalBlocker blocker(edit);
        Q_UNUSED(blocker)

        QTextCursor cursor(edit->document());

        cursor.movePosition(QTextCursor::Start);

        do {
            QTextBlockFormat format = cursor.blockFormat();

            format.setLineHeight(TEXT_LINE_HEIGHT, QTextBlockFormat::FixedHeight);
            cursor.setBlockFormat(format);
        } while (cursor.movePosition(QTextCursor::NextBlock));
    });
}

bool FileIconItem::event(QEvent *ee)
{
    if(ee->type() == QEvent::DeferredDelete) {
        if(!canDeferredDelete) {
            ee->accept();
            return true;
        }
    } else if(ee->type() == QEvent::Resize) {
        edit->setFixedWidth(width());

        resize(width(), icon->height() + edit->height());
    }

    return QFrame::event(ee);
}

bool FileIconItem::eventFilter(QObject *obj, QEvent *ee)
{
    if(ee->type() == QEvent::Resize) {
        if(obj == icon || obj == edit) {
            resize(width(), icon->height() + edit->height());
        }
    } else if(ee->type() == QEvent::KeyPress) {
        if(obj != edit) {
           return QFrame::eventFilter(obj, ee);
        }

        QKeyEvent *event = static_cast<QKeyEvent*>(ee);

        if(event->key() != Qt::Key_Enter && event->key() != Qt::Key_Return) {
            return QFrame::eventFilter(obj, ee);
        }

        if(!(event->modifiers() & Qt::ShiftModifier)) {
            ee->accept();
            parentWidget()->setFocus();

            return true;
        } else {
            event->accept();
            return false;
        }
    }

    return QFrame::eventFilter(obj, ee);
}
