# Expense Tracker Comparison

## Project Overview
This project contains two console-based expense tracker applications that provide the same user-facing functionality:

1. Python Expense Tracker
2. C++ Expense Tracker

Both implementations support adding expenses, viewing sorted records, filtering by category and date range, and calculating totals overall and by category.

## Feature Parity Between Python and C++
The Python and C++ versions implement identical menu options and validation behavior:

1) Add new expense  
2) View all expenses (sorted by date ascending)  
3) Filter expenses by category (case-insensitive)  
4) Filter expenses by date range (inclusive)  
5) Calculate totals (overall and by category)  
6) Exit

## Repository Structure
```text
Expense-Tracker-Comparison/
├── python-expense-tracker/
│   ├── expense_tracker.py
│   └── expenses.json
├── cpp-expense-tracker/
│   ├── main.cpp
│   ├── Makefile
│   └── expenses.csv
└── README.md
```

## Run the Python Version
```bash
cd python-expense-tracker
python expense_tracker.py
```

## Run the C++ Version
```bash
cd cpp-expense-tracker
make
./expense_tracker
```

## Language-Specific Features Demonstrated

### Python
- Dynamic typing
- `datetime` module for date parsing and validation
- List comprehensions for filtering
- Automatic memory management

### C++
- Static typing
- Class-based design (`Expense`)
- STL containers and algorithms (`std::vector`, `std::sort`, `std::copy_if`, `std::unordered_map`)
- Explicit validation helpers and robust input handling with `std::getline`
- RAII-style resource management (standard containers/streams, no raw `new`/`delete`)
