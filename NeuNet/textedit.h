
#ifndef TEXTEDIT_H
#define TEXTEDIT_H

#include <QTextEdit>

QT_BEGIN_NAMESPACE
class QCompleter;
class QPaintEvent;
class QResizeEvent;
class QSize;
class QWidget;
QT_END_NAMESPACE
class LineNumberArea;
//! [0]
class TextEdit : public QTextEdit
{
    Q_OBJECT

public:
    TextEdit(QWidget *parent = 0);
    ~TextEdit();

   void setCompleter(QCompleter *c);
    QCompleter *completer() const;
	void lineNumberAreaPaintEvent(QPaintEvent *event);
	int lineNumberAreaWidth();
protected:
    void keyPressEvent(QKeyEvent *e) override;
    void focusInEvent(QFocusEvent *e) override;
	void resizeEvent(QResizeEvent *event) override;
private slots:
    void insertCompletion(const QString &completion);
	void updateLineNumberAreaWidth(int newBlockCount);
	void highlightCurrentLine();
	void updateLineNumberArea(const QRect &, int);

private:
    QString textUnderCursor() const;
	QWidget *lineNumberArea;
private:
    QCompleter *c;
};
//! [0]

#endif // TEXTEDIT_H

class LineNumberArea : public QWidget
{
public:
	LineNumberArea(TextEdit *editor) : QWidget(editor) {
		codeEditor = editor;
	}

	QSize sizeHint() const override {
		return QSize(codeEditor->lineNumberAreaWidth(), 0);
	}

protected:
	void paintEvent(QPaintEvent *event) override {
		codeEditor->lineNumberAreaPaintEvent(event);
	}

private:
	TextEdit *codeEditor;
};