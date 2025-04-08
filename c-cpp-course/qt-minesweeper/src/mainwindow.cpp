#include "include/mainwindow.h"

MainWindow::MainWindow(bool isDebug, QWidget* parent) : QMainWindow(parent)
{
    QFontDatabase::addApplicationFont("tohofont.ttf");
	fontLittle = new QFont("DFPPOPCorn-W12", 10);
	font = new QFont("DFPPOPCorn-W12", 14);
	fontBold = new QFont("DFPPOPCorn-W12", 12);
	fontBold->setBold(true);

	fonts[0] = new QFont("Helvetica", 10);
	fonts[1] = new QFont("Helvetica", 12);
	fonts[2] = new QFont("DFPPOPCorn-W12", 12);
	fonts[3] = new QFont("DFPPOPCorn-W12", 14);
	fonts[4] = new QFont("Meiryo UI", 14);
	fonts[5] = new QFont("Meiryo UI", 12);

	for (int i = 0; i < 6; i++)
	{
		fonts[i]->setBold(true);
	}

	settings = new QSettings(settingsPath, QSettings::IniFormat);

	if (isDebug)
	{
		QStatusBar* statusBar = new QStatusBar(this);
		this->setStatusBar(statusBar);
		dbgCheckbox = new QCheckBox("DEBUG MODE");
		dbgCheckbox->setStyleSheet("QCheckBox { font-weight: bold; }");
		statusBar->addWidget(dbgCheckbox, 1);
		connect(dbgCheckbox, &QCheckBox::stateChanged, this, &MainWindow::processDebug);
	}

    QIcon icon(":/bomb.png");
	setWindowIcon(icon);
    this->setStyleSheet("MainWindow { background-image: url(:/mainBackground.png); background-repeat: no-repeat; "
						"background-position: center;}");

	QFileInfo settingsFile(settingsPath);
	if (settingsFile.size() == 0)
	{
		startGame();
	}
	else
	{
		settings->beginGroup("Properties");
		rows = settings->value("Rows", 0).toInt();
		columns = settings->value("Columns", 0).toInt();
		mines = settings->value("Mines", 0).toInt();
		cellsLeft = settings->value("Left", 0).toInt();
		lang = settings->value("Language", 1).toInt();
		if (lang < 0 || lang > 2)
			lang = 1;
		if (cellsLeft != rows * columns - mines)
			loadingState = true;
		settings->endGroup();
		bool checkFlag = checkLoadedProperties();
		if (!checkFlag)
		{
			settings->clear();
			loadingState = false;
			startGame();
		}
		else
			buildMap(rows, columns, true, true);
	}
}

bool MainWindow::checkLoadedProperties()
{
	bool checkFlag = true;
	settings->beginGroup("Cells");

	if (mines >= 1 && rows >= 5 && columns >= 5 && rows <= 25 && columns <= 25 && mines + 1 < rows * columns)
	{
		int minesCheck = 0;
		int disabCheck = 0;
		for (int row = 0; row < rows; ++row)
		{
			if (!checkFlag)
				break;
			for (int col = 0; col < columns; ++col)
			{
				QString key = QString("%1,%2").arg(row).arg(col);

				if (!settings->contains(key))
				{
					checkFlag = false;
					break;
				}

				QString combinedValue = settings->value(key).toString();
				QStringList values = combinedValue.split(':');

				if (values.size() != 2)
				{
					checkFlag = false;
					break;
				}

				bool intCellCode = false;
				bool intFlagCode = false;

				int encodedCell = values.at(0).toInt(&intCellCode);
				int hasFlag = values.at(1).toInt(&intFlagCode);

				if (!(intCellCode && intFlagCode && (encodedCell == -1 || encodedCell == 0 || encodedCell == 1) &&
					  (hasFlag == 0 || hasFlag == 1)))
				{
					checkFlag = false;
					break;
				}
				if (encodedCell == -1)
					minesCheck++;
				if (encodedCell == 0)
					disabCheck++;
			}
		}
		if (mines + 1 >= rows * columns || (minesCheck != mines && minesCheck != 0) || rows * columns - mines - disabCheck != cellsLeft)
			checkFlag = false;
	}
	else
		checkFlag = false;

	settings->endGroup();
	return checkFlag;
}
void MainWindow::setWindowSize()
{
	int width = this->width();
	int height = this->height();
	int maxdim = (rows <= 7 || rows <= 7) ? 750 : 1000;
	if (width > height)
		setMaximumSize(maxdim, maxdim / aspectRatio);
	else
		setMaximumSize(maxdim * aspectRatio, maxdim);
}
void MainWindow::processDebug(int st)
{
	colorAllCells(st == Qt::Checked);
}

void MainWindow::checkForTextAlignment(QPushButton* button)
{
	if (!button->text().isEmpty())
	{
		QFont font = button->font();
		font.setPointSize(button->size().width() / 3);
		button->setFont(font);
	}
}

void MainWindow::colorAllCells(bool toColor)
{
	for (int i = 0; i < rows; ++i)
	{
		for (int j = 0; j < columns; ++j)
		{
			MinesweeperCell* cell = cells[i][j];
			if (cell->getIsEnabled() && !cell->getHasFlag())
			{
				if (toColor)
					cell->setStyleSheet(cell->getHasMine() ? cell->getBombStyle(minimumCellSize) : cell->getDefaultStyle(minimumCellSize));
				else
					cell->setStyleSheet(cell->getDefaultStyle(minimumCellSize));
			}
		}
	}
}

void MainWindow::startGame(bool firstTime)
{
	if (firstTime)
	{
		DialogSetter dialogSetter(lang, this);
		if (dialogSetter.exec() == QDialog::Accepted)
		{
			rows = dialogSetter.getRows();
			columns = dialogSetter.getColumns();
			mines = dialogSetter.getMines();
			lang = dialogSetter.getLang();
		}
		else
		{
			dialogQuitted = true;
			close();
			return;
		}
	}

	lose = false;
	firstTap = true;
	cellsLeft = rows * columns - mines;
	buildMap(rows, columns);
}
void MainWindow::closeAndStart(bool setFirstTime)
{
	startGame(setFirstTime);
}
void MainWindow::deploySavedMap(QGridLayout* layout)
{
	settings->beginGroup("Cells");

	for (int row = 0; row < rows; row++)
	{
		cells[row].resize(columns);

		for (int col = 0; col < columns; col++)
		{
			MinesweeperCell* cell = new MinesweeperCell(row, col, this);
			cell->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
			connect(cell, &MinesweeperCell::pressed, this, &MainWindow::processClick);
			connect(cell, &MinesweeperCell::released, this, &MainWindow::processRelease);
			layout->addWidget(cell, row + 1, col);
			cells[row][col] = cell;
			QString key = QString("%1,%2").arg(row).arg(col);
			QString combinedValue = settings->value(key).toString();

			QStringList values = combinedValue.split(':');

			int encodedCell = values.at(0).toInt();
			int hasFlag = values.at(1).toInt();

			if (encodedCell == -1)
			{
				cell->setHasMine(true);
				cell->setStyleSheet(cell->getDefaultStyle(minimumCellSize));
			}
			if (encodedCell == 1)
				cell->setStyleSheet(cell->getDefaultStyle(minimumCellSize));
			if (hasFlag)
				setFlag(row, col);
		}
	}
	if (cellsLeft == rows * columns - mines)
	{
		firstTap = true;
		return;
	}
	setCountArounds();

	for (int row = 0; row < rows; row++)
	{
		cells[row].resize(columns);
		for (int col = 0; col < columns; col++)
		{
			QString key = QString("%1,%2").arg(row).arg(col);
			QString combinedValue = settings->value(key).toString();

			QStringList values = combinedValue.split(':');

			int encodedCell = values.at(0).toInt();

			if (encodedCell == 0 && cells[row][col]->getIsEnabled())
				openingAction(row, col);
		}
	}
	settings->endGroup();
	updCellsLeft(cellsLeft);
	loadingState = false;
}

void MainWindow::buildMap(int rows, int columns, bool firstTime, bool loadMap)
{
	QWidget* widget = new QWidget(this);
	QVBoxLayout* mainLayout = new QVBoxLayout(widget);
	QGridLayout* layout = new QGridLayout();

	setCentralWidget(widget);
	setCellSize();

	layout->setSpacing(0);

	initCellsLeftLabel();
	initOptionsButton();

	layout->addWidget(optionsButton, 0, 0);
	layout->addWidget(cellsLeftLabel, 0, columns - 1);

	cells.resize(rows);
	connect(this, &MainWindow::cellsLeftChanged, this, &MainWindow::updCellsLeft);
	updCellsLeft(cellsLeft);
	if (loadMap)
	{
		firstTap = false;
		deploySavedMap(layout);
	}
	else
	{
		for (int row = 0; row < rows; row++)
		{
			cells[row].resize(columns);
			for (int col = 0; col < columns; col++)
			{
				MinesweeperCell* cell = new MinesweeperCell(row, col, this);
				cell->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
				connect(cell, &MinesweeperCell::pressed, this, &MainWindow::processClick);
				connect(cell, &MinesweeperCell::released, this, &MainWindow::processRelease);
				layout->addWidget(cell, row + 1, col);
				cells[row][col] = cell;
				cell->setStyleSheet(cell->getDefaultStyle(minimumCellSize));
			}
		}
	}

	initMenu();
	optionsButton->setMenu(menu);
	mainLayout->addLayout(layout);

	if (firstTime)
		adjustSize();

	aspectRatio = static_cast< double >(width()) / height();
	setWindowSize();
}

void MainWindow::initCellsLeftLabel()
{
	if (cellsLeftLabel != nullptr)
		return;

	cellsLeftLabel = new TextAlignableQLabel();
	cellsLeftLabel->setFont(*font);
	cellsLeftLabel->setStyleSheet(
		QString(
			"QLabel"
			"{"
            "    border-image: url(:/leftLabel.png) -0.5 -0.5 -0.5 -0.5 no-repeat stretch stretch;"
			"    border: 0.3 solid gray;"
			"    border-radius: 30px;"
			"    min-width: %1px;"
			"    min-height: %1px;"
			"}")
			.arg(minimumCellSize));
}

void MainWindow::initOptionsButton()
{
	if (optionsButton != nullptr)
		return;
	optionsButton = new SquareButton();

	optionsButton->setStyleSheet(
		QString(
			"QPushButton"
			"{"
            "    border-image: url(:/helpingStone.png) -0.5 -0.5 -0.5 -0.5 no-repeat stretch stretch;"
			"    border: 0.3 solid gray;"
			"    border-radius: 17px;"
			"    min-width: %1px;"
			"    min-height: %1px;"
			"}"
			"QPushButton:pressed {"
            "    border-image: url(:/helpingStoneTransparent.png) -0.5 -0.5 -0.5 -0.5 no-repeat stretch stretch;"
			"}"
			"QPushButton::menu-indicator { width: 0px; };")
			.arg(minimumCellSize));

	optionsButton->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
	optionsButton->resize(minimumCellSize, minimumCellSize);
}
void MainWindow::initMenu()
{
	menu = new QMenu();

	restartGameAction = new QAction(*menuLabels[lang][0], this);

	newGameAction = new QAction(*menuLabels[lang][1], this);

	languageMenu = new QMenu(*menuLabels[lang][2], this);

	quitAction = new QAction(*menuLabels[lang][3], this);

	rusAction = new QAction("Русский", this);
	rusAction->setCheckable(true);
	engAction = new QAction("English", this);
	engAction->setCheckable(true);
	nihonAction = new QAction("日本語", this);
	nihonAction->setCheckable(true);
	langActions[0] = rusAction;
	langActions[1] = engAction;
	langActions[2] = nihonAction;

	connect(rusAction, &QAction::triggered, this, [this]() { changeLang(0); });
	connect(engAction, &QAction::triggered, this, [this]() { changeLang(1); });
	connect(nihonAction, &QAction::triggered, this, [this]() { changeLang(2); });

	switch (lang)
	{
	case 0:
		rusAction->setChecked(true);
		break;
	case 1:
		engAction->setChecked(true);
		break;
	case 2:
		nihonAction->setChecked(true);
		break;
	}

	languageMenu->addAction(rusAction);
	languageMenu->addAction(engAction);
	languageMenu->addAction(nihonAction);

	menu->addAction(restartGameAction);
	menu->addAction(newGameAction);
	menu->addMenu(languageMenu);
	menu->addAction(quitAction);

	connect(restartGameAction, &QAction::triggered, this, [this]() { closeAndStart(false); });
	connect(newGameAction, &QAction::triggered, this, [this]() { closeAndStart(true); });
	connect(quitAction, &QAction::triggered, this, &MainWindow::close);
}
void MainWindow::changeLang(int langCode)
{
	lang = langCode;
	for (int i = 0; i <= 2; i++)
	{
		if (i != langCode)
			langActions[i]->setChecked(false);
	}

	restartGameAction->setText(*menuLabels[lang][0]);
	newGameAction->setText(*menuLabels[lang][1]);
	languageMenu->setTitle(*menuLabels[lang][2]);
	quitAction->setText(*menuLabels[lang][3]);
}
void MainWindow::setCellSize()
{
	if (rows <= 7 || columns <= 7)
	{
		minimumCellSize = 48;
		font->setPointSize(14);
	}
	if (rows >= 17 || columns >= 17)
	{
		minimumCellSize = 35;
		font->setPointSize(10);
	}
}

void MainWindow::placeMines(int rowNotToPlace, int columnNotToPlace)
{
	int minesLeft = mines;
	int genRow, genColumn;
	while (minesLeft)
	{
		genRow = rand() % rows;
		genColumn = rand() % columns;

		if (cells[genRow][genColumn]->getHasMine() || (genRow == rowNotToPlace && genColumn == columnNotToPlace))
		{
			continue;
		}

		cells[genRow][genColumn]->setHasMine(true);
		minesLeft--;
	}
}

int MainWindow::countAround(int row, int column)
{
	int counter = 0;

	for (int rowsCheck = -1; rowsCheck <= 1; rowsCheck++)
	{
		for (int columnsCheck = -1; columnsCheck <= 1; columnsCheck++)
		{
			int rowToCheck = row + rowsCheck;
			int columnToCheck = column + columnsCheck;

			if (rowToCheck >= 0 && rowToCheck < rows && columnToCheck >= 0 && columnToCheck < columns &&
				cells[rowToCheck][columnToCheck]->getHasMine())
			{
				counter++;
			}
		}
	}

	return counter;
}

void MainWindow::setCountArounds()
{
	for (int row = 0; row < rows; row++)
	{
		for (int col = 0; col < columns; col++)
			cells[row][col]->setCountAround(countAround(row, col));
	}
}

void MainWindow::setCellsLeft(int newCellsValue)
{
	if (cellsLeft != newCellsValue)
	{
		cellsLeft = newCellsValue;
		emit cellsLeftChanged(cellsLeft);
	}
}

void MainWindow::updCellsLeft(int cellsLeft)
{
	cellsLeftLabel->setAlignment(Qt::AlignHCenter | Qt::AlignVCenter);
	cellsLeftLabel->setText(QString("%1").arg(cellsLeft));
}

void MainWindow::automaticOpen(int row, int column)
{
	for (int i = row - 1; i <= row + 1; i++)
	{
		for (int j = column - 1; j <= column + 1; j++)
		{
			if (!(i == row && j == column) && i >= 0 && i < rows && j >= 0 && j < columns && cells[i][j]->getIsEnabled())
				openingAction(i, j);
			if (lose)
				return;
		}
	}
}
bool MainWindow::openCell(int row, int column, bool notCountLeft)
{
	MinesweeperCell* cell = cells[row][column];

	bool curEnabled = cell->getIsEnabled();
	bool isLost = cell->open();

	int aroundMines = cell->getCountAround();
	if (!notCountLeft && !isLost && curEnabled && !loadingState)
		setCellsLeft(cellsLeft - 1);
	if (!cell->getHasMine())
	{
		cell->setStyleSheet(cell->getTransparentStyle(minimumCellSize));
		if (aroundMines)
		{
			cell->setText(QString::number(aroundMines));

			cell->setFont(*fontBold);
			checkForTextAlignment(cell);
		}
		else if (!notCountLeft)
		{
			automaticOpen(row, column);
		}
	}
	return isLost;
}

void MainWindow::openingAction(int row, int column)
{
	if (firstTap)
	{
		placeMines(row, column);
		if (dbgCheckbox)
			processDebug(dbgCheckbox->checkState());
		setCountArounds();
		firstTap = false;
	}
	lose = openCell(row, column);
	if (lose)
	{
		gameOver(row, column);
		return;
	}
	else if (!cellsLeft)
		quitMsg(true);
}
void MainWindow::setCellsStyleAround(int row, int column, bool defaultStyle)
{
	for (int i = row - 1; i <= row + 1; ++i)
	{
		for (int j = column - 1; j <= column + 1; ++j)
		{
			if (i >= 0 && i < rows && j >= 0 && j < columns)
			{
				MinesweeperCell* cell = cells[i][j];
				if (!cell->getHasFlag())
				{
					if (cell->getHasMine() && dbgCheckbox && dbgCheckbox->isChecked())
					{
						cell->setStyleSheet(cell->getBombStyle(minimumCellSize));
						continue;
					}
					if (cell->getIsEnabled())
					{
						cell->setStyleSheet(defaultStyle ? (cell->getDefaultStyle(minimumCellSize)) : cell->getTransparentStyle(minimumCellSize));
						checkForTextAlignment(cell);
					}
				}
			}
		}
	}
}

int MainWindow::flagsCheck(int row, int column)
{
	int flagsOnMinesCounter = 0;
	for (int rowsCheck = -1; rowsCheck <= 1; rowsCheck++)
	{
		for (int columnsCheck = -1; columnsCheck <= 1; columnsCheck++)
		{
			int rowToCheck = row + rowsCheck;
			int columnToCheck = column + columnsCheck;
			if (rowToCheck >= 0 && rowToCheck < rows && columnToCheck >= 0 && columnToCheck < columns)
			{
				MinesweeperCell* cell = cells[rowToCheck][columnToCheck];
				if (cell->getHasFlag() && cell->getHasMine())
					flagsOnMinesCounter++;
			}
		}
	}
	return flagsOnMinesCounter;
}
void MainWindow::fastOpen(int row, int column)
{
	for (int stepRow = -1; stepRow <= 1; stepRow++)
	{
		for (int stepCol = -1; stepCol <= 1; stepCol++)
		{
			int rowOpen = row + stepRow;
			int columnOpen = column + stepCol;
			if (rowOpen >= 0 && rowOpen < rows && columnOpen >= 0 && columnOpen < columns &&
				!cells[rowOpen][columnOpen]->getHasFlag() && cells[rowOpen][columnOpen]->getIsEnabled())
			{
				openingAction(rowOpen, columnOpen);
				if (lose)
					return;
			}
		}
	}
}
void MainWindow::setFlag(int row, int column)
{
	MinesweeperCell* cell = cells[row][column];
	if (!cell->getIsEnabled())
		return;

	if (cell->getHasFlag())
	{
		if (cell->getHasMine() && dbgCheckbox && dbgCheckbox->isChecked())
			cell->setStyleSheet(cell->getBombStyle());
		else
			cell->setStyleSheet(cell->getDefaultStyle(minimumCellSize));
	}
	else
		cell->setStyleSheet(cell->getFlagStyle(minimumCellSize));

	cell->setHasFlag(!cell->getHasFlag());
}
void MainWindow::gameOver(int row, int column)
{
	for (int row = 0; row < rows; row++)
	{
		for (int col = 0; col < columns; col++)
		{
			if (cells[row][col])
				openCell(row, col, true);
		}
	}
	cells[row][column]->setStyleSheet(cells[row][column]->getBangedBombStyle(minimumCellSize));
	quitMsg(false);
}

void MainWindow::quitMsg(bool isWin)
{
	QMessageBox msgBox(this);
	msgBox.setWindowTitle(overTitles[lang]);

	msgBox.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
	msgBox.setDefaultButton(QMessageBox::No);

	msgBox.setStyleSheet("QMessageBox {text-align: center;}");

	msgBox.setText(isWin ? winBanners[lang] : loseBanners[lang]);
	msgBox.setFont(*fonts[lang + 1]);

	settings->clear();
	int result = msgBox.exec();
	if (result == QMessageBox::No)
	{
		QApplication::quit();
	}
	else
	{
		lose = false;
		firstTap = true;
		cellsLeft = rows * columns - mines;

		buildMap(rows, columns, false);
	}
}
void MainWindow::resizeEvent(QResizeEvent* event)
{
	QMainWindow::resizeEvent(event);

	int newWidth = event->size().width();
	int newHeight = newWidth / aspectRatio;
	resize(newWidth, newHeight);
}

void MainWindow::processClick(int row, int col, Qt::MouseButton mouseButton)
{
	MinesweeperCell* cell = cells[row][col];
	if (!cell->getHasFlag())
	{
		if (mouseButton == Qt::LeftButton && cell->getIsEnabled())
			cell->setStyleSheet(cell->getTransparentStyle(minimumCellSize));

		if (mouseButton == Qt::MiddleButton)
		{
			if (cell->getIsEnabled())
				cell->setStyleSheet(cell->getTransparentStyle(minimumCellSize));
			else if (cell->getCountAround() != 0)
				setCellsStyleAround(row, col, false);
		}
	}

	if (mouseButton == Qt::RightButton)
		setFlag(row, col);
}

void MainWindow::processRelease(int row, int col, Qt::MouseButton mouseButton)
{
	MinesweeperCell* cell = cells[row][col];
	if (cell->getHasFlag())
		return;

	if (mouseButton == Qt::LeftButton)
	{
		if (cell->getIsEnabled())
		{
			if (cell->rect().contains(cell->mapFromGlobal(QCursor::pos())))
				openingAction(row, col);
			else
				setCellsStyleAround(row, col, true);
		}
	}

	if (mouseButton == Qt::MiddleButton)
	{
		if (cell->rect().contains(cell->mapFromGlobal(QCursor::pos())))
		{
			if (cell->getIsEnabled())
				openingAction(row, col);
			else
			{
				if (cell->getCountAround() != 0)
				{
					if (flagsCheck(row, col) == (cell->getCountAround()))
						fastOpen(row, col);
					else
						setCellsStyleAround(row, col, true);
				}
			}
		}
		else
			setCellsStyleAround(row, col, true);
	}
}

void MainWindow::closeEvent(QCloseEvent* event)
{
	if (!dialogQuitted)
		saveGame();
	QMainWindow::closeEvent(event);
}
int MainWindow::encodeCell(MinesweeperCell* cell)
{
	// -1 - bomb
	// 1 - enab
	// 0 - disab
	if (cell->getHasMine())
		return -1;
	if (cell->getIsEnabled())
		return 1;
	else
		return 0;
}
void MainWindow::saveGame()
{
	settings->clear();
	settings->beginGroup("Properties");
	settings->setValue("Rows", rows);
	settings->setValue("Columns", columns);
	settings->setValue("Mines", mines);
	settings->setValue("Left", cellsLeft);
	settings->setValue("Language", lang);
	settings->endGroup();

	settings->beginGroup("Cells");
	settings->remove("");
	settings->endGroup();

	settings->beginGroup("Cells");
	for (int row = 0; row < rows; row++)
	{
		for (int col = 0; col < columns; col++)
		{
			MinesweeperCell* cell = cells[row][col];
			QString key = QString("%1,%2").arg(row).arg(col);

			QString combinedValue = QString("%1:%2").arg(encodeCell(cells[row][col])).arg(cell->getHasFlag() ? 1 : 0);

			settings->setValue(key, combinedValue);
		}
	}

	settings->endGroup();
}
MainWindow::~MainWindow() {}
