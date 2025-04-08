#include "include/squarebutton.h"

SquareButton::SquareButton(QWidget* parent) : QPushButton(parent) {}

SquareButton::SquareButton(QString setString, QWidget* parent) : QPushButton(parent)
{
	setText(setString);
}

void SquareButton::resizeEvent(QResizeEvent* event)
{
	QPushButton::resizeEvent(event);
	int size = qMin(event->size().width(), event->size().height());
	resize(size, size);
	if (!text().isEmpty())
	{
		QFont font = this->font();
		font.setPointSize(size / 3);
		this->setFont(font);
	}
}
