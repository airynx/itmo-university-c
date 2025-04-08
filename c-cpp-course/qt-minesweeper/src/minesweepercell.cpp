#include "include/minesweepercell.h"

MinesweeperCell::MinesweeperCell(int row, int column, QWidget *parent) : SquareButton(parent), row(row), column(column)
{
}

bool MinesweeperCell::open()
{
	if (hasMine)
	{
		this->setStyleSheet(getBombStyle(this->size().width()));
	}

	isEnabled = false;
	return hasMine;
}
void MinesweeperCell::mousePressEvent(QMouseEvent *mouseEvent)
{
	emit pressed(row, column, mouseEvent->button());
}

void MinesweeperCell::mouseReleaseEvent(QMouseEvent *mouseEvent)
{
	emit released(row, column, mouseEvent->button());
}

void MinesweeperCell::enterEvent(QEvent *event)
{
	QPushButton::enterEvent(event);
	cursorInBox = true;
}

void MinesweeperCell::leaveEvent(QEvent *event)
{
	QPushButton::leaveEvent(event);
	cursorInBox = false;
}
