#!/usr/bin/env python3
"""Console-based expense tracker with JSON persistence."""

from __future__ import annotations

import json
from datetime import date, datetime
from pathlib import Path
from typing import Any

DATA_FILE = Path(__file__).with_name("expenses.json")


def parse_date(date_str: str) -> date | None:
    """Parse YYYY-MM-DD date string into a date object."""
    try:
        return datetime.strptime(date_str.strip(), "%Y-%m-%d").date()
    except ValueError:
        return None


def prompt_date(prompt: str) -> date:
    """Prompt until a valid date is entered."""
    while True:
        raw = input(prompt).strip()
        parsed = parse_date(raw)
        if parsed is not None:
            return parsed
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


def load_expenses() -> list[dict[str, Any]]:
    """Load expenses from JSON if available."""
    if not DATA_FILE.exists():
        return []

    try:
        with DATA_FILE.open("r", encoding="utf-8") as file:
            raw_data = json.load(file)
    except (json.JSONDecodeError, OSError):
        print("Warning: Could not read expenses.json. Starting with an empty list.")
        return []

    expenses: list[dict[str, Any]] = []
    for item in raw_data if isinstance(raw_data, list) else []:
        if not isinstance(item, dict):
            continue
        parsed_date = parse_date(str(item.get("date", "")))
        try:
            amount = float(item.get("amount", 0))
        except (TypeError, ValueError):
            continue
        category = str(item.get("category", "")).strip()
        description = str(item.get("description", "")).strip()

        if parsed_date and amount > 0 and category:
            expenses.append(
                {
                    "date": parsed_date,
                    "amount": amount,
                    "category": category,
                    "description": description,
                }
            )

    return expenses


def save_expenses(expenses: list[dict[str, Any]]) -> None:
    """Persist expenses to JSON."""
    serializable = [
        {
            "date": entry["date"].isoformat(),
            "amount": entry["amount"],
            "category": entry["category"],
            "description": entry["description"],
        }
        for entry in expenses
    ]

    with DATA_FILE.open("w", encoding="utf-8") as file:
        json.dump(serializable, file, indent=2)


def print_table(expenses: list[dict[str, Any]]) -> None:
    """Print expenses in tabular format."""
    if not expenses:
        print("No expenses to display.")
        return

    header = f"{'Date':<12} | {'Amount':>10} | {'Category':<20} | Description"
    print(header)
    print("-" * len(header))
    for item in expenses:
        print(
            f"{item['date'].isoformat():<12} | "
            f"{item['amount']:>10.2f} | "
            f"{item['category']:<20} | "
            f"{item['description']}"
        )


def add_expense(expenses: list[dict[str, Any]]) -> None:
    """Collect and add a new expense."""
    print("\nAdd New Expense")
    expense_date = prompt_date("Date (YYYY-MM-DD): ")
    amount = prompt_positive_amount()
    category = prompt_non_empty("Category: ")
    description = input("Description: ").strip()

    expenses.append(
        {
            "date": expense_date,
            "amount": amount,
            "category": category,
            "description": description,
        }
    )
    save_expenses(expenses)
    print("Expense added successfully.")


def view_expenses(expenses: list[dict[str, Any]]) -> None:
    """View all expenses sorted by date ascending."""
    print("\nAll Expenses (sorted by date)")
    sorted_expenses = sorted(expenses, key=lambda item: item["date"])
    print_table(sorted_expenses)


def filter_by_category(expenses: list[dict[str, Any]]) -> None:
    """Filter expenses by category, case-insensitive."""
    print("\nFilter by Category")
    category = prompt_non_empty("Category to filter: ")
    filtered = [
        item for item in expenses if item["category"].strip().lower() == category.lower()
    ]
    print_table(sorted(filtered, key=lambda item: item["date"]))


def filter_by_date_range(expenses: list[dict[str, Any]]) -> None:
    """Filter expenses by inclusive date range."""
    print("\nFilter by Date Range")
    while True:
        start = prompt_date("Start date (YYYY-MM-DD): ")
        end = prompt_date("End date (YYYY-MM-DD): ")
        if start <= end:
            break
        print("Start date cannot be after end date. Please re-enter.")

    filtered = [item for item in expenses if start <= item["date"] <= end]
    print_table(sorted(filtered, key=lambda item: item["date"]))


def calculate_totals(expenses: list[dict[str, Any]]) -> None:
    """Calculate and display overall and per-category totals."""
    print("\nTotals")
    overall = sum(item["amount"] for item in expenses)
    by_category: dict[str, float] = {}
    for item in expenses:
        category = item["category"]
        by_category[category] = by_category.get(category, 0.0) + item["amount"]

    print(f"Overall total: {overall:.2f}")
    if not by_category:
        print("No category totals available.")
        return

    print("Totals by category:")
    for category in sorted(by_category):
        print(f"- {category}: {by_category[category]:.2f}")


def main() -> None:
    """Run the expense tracker menu loop."""
    expenses = load_expenses()

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
            save_expenses(expenses)
            print("Goodbye!")
            break

        action = actions.get(choice)
        if action is None:
            print("Invalid choice. Please select a menu option from 1 to 6.")
            continue

        action(expenses)


if __name__ == "__main__":
    main()
