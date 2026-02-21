from django.core.validators import MinValueValidator
from django.db import models


class Expense(models.Model):
    date = models.DateField()
    amount = models.FloatField(validators=[MinValueValidator(0.01)])
    category = models.CharField(max_length=255)
    description = models.TextField(blank=True)

    class Meta:
        db_table = "expenses"
        managed = False
        ordering = ["date", "id"]

    def __str__(self) -> str:
        return f"{self.date} - {self.category} - {self.amount:.2f}"
