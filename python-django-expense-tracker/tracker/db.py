from django.db import connection


def ensure_expenses_table() -> None:
    """Ensure the shared expenses table exists for both console and Django apps."""
    with connection.cursor() as cursor:
        cursor.execute(
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
