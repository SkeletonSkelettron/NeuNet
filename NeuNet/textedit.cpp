#include "textedit.h"
#include <QCompleter>
#include <QKeyEvent>
#include <QAbstractItemView>
#include <QtDebug>
#include <QApplication>
#include <QModelIndex>
#include <QAbstractItemModel>
#include <QScrollBar>
#include <QObject>
#include <qpainter.h>

//! [0]
TextEdit::TextEdit(QWidget *parent)
	: QTextEdit(parent), c(0)
{
	lineNumberArea = new LineNumberArea(this);

	connect(this, SIGNAL(blockCountChanged(int)), this, SLOT(updateLineNumberAreaWidth(int)));
	connect(this, SIGNAL(updateRequest(QRect, int)), this, SLOT(updateLineNumberArea(QRect, int)));
	connect(this, SIGNAL(cursorPositionChanged()), this, SLOT(highlightCurrentLine()));

	updateLineNumberAreaWidth(0);
	highlightCurrentLine();
}
//! [0]

//! [1]
TextEdit::~TextEdit()
{
}
//! [1]

//! [2]
void TextEdit::setCompleter(QCompleter *completer)
{
	if (c)
		QObject::disconnect(c, 0, this, 0);

	c = completer;

	if (!c)
		return;

	c->setWidget(this);
	c->setCaseSensitivity(Qt::CaseInsensitive);
	c->setCompletionMode(QCompleter::PopupCompletion);
	c->setCaseSensitivity(Qt::CaseInsensitive);
	QObject::connect(c, SIGNAL(activated(QString)),
		this, SLOT(insertCompletion(QString)));
}
//! [2]

//! [3]
QCompleter *TextEdit::completer() const
{
	return c;
}
//! [3]

//! [4]
void TextEdit::insertCompletion(const QString& completion)
{
	if (c->widget() != this)
		return;
	QTextCursor tc = textCursor();
	int extra = completion.length() - c->completionPrefix().length();
	tc.movePosition(QTextCursor::StartOfWord, QTextCursor::KeepAnchor);
	tc.insertText(completion);
	setTextCursor(tc);
}
//! [4]

//! [5]
QString TextEdit::textUnderCursor() const
{
	QTextCursor tc = textCursor();
	tc.select(QTextCursor::WordUnderCursor);
	return tc.selectedText();
}
//! [5]

//! [6]
void TextEdit::focusInEvent(QFocusEvent *e)
{
	if (c)
		c->setWidget(this);
	QTextEdit::focusInEvent(e);
}
//! [6]

//! [7]
void TextEdit::keyPressEvent(QKeyEvent *e)
{
	if (c && c->popup()->isVisible()) {
		// The following keys are forwarded by the completer to the widget
		switch (e->key()) {
		case Qt::Key_Enter:
		case Qt::Key_Return:
		case Qt::Key_Escape:
		case Qt::Key_Tab:
		case Qt::Key_Backtab:
			e->ignore();
			return; // let the completer do default behavior
		default:
			break;
		}
	}

	bool isShortcut = ((e->modifiers() & Qt::ControlModifier) && e->key() == Qt::Key_E); // CTRL+E
	if (!c || !isShortcut) // do not process the shortcut when we have a completer
		QTextEdit::keyPressEvent(e);


	const bool ctrlOrShift = e->modifiers() & (Qt::ControlModifier | Qt::ShiftModifier);
	if (!c || (ctrlOrShift && e->text().isEmpty()))
		return;

	static QString eow("~!@#$%^&*()_+{}|:\"<>?,/;'[]\\-="); // end of word
	bool hasModifier = (e->modifiers() != Qt::NoModifier) && !ctrlOrShift;
	QString completionPrefix = textUnderCursor();

	if (!isShortcut && (hasModifier || e->text().isEmpty() //|| completionPrefix.length() < 2
		|| eow.contains(e->text().right(1)))) {
		c->popup()->hide();
		return;
	}

	if (completionPrefix != c->completionPrefix()) {
		c->setCompletionPrefix(completionPrefix);
		c->popup()->setCurrentIndex(c->completionModel()->index(0, 1));
	}

	if (!c->popup()->isVisible())
	{
		QRect cr = cursorRect();
		cr.setWidth(c->popup()->sizeHintForColumn(0)
			+ c->popup()->verticalScrollBar()->sizeHint().width());
		c->complete(cr); // popup it up!
	}
}

//! [8]
int TextEdit::lineNumberAreaWidth()
{
	//int digits = 1;
	//int max = qMax(1, blockCount());
	//while (max >= 10) {
	//	max /= 10;
	//	++digits;
	//}

	//int space = 3 + fontMetrics().horizontalAdvance(QLatin1Char('9')) * digits;

	//return space;
	return 3;
}

void TextEdit::updateLineNumberAreaWidth(int /* newBlockCount */)
{
	setViewportMargins(lineNumberAreaWidth(), 0, 0, 0);
}

void TextEdit::updateLineNumberArea(const QRect &rect, int dy)
{
	if (dy)
		lineNumberArea->scroll(0, dy);
	else
		lineNumberArea->update(0, rect.y(), lineNumberArea->width(), rect.height());

	if (rect.contains(viewport()->rect()))
		updateLineNumberAreaWidth(0);
}

void TextEdit::resizeEvent(QResizeEvent *e)
{
	QTextEdit::resizeEvent(e);

	QRect cr = contentsRect();
	lineNumberArea->setGeometry(QRect(cr.left(), cr.top(), lineNumberAreaWidth(), cr.height()));
}


void TextEdit::highlightCurrentLine()
{
	QList<QTextEdit::ExtraSelection> extraSelections;

	if (!isReadOnly()) {
		QTextEdit::ExtraSelection selection;

		QColor lineColor = QColor(Qt::green).lighter(160);

		selection.format.setBackground(lineColor);
		selection.format.setProperty(QTextFormat::FullWidthSelection, true);
		selection.cursor = textCursor();
		selection.cursor.clearSelection();
		extraSelections.append(selection);
	}

	setExtraSelections(extraSelections);
}
//
void TextEdit::lineNumberAreaPaintEvent(QPaintEvent *event)
{
	//QPainter painter(lineNumberArea);
	//painter.fillRect(event->rect(), Qt::lightGray);

	////![extraAreaPaintEvent_0]

	////![extraAreaPaintEvent_1]
	//QTextBlock block = firstVisibleBlock();
	//int blockNumber = block.blockNumber();
	//int top = (int)blockBoundingGeometry(block).translated(contentOffset()).top();
	//int bottom = top + (int)blockBoundingRect(block).height();
	////![extraAreaPaintEvent_1]

	////![extraAreaPaintEvent_2]
	//while (block.isValid() && top <= event->rect().bottom()) {
	//	if (block.isVisible() && bottom >= event->rect().top()) {
	//		QString number = QString::number(blockNumber + 1);
	//		painter.setPen(Qt::black);
	//		painter.drawText(0, top, lineNumberArea->width(), fontMetrics().height(),
	//			Qt::AlignRight, number);
	//	}

	//	block = block.next();
	//	top = bottom;
	//	bottom = top + (int)blockBoundingRect(block).height();
	//	++blockNumber;
	//}
}