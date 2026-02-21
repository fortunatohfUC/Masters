# Expense Tracker Comparison (Python Console, C++ Console, Django Web, Qt GUI)

This repository compares **four implementations** of the same Expense Tracker functionality:

1. **Python console app (SQLite)**
2. **C++ console app (CSV)**
3. **Django web app (SQLite)**
4. **Qt C++ GUI app (SQLite)**

The **Python console app, Django web app, and Qt C++ GUI app share one SQLite database**:

- `expenses.db` (located at the repository root)

All three read/write the same `expenses` table, so entries added in one app appear in the others.

---

## Feature Parity (All Versions)
All versions support:

1) Add new expense  
2) View all expenses (sorted by date ascending)  
3) Filter expenses by category (case-insensitive)  
4) Filter expenses by date range (inclusive)  
5) Calculate totals (overall and by category)

Console versions also include:
6) Exit

---

## Repository Structure
```text
Expense-Tracker-Comparison/
├── expenses.db
├── python-expense-tracker/
│   └── expense_tracker.py
├── cpp-expense-tracker/
│   ├── main.cpp
│   ├── Makefile
│   └── expenses.csv
├── python-django-expense-tracker/
│   ├── manage.py
│   ├── expense_site/
│   ├── tracker/
│   ├── templates/
│   ├── static/
│   ├── requirements.txt
│   └── README.md
├── cpp-qt-expense-tracker/
│   ├── CMakeLists.txt
│   ├── main.cpp
│   ├── MainWindow.h
│   ├── MainWindow.cpp
│   └── README.md
└── README.md