#include <algorithm>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <limits>
#include <sstream>
#include <string>
#include <unordered_map>
#include <vector>

class Expense {
public:
    std::string date;
    double amount;
    std::string category;
    std::string description;
};

const std::string DATA_FILE = "expenses.csv";

std::string trim(const std::string &input) {
    const auto start = input.find_first_not_of(" \t\n\r\f\v");
    if (start == std::string::npos) {
        return "";
    }
    const auto end = input.find_last_not_of(" \t\n\r\f\v");
    return input.substr(start, end - start + 1);
}

bool isLeapYear(int year) {
    return (year % 4 == 0 && year % 100 != 0) || (year % 400 == 0);
}

int daysInMonth(int year, int month) {
    static const int days[] = {0, 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
    if (month == 2) {
        return isLeapYear(year) ? 29 : 28;
    }
    return days[month];
}

bool parseDateParts(const std::string &date, int &year, int &month, int &day) {
    if (date.size() != 10 || date[4] != '-' || date[7] != '-') {
        return false;
    }

    try {
        year = std::stoi(date.substr(0, 4));
        month = std::stoi(date.substr(5, 2));
        day = std::stoi(date.substr(8, 2));
    } catch (...) {
        return false;
    }

    if (year < 1 || month < 1 || month > 12) {
        return false;
    }
    if (day < 1 || day > daysInMonth(year, month)) {
        return false;
    }

    return true;
}

bool isValidDate(const std::string &date) {
    int year = 0;
    int month = 0;
    int day = 0;
    return parseDateParts(date, year, month, day);
}

int dateToKey(const std::string &date) {
    int year = 0;
    int month = 0;
    int day = 0;
    if (!parseDateParts(date, year, month, day)) {
        return -1;
    }
    return year * 10000 + month * 100 + day;
}

std::string toLower(std::string value) {
    std::transform(value.begin(), value.end(), value.begin(), [](unsigned char c) {
        return static_cast<char>(std::tolower(c));
    });
    return value;
}

std::vector<std::string> splitCsvLine(const std::string &line) {
    std::vector<std::string> fields;
    std::string current;
    bool inQuotes = false;

    for (size_t i = 0; i < line.size(); ++i) {
        char c = line[i];
        if (c == '"') {
            if (inQuotes && i + 1 < line.size() && line[i + 1] == '"') {
                current.push_back('"');
                ++i;
            } else {
                inQuotes = !inQuotes;
            }
        } else if (c == ',' && !inQuotes) {
            fields.push_back(current);
            current.clear();
        } else {
            current.push_back(c);
        }
    }

    fields.push_back(current);
    return fields;
}

std::string escapeCsv(const std::string &field) {
    if (field.find_first_of(",\"") == std::string::npos) {
        return field;
    }
    std::string escaped = "\"";
    for (char c : field) {
        if (c == '"') {
            escaped += "\"\"";
        } else {
            escaped.push_back(c);
        }
    }
    escaped += "\"";
    return escaped;
}

void saveExpenses(const std::vector<Expense> &expenses) {
    std::ofstream out(DATA_FILE, std::ios::trunc);
    if (!out) {
        std::cerr << "Warning: Could not save to " << DATA_FILE << ".\n";
        return;
    }

    for (const auto &expense : expenses) {
        out << escapeCsv(expense.date) << ','
            << std::fixed << std::setprecision(2) << expense.amount << ','
            << escapeCsv(expense.category) << ','
            << escapeCsv(expense.description) << '\n';
    }
}

std::vector<Expense> loadExpenses() {
    std::vector<Expense> expenses;
    std::ifstream in(DATA_FILE);
    if (!in) {
        return expenses;
    }

    std::string line;
    while (std::getline(in, line)) {
        if (trim(line).empty()) {
            continue;
        }

        auto fields = splitCsvLine(line);
        if (fields.size() != 4) {
            continue;
        }

        Expense e;
        e.date = trim(fields[0]);
        e.category = trim(fields[2]);
        e.description = trim(fields[3]);

        try {
            e.amount = std::stod(trim(fields[1]));
        } catch (...) {
            continue;
        }

        if (isValidDate(e.date) && e.amount > 0.0 && !e.category.empty()) {
            expenses.push_back(e);
        }
    }

    return expenses;
}

std::string promptLine(const std::string &prompt) {
    std::string input;
    std::cout << prompt;
    std::getline(std::cin, input);
    return input;
}

std::string promptDate(const std::string &prompt) {
    while (true) {
        std::string date = trim(promptLine(prompt));
        if (isValidDate(date)) {
            return date;
        }
        std::cout << "Invalid date format. Please use YYYY-MM-DD with a real date.\n";
    }
}

double promptAmount() {
    while (true) {
        std::string input = trim(promptLine("Amount: "));
        try {
            size_t processed = 0;
            const double amount = std::stod(input, &processed);
            if (processed != input.size()) {
                throw std::invalid_argument("Trailing characters");
            }
            if (amount > 0.0) {
                return amount;
            }
            std::cout << "Amount must be greater than 0.\n";
        } catch (...) {
            std::cout << "Invalid numeric input. Enter a valid positive number.\n";
        }
    }
}

std::string promptCategory(const std::string &prompt = "Category: ") {
    while (true) {
        std::string category = trim(promptLine(prompt));
        if (!category.empty()) {
            return category;
        }
        std::cout << "Category cannot be empty.\n";
    }
}

void printTable(const std::vector<Expense> &expenses) {
    if (expenses.empty()) {
        std::cout << "No expenses to display.\n";
        return;
    }

    std::cout << std::left << std::setw(12) << "Date"
              << " | " << std::right << std::setw(10) << "Amount"
              << " | " << std::left << std::setw(20) << "Category"
              << " | Description\n";
    std::cout << "---------------------------------------------------------------\n";

    for (const auto &expense : expenses) {
        std::cout << std::left << std::setw(12) << expense.date
                  << " | " << std::right << std::setw(10) << std::fixed << std::setprecision(2)
                  << expense.amount
                  << " | " << std::left << std::setw(20) << expense.category
                  << " | " << expense.description << '\n';
    }
}

void addExpense(std::vector<Expense> &expenses) {
    std::cout << "\nAdd New Expense\n";
    Expense expense;
    expense.date = promptDate("Date (YYYY-MM-DD): ");
    expense.amount = promptAmount();
    expense.category = promptCategory();
    expense.description = trim(promptLine("Description: "));

    expenses.push_back(expense);
    saveExpenses(expenses);
    std::cout << "Expense added successfully.\n";
}

void viewExpenses(const std::vector<Expense> &expenses) {
    std::cout << "\nAll Expenses (sorted by date)\n";
    std::vector<Expense> sorted = expenses;
    std::sort(sorted.begin(), sorted.end(), [](const Expense &a, const Expense &b) {
        return dateToKey(a.date) < dateToKey(b.date);
    });
    printTable(sorted);
}

void filterByCategory(const std::vector<Expense> &expenses) {
    std::cout << "\nFilter by Category\n";
    const std::string target = toLower(promptCategory("Category to filter: "));

    std::vector<Expense> filtered;
    std::copy_if(expenses.begin(), expenses.end(), std::back_inserter(filtered),
                 [&target](const Expense &e) { return toLower(e.category) == target; });

    std::sort(filtered.begin(), filtered.end(), [](const Expense &a, const Expense &b) {
        return dateToKey(a.date) < dateToKey(b.date);
    });
    printTable(filtered);
}

void filterByDateRange(const std::vector<Expense> &expenses) {
    std::cout << "\nFilter by Date Range\n";
    std::string start;
    std::string end;

    while (true) {
        start = promptDate("Start date (YYYY-MM-DD): ");
        end = promptDate("End date (YYYY-MM-DD): ");
        if (dateToKey(start) <= dateToKey(end)) {
            break;
        }
        std::cout << "Start date cannot be after end date. Please re-enter.\n";
    }

    const int startKey = dateToKey(start);
    const int endKey = dateToKey(end);

    std::vector<Expense> filtered;
    std::copy_if(expenses.begin(), expenses.end(), std::back_inserter(filtered),
                 [startKey, endKey](const Expense &e) {
                     const int key = dateToKey(e.date);
                     return key >= startKey && key <= endKey;
                 });

    std::sort(filtered.begin(), filtered.end(), [](const Expense &a, const Expense &b) {
        return dateToKey(a.date) < dateToKey(b.date);
    });
    printTable(filtered);
}

void calculateTotals(const std::vector<Expense> &expenses) {
    std::cout << "\nTotals\n";
    double overall = 0.0;
    std::unordered_map<std::string, double> byCategory;

    for (const auto &expense : expenses) {
        overall += expense.amount;
        byCategory[expense.category] += expense.amount;
    }

    std::cout << "Overall total: " << std::fixed << std::setprecision(2) << overall << '\n';

    if (byCategory.empty()) {
        std::cout << "No category totals available.\n";
        return;
    }

    std::vector<std::string> categories;
    categories.reserve(byCategory.size());
    for (const auto &entry : byCategory) {
        categories.push_back(entry.first);
    }
    std::sort(categories.begin(), categories.end());

    std::cout << "Totals by category:\n";
    for (const auto &category : categories) {
        std::cout << "- " << category << ": " << std::fixed << std::setprecision(2)
                  << byCategory[category] << '\n';
    }
}

int readMenuChoice() {
    while (true) {
        std::string raw = trim(promptLine("Choose an option (1-6): "));
        try {
            size_t processed = 0;
            int choice = std::stoi(raw, &processed);
            if (processed == raw.size() && choice >= 1 && choice <= 6) {
                return choice;
            }
        } catch (...) {
        }
        std::cout << "Invalid choice. Please select a menu option from 1 to 6.\n";
    }
}

int main() {
    std::vector<Expense> expenses = loadExpenses();

    while (true) {
        std::cout << "\nExpense Tracker Menu\n"
                  << "1) Add new expense\n"
                  << "2) View all expenses (sorted by date ascending)\n"
                  << "3) Filter expenses by category\n"
                  << "4) Filter expenses by date range\n"
                  << "5) Calculate totals (overall and by category)\n"
                  << "6) Exit\n";

        const int choice = readMenuChoice();

        switch (choice) {
        case 1:
            addExpense(expenses);
            break;
        case 2:
            viewExpenses(expenses);
            break;
        case 3:
            filterByCategory(expenses);
            break;
        case 4:
            filterByDateRange(expenses);
            break;
        case 5:
            calculateTotals(expenses);
            break;
        case 6:
            saveExpenses(expenses);
            std::cout << "Goodbye!\n";
            return 0;
        default:
            break;
        }
    }
}
