#include "include/textalignableqlabel.h"

TextAlignableQLabel::TextAlignableQLabel(QWidget* parent) : QLabel(parent) {}

void TextAlignableQLabel::resizeEvent(QResizeEvent* event)
{
	QLabel::resizeEvent(event);
	int size = qMin(event->size().width(), event->size().height());
	resize(size, size);
	if (!text().isEmpty())
	{
		int size = this->size().width();
		QFont font = this->font();
		font.setPointSize(size / 3);
		this->setFont(font);
	}
}
