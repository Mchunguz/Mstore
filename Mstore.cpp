// C++ Program to demonstrate simple ATM
#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <iomanip>
#include <cstdlib> // For system("pause") function
#include <ctime>   // For timestamp in transactions
#include <limits>
#include <cstring>
#include <filesystem>
#include <openssl/ssl.h>
#include <openssl/sha.h> // Include the OpenSSL library for SHA-256 hashing

using namespace std;

// Structure to represent account information
struct Account {
    string username;
    string hashedPassword; // For demonstration; in practice, use secure methods for password storage
    int pin;         // Added PIN for each user
    float balance;
    vector<string> transactions; // Vector to store transaction history
};

// Structure to represent user roles
enum class UserRole { Normal, Admin };

// Function declarations
void displayMainMenu();
void displayAdminMenu();
void displayUserMenu(Account& account);
void checkBalance(const Account& account);
void depositMoney(Account& account);
void withdrawMoney(Account& account);
void viewAccountDetails(const Account& account);
void changePassword(Account& account);
void viewTransactionHistory(const Account& account); // New function for transaction history
void logTransaction(Account& account, const string& transaction);
void addUser();
void deleteUser();
void viewAllUsers();
void editUser();
bool authenticateUser(const string& username, const string& hashedPassword, int pin, UserRole& role);
//bool authenticateUser(const string& username, const string& password, int pin, UserRole& role);
Account loadAccount(const string& username);
void saveAccount(const Account& account);
void clearInputBuffer();
string currentDateTime();
void initializeAdmin(); // Ensure at least one admin exists

// Global variables
Account currentUser;
UserRole currentUserRole;

// Function to hash Passwords(Using SHA-256)
string hashPassword(const string& password) {
    unsigned char hash[SHA256_DIGEST_LENGTH];
    SHA256(reinterpret_cast<const unsigned char*>(password.c_str()), password.size(), hash);
    stringstream ss;
    for (auto byte : hash)
    {
        ss <<hex << setw(2) <<setfill('0') <<static_cast<int>(byte);
    }
    return ss.str();
    
}

// Function to get a valid integer input
int getValidInteger() {
    int number;
    while (true)
    {
        cin >>number;
        if (cin.fail())
        {
            cin.clear();
            cin.ignore(numeric_limits<streamsize>::max(),'\n');
            cout <<"Invalid input. Please enter a valid integer."<<endl;
        }else
        {
            cin.ignore(numeric_limits<streamsize>::max(),'\n');
            return number;
        }
        
        
    }
    
}
// Main function
void displayMainMenu() {
    string username, password;
    int choice, pin = 0;
    UserRole role;

    cout << "===== WELCOME TO MSTORE =====" << endl;
    cout << "1. Login " << endl;
    cout << "2. Register " << endl;
    cout << "Enter your choice: ";

    choice = getValidInteger();

    switch (choice) {
        case 1:
            cout << "Enter your username: ";
            cin >> username;
            cout << "Enter your password: ";
            cin >> password;
            cout << "Enter your PIN (enter 0 if not applicable): ";
            pin = getValidInteger();

            if (authenticateUser(username, hashPassword(password), pin, role)) {
                currentUser = loadAccount(username);
                currentUserRole = role;

                if (currentUserRole == UserRole::Admin) {
                    cout << "Authentication successful. Welcome, Admin!" << endl;
                    displayAdminMenu();
                } else {
                    cout << "Authentication successful. Welcome, " << username << "!" << endl;
                    displayUserMenu(currentUser);
                }
            } else {
                cout << "Authentication failed. Invalid username, password or PIN." << endl;
            }
            break;

        case 2:
            addUser();
            break;

        default:
            cout << "Invalid choice. Please enter 1 for login or 2 for registration." << endl;
            break;
    }
}

// Main Driven
int main(int argc, char const *argv[])
{
    
    displayMainMenu();

    return 0;
}

// Function to authenticate user
bool authenticateUser(const string& username, const string& hashedPassword, int pin, UserRole& role) {
    ifstream fin("account.txt");
    if (!fin)
    {
       cout <<"Error opening file!"<<endl;
       return false;
    }
    string line;
    while (getline(fin, line))
    {
        stringstream ss(line);
        Account account;

        ss >>account.username >>account.hashedPassword >>account.pin >>account.balance;
        if (account.username == username && account.hashedPassword == hashedPassword)
        {
            // Determine role and validate PIN for regular user
            if (account.pin == 0)
            {
                /* Admin */
                role = UserRole::Admin;
            }else if(account.pin == pin)
            {
                /* Regular user */
                role = UserRole::Normal;
            } else {
                continue; // Invalid PIN
            }
            
            fin.close();
            return true;
        }
        
    }
    fin.close();
    return false;
    
}

// Function to display admin menu
void displayAdminMenu() {
    int choice;
    while (true) {
        cout << "\n===== ADMIN MENU =====" << endl;
        cout << "1. Add User" << endl;
        cout << "2. Delete User" << endl;
        cout << "3. View All Users" << endl;
        cout << "4. Edit User" << endl;
        cout << "5. Exit" << endl;
        cout << "Enter your choice: ";
        cin >> choice;

        switch (choice) {
            case 1:
                addUser();
                break;
            case 2:
                deleteUser();
                break;
            case 3:
                viewAllUsers();
                break;
            case 4:
                editUser();
                break;
            case 5:
                cout << "Exiting Admin menu." << endl;
                return;
            default:
                cout << "Invalid choice. Please enter a valid option." << endl;
        }
        cout << endl;
    }
}

// Function to display user menu
void displayUserMenu(Account& account) {
    int choice;
    while (true) {
        cout << "===== USER MENU =====" << endl;
        cout << "1. Check Balance" << endl;
        cout << "2. Deposit Money" << endl;
        cout << "3. Withdraw Money" << endl;
        cout << "4. View Account Details" << endl;
        cout << "5. Change Password" << endl;
        cout << "6. Exit" << endl;
        cout << "Enter your choice: ";
        cin >> choice;

        switch (choice) {
            case 1:
                checkBalance(account);
                break;
            case 2:
                depositMoney(account);
                break;
            case 3:
                withdrawMoney(account);
                break;
            case 4:
                viewAccountDetails(account);
                break;
            case 5:
                changePassword(account);
                break;
            case 6:
                cout << "Exiting User menu." << endl;
                return;
            default:
                cout << "Invalid choice. Please enter a valid option.\n" << endl;
        }
        cout << endl;
    }
}

// Function to check balance
void checkBalance(const Account& account) {
    cout << "Your balance is: $" << fixed << setprecision(2) << account.balance << endl;
}

// Function to deposit money
void depositMoney(Account& account) {
    ifstream fin("account.txt");
    ofstream fout("temp.txt");

    if (!fin || !fout) {
        cout << "Error in file operation." << endl;
        return;
    }

    string line;
    bool accountFound = false;

    while (getline(fin, line)) {
        stringstream ss(line);
        Account tempAccount;
        ss >> tempAccount.username >> tempAccount.hashedPassword >> tempAccount.pin >> tempAccount.balance;

        if (tempAccount.username == account.username) {
            float amount;
            cout << "Enter amount to deposit: $";
            cin >> amount;
            tempAccount.balance += amount;
            // Record transaction
            time_t now = time(0);
            string timestamp = ctime(&now);
            string transaction = "Deposit: +" + to_string(amount) + " on " + timestamp;
            tempAccount.transactions.push_back(transaction);
            accountFound = true;
            cout << "Deposit successful. Your new balance is: $" << fixed << setprecision(2) << tempAccount.balance << endl;
        }

        fout << tempAccount.username << " " << tempAccount.hashedPassword << " " << tempAccount.pin << " " << tempAccount.balance << endl;
    }

    if (!accountFound) {
        cout << "Account not found." << endl;
    }

    fin.close();
    fout.close();

    remove("account.txt");
    rename("temp.txt", "account.txt");
}

// Function to withdraw money
void withdrawMoney(Account& account) {
    float amount;
    cout << "Enter amount to withdraw: $";
    cin >> amount;
    if (account.balance >= amount) {
        account.balance -= amount;
        // Record transaction
        time_t now = time(0);
        string timestamp = ctime(&now);
        string transaction = "Withdrawal: -" + to_string(amount) + " on " + timestamp;
        account.transactions.push_back(transaction);
        cout << "Withdrawal successful. Your new balance is: $" << fixed << setprecision(2) << account.balance << endl;
        saveAccount(account); // Save updated account information
    } else {
        cout << "Insufficient funds. Withdrawal failed.\n" << endl;
    }
}

// Function to view account details
void viewAccountDetails(const Account& account) {
    cout << "Username: " << account.username << endl;
    cout << "Balance: $" << fixed << setprecision(2) << account.balance << endl;
}

// Function to add a new user (admin only)
void addUser() {
    if (currentUserRole != UserRole::Admin) {
        cout << "Only admin can add new users." << endl;
        return;
    }

    ifstream fin("account.txt");
    ofstream fout("temp.txt");

    if (!fin || !fout) {
        cout << "Error in file operation!" << endl;
        return;
    }

    string newUsername, newPassword;
    int newPin;
    bool usernameExists = false;

    cout << "Enter new username: ";
    cin >> newUsername;

    string line;
    while (getline(fin, line)) {
        stringstream ss(line);
        Account existingAccount;
        ss >> existingAccount.username >> existingAccount.hashedPassword >> existingAccount.pin >> existingAccount.balance;

        if (existingAccount.username == newUsername) {
            usernameExists = true;
            break;
        }
    }

    if (usernameExists) {
        cout << "Username already exists. Please choose a different username." << endl;
        fin.close();
        fout.close();
        return;
    }

    cout << "Enter password for " << newUsername << ": ";
    cin >> newPassword;
    string hashedPassword = hashPassword(newPassword);

    char roleChoice;
    cout << "Is this user an admin (y/n)? ";
    cin >> roleChoice;
    UserRole newUserRole;
    if (roleChoice == 'y' || roleChoice == 'Y') {
        newUserRole = UserRole::Admin;
        newPin = 0; // Admin does not use PIN
    } else {
        newUserRole = UserRole::Normal;
        cout << "Enter PIN for " << newUsername << ": ";
        cin >> newPin;
    }

    Account newAccount;
    newAccount.username = newUsername;
    newAccount.hashedPassword = hashedPassword;
    newAccount.pin = newPin;
    newAccount.balance = 0.00;

    fout << newAccount.username << " " << newAccount.hashedPassword << " " << newAccount.pin << " " << newAccount.balance << endl;

    while (getline(fin, line)) {
        fout << line << endl;
    }

    fin.close();
    fout.close();

    remove("account.txt");
    rename("temp.txt", "account.txt");

    cout << "User added successfully." << endl;
}


// Function to change password
void changePassword(Account& account) {
    string oldPassword, newPassword;
    
    cout << "Enter current password: ";
    cin >> oldPassword;

    // Verify old password
    if (oldPassword != account.hashedPassword) {
        cout << "Incorrect current password." << endl;
        return;
    }

    cout << "Enter new password: ";
    cin >> newPassword;

    // Update password in account object
    account.hashedPassword = newPassword;

    // Save updated account information
    saveAccount(account);
    cout << "Password changed successfully." << endl;
}

// Function to load Account

Account loadAccount(const string& username) {
    ifstream fin("account.txt");
    if (!fin) {
        cout << "Error opening file." << endl;
        exit(1); // Handle the error appropriately
    }

    string line;
    while (getline(fin, line)) {
        stringstream ss(line);
        Account account;
        ss >> account.username >> account.hashedPassword >> account.pin >> account.balance;

        if (account.username == username) {
            fin.close();
            return account;
        }
    }

    fin.close();

    // Return an empty account object if not found
    Account emptyAccount;
    return emptyAccount;
}

// Function to delete user
void deleteUser() {
    if (currentUserRole != UserRole::Admin) {
        cout << "Only admin can delete users." << endl;
        return;
    }

    string usernameToDelete;
    cout << "Enter username to delete: ";
    cin >> usernameToDelete;

    ifstream fin("account.txt");
    ofstream fout("temp.txt");

    if (!fin || !fout) {
        cout << "Error opening files." << endl;
        return;
    }

    string line;
    bool userFound = false;

    while (getline(fin, line)) {
        stringstream ss(line);
        Account account;
        ss >> account.username >> account.hashedPassword >> account.pin >> account.balance;

        if (account.username == usernameToDelete) {
            userFound = true;
        } else {
            fout << line << endl; // Copy non-matching lines to temp file
        }
    }

    fin.close();
    fout.close();

    if (userFound) {
        remove("account.txt");
        rename("temp.txt", "account.txt");
        cout << "User " << usernameToDelete << " deleted successfully." << endl;
    } else {
        remove("temp.txt"); // Clean up temp file if user not found
        cout << "User " << usernameToDelete << " does not exist." << endl;
    }
}


// Function to view all users (admin only)
void viewAllUsers() {
    if (currentUserRole != UserRole::Admin) {
        cout << "Only admin can view all users." << endl;
        return;
    }

    cout << "===== LIST OF USERS =====" << endl;
    ifstream inFile("account.txt");

    if (inFile.is_open()) {
        string line;
        while (getline(inFile, line)) {
            stringstream ss(line);
            Account account;
            ss >> account.username; // Only read the username
            cout << account.username << endl;
        }
        inFile.close();
    } else {
        cout << "Error: Unable to open account file." << endl;
    }
}


// Function to edit user details (admin only)
void editUser() {
    if (currentUserRole != UserRole::Admin) {
        cout << "Only admin can edit user details." << endl;
        return;
    }

    string usernameToEdit;
    cout << "Enter username to edit: ";
    cin >> usernameToEdit;

    ifstream fin("account.txt");
    ofstream fout("temp.txt");

    if (!fin || !fout) {
        cout << "Error opening files." << endl;
        return;
    }

    string line;
    bool userFound = false;
    Account accountToEdit;

    while (getline(fin, line)) {
        stringstream ss(line);
        Account account;
        ss >> account.username >> account.hashedPassword >> account.pin >> account.balance;

        if (account.username == usernameToEdit) {
            userFound = true;
            // Modify user details
            cout << "Editing details for user " << usernameToEdit << endl;
            cout << "Enter new password: ";
            cin >> account.hashedPassword;
            cout << "Enter new PIN: ";
            cin >> account.pin;
            cout << "Enter new balance: $";
            cin >> account.balance;
        }

        fout << account.username << " " << account.hashedPassword << " " << account.pin << " " << account.balance << endl;
    }

    fin.close();
    fout.close();

    if (userFound) {
        remove("account.txt");
        rename("temp.txt", "account.txt");
        cout << "User " << usernameToEdit << " edited successfully." << endl;
    } else {
        remove("temp.txt"); // Clean up temp file if user not found
        cout << "User " << usernameToEdit << " does not exist." << endl;
    }
}

// Function to Save Account
void saveAccount(const Account& account) {
    ifstream fin("account.txt");
    ofstream fout("temp.txt");

    if (!fin || !fout) {
        cout << "Error opening files." << endl;
        return;
    }

    string line;
    bool accountUpdated = false;

    while (getline(fin, line)) {
        stringstream ss(line);
        Account tempAccount;
        ss >> tempAccount.username >> tempAccount.hashedPassword >> tempAccount.pin >> tempAccount.balance;

        if (tempAccount.username == account.username) {
            // Update the existing account record with the new data
            fout << account.username << " " << account.hashedPassword << " " << account.pin << " " << account.balance << endl;
            accountUpdated = true;
        } else {
            // Copy the other account records unchanged
            fout << line << endl;
        }
    }

    // If the account was not found in the file, add it at the end
    if (!accountUpdated) {
        fout << account.username << " " << account.hashedPassword << " " << account.pin << " " << account.balance << endl;
    }

    fin.close();
    fout.close();

    // Replace the old file with the updated file
    remove("account.txt");
    rename("temp.txt", "account.txt");

    cout << "Account details updated successfully." << endl;
}

// Functions to log transactions. Add a function to view transaction history.
void logTransaction(Account& account, const string& transaction) {
    ofstream fout(account.username + "_transactions.txt", ios::app);
    if (!fout) {
        cout << "Error opening transaction log file." << endl;
        return;
    }
    fout << transaction << endl;
    fout.close();
}

void viewTransactionHistory(const Account& account) {
    ifstream fin(account.username + "_transactions.txt");
    if (!fin) {
        cout << "Error opening transaction history file." << endl;
        return;
    }

    string line;
    while (getline(fin, line)) {
        cout << line << endl;
    }
    fin.close();
}

// Function to initialize Admin
/*
void initializeAdmin() {
    ifstream fin("account.txt");
    if (!fin.is_open()) {
        cerr << "Error opening account file for reading." << endl;
        exit(1); // Or handle the error as appropriate
    }

    if (fin.peek() == ifstream::traits_type::eof()) {
        // File is empty; create an admin account
        string adminUsername, adminPassword;
        int adminPin = -1; // Sentinel value for admin

        cout << "No admin account found. Setting up an admin account." << endl;
        cout << "Enter admin username: ";
        cin >> adminUsername;
        cout << "Enter admin password: ";
        cin >> adminPassword;

        // Create and save admin Account
        Account adminAccount;
        adminAccount.username = adminUsername;
        adminAccount.password = adminPassword;
        adminAccount.pin = adminPin;
        adminAccount.balance = 0.00;

        ofstream fout("account.txt");
        if (!fout) {
            cout << "Error creating account file." << endl;
            return;
        }

        fout << adminAccount.username << " " << adminAccount.password << " " << adminAccount.pin << " " << adminAccount.balance << endl;
        fout.close();
        cout << "Admin account created successfully." << endl;
    }
    fin.close();
}*/

/*


// Function to authenticate user
bool authenticateUser(const string& username, const string& hashedPassword, int pin, UserRole& role) {
    ifstream fin("account.txt");
    if (!fin) {
        cout << "Error opening file!" << endl;
        return false;
    }
    string line;
    while (getline(fin, line)) {
        stringstream ss(line);
        Account account;

        ss >> account.username >> account.hashedPassword >> account.pin >> account.balance;
        if (account.username == username && account.hashedPassword == hashedPassword) {
            if (account.pin == 0) {
                role = UserRole::Admin;
            } else if (account.pin == pin) {
                role = UserRole::Normal;
            } else {
                continue;
            }
            fin.close();
            return true;
        }
    }
    fin.close();
    return false;
}

*/

// Main Function of Old version
/*
    string username, password;
    int choice, pin = 0; // Initialize pin to a default value
    UserRole role;

    cout << "===== WELCOME TO MSTORE =====" << endl;
    cout << "1. Login "<<endl;
    cout << "2. Register "<<endl;
    cout <<"Enter your choice: ";
    cin >> choice;
    
    switch (choice)
    {
    case 1:
        cout <<"Enter your username: ";
        cin>>username;
        cout <<"Enter your password: ";
        cin>>password;
        cout <<"Enter your PIN (enter 0 if not applicable): ";
        cin>>pin;

        if (authenticateUser(username, password, pin, role))
        {
            currentUser = loadAccount(username);
            currentUserRole = role;

            if (currentUserRole == UserRole::Admin) {
                    cout << "Authentication successful. Welcome, Admin!" << endl;
                    displayAdminMenu();
            } else {
                    cout << "Authentication successful. Welcome, " << username << "!" << endl;
                    displayUserMenu(currentUser);
            }
            
        }else {
            cout <<"Authentication failed. Invalid username, password or PIN."<<endl;
        }
        
        break;
    
    case 2: // Rigister
        addUser();
    default:
        cout <<"Invalid choice. Please enter 1 for login or 2 for registration: ";
        break;
    }
    */
