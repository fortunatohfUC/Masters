#include "MainWindow.h"

#include <QAbstractItemView>
#include <QFormLayout>
#include <QGroupBox>
#include <QHeaderView>
#include <QHBoxLayout>
#include <QMessageBox>
#include <QSqlError>
#include <QSqlQuery>
#include <QVBoxLayout>
#include <QWidget>

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent) {
    setWindowTitle("Qt Expense Tracker");
    resize(1050, 760);

    buildUi();

    if (!setupDatabase() || !ensureTableExists()) {
        addButton->setEnabled(false);
        applyFiltersButton->setEnabled(false);
        clearFiltersButton->setEnabled(false);
        deleteSelectedButton->setEnabled(false);
        return;
    }

    loadExpenses();
    loadTotals();
}

MainWindow::~MainWindow() {
    if (db.isOpen()) {
        db.close();
    }
}

void MainWindow::buildUi() {
    auto *central = new QWidget(this);
    auto *mainLayout = new QVBoxLayout(central);

    auto *addGroup = new QGroupBox("Add Expense", central);
    auto *addLayout = new QFormLayout(addGroup);

    dateInput = new QDateEdit(QDate::currentDate(), addGroup);
    dateInput->setDisplayFormat("yyyy-MM-dd");
    dateInput->setCalendarPopup(true);

    amountInput = new QDoubleSpinBox(addGroup);
    amountInput->setRange(0.01, 100000000.00);
    amountInput->setDecimals(2);
    amountInput->setSingleStep(0.50);

    categoryInput = new QLineEdit(addGroup);
    descriptionInput = new QLineEdit(addGroup);
    addButton = new QPushButton("Add", addGroup);

    addLayout->addRow("Date", dateInput);
    addLayout->addRow("Amount", amountInput);
    addLayout->addRow("Category", categoryInput);
    addLayout->addRow("Description", descriptionInput);
    addLayout->addRow(addButton);

    auto *filterGroup = new QGroupBox("Filters", central);
    auto *filterLayout = new QHBoxLayout(filterGroup);

    filterCategoryInput = new QLineEdit(filterGroup);
    filterCategoryInput->setPlaceholderText("Category (case-insensitive)");

    filterStartDateInput = new QLineEdit(filterGroup);
    filterStartDateInput->setPlaceholderText("Start date (YYYY-MM-DD)");

    filterEndDateInput = new QLineEdit(filterGroup);
    filterEndDateInput->setPlaceholderText("End date (YYYY-MM-DD)");

    applyFiltersButton = new QPushButton("Apply Filters", filterGroup);
    clearFiltersButton = new QPushButton("Clear Filters", filterGroup);

    filterLayout->addWidget(new QLabel("Category", filterGroup));
    filterLayout->addWidget(filterCategoryInput);
    filterLayout->addWidget(new QLabel("Start", filterGroup));
    filterLayout->addWidget(filterStartDateInput);
    filterLayout->addWidget(new QLabel("End", filterGroup));
    filterLayout->addWidget(filterEndDateInput);
    filterLayout->addWidget(applyFiltersButton);
    filterLayout->addWidget(clearFiltersButton);

    auto *tableGroup = new QGroupBox("Expenses", central);
    auto *tableLayout = new QVBoxLayout(tableGroup);

    expensesTable = new QTableWidget(tableGroup);
    expensesTable->setColumnCount(5);
    expensesTable->setHorizontalHeaderLabels({"Date", "Amount", "Category", "Description", "ID"});
    expensesTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    expensesTable->setSelectionMode(QAbstractItemView::SingleSelection);
    expensesTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
    expensesTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    expensesTable->setColumnHidden(4, true);

    deleteSelectedButton = new QPushButton("Delete Selected", tableGroup);
    tableLayout->addWidget(expensesTable);
    tableLayout->addWidget(deleteSelectedButton);

    auto *summaryGroup = new QGroupBox("Summary", central);
    auto *summaryLayout = new QVBoxLayout(summaryGroup);

    overallTotalLabel = new QLabel("Overall Total: $0.00", summaryGroup);
    totalsByCategoryTable = new QTableWidget(summaryGroup);
    totalsByCategoryTable->setColumnCount(2);
    totalsByCategoryTable->setHorizontalHeaderLabels({"Category", "Total"});
    totalsByCategoryTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
    totalsByCategoryTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);

    summaryLayout->addWidget(overallTotalLabel);
    summaryLayout->addWidget(totalsByCategoryTable);

    mainLayout->addWidget(addGroup);
    mainLayout->addWidget(filterGroup);
    mainLayout->addWidget(tableGroup);
    mainLayout->addWidget(summaryGroup);

    setCentralWidget(central);

    connect(addButton, &QPushButton::clicked, this, &MainWindow::addExpense);
    connect(applyFiltersButton, &QPushButton::clicked, this, &MainWindow::applyFilters);
    connect(clearFiltersButton, &QPushButton::clicked, this, &MainWindow::clearFilters);
    connect(deleteSelectedButton, &QPushButton::clicked, this, &MainWindow::deleteSelectedExpense);
}

bool MainWindow::setupDatabase() {
    db = QSqlDatabase::addDatabase("QSQLITE");
    db.setDatabaseName("../expenses.db");

    if (!db.open()) {
        showError(QString("Database connection failed: %1").arg(db.lastError().text()));
        return false;
    }

    return true;
}

bool MainWindow::ensureTableExists() {
    QSqlQuery query(db);
    const QString createSql =
        "CREATE TABLE IF NOT EXISTS expenses ("
        "id INTEGER PRIMARY KEY AUTOINCREMENT,"
        "date TEXT NOT NULL,"
        "amount REAL NOT NULL,"
        "category TEXT NOT NULL,"
        "description TEXT"
        ")";

    if (!query.exec(createSql)) {
        showError(QString("Failed to create or verify table: %1").arg(query.lastError().text()));
        return false;
    }

    return true;
}

bool MainWindow::isValidDate(const QDate &date) const {
    return date.isValid();
}

bool MainWindow::parseFilterDate(const QString &dateText, QDate &outDate) const {
    if (dateText.trimmed().isEmpty()) {
        return false;
    }

    const QDate parsed = QDate::fromString(dateText.trimmed(), "yyyy-MM-dd");
    if (!parsed.isValid()) {
        showError(QString("Invalid date '%1'. Use YYYY-MM-DD.").arg(dateText));
        return false;
    }

    outDate = parsed;
    return true;
}

void MainWindow::addExpense() {
    const QDate expenseDate = dateInput->date();
    const double amount = amountInput->value();
    const QString category = categoryInput->text().trimmed();
    const QString description = descriptionInput->text().trimmed();

    if (!isValidDate(expenseDate)) {
        showError("Date is invalid. Please use a real calendar date.");
        return;
    }

    if (amount <= 0.0) {
        showError("Amount must be greater than 0.");
        return;
    }

    if (category.isEmpty()) {
        showError("Category cannot be empty.");
        return;
    }

    QSqlQuery query(db);
    query.prepare("INSERT INTO expenses (date, amount, category, description) VALUES (:date, :amount, :category, :description)");
    query.bindValue(":date", expenseDate.toString("yyyy-MM-dd"));
    query.bindValue(":amount", amount);
    query.bindValue(":category", category);
    query.bindValue(":description", description);

    if (!query.exec()) {
        showError(QString("Failed to add expense: %1").arg(query.lastError().text()));
        return;
    }

    amountInput->setValue(0.01);
    categoryInput->clear();
    descriptionInput->clear();
    loadExpenses();
    loadTotals();
}

void MainWindow::applyFilters() {
    const QString startText = filterStartDateInput->text().trimmed();
    const QString endText = filterEndDateInput->text().trimmed();

    QDate startDate;
    QDate endDate;
    const bool hasStart = parseFilterDate(startText, startDate);
    if (!startText.isEmpty() && !hasStart) {
        return;
    }

    const bool hasEnd = parseFilterDate(endText, endDate);
    if (!endText.isEmpty() && !hasEnd) {
        return;
    }

    if (hasStart && hasEnd && startDate > endDate) {
        showError("Start date cannot be after end date.");
        return;
    }

    loadExpenses();
    loadTotals();
}

void MainWindow::clearFilters() {
    filterCategoryInput->clear();
    filterStartDateInput->clear();
    filterEndDateInput->clear();

    loadExpenses();
    loadTotals();
}

void MainWindow::deleteSelectedExpense() {
    const int row = expensesTable->currentRow();
    if (row < 0) {
        showError("Please select an expense to delete.");
        return;
    }

    const auto *idItem = expensesTable->item(row, 4);
    if (!idItem) {
        showError("Could not determine selected expense ID.");
        return;
    }

    bool ok = false;
    const int id = idItem->text().toInt(&ok);
    if (!ok) {
        showError("Selected expense ID is invalid.");
        return;
    }

    const auto answer = QMessageBox::question(
        this,
        "Delete Expense",
        "Are you sure you want to delete the selected expense?",
        QMessageBox::Yes | QMessageBox::No,
        QMessageBox::No
    );

    if (answer != QMessageBox::Yes) {
        return;
    }

    QSqlQuery query(db);
    query.prepare("DELETE FROM expenses WHERE id = :id");
    query.bindValue(":id", id);

    if (!query.exec()) {
        showError(QString("Failed to delete expense: %1").arg(query.lastError().text()));
        return;
    }

    loadExpenses();
    loadTotals();
}

void MainWindow::loadExpenses() {
    QString sql =
        "SELECT id, date, amount, category, description "
        "FROM expenses WHERE 1 = 1";

    const QString category = filterCategoryInput->text().trimmed();
    const QString startText = filterStartDateInput->text().trimmed();
    const QString endText = filterEndDateInput->text().trimmed();

    if (!category.isEmpty()) {
        sql += " AND LOWER(category) = LOWER(:category)";
    }
    if (!startText.isEmpty()) {
        sql += " AND date >= :startDate";
    }
    if (!endText.isEmpty()) {
        sql += " AND date <= :endDate";
    }

    sql += " ORDER BY date ASC, id ASC";

    QSqlQuery query(db);
    query.prepare(sql);

    if (!category.isEmpty()) {
        query.bindValue(":category", category);
    }
    if (!startText.isEmpty()) {
        query.bindValue(":startDate", startText);
    }
    if (!endText.isEmpty()) {
        query.bindValue(":endDate", endText);
    }

    if (!query.exec()) {
        showError(QString("Failed to load expenses: %1").arg(query.lastError().text()));
        return;
    }

    expensesTable->setRowCount(0);

    while (query.next()) {
        const int row = expensesTable->rowCount();
        expensesTable->insertRow(row);

        auto *dateItem = new QTableWidgetItem(query.value("date").toString());
        auto *amountItem = new QTableWidgetItem(QString::number(query.value("amount").toDouble(), 'f', 2));
        auto *categoryItem = new QTableWidgetItem(query.value("category").toString());
        auto *descriptionItem = new QTableWidgetItem(query.value("description").toString());
        auto *idItem = new QTableWidgetItem(query.value("id").toString());

        expensesTable->setItem(row, 0, dateItem);
        expensesTable->setItem(row, 1, amountItem);
        expensesTable->setItem(row, 2, categoryItem);
        expensesTable->setItem(row, 3, descriptionItem);
        expensesTable->setItem(row, 4, idItem);
    }
}

void MainWindow::loadTotals() {
    QString whereClause = " WHERE 1 = 1";
    const QString category = filterCategoryInput->text().trimmed();
    const QString startText = filterStartDateInput->text().trimmed();
    const QString endText = filterEndDateInput->text().trimmed();

    if (!category.isEmpty()) {
        whereClause += " AND LOWER(category) = LOWER(:category)";
    }
    if (!startText.isEmpty()) {
        whereClause += " AND date >= :startDate";
    }
    if (!endText.isEmpty()) {
        whereClause += " AND date <= :endDate";
    }

    QSqlQuery overallQuery(db);
    overallQuery.prepare("SELECT COALESCE(SUM(amount), 0) AS total FROM expenses" + whereClause);
    if (!category.isEmpty()) {
        overallQuery.bindValue(":category", category);
    }
    if (!startText.isEmpty()) {
        overallQuery.bindValue(":startDate", startText);
    }
    if (!endText.isEmpty()) {
        overallQuery.bindValue(":endDate", endText);
    }

    if (!overallQuery.exec() || !overallQuery.next()) {
        showError(QString("Failed to calculate overall total: %1").arg(overallQuery.lastError().text()));
        return;
    }

    const double overall = overallQuery.value("total").toDouble();
    overallTotalLabel->setText(QString("Overall Total: $%1").arg(QString::number(overall, 'f', 2)));

    QSqlQuery categoryQuery(db);
    categoryQuery.prepare(
        "SELECT category, SUM(amount) AS total FROM expenses" + whereClause +
        " GROUP BY category ORDER BY category ASC"
    );

    if (!category.isEmpty()) {
        categoryQuery.bindValue(":category", category);
    }
    if (!startText.isEmpty()) {
        categoryQuery.bindValue(":startDate", startText);
    }
    if (!endText.isEmpty()) {
        categoryQuery.bindValue(":endDate", endText);
    }

    if (!categoryQuery.exec()) {
        showError(QString("Failed to calculate category totals: %1").arg(categoryQuery.lastError().text()));
        return;
    }

    totalsByCategoryTable->setRowCount(0);
    while (categoryQuery.next()) {
        const int row = totalsByCategoryTable->rowCount();
        totalsByCategoryTable->insertRow(row);

        auto *categoryItem = new QTableWidgetItem(categoryQuery.value("category").toString());
        auto *totalItem = new QTableWidgetItem(QString::number(categoryQuery.value("total").toDouble(), 'f', 2));

        totalsByCategoryTable->setItem(row, 0, categoryItem);
        totalsByCategoryTable->setItem(row, 1, totalItem);
    }
}

void MainWindow::showError(const QString &message) const {
    QMessageBox::critical(const_cast<MainWindow *>(this), "Error", message);
}
