#include "NeuNet.h"
#include <QtWidgets/QApplication>

int main(int argc, char *argv[])
{
	Q_INIT_RESOURCE(NeuNetCompleter);
	QApplication a(argc, argv);
	NeuNet w;
	w.show();
	return a.exec();
}
