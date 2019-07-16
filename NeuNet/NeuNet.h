#pragma once

#include <QtWidgets/QMainWindow>
#include "ui_NeuNet.h"
#include "highlighter.h"
#include "treemodelcompleter.h"
#include "NeuralNetwork.h"

QT_BEGIN_NAMESPACE
class QAbstractItemModel;
class QComboBox;
class QCompleter;
class QLabel;
class QLineEdit;
class QProgressBar;
QT_END_NAMESPACE
class TextEdit;

class NeuNet : public QMainWindow
{
	Q_OBJECT

public:
	NeuNet(QWidget *parent = Q_NULLPTR);

private:
	Ui::NeuNetClass ui;
	void setupEditor();
	Highlighter *highlighter;
	TreeModelCompleter *completer;
	TextEdit *completingTextEdit;
	template<typename T> void initNeUnetFromJson(NeuralNetwork<T> &neuralNetwork);
	QAbstractItemModel *modelFromFile(const QString& fileName);
	void updateContentsLabel(const QString&);
	QLabel *contentsLabel;
	QLineEdit *lineEdit;
private slots:
	void on_pushButton_clicked();

	void on_pushButton_2_clicked();
};
