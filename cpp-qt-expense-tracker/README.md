# Qt Widgets Expense Tracker (C++)

This GUI app uses **Qt 6 Widgets** and **QtSql**, and reads/writes the shared SQLite database:

- `../expenses.db` (from this folder, resolves to repo root `Expense-Tracker-Comparison/expenses.db`)

## Prerequisites

- Qt 6 (with Widgets and Sql modules)
- CMake 3.16+
- A C++17-compatible compiler

## Build and Run

```bash
cd cpp-qt-expense-tracker
mkdir -p build
cd build
cmake ..
cmake --build .
./expense_tracker_gui
```

> On Windows, run the produced `.exe` from the build directory.

## Features

- Add expense with validation
- View expenses sorted by date ascending
- Filter by category (case-insensitive)
- Filter by inclusive date range
- Summary totals (overall and by category)
- Delete selected expense with confirmation

All data is persisted in the shared `expenses` table used by the Python console and Django apps.
