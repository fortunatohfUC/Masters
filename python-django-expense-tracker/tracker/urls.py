from django.urls import path

from . import views

urlpatterns = [
    path("", views.expense_list, name="expense-list"),
    path("add/", views.expense_create, name="expense-add"),
    path("delete/<int:pk>/", views.expense_delete, name="expense-delete"),
]
