#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QDate>
#include <QDateEdit>
#include <QDoubleSpinBox>
#include <QLabel>
#include <QLineEdit>
#include <QMainWindow>
#include <QPushButton>
#include <QSqlDatabase>
#include <QTableWidget>

class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow() override;

private slots:
    void addExpense();
    void applyFilters();
    void clearFilters();
    void deleteSelectedExpense();

private:
    bool setupDatabase();
    bool ensureTableExists();
    bool isValidDate(const QDate &date) const;
    bool parseFilterDate(const QString &dateText, QDate &outDate) const;
    void buildUi();
    void loadExpenses();
    void loadTotals();
    void showError(const QString &message) const;

    QSqlDatabase db;

    QDateEdit *dateInput;
    QDoubleSpinBox *amountInput;
    QLineEdit *categoryInput;
    QLineEdit *descriptionInput;
    QPushButton *addButton;

    QLineEdit *filterCategoryInput;
    QLineEdit *filterStartDateInput;
    QLineEdit *filterEndDateInput;
    QPushButton *applyFiltersButton;
    QPushButton *clearFiltersButton;

    QTableWidget *expensesTable;
    QPushButton *deleteSelectedButton;

    QLabel *overallTotalLabel;
    QTableWidget *totalsByCategoryTable;
};

#endif
