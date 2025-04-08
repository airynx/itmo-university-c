#ifndef MINESWEEPERCELL_H
#define MINESWEEPERCELL_H

#include "squarebutton.h"

#include <QIcon>
#include <QMouseEvent>
#include <QString>

class MinesweeperCell : public SquareButton
{
	Q_OBJECT

  public:
	explicit MinesweeperCell(int row, int column, QWidget* parent = nullptr);

	bool open();
	bool cursorInBox;

	int getCountAround() { return countAround; }
	void setCountAround(int countAround) { this->countAround = countAround; }

	void setHasMine(bool value) { hasMine = value; }
	bool getHasMine() { return hasMine; }

	void setHasFlag(bool value) { hasFlag = value; }
	bool getHasFlag() { return hasFlag; }

	void setIsEnabled(bool value) { isEnabled = value; }
	bool getIsEnabled() { return isEnabled; }

	void setIsReleased(bool value) { isReleased = value; }
	bool getIsReleased() { return isReleased; }

	QString getBombStyle(int sizePx = 40)
	{
		return QString("QPushButton {"
                       "    border-image: url(:/bomb.png) -0.5 -0.5 -0.5 -0.5 no-repeat stretch stretch;"
					   "    border: 0.3 solid gray;"
					   "    border-radius: 17px;"
					   "    min-width: %1px;"
					   "    min-height: %1px;"
					   "}")
			.arg(sizePx);
	}

	QString getBangedBombStyle(int sizePx = 40)
	{
		return QString("QPushButton {"
                       "    border-image: url(:/bangedBomb.png) -0.5 -0.5 -0.5 -0.5 no-repeat stretch stretch;"
					   "    border: 0.3 solid gray;"
					   "    border-radius: 17px;"
					   "    min-width: %1px;"
					   "    min-height: %1px;"
					   "}")
			.arg(sizePx);
	}
	QString getFlagStyle(int sizePx = 40)
	{
		return QString(
				   "QPushButton {"
                   "    border-image: url(:/redStone.png) -0.8 -0.8 -0.8 -0.8 no-repeat stretch stretch;"
				   "    border: 0.3 solid gray;"
				   "    border-radius: 17px;"
				   "    min-width: %1px;"
				   "    min-height: %1px;"
				   "}"
				   "QPushButton:hover {"
				   "    background-color: red;"
				   "}"
				   "QPushButton:pressed {"
				   "    background-color: blue;"
				   "}")
			.arg(sizePx);
	}
	QString getDefaultStyle(int sizePx = 40)
	{
		return QString(
				   "QPushButton {"
                   "    border-image: url(:/framedStone.png) -0.8 -0.8 -0.8 -0.8 no-repeat stretch stretch;"
				   "    border: 0.3 solid gray;"
				   "    border-radius: 17px;"
				   "    min-width: %1px;"
				   "    min-height: %1px;"
				   "}"
				   "QPushButton:hover {"
				   "    background-color: gray;"
				   "}"
				   "QPushButton:pressed {"
				   "    background-color: blue;"
				   "}")
			.arg(sizePx);
	}

	QString getTransparentStyle(int sizePx = 40)
	{
		return QString("QPushButton {"
                       "    border-image: url(:/transparent-framedStone.png) -0.8 -0.8 -0.8 -0.8 no-repeat stretch "
					   "stretch;"
					   "    border: 0.3 solid gray;"
					   "    border-radius: 17px;"
					   "    min-width: %1px;"
					   "    min-height: %1px;"
					   "}")
			.arg(sizePx);
	}

  protected:
	void enterEvent(QEvent* event) override;
	void leaveEvent(QEvent* event) override;

  signals:
	void pressed(int row, int column, Qt::MouseButton mouseButton);
	void released(int row, int column, Qt::MouseButton mouseButton);

  protected:
	void mousePressEvent(QMouseEvent* mouseEvent) override;
	void mouseReleaseEvent(QMouseEvent* mouseEvent) override;

  private:
	bool hasMine = false, hasFlag = false, isEnabled = true, isReleased = true;
	int row, column;
	int countAround;
};
#endif	  // MINESWEEPERCELL_H
