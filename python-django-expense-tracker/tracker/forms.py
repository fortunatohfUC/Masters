from django import forms

from .models import Expense


class ExpenseForm(forms.ModelForm):
    class Meta:
        model = Expense
        fields = ["date", "amount", "category", "description"]
        widgets = {
            "date": forms.DateInput(attrs={"type": "date"}),
            "description": forms.TextInput(),
        }

    def clean_category(self) -> str:
        category = (self.cleaned_data.get("category") or "").strip()
        if not category:
            raise forms.ValidationError("Category cannot be empty.")
        return category

    def clean_amount(self) -> float:
        amount = self.cleaned_data.get("amount")
        if amount is None or amount <= 0:
            raise forms.ValidationError("Amount must be greater than 0.")
        return amount
