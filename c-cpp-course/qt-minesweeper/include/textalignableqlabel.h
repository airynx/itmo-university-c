#ifndef TEXTALIGNABLEQLABEL_H
#define TEXTALIGNABLEQLABEL_H

#include <QLabel>
#include <QResizeEvent>
#include <QWidget>

class TextAlignableQLabel : public QLabel
{
	Q_OBJECT

  public:
	explicit TextAlignableQLabel(QWidget* parent = nullptr);

  protected:
	void resizeEvent(QResizeEvent* event) override;
};

#endif	  // TEXTALIGNABLEQLABEL_H
