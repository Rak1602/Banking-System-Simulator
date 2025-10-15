#include <iostream>
#include <fstream>
#include <sstream>
#include <map>
#include <string>
#include <queue>
#include <algorithm>
#include <random>
#include <unordered_set>
#include <ctime>
#include <vector>
#include <unordered_map>
#include <stack>

using namespace std;

// ======================== CARD BASE CLASS AND DERIVED CLASSES ========================

class Card {
protected:
    string accountNumber;
    string cardNumber;
    string expirationDate;
    string cvv;
    string status;
    string pin;
    string issueDate;
    string cardType;
    string cardHolderName;

public:
    Card() {}
    Card(string accNum, string cNum, string expDate, string cvv,
         string status, string pin, string issueDate, string cType, string holderName)
        : accountNumber(accNum), cardNumber(cNum), expirationDate(expDate), cvv(cvv),
          status(status), pin(pin), issueDate(issueDate), cardType(cType), cardHolderName(holderName) {}

    virtual ~Card() {} // Virtual destructor

    virtual void displayCardDetails() const {
        cout << "Account Number: " << accountNumber << "\nCard Number: " << cardNumber
             << "\nExpiration Date: " << expirationDate << "\nCVV: " << cvv
             << "\nStatus: " << status << "\nPIN: " << pin << "\nIssue Date: " << issueDate
             << "\nCard Type: " << cardType << "\nCard Holder Name: " << cardHolderName << endl;
    }

    virtual Card* clone() const = 0;

    string toCSV() const {
        return accountNumber + "," + cardNumber + "," + expirationDate + "," + cvv + "," +
               status + "," + pin + "," + issueDate + "," + cardType + "," + cardHolderName;
    }

    // Getters
    string getAccountNumber() const { return accountNumber; }
    string getCardNumber() const { return cardNumber; }
    string getExpirationDate() const { return expirationDate; }
    string getCVV() const { return cvv; }
    string getStatus() const { return status; }
    string getPIN() const { return pin; }
    string getIssueDate() const { return issueDate; }
    string getCardType() const { return cardType; }
    string getCardHolderName() const { return cardHolderName; }
    
    void setStatus(const string& newStatus) {
        status = newStatus;
    }
};

class DebitCard : public Card {
public:
    DebitCard(string accNum, string cNum, string expDate, string cvv,
              string status, string pin, string issueDate, string cType, string holderName)
        : Card(accNum, cNum, expDate, cvv, status, pin, issueDate, cType, holderName) {}

    void displayCardDetails() const override {
        cout << "Debit Card Details:\n";
        Card::displayCardDetails();
    }

    Card* clone() const override {
        return new DebitCard(*this);
    }
};

class Creditcard : public Card {
public:
    Creditcard(string accNum, string cNum, string expDate, string cvv,
               string status, string pin, string issueDate, string cType, string holderName)
        : Card(accNum, cNum, expDate, cvv, status, pin, issueDate, cType, holderName) {}

    void displayCardDetails() const override {
        cout << "Credit Card Details:\n";
        Card::displayCardDetails();
    }

    Card* clone() const override {
        return new Creditcard(*this);
    }
};

// ======================== CARD MANAGEMENT CLASS (ADMIN) ========================

class CardManagement {
private:
    unordered_map<string, Card*> debitCards;
    unordered_map<string, Card*> creditCards;
    string debitCardFile;
    string creditCardFile;

    void saveCards(const unordered_map<string, Card*>& cardMap, const string& filename) {
        ofstream file(filename, ios::app);
        if (!file.is_open()) {
            cerr << "Error opening file: " << filename << endl;
            return;
        }

        ifstream checkFile(filename);
        bool fileIsEmpty = (checkFile.peek() == ifstream::traits_type::eof());
        checkFile.close();

        if (fileIsEmpty) {
            file << "AccountNumber,CardNumber,ExpirationDate,CVV,Status,PIN,IssueDate,CardType,CardHolderName" << endl;
        }

        unordered_set<string> existingRecords;
        ifstream existingFile(filename);
        if (!existingFile.is_open()) {
            cerr << "Error opening file: " << filename << endl;
            file.close();
            return;
        }
        string line;
        while (getline(existingFile, line)) {
            existingRecords.insert(line);
        }
        existingFile.close();

        for (const auto& pair : cardMap) {
            const Card* card = pair.second;
            string cardCSV = card->toCSV();
            if (existingRecords.find(cardCSV) == existingRecords.end()) {
                file << cardCSV << endl;
            }
        }

        file.close();
    }

    void loadCards(const string& filename, unordered_map<string, Card*>& cardMap, bool isDebit) {
        ifstream file(filename);
        if (!file.is_open()) {
            cerr << "Error opening file: " << filename << endl;
            return;
        }
        string line;
        getline(file, line); // Skip header
        while (getline(file, line)) {
            stringstream ss(line);
            string accNum, cNum, expDate, cvv, status, pin, issueDate, cType, holderName;
            getline(ss, accNum, ',');
            getline(ss, cNum, ',');
            getline(ss, expDate, ',');
            getline(ss, cvv, ',');
            getline(ss, status, ',');
            getline(ss, pin, ',');
            getline(ss, issueDate, ',');
            getline(ss, cType, ',');
            getline(ss, holderName, ',');
            Card* card = isDebit
                ? (Card*)new DebitCard(accNum, cNum, expDate, cvv, status, pin, issueDate, cType, holderName)
                : (Card*)new Creditcard(accNum, cNum, expDate, cvv, status, pin, issueDate, cType, holderName);
            cardMap[cNum] = card;
        }
        file.close();
    }

    string generateRandomNumber(int length) {
        static random_device rd;
        static mt19937 gen(rd());
        uniform_int_distribution<> dis(0, 9);
        string number;
        for (int i = 0; i < length; ++i) {
            number += to_string(dis(gen));
        }
        return number;
    }

public:
    CardManagement(const string& debitFile, const string& creditFile)
        : debitCardFile(debitFile), creditCardFile(creditFile) {
        loadCards(debitCardFile, debitCards, true);
        loadCards(creditCardFile, creditCards, false);
    }

    ~CardManagement() {
        for (auto& pair : debitCards) delete pair.second;
        for (auto& pair : creditCards) delete pair.second;
    }

    void issueCard(bool isDebit, const Card& card) {
        unordered_map<string, Card*>& cardMap = isDebit ? debitCards : creditCards;
        cardMap[card.getCardNumber()] = card.clone();
        saveCards(cardMap, isDebit ? debitCardFile : creditCardFile);
    }

    void displayAllCards(bool isDebit) const {
        const unordered_map<string, Card*>& cardMap = isDebit ? debitCards : creditCards;
        for (const auto& pair : cardMap) {
            pair.second->displayCardDetails();
            cout << endl;
        }
    }

    void deactivateCard() {
        char cardTypeChoice;
        cout << "Deactivate which type of card? (d for Debit, c for Credit): ";
        cin >> cardTypeChoice;

        bool isDebit = (cardTypeChoice == 'd');

        string cardNumber;
        cout << "Enter the card number to deactivate: ";
        cin >> cardNumber;

        cout << "Reason for deactivation:\n";
        cout << "1. User's request\n";
        cout << "2. Card expired\n";
        int reasonChoice;
        cin >> reasonChoice;
        string reason = (reasonChoice == 1) ? "user's request" : "card expired";

        string filename = isDebit ? debitCardFile : creditCardFile;
        fstream file(filename, ios::in | ios::out);

        if (!file.is_open()) {
            cerr << "Error opening file.\n";
            return;
        }

        string line;
        stringstream buffer;
        bool cardFound = false;

        while (getline(file, line)) {
            stringstream ss(line);
            string accNum, cNum, expDate, cvv, status, pin, issueDate, cType, holderName;
            getline(ss, accNum, ',');
            getline(ss, cNum, ',');
            getline(ss, expDate, ',');
            getline(ss, cvv, ',');
            getline(ss, status, ',');
            getline(ss, pin, ',');
            getline(ss, issueDate, ',');
            getline(ss, cType, ',');
            getline(ss, holderName, ',');

            if (cNum == cardNumber) {
                cardFound = true;
                status = "deactive";
            }
            buffer << accNum << ',' << cNum << ',' << expDate << ',' << cvv << ','
                   << status << ',' << pin << ',' << issueDate << ',' << cType
                   << ',' << holderName << '\n';
        }

        file.close();

        if (cardFound) {
            ofstream outFile(filename);
            if (!outFile.is_open()) {
                cerr << "Error opening file for writing.\n";
                return;
            }
            outFile << buffer.str();
            outFile.close();
            cout << "Card " << cardNumber << " has been deactivated due to " << reason << ".\n";
        } else {
            cout << "Card " << cardNumber << " not found.\n";
        }
    }

    void interactiveIssueCard() {
        string accountNumber = generateRandomNumber(10);
        string cardNumber = generateRandomNumber(16);
        string pin = generateRandomNumber(4);
        string expirationDate, cvv, status = "active", issueDate, cardType, cardHolderName;

        cout << "Generated Account Number: " << accountNumber << endl;
        cout << "Generated Card Number: " << cardNumber << endl;
        cout << "Generated PIN: " << pin << endl;
        cout << "Enter expiration date (MM-YYYY): ";
        cin >> expirationDate;
        cout << "Enter CVV: ";
        cin >> cvv;
        cout << "Enter issue date (YYYY-MM-DD): ";
        cin >> issueDate;
        cout << "Enter card holder name: ";
        cin.ignore();
        getline(cin, cardHolderName);
        cout << "Is this a debit card or credit card? (d/c): ";
        char cardTypeChoice;
        cin >> cardTypeChoice;

        bool isDebit = (cardTypeChoice == 'd');

        if (isDebit) {
            cout << "Select Debit Card Type:\n";
            cout << "1. Maestro Debit Card\n";
            cout << "2. Visa Electro Debit Card\n";
            cout << "3. RuPay Debit Card\n";
            cout << "4. Contactless Debit Card\n";
            cout << "5. MasterCard Debit Card\n";
            cout << "6. Visa Debit Card\n";
            int choice;
            cin >> choice;
            switch (choice) {
                case 1: cardType = "Maestro Debit Card"; break;
                case 2: cardType = "Visa Electro Debit Card"; break;
                case 3: cardType = "RuPay Debit Card"; break;
                case 4: cardType = "Contactless Debit Card"; break;
                case 5: cardType = "MasterCard Debit Card"; break;
                case 6: cardType = "Visa Debit Card"; break;
                default: cout << "Invalid choice! Defaulting to 'Visa Debit Card'.\n"; cardType = "Visa Debit Card"; break;
            }
            DebitCard newDebitCard(accountNumber, cardNumber, expirationDate, cvv, status, pin, issueDate, cardType, cardHolderName);
            issueCard(true, newDebitCard);
        } else {
            cout << "Select Credit Card Type:\n";
            cout << "1. Regular Credit Card\n";
            cout << "2. Premium Credit Card\n";
            cout << "3. Commercial or Business Credit Card\n";
            cout << "4. CashBack Credit Card\n";
            cout << "5. Travel Credit Card\n";
            cout << "6. Shopping Credit Card\n";
            cout << "7. Fuel Credit Card\n";
            int choice;
            cin >> choice;
            switch (choice) {
                case 1: cardType = "Regular Credit Card"; break;
                case 2: cardType = "Premium Credit Card"; break;
                case 3: cardType = "Commercial or Business Credit Card"; break;
                case 4: cardType = "CashBack Credit Card"; break;
                case 5: cardType = "Travel Credit Card"; break;
                case 6: cardType = "Shopping Credit Card"; break;
                case 7: cardType = "Fuel Credit Card"; break;
                default: cout << "Invalid choice! Defaulting to 'Regular Credit Card'.\n"; cardType = "Regular Credit Card"; break;
            }
            Creditcard newCreditCard(accountNumber, cardNumber, expirationDate, cvv, status, pin, issueDate, cardType, cardHolderName);
            issueCard(false, newCreditCard);
        }

        cout << "Card issued successfully!\n";
    }

    void runCardManagementSystem() {
        char choice;
        do {
            cout << "\nAdmin Menu:\n";
            cout << "1. Issue a new card\n";
            cout << "2. Display all debit cards\n";
            cout << "3. Display all credit cards\n";
            cout << "4. Deactivate a card\n";
            cout << "5. Exit\n";
            cout << "Enter your choice: ";
            cin >> choice;

            switch (choice) {
                case '1':
                    interactiveIssueCard();
                    break;
                case '2':
                    cout << "All Debit Cards:\n";
                    displayAllCards(true);
                    break;
                case '3':
                    cout << "All Credit Cards:\n";
                    displayAllCards(false);
                    break;
                case '4':
                    cout << "Deactivate a card:\n";
                    deactivateCard();
                    break;
                case '5':
                    cout << "Exiting...\n";
                    break;
                default:
                    cout << "Invalid choice! Please try again.\n";
            }
        } while (choice != '5');
    }
};

// ======================== SPLAY TREE FOR USER CARD OPERATIONS ========================

class splaynode {
public:
    string cardNumber;
    string cardDetails;
    splaynode* left;
    splaynode* right;

    splaynode(string cardNumber, string cardDetails)
        : cardNumber(cardNumber), cardDetails(cardDetails), left(nullptr), right(nullptr) {}
};

class splaytree {
private:
    splaynode* root;

    splaynode* rightRotate(splaynode* x) {
        splaynode* y = x->left;
        x->left = y->right;
        y->right = x;
        return y;
    }

    splaynode* leftRotate(splaynode* x) {
        splaynode* y = x->right;
        x->right = y->left;
        y->left = x;
        return y;
    }

    splaynode* splay(splaynode* root, const string& cardNumber) {
        if (!root || root->cardNumber == cardNumber)
            return root;

        if (root->cardNumber > cardNumber) {
            if (!root->left) return root;
            if (root->left->cardNumber > cardNumber) {
                root->left->left = splay(root->left->left, cardNumber);
                root = rightRotate(root);
            } else if (root->left->cardNumber < cardNumber) {
                root->left->right = splay(root->left->right, cardNumber);
                if (root->left->right)
                    root->left = leftRotate(root->left);
            }
            return root->left ? rightRotate(root) : root;
        } else {
            if (!root->right) return root;
            if (root->right->cardNumber > cardNumber) {
                root->right->left = splay(root->right->left, cardNumber);
                if (root->right->left)
                    root->right = rightRotate(root->right);
            } else if (root->right->cardNumber < cardNumber) {
                root->right->right = splay(root->right->right, cardNumber);
                root = leftRotate(root);
            }
            return root->right ? leftRotate(root) : root;
        }
    }

public:
    splaytree() : root(nullptr) {}

    void insert(const string& cardNumber, const string& cardDetails) {
        if (!root) {
            root = new splaynode(cardNumber, cardDetails);
            return;
        }
        root = splay(root, cardNumber);
        if (root->cardNumber == cardNumber) return;

        splaynode* newNode = new splaynode(cardNumber, cardDetails);
        if (root->cardNumber > cardNumber) {
            newNode->right = root;
            newNode->left = root->left;
            root->left = nullptr;
        } else {
            newNode->left = root;
            newNode->right = root->right;
            root->right = nullptr;
        }
        root = newNode;
    }

    splaynode* search(const string& cardNumber) {
        root = splay(root, cardNumber);
        return (root && root->cardNumber == cardNumber) ? root : nullptr;
    }
};

// ======================== AVL TREE FOR PIN MANAGEMENT ========================

class avlNode {
public:
    string cardNumber;
    string pin;
    int height;
    avlNode* left;
    avlNode* right;

    avlNode(string cardNumber, string pin)
        : cardNumber(cardNumber), pin(pin), height(1), left(nullptr), right(nullptr) {}
};

class avlTree {
private:
    avlNode* root;

    int height(avlNode* node) {
        return node ? node->height : 0;
    }

    int balanceFactor(avlNode* node) {
        return node ? height(node->left) - height(node->right) : 0;
    }

    avlNode* rotateRight(avlNode* y) {
        avlNode* x = y->left;
        avlNode* T2 = x->right;

        x->right = y;
        y->left = T2;

        y->height = max(height(y->left), height(y->right)) + 1;
        x->height = max(height(x->left), height(x->right)) + 1;

        return x;
    }

    avlNode* rotateLeft(avlNode* x) {
        avlNode* y = x->right;
        avlNode* T2 = y->left;

        y->left = x;
        x->right = T2;

        x->height = max(height(x->left), height(x->right)) + 1;
        y->height = max(height(y->left), height(y->right)) + 1;

        return y;
    }

    avlNode* search(avlNode* node, const string& cardNumber) {
        if (!node || node->cardNumber == cardNumber)
            return node;
        if (node->cardNumber < cardNumber)
            return search(node->right, cardNumber);
        return search(node->left, cardNumber);
    }

public:
    avlTree() : root(nullptr) {}

    void updatePin(const string& cardNumber, const string& newPin) {
        avlNode* node = search(root, cardNumber);
        if (node) {
            node->pin = newPin;
        }
    }

    void insert(const string& cardNumber, const string& pin) {
        root = insert(root, cardNumber, pin);
    }

    avlNode* insert(avlNode* node, const string& cardNumber, const string& pin) {
        if (!node)
            return new avlNode(cardNumber, pin);

        if (cardNumber < node->cardNumber)
            node->left = insert(node->left, cardNumber, pin);
        else if (cardNumber > node->cardNumber)
            node->right = insert(node->right, cardNumber, pin);

        node->height = 1 + max(height(node->left), height(node->right));

        int balance = balanceFactor(node);

        if (balance > 1 && cardNumber < node->left->cardNumber)
            return rotateRight(node);

        if (balance < -1 && cardNumber > node->right->cardNumber)
            return rotateLeft(node);

        if (balance > 1 && cardNumber > node->left->cardNumber) {
            node->left = rotateLeft(node->left);
            return rotateRight(node);
        }

        if (balance < -1 && cardNumber < node->right->cardNumber) {
            node->right = rotateRight(node->right);
            return rotateLeft(node);
        }

        return node;
    }

    string search(const string& cardNumber) {
        avlNode* node = search(root, cardNumber);
        return node ? node->pin : "Card not found";
    }
};

// ======================== GRAPH FOR CARD BLOCKING ========================

class CardGraph {
private:
    unordered_map<string, vector<string>> adjList;
    unordered_map<string, bool> blockedStatus;

public:
    void addEdge(const string& cardNumber, bool blocked) {
        if (blocked) {
            return;
        }

        for (const auto& entry : blockedStatus) {
            const string& otherCard = entry.first;
            if (otherCard != cardNumber && !entry.second) {
                addEdge(cardNumber, otherCard);
            }
        }
    }

    void addEdge(const string& card1, const string& card2) {
        adjList[card1].push_back(card2);
        adjList[card2].push_back(card1);
    }

    vector<string> getNeighbors(const string& cardNumber) const {
        auto it = adjList.find(cardNumber);
        if (it != adjList.end()) {
            return it->second;
        }
        return {};
    }

    void bfsBlockUnblock(const string& startCard, bool block) {
        unordered_set<string> visited;
        queue<string> q;
        q.push(startCard);
        visited.insert(startCard);

        while (!q.empty()) {
            string cardNumber = q.front();
            q.pop();

            blockedStatus[cardNumber] = block;

            for (const string& neighbor : getNeighbors(cardNumber)) {
                if (visited.find(neighbor) == visited.end()) {
                    q.push(neighbor);
                    visited.insert(neighbor);
                }
            }
        }
    }

    bool isBlocked(const string& cardNumber) const {
        auto it = blockedStatus.find(cardNumber);
        return it != blockedStatus.end() && it->second;
    }

    void setBlocked(const string& cardNumber, bool blocked) {
        blockedStatus[cardNumber] = blocked;
    }
};

// ======================== USER CARD OPERATIONS ========================

splaytree creditSplayTree;
splaytree debitSplayTree;
avlTree avltree;
CardGraph cardGraph;

void loadCardsFromCSV(const string& filePath, splaytree& splayTree, avlTree& avltree, CardGraph& cardGraph) {
    ifstream file(filePath);
    string line;

    getline(file, line); // Skip header

    while (getline(file, line)) {
        if (line.empty()) continue;

        stringstream ss(line);
        string accountNumber, cardNumber, expirationDate, cvv, status, pin, issueDate, cardType, cardHolderName;

        getline(ss, accountNumber, ',');
        getline(ss, cardNumber, ',');
        getline(ss, expirationDate, ',');
        getline(ss, cvv, ',');
        getline(ss, status, ',');
        getline(ss, pin, ',');
        getline(ss, issueDate, ',');
        getline(ss, cardType, ',');
        getline(ss, cardHolderName, ',');

        string cardDetails = accountNumber + "," + cardNumber + "," + expirationDate + "," + cvv + "," + status + "," + pin + "," + issueDate + "," + cardType + "," + cardHolderName;

        splayTree.insert(cardNumber, cardDetails);
        avltree.insert(cardNumber, pin);
        cardGraph.addEdge(cardNumber, status == "blocked");
    }

    file.close();
}

void updateCSVFile(const string& filePath, splaytree& splayTree, avlTree& avltree, CardGraph& cardGraph) {
    ifstream file(filePath);
    ofstream tempFile("temp.csv");

    string line;
    getline(file, line);
    tempFile << line << endl;

    while (getline(file, line)) {
        if (line.empty()) continue;

        stringstream ss(line);
        string accountNumber, cardNumber, expirationDate, cvv, status, pin, issueDate, cardType, cardHolderName;

        getline(ss, accountNumber, ',');
        getline(ss, cardNumber, ',');
        getline(ss, expirationDate, ',');
        getline(ss, cvv, ',');
        getline(ss, status, ',');
        getline(ss, pin, ',');
        getline(ss, issueDate, ',');
        getline(ss, cardType, ',');
        getline(ss, cardHolderName, ',');

        if (cardNumber.empty()) continue;

        splaynode* node = splayTree.search(cardNumber);
        if (node) {
            string newStatus = cardGraph.isBlocked(cardNumber) ? "blocked" : "active";
            string updatedDetails = accountNumber + "," + cardNumber + "," + expirationDate + "," + cvv + "," + newStatus + "," + pin + "," + issueDate + "," + cardType + "," + cardHolderName;
            node->cardDetails = updatedDetails;
            tempFile << updatedDetails << endl;
        } else {
            tempFile << line << endl;
        }
    }

    file.close();
    tempFile.close();

    remove(filePath.c_str());
    rename("temp.csv", filePath.c_str());
}

void displayCardDetails(splaynode* node) {
    if (node) {
        stringstream ss(node->cardDetails);
        string accountNumber, cardNumber, expirationDate, cvv, status, pin, issueDate, cardType, cardHolderName;

        getline(ss, accountNumber, ',');
        getline(ss, cardNumber, ',');
        getline(ss, expirationDate, ',');
        getline(ss, cvv, ',');
        getline(ss, status, ',');
        getline(ss, pin, ',');
        getline(ss, issueDate, ',');
        getline(ss, cardType, ',');
        getline(ss, cardHolderName, ',');

        cout << "Account Number: " << accountNumber << endl;
        cout << "Card Number: " << cardNumber << endl;
        cout << "Expiration Date: " << expirationDate << endl;
        cout << "CVV: " << cvv << endl;
        cout << "Status: " << status << endl;
        cout << "PIN: " << pin << endl;
        cout << "Issue Date: " << issueDate << endl;
        cout << "Card Type: " << cardType << endl;
        cout << "Card Holder Name: " << cardHolderName << endl;
    } else {
        cout << "Card not found." << endl;
    }
}

void managePin(avlTree& avltree, const string& cardNumber, const string& newPin) {
    avltree.updatePin(cardNumber, newPin);
    cout << "PIN updated successfully." << endl;
}

void blockOrUnblockCard(CardGraph& cardGraph, const string& cardNumber, bool block) {
    cardGraph.bfsBlockUnblock(cardNumber, block);
    if (block) {
        cout << "Card blocked successfully." << endl;
    } else {
        cout << "Card unblocked successfully." << endl;
    }
}

void handleOperations(const string& debitCardFilePath, const string& creditCardFilePath, splaytree& debitSplayTree, splaytree& creditSplayTree, avlTree& avltree, CardGraph& cardGraph) {
    while (true) {
        cout << "Menu:\n"
             << "1. Display Card Details\n"
             << "2. Manage PIN\n"
             << "3. Block Card\n"
             << "4. Unblock Card\n"
             << "5. Exit\n"
             << "Enter your choice: ";
        int choice;
        cin >> choice;

        if (choice == 5) break;

        cout << "Enter card type (1 for debit, 2 for credit): ";
        int cardType;
        cin >> cardType;

        string cardFilePath = (cardType == 1) ? debitCardFilePath : creditCardFilePath;
        splaytree& splayTree = (cardType == 1) ? debitSplayTree : creditSplayTree;

        cout << "Enter card number: ";
        string cardNumber;
        cin >> cardNumber;

        switch (choice) {
            case 1: {
                splaynode* node = splayTree.search(cardNumber);
                displayCardDetails(node);
                break;
            }
            case 2: {
                cout << "Enter new PIN: ";
                string newPin;
                cin >> newPin;
                managePin(avltree, cardNumber, newPin);
                updateCSVFile(cardFilePath, splayTree, avltree, cardGraph);
                break;
            }
            case 3: {
                blockOrUnblockCard(cardGraph, cardNumber, true);
                updateCSVFile(cardFilePath, splayTree, avltree, cardGraph);
                break;
            }
            case 4: {
                blockOrUnblockCard(cardGraph, cardNumber, false);
                updateCSVFile(cardFilePath, splayTree, avltree, cardGraph);
                break;
            }
            default:
                cout << "Invalid choice." << endl;
        }
    }
}

// ======================== MAIN FUNCTION ========================

int main() {
    int choice;
    
    cout << "==========================================\n";
    cout << "   CARD MANAGEMENT SYSTEM\n";
    cout << "==========================================\n\n";
    
    cout << "Select User Type:\n";
    cout << "1. Admin\n";
    cout << "2. Customer\n";
    cout << "3. View Card Discounts\n";
    cout << "4. Exit\n";
    cout << "Enter your choice: ";
    cin >> choice;
    
    switch (choice) {
        case 1: {
            // Admin operations
            CardManagement manager("debitcards.csv", "creditcards.csv");
            manager.runCardManagementSystem();
            break;
        }
        case 2: {
            // Customer operations
            const string debitCardFilePath = "debitcards.csv";
            const string creditCardFilePath = "creditcards.csv";

            splaytree debitSplayTree;
            splaytree creditSplayTree;
            avlTree avlTree;
            CardGraph cardGraph;

            // Load cards from CSV files
            loadCardsFromCSV(debitCardFilePath, debitSplayTree, avlTree, cardGraph);
            loadCardsFromCSV(creditCardFilePath, creditSplayTree, avlTree, cardGraph);

            handleOperations(debitCardFilePath, creditCardFilePath, debitSplayTree, creditSplayTree, avlTree, cardGraph);
            break;
        }
        case 3: {
            // View discounts
            showdiscounts();
            break;
        }
        case 4:
            cout << "Exiting program. Goodbye!\n";
            break;
        default:
            cout << "Invalid choice!\n";
    }
    
    return 0;
}