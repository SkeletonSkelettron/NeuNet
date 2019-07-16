#include "NeuNet.h"
#include <QtWidgets/QApplication>

int main(int argc, char *argv[])
{
	QApplication a(argc, argv);
	NeuNet w;
	w.show();
	return a.exec();
}
