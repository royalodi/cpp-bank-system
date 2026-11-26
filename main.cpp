// C++ Simple Banking System
// Edition: Demo Suite
// Compile: g++ main.cpp -o bank
// Run: bank.exe (Windows) or ./bank (Linux/Mac)

#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <string>
#include <iomanip>
#include <algorithm>
#include <ctime>
#include <cstdlib>
#include <limits>
using namespace std;

// ── Transaction Record ──────────────────────────────────────
struct Transaction {
    string type;   // DEPOSIT, WITHDRAW, TRANSFER_IN, TRANSFER_OUT
    double amount;
    string note;
    string timestamp;

    string toString() const {
        return "[" + timestamp + "] " + type + " LKR " +
               to_string((int)amount) + " — " + note;
    }
};

// ── Account Class ───────────────────────────────────────────
class Account {
private:
    int id;
    string owner;
    string pin;
    double balance;
    vector<Transaction> history;

    static int nextId;

    string currentTime() {
        time_t t = time(nullptr);
        char buf[20];
        strftime(buf, sizeof(buf), "%Y-%m-%d %H:%M", localtime(&t));
        return string(buf);
    }

public:
    Account(const string& name, const string& pin, double initial = 0.0)
        : id(nextId++), owner(name), pin(pin), balance(initial) {}

    Account(int id, const string& name, const string& pin, double bal)
        : id(id), owner(name), pin(pin), balance(bal) {
        if (id >= nextId) nextId = id + 1;
    }

    bool verifyPin(const string& p) const { return p == pin; }

    bool deposit(double amount, const string& note = "Cash deposit") {
        if (amount <= 0) return false;
        balance += amount;
        history.push_back({"DEPOSIT", amount, note, currentTime()});
        return true;
    }

    bool withdraw(double amount, const string& note = "Cash withdrawal") {
        if (amount <= 0 || amount > balance) return false;
        balance -= amount;
        history.push_back({"WITHDRAW", amount, note, currentTime()});
        return true;
    }

    void receiveTransfer(double amount, int fromId) {
        balance += amount;
        history.push_back({"TRANSFER_IN", amount, "From Acc#" + to_string(fromId), currentTime()});
    }

    bool sendTransfer(double amount, int toId) {
        if (amount <= 0 || amount > balance) return false;
        balance -= amount;
        history.push_back({"TRANSFER_OUT", amount, "To Acc#" + to_string(toId), currentTime()});
        return true;
    }

    void printSummary() const {
        cout << "+------------------------------------------+" << endl;
        cout << "| Account #" << setw(4) << id << "  Owner: " << setw(20) << left << owner << "|" << endl;
        cout << "| Balance: LKR " << setw(12) << fixed << setprecision(2) << right << balance << "              |" << endl;
        cout << "+------------------------------------------+" << endl;
    }

    void printStatement() const {
        cout << "\n=== Statement: " << owner << " (Acc #" << id << ") ===" << endl;
        cout << "Balance: LKR " << fixed << setprecision(2) << balance << endl;
        if (history.empty()) { cout << "No transactions yet." << endl; return; }
        cout << "\nTransaction History (" << history.size() << " records):" << endl;
        int start = max(0, (int)history.size() - 10);
        for (int i = start; i < (int)history.size(); i++) {
            cout << "  " << history[i].toString() << endl;
        }
    }

    // Getters
    int getId() const { return id; }
    string getOwner() const { return owner; }
    double getBalance() const { return balance; }

    // CSV serialization
    string toCSV() const {
        string cleanOwner = owner;
        replace(cleanOwner.begin(), cleanOwner.end(), ',', ' ');
        string cleanPin = pin;
        replace(cleanPin.begin(), cleanPin.end(), ',', ' ');
        return to_string(id) + "," + cleanOwner + "," + cleanPin + "," + to_string(balance);
    }
};
int Account::nextId = 1001;

// ── Bank Class ──────────────────────────────────────────────
class Bank {
private:
    vector<Account*> accounts;
    const string DATA_FILE = "bank_data.txt";

    Account* findAccount(int id) {
        for (auto* acc : accounts)
            if (acc->getId() == id) return acc;
        return nullptr;
    }

    void save() {
        ofstream f(DATA_FILE);
        for (auto* acc : accounts) f << acc->toCSV() << "\n";
    }

    int readInt(const string& prompt) {
        int val;
        while (true) {
            cout << prompt;
            if (cin >> val) {
                cin.ignore(numeric_limits<streamsize>::max(), '\n');
                return val;
            }
            cin.clear();
            cin.ignore(numeric_limits<streamsize>::max(), '\n');
            cout << "Invalid number. Please try again.\n";
        }
    }

    double readDouble(const string& prompt) {
        double val;
        while (true) {
            cout << prompt;
            if (cin >> val) {
                cin.ignore(numeric_limits<streamsize>::max(), '\n');
                return val;
            }
            cin.clear();
            cin.ignore(numeric_limits<streamsize>::max(), '\n');
            cout << "Invalid amount. Please try again.\n";
        }
    }

    void load() {
        ifstream f(DATA_FILE);
        if (!f.is_open()) return;
        string line;
        while (getline(f, line)) {
            if (line.empty()) continue;
            istringstream ss(line);
            string tok;
            vector<string> parts;
            while (getline(ss, tok, ',')) parts.push_back(tok);
            if (parts.size() >= 4) {
                try {
                    accounts.push_back(new Account(
                        stoi(parts[0]), parts[1], parts[2], stod(parts[3])
                    ));
                } catch (...) {}
            }
        }
        cout << "Loaded " << accounts.size() << " account(s).\n";
    }

public:
    Bank() { load(); }
    ~Bank() { save(); for (auto* a : accounts) delete a; }

    void run() {
        printBanner();
        int choice;
        do {
            cout << "\n=============================" << endl;
            cout << "  PO BANK — Main Menu" << endl;
            cout << "=============================" << endl;
            cout << "  1. Create Account" << endl;
            cout << "  2. Login to Account" << endl;
            cout << "  3. List All Accounts" << endl;
            cout << "  4. Delete Account" << endl;
            cout << "  0. Exit" << endl;
            choice = readInt("Choose: ");

            switch (choice) {
                case 1: createAccount(); break;
                case 2: loginMenu(); break;
                case 3: listAccounts(); break;
                case 4: deleteAccount(); break;
                case 0: cout << "Goodbye! Data saved.\n"; break;
                default: cout << "Invalid option.\n";
            }
        } while (choice != 0);
    }

    void createAccount() {
        cout << "\n-- Create New Account --" << endl;
        cout << "Owner Name : "; string name; getline(cin, name);
        if (name.empty()) { cout << "Owner name cannot be empty.\n"; return; }
        cout << "Set PIN    : "; string pin; getline(cin, pin);
        if (pin.empty()) { cout << "PIN cannot be empty.\n"; return; }
        double amt = readDouble("Initial Deposit (LKR): ");
        if (amt < 0) amt = 0;
        Account* acc = new Account(name, pin, amt);
        accounts.push_back(acc);
        cout << "Account created! ID: " << acc->getId() << endl;
        save();
    }

    void loginMenu() {
        int id = readInt("\nEnter Account ID: ");
        Account* acc = findAccount(id);
        if (!acc) { cout << "Account not found.\n"; return; }
        cout << "Enter PIN: "; string pin; getline(cin, pin);
        if (!acc->verifyPin(pin)) { cout << "Wrong PIN!\n"; return; }
        cout << "Welcome, " << acc->getOwner() << "!\n";

        int choice;
        do {
            cout << "\n-- Account Menu --" << endl;
            cout << "  1. Deposit" << endl;
            cout << "  2. Withdraw" << endl;
            cout << "  3. Transfer" << endl;
            cout << "  4. Balance" << endl;
            cout << "  5. Statement" << endl;
            cout << "  0. Logout" << endl;
            choice = readInt("Choose: ");

            switch (choice) {
                case 1: {
                    double a = readDouble("Amount to deposit: ");
                    if (acc->deposit(a)) { cout << "Deposited LKR " << a << "\n"; save(); }
                    else cout << "Invalid amount.\n";
                    break;
                }
                case 2: {
                    double a = readDouble("Amount to withdraw: ");
                    if (acc->withdraw(a)) { cout << "Withdrawn LKR " << a << "\n"; save(); }
                    else cout << "Insufficient funds or invalid amount.\n";
                    break;
                }
                case 3: {
                    int tid = readInt("Transfer to Account ID: ");
                    if (tid == acc->getId()) { cout << "Cannot transfer to the same account.\n"; break; }
                    Account* to = findAccount(tid);
                    if (!to) { cout << "Target account not found.\n"; break; }
                    double a = readDouble("Amount: ");
                    if (acc->sendTransfer(a, tid)) {
                        to->receiveTransfer(a, acc->getId());
                        cout << "Transferred LKR " << a << " to " << to->getOwner() << "\n";
                        save();
                    } else cout << "Insufficient funds or invalid amount.\n";
                    break;
                }
                case 4: acc->printSummary(); break;
                case 5: acc->printStatement(); break;
                case 0: cout << "Logged out.\n"; break;
                default: cout << "Invalid.\n";
            }
        } while (choice != 0);
    }

    void listAccounts() {
        if (accounts.empty()) { cout << "No accounts.\n"; return; }
        cout << "\n-- All Accounts (" << accounts.size() << ") --\n";
        for (auto* acc : accounts) acc->printSummary();
    }

    void deleteAccount() {
        int id = readInt("Enter Account ID to delete: ");
        cout << "Admin PIN: "; string p; getline(cin, p);
        if (p != "admin123") { cout << "Wrong admin PIN.\n"; return; }
        for (auto it = accounts.begin(); it != accounts.end(); ++it) {
            if ((*it)->getId() == id) {
                cout << "Deleted account of " << (*it)->getOwner() << "\n";
                delete *it;
                accounts.erase(it);
                save();
                return;
            }
        }
        cout << "Account not found.\n";
    }

    void printBanner() {
        cout << "╔════════════════════════════════╗" << endl;
        cout << "║   DEMO BANK — Banking System  ║" << endl;
        cout << "║   Edition: Demo Suite          ║" << endl;
        cout << "║   Language: C++                ║" << endl;
        cout << "╚════════════════════════════════╝" << endl;
    }
};

int main() {
    Bank bank;
    bank.run();
    return 0;
}
