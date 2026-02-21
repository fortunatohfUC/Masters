from django.contrib import messages
from django.db.models import Sum
from django.shortcuts import get_object_or_404, redirect, render

from .db import ensure_expenses_table
from .forms import ExpenseForm
from .models import Expense


def expense_list(request):
    ensure_expenses_table()
    expenses = Expense.objects.all().order_by("date", "id")

    category_query = (request.GET.get("category") or "").strip()
    start_date = (request.GET.get("start_date") or "").strip()
    end_date = (request.GET.get("end_date") or "").strip()

    if category_query:
        expenses = expenses.filter(category__iexact=category_query)

    if start_date and end_date and start_date > end_date:
        messages.error(request, "Start date cannot be after end date.")
    else:
        if start_date:
            expenses = expenses.filter(date__gte=start_date)
        if end_date:
            expenses = expenses.filter(date__lte=end_date)

    overall_total = expenses.aggregate(total=Sum("amount"))["total"] or 0
    totals_by_category = (
        expenses.values("category")
        .annotate(total=Sum("amount"))
        .order_by("category")
    )

    context = {
        "expenses": expenses,
        "overall_total": overall_total,
        "totals_by_category": totals_by_category,
        "category_query": category_query,
        "start_date": start_date,
        "end_date": end_date,
    }
    return render(request, "tracker/expense_list.html", context)


def expense_create(request):
    ensure_expenses_table()
    if request.method == "POST":
        form = ExpenseForm(request.POST)
        if form.is_valid():
            form.save()
            messages.success(request, "Expense added successfully.")
            return redirect("expense-list")
    else:
        form = ExpenseForm()
    return render(request, "tracker/expense_form.html", {"form": form})


def expense_delete(request, pk: int):
    ensure_expenses_table()
    expense = get_object_or_404(Expense, pk=pk)
    if request.method == "POST":
        expense.delete()
        messages.success(request, "Expense deleted successfully.")
        return redirect("expense-list")
    return render(request, "tracker/expense_confirm_delete.html", {"expense": expense})
