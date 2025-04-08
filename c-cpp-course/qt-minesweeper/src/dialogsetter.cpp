#include "include/dialogsetter.h"

DialogSetter::DialogSetter(int lang, QWidget *parent) : QDialog(parent), langCode(lang)
{
	this->setWindowTitle("Touhou Minesweeper");
	QFormLayout *formLayout = new QFormLayout();
	QVBoxLayout *verticalLayout = new QVBoxLayout();
	QHBoxLayout *mainLayout = new QHBoxLayout();

	welcomeLabel = new QLabel("Welcome to Touhou Minesweeper!", this);
    QFontDatabase::addApplicationFont("tohofont.ttf");
	fonts[0] = new QFont("Helvetica", 10);
	fonts[1] = new QFont("Helvetica", 12);
	fonts[2] = new QFont("DFPPOPCorn-W12", 10);
	fonts[3] = new QFont("DFPPOPCorn-W12", 12);
	fonts[4] = new QFont("Meiryo UI", 14);
	fonts[5] = new QFont("Meiryo UI", 12);
	for (QFont *font : fonts)
	{
		font->setBold(true);
	}

	formLayout->addRow(welcomeLabel);

	rowsLabel = new QLabel();
	rowsLine = new QLineEdit();
	formLayout->addRow(rowsLabel, rowsLine);

	columnsLabel = new QLabel();
	columnsLine = new QLineEdit();
	formLayout->addRow(columnsLabel, columnsLine);

	minesLabel = new QLabel();
	minesLine = new QLineEdit();
	formLayout->addRow(minesLabel, minesLine);

	verticalLayout->addLayout(formLayout);
	confirmButton = new QPushButton();
	confirmButton->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

	connect(confirmButton, &QPushButton::clicked, this, &DialogSetter::validateSettings);
	verticalLayout->addWidget(confirmButton);

	QLabel *welcomeImageLabel = new QLabel();
    QPixmap pixmap(":funnyFlagHoldingCharacter.png");

	QPixmap scaledPixmap = pixmap.scaled(170, 170, Qt::KeepAspectRatio, Qt::SmoothTransformation);
	welcomeImageLabel->setPixmap(scaledPixmap);

	mainLayout->addLayout(verticalLayout);

	mainLayout->addWidget(welcomeImageLabel);

	QHBoxLayout *languageLayout = new QHBoxLayout();
	QPushButton *engButton = new QPushButton("English");
	QPushButton *ruButton = new QPushButton("Русский");
	QPushButton *jpButton = new QPushButton("日本語");
	engButton->setFont(*fonts[0]);
	ruButton->setFont(*fonts[0]);
	jpButton->setFont(*fonts[0]);

	connect(ruButton, &QPushButton::clicked, this, [this]() { changeLang(0); });
	connect(engButton, &QPushButton::clicked, this, [this]() { changeLang(1); });
	connect(jpButton, &QPushButton::clicked, this, [this]() { changeLang(2); });

	languageLayout->addWidget(engButton);
	languageLayout->addWidget(ruButton);
	languageLayout->addWidget(jpButton);

	verticalLayout->addLayout(languageLayout);

	setLayout(mainLayout);

	changeLang(langCode);
}
void DialogSetter::changeLang(int mode)
{
	rowsLabel->setText(*labels[mode][0]);
	columnsLabel->setText(*labels[mode][1]);
	minesLabel->setText(*labels[mode][2]);
	welcomeLabel->setText(*labels[mode][3]);
	confirmButton->setText(*labels[mode][4]);

	rowsLabel->setFont(*fonts[mode * 2]);
	columnsLabel->setFont(*fonts[mode * 2]);
	minesLabel->setFont(*fonts[mode * 2]);
	welcomeLabel->setFont(*fonts[mode * 2 + 1]);
	confirmButton->setFont(*fonts[mode * 2 + 1]);

	langCode = mode;
	adjustSize();
	setFixedSize(sizeHint());
}

int DialogSetter::getRows() const
{
	return rows;
}
int DialogSetter::getColumns() const
{
	return columns;
}
int DialogSetter::getMines() const
{
	return mines;
}

int DialogSetter::getLang() const
{
	return langCode;
}

void DialogSetter::validateSettings()
{
	bool rowsCheck, columnsCheck, minesCheck;
	QString rowsText = rowsLine->text();
	QString columnsText = columnsLine->text();
	QString minesText = minesLine->text();

	rows = rowsText.isEmpty() ? 10 : rowsText.toInt(&rowsCheck);
	columns = columnsText.isEmpty() ? 10 : columnsText.toInt(&columnsCheck);
	int minesToInt = minesText.toInt(&minesCheck);
	mines = minesText.isEmpty() ? rows * columns * 0.1 : minesToInt;
	if ((!minesText.isEmpty() && minesToInt < 1) || rows <= 4 || columns <= 4 || rows > 25 || columns > 25 || mines + 1 >= rows * columns)
	{
		setWarning(*warningLabels[langCode][0], *warningLabels[langCode][1]);
		return;
	}
	if (!mines)
		mines = 1;

	wasWarning = false;
	QDialog::accept();
}

void DialogSetter::setWarning(const QString &title, const QString &msg)
{
	QMessageBox warningMsgBox;
	warningMsgBox.setIcon(QMessageBox::Warning);
	warningMsgBox.setWindowTitle(title);
	warningMsgBox.setText(msg);

	warningMsgBox.exec();
}
DialogSetter::~DialogSetter()
{
	for (int i = 0; i < 3; i++)
	{
		for (int j = 0; j < 5; j++)
		{
			delete labels[i][j];
		}
		for (int j = 0; j < 2; j++)
		{
			delete warningLabels[i][j];
		}
	}
}
