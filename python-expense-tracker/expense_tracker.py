#!/usr/bin/env python3
"""Console-based expense tracker backed by a shared SQLite database."""

from __future__ import annotations

import sqlite3
from datetime import date, datetime
from pathlib import Path

DB_PATH = Path(__file__).resolve().parent.parent / "expenses.db"


def parse_date(date_str: str) -> date | None:
    """Parse YYYY-MM-DD date string into a date object."""
    try:
        return datetime.strptime(date_str.strip(), "%Y-%m-%d").date()
    except ValueError:
        return None


def initialize_database(connection: sqlite3.Connection) -> None:
    """Create the shared expenses table if it doesn't exist."""
    connection.execute(
        """
        CREATE TABLE IF NOT EXISTS expenses (
            id INTEGER PRIMARY KEY AUTOINCREMENT,
            date TEXT NOT NULL,
            amount REAL NOT NULL,
            category TEXT NOT NULL,
            description TEXT
        )
        """
    )
    connection.commit()


def prompt_date(prompt: str) -> str:
    """Prompt until a valid date is entered. Returns ISO string YYYY-MM-DD."""
    while True:
        raw = input(prompt).strip()
        parsed = parse_date(raw)
        if parsed is not None:
            return parsed.isoformat()
        print("Invalid date format. Please use YYYY-MM-DD.")


def prompt_positive_amount() -> float:
    """Prompt until a positive amount is entered."""
    while True:
        raw = input("Amount: ").strip()
        try:
            amount = float(raw)
            if amount > 0:
                return amount
            print("Amount must be greater than 0.")
        except ValueError:
            print("Invalid numeric input. Enter a valid number.")


def prompt_non_empty(prompt: str) -> str:
    """Prompt until a non-empty string is entered."""
    while True:
        value = input(prompt).strip()
        if value:
            return value
        print("Category cannot be empty.")


def print_table(expenses: list[sqlite3.Row]) -> None:
    """Print expenses in tabular format."""
    if not expenses:
        print("No expenses to display.")
        return

    header = f"{'Date':<12} | {'Amount':>10} | {'Category':<20} | Description"
    print(header)
    print("-" * len(header))
    for item in expenses:
        print(
            f"{item['date']:<12} | "
            f"{float(item['amount']):>10.2f} | "
            f"{item['category']:<20} | "
            f"{item['description'] or ''}"
        )


def add_expense(connection: sqlite3.Connection) -> None:
    """Collect and add a new expense."""
    print("\nAdd New Expense")
    expense_date = prompt_date("Date (YYYY-MM-DD): ")
    amount = prompt_positive_amount()
    category = prompt_non_empty("Category: ")
    description = input("Description: ").strip()

    connection.execute(
        "INSERT INTO expenses (date, amount, category, description) VALUES (?, ?, ?, ?)",
        (expense_date, amount, category, description),
    )
    connection.commit()
    print("Expense added successfully.")


def view_expenses(connection: sqlite3.Connection) -> None:
    """View all expenses sorted by date ascending."""
    print("\nAll Expenses (sorted by date)")
    rows = connection.execute(
        "SELECT id, date, amount, category, description FROM expenses ORDER BY date ASC, id ASC"
    ).fetchall()
    print_table(rows)


def filter_by_category(connection: sqlite3.Connection) -> None:
    """Filter expenses by category, case-insensitive."""
    print("\nFilter by Category")
    category = prompt_non_empty("Category to filter: ")
    rows = connection.execute(
        """
        SELECT id, date, amount, category, description
        FROM expenses
        WHERE LOWER(category) = LOWER(?)
        ORDER BY date ASC, id ASC
        """,
        (category,),
    ).fetchall()
    print_table(rows)


def filter_by_date_range(connection: sqlite3.Connection) -> None:
    """Filter expenses by inclusive date range."""
    print("\nFilter by Date Range")
    while True:
        start = prompt_date("Start date (YYYY-MM-DD): ")
        end = prompt_date("End date (YYYY-MM-DD): ")
        if start <= end:
            break
        print("Start date cannot be after end date. Please re-enter.")

    rows = connection.execute(
        """
        SELECT id, date, amount, category, description
        FROM expenses
        WHERE date BETWEEN ? AND ?
        ORDER BY date ASC, id ASC
        """,
        (start, end),
    ).fetchall()
    print_table(rows)


def calculate_totals(connection: sqlite3.Connection) -> None:
    """Calculate and display overall and per-category totals."""
    print("\nTotals")
    overall_row = connection.execute(
        "SELECT COALESCE(SUM(amount), 0) AS total FROM expenses"
    ).fetchone()
    overall = float(overall_row["total"] if overall_row else 0.0)
    print(f"Overall total: {overall:.2f}")

    category_rows = connection.execute(
        """
        SELECT category, SUM(amount) AS total
        FROM expenses
        GROUP BY category
        ORDER BY category ASC
        """
    ).fetchall()

    if not category_rows:
        print("No category totals available.")
        return

    print("Totals by category:")
    for row in category_rows:
        print(f"- {row['category']}: {float(row['total']):.2f}")


def main() -> None:
    """Run the expense tracker menu loop."""
    connection = sqlite3.connect(DB_PATH)
    connection.row_factory = sqlite3.Row

    try:
        initialize_database(connection)

        menu = (
            "\nExpense Tracker Menu\n"
            "1) Add new expense\n"
            "2) View all expenses (sorted by date ascending)\n"
            "3) Filter expenses by category\n"
            "4) Filter expenses by date range\n"
            "5) Calculate totals (overall and by category)\n"
            "6) Exit"
        )

        actions = {
            "1": add_expense,
            "2": view_expenses,
            "3": filter_by_category,
            "4": filter_by_date_range,
            "5": calculate_totals,
        }

        while True:
            print(menu)
            choice = input("Choose an option (1-6): ").strip()

            if choice == "6":
                print("Goodbye!")
                break

            action = actions.get(choice)
            if action is None:
                print("Invalid choice. Please select a menu option from 1 to 6.")
                continue

            action(connection)
    finally:
        connection.close()


if __name__ == "__main__":
    main()