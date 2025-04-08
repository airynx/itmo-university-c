#ifndef SQUAREBUTTON_H
#define SQUAREBUTTON_H

#include <QPushButton>
#include <QResizeEvent>

class SquareButton : public QPushButton
{
	Q_OBJECT

  public:
	explicit SquareButton(QWidget* parent = nullptr);
	explicit SquareButton(QString setString, QWidget* parent = nullptr);

  protected:
	void resizeEvent(QResizeEvent* event) override;
};
#endif	  // SQUAREBUTTON_H
