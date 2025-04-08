#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "dialogsetter.h"
#include "minesweepercell.h"
#include "textalignableqlabel.h"

#include <QApplication>
#include <QCheckBox>
#include <QCloseEvent>
#include <QCoreApplication>
#include <QCursor>
#include <QDir>
#include <QFileInfo>
#include <QFont>
#include <QFontDatabase>
#include <QGridLayout>
#include <QMainWindow>
#include <QMenu>
#include <QPropertyAnimation>
#include <QSettings>
#include <QSizePolicy>
#include <QStatusBar>
#include <QStringList>
#include <QVBoxLayout>
#include <QVector>
#include <QWidget>

class MainWindow : public QMainWindow
{
	Q_OBJECT

  public:
	MainWindow(bool isDebug, QWidget* parent = nullptr);
	bool getDialogQuitted() { return dialogQuitted; }
	~MainWindow();
  public slots:
	void processDebug(int st);

  protected:
	void resizeEvent(QResizeEvent* event) override;
	void closeEvent(QCloseEvent* event) override;

  signals:
	void cellsLeftChanged(int cellsLeft);
  private slots:
	void processClick(int row, int column, Qt::MouseButton mouseButton);
	void processRelease(int row, int column, Qt::MouseButton mouseButton);
	void updCellsLeft(int cellsLeft);

  private:
	QWidget* widget;
	QMenu* menu = nullptr;
	QMenu* languageMenu = nullptr;
	QCheckBox* dbgCheckbox = nullptr;
	QGridLayout* layout;
	QPushButton* optionsButton = nullptr;
	TextAlignableQLabel* cellsLeftLabel = nullptr;
	QVector< QVector< MinesweeperCell* > > cells;
	QString settingsPath = QCoreApplication::applicationDirPath() + QDir::separator() + "settings.ini";
	QSettings* settings;

	QAction* restartGameAction;
	QAction* newGameAction;
	QAction* quitAction;
	QAction* rusAction;
	QAction* engAction;
	QAction* nihonAction;

	QAction* langActions[3];

	QFont* fontLittle;
	QFont* font;
	QFont* fontBold;
	QFont* fonts[6];
	QString overTitles[3] = { "Игра окончена!", "Game is over!", "ゲームオーバー" };
	QString winBanners[3] = {
		"<div style='text-align: center;'>"
		"Ура! :) Вы победили!<br></br>"
		"Хотите сыграть еще раз?</div>",

		"<div style='text-align: center;'>"
		"Congratulations! :) You win!<br></br>"
		"Do you want to play one more time?</div>",

		"<div style='text-align: center;'>"
		"おめでとう！ :) あなたは勝ちました！<br></br>"
		"もう一度プレイしますか？</div>"
	};

	QString loseBanners[3] = {
		"<div style='text-align: center;'>"
		"Очень жаль, вы проиграли :(<br></br>"
		"Хотите сыграть еще раз?</div>",

		"<div style='text-align: center;'>"
		"You've lost the game :(<br></br>"
		"Do you want to play one more time?</div",

		"<div style='text-align: center;'>"
		"ゲームに負けてしまいました :(<br></br>"
		"もう一度プレイしますか？</div>"
	};

	QString loseBanner =
		"<div style='text-align: center;'>"
		"You've lost the game :(<br></br>"
		"Do you want to play one more time?</div>";

	QString* menuLabels[3][4] = {
		{ new QString("Начать сначала"), new QString("Новая игра"), new QString("Язык"), new QString("Выход") },
		{ new QString("Restart game"), new QString("New game"), new QString("Language"), new QString("Quit") },
		{ new QString("ゲームを再起動"),

		  new QString("新しいゲーム"),

		  new QString("言語"),

		  new QString("終了")

		}
	};

	int rows, columns, mines, cellsLeft, minimumCellSize = 40, lang = 1;
	bool firstTap = true, lose = false, loadingState = false, dialogQuitted = false;
	double aspectRatio;

	void changeLang(int langCode);

	void colorAllCells(bool toColor);
	void saveGame();
	void deploySavedMap(QGridLayout* layout);
	bool checkLoadedProperties();
	int encodeCell(MinesweeperCell* cell);
	void setWindowSize();
	void checkForTextAlignment(QPushButton* button);

	void startGame(bool firstTime = true);
	void closeAndStart(bool setFirstTime = false);

	void initCellsLeftLabel();
	void initOptionsButton();
	void initMenu();
	void buildMap(int rows, int columns, bool firstTime = true, bool loadMap = false);
	void placeMines(int rowNotToPlace, int columnNotToPlace);

	void openingAction(int row, int column);
	void automaticOpen(int row, int column);
	void fastOpen(int row, int column);
	int flagsCheck(int row, int column);
	bool checkForBounds(int row, int column);

	bool openCell(int row, int column, bool notCountLeft = false);
	void setFlag(int row, int column);
	void setCellSize();
	int countAround(int row, int column);

	void gameOver(int row, int column);
	void quitMsg(bool isWin);

	void setCellsStyleAround(int row, int column, bool defaultStyle);
	void setCountArounds();
	void setCellsLeft(int newCellsValue);
};
#endif	  // MAINWINDOW_H
