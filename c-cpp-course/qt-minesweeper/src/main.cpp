#include "include/mainwindow.h"

int main(int argc, char *argv[])
{
	QApplication a(argc, argv);

	MainWindow w(argc > 1 && QString(argv[1]) == "dbg");
	if (!w.getDialogQuitted())
	{
		w.setWindowTitle("Touhou Minesweeper");
		w.show();
		return a.exec();
	}
	else
		return 0;
}
