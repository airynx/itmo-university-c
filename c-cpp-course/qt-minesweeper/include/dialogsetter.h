#ifndef DIALOGSETTER_H
#define DIALOGSETTER_H

#include <QApplication>
#include <QComboBox>
#include <QDialog>
#include <QFontDatabase>
#include <QFormLayout>
#include <QLabel>
#include <QLineEdit>
#include <QMessageBox>
#include <QPixmap>
#include <QPushButton>
#include <QSpacerItem>
#include <QString>
#include <QVBoxLayout>

class DialogSetter : public QDialog
{
	Q_OBJECT

  public:
	DialogSetter(int langCode, QWidget* parent = nullptr);
	~DialogSetter();

	bool getWasWarning() { return wasWarning; };
	int getRows() const;
	int getColumns() const;
	int getMines() const;
	int getLang() const;

  private slots:
	void validateSettings();

  private:
	QLineEdit* rowsLine;
	QLineEdit* columnsLine;
	QLineEdit* minesLine;
	QLabel* welcomeLabel;
	QLabel* rowsLabel;
	QLabel* columnsLabel;
	QLabel* minesLabel;
	QPushButton* confirmButton;
	QFont* fontEn;
	QFont* fontEnBig;
	QFont* fontRu;
	QFont* fontRuBig;
	QFont* fontJp;
	QFont* fontJpBig;
	QFont* fonts[6];
	void setWarning(const QString& title, const QString& msg);

	void updTranslations();
	void changeLang(int mode);
	int rows, columns, mines;

	bool wasWarning = false;
	int langCode;

	QString* labels[3][5] = {
		{ new QString("В ряд: "), new QString("В колонку: "), new QString("Мин: "), new QString("Добро пожаловать в Touhou Minesweeper!"), new QString("Играем!") },
		{ new QString("Rows: "), new QString("Columns: "), new QString("Mines: "), new QString("Welcome to Touhou Minesweeper!"), new QString("Play!") },
		{ new QString("列: "), new QString("行: "), new QString("地雷: "), new QString("東方地雷原へようこそ！"), new QString("スタート!") }
	};

	QString* warningLabels[3][2] = {
		{
			new QString("Неверный ввод"),
			new QString("Пожалуйста, введите значения, согласно правилам:\n\n1. 5 <= Рядов, Столбиков <= 25\n2. Хотя "
						"бы две клетки должны быть без мин, и хотя бы одна с миной\n\nВАЖНО: Если какое-то поле "
						"останется пустым,"
						" оно автоматически заполнится, согласно списку: \nВ ряд = 10,\nВ колонку = 10,\nМин = 10% от "
						"(В ряд * В колонку)"),
		},
		{
			new QString("Wrong input"),
			new QString("Please, input values using these rules:\n\n1. 5 <= Rows, Columns <= 25\n2. Minimum two cells "
						"should be without mines in them, but at least one with mine\n\nNOTE: If field is empty,"
						" it would be filled with values: \nRows = 10,\nColumns = 10,\nMines = 10% from (Rows * "
						"Columns)"),
		},
		{
			new QString("入力が正しくありません"),
			new QString("ルールに従って値を入力してください:\n\n1. 5 <= 行数, 列数 <= 25\n2. "
						"少なくとも2つのセルは地雷なしで、少なくとも1つのセルは地雷あり\n\n重要: "
						"空白のフィールドがあると、自動的に次のリストに従って入力されます:\n行数 = 10,\n列数 = "
						"10,\n地雷 = (行数 * 列数)の10%"),

		}
	};
};

#endif	  // DIALOGSETTER_H
