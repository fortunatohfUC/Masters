# Django Expense Tracker (Shared SQLite)

This Django app uses the same SQLite database as the console Python app:

- Shared DB path: `../expenses.db` (repo root `Expense-Tracker-Comparison/expenses.db`)
- Shared table: `expenses`

## Setup

```bash
cd python-django-expense-tracker
python -m venv .venv
source .venv/bin/activate
pip install -r requirements.txt
```

## Database Notes

This project uses a shared existing table (`expenses`) and maps it with `managed = False`.
The app auto-creates the table at runtime if it does not exist.

You can still run migrate for built-in Django apps (admin/auth/sessions):

```bash
python manage.py migrate
```

## Run the Server

```bash
python manage.py runserver
```

Optional admin user:

```bash
python manage.py createsuperuser
```

## Demo Script (Class Walkthrough)

1. Start console app and add:
   - `2026-02-01, 15.50, Food, Lunch`
   - `2026-02-03, 60.00, Transport, Gas`
   - `2026-02-05, 20.00, Food, Groceries`
2. Start Django server and open `http://127.0.0.1:8000/`.
3. Confirm list and summary show:
   - Overall: `95.50`
   - Food: `35.50`
   - Transport: `60.00`
4. Filter by category `Food` and verify 2 rows.
5. Filter by date range `2026-02-02` to `2026-02-04` and verify 1 row.
6. Add/delete an expense in Django and confirm the same change appears in console app.
