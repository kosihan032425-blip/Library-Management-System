/*
 * ==============================================================
 *        LIBRARY MANAGEMENT SYSTEM  -  C++
 *   OOP  |  File Handling  |  Issue/Return  |  Search
 * ==============================================================
 */

#include <iostream>
#include <fstream>
#include <iomanip>
#include <string>
#include <vector>
#include <sstream>
#include <algorithm>
#include <limits>
#include <ctime>
#include <climits>

#ifdef _WIN32
#include <windows.h>
#endif

using namespace std;

// ==============================================================
//  UTILITY  -  Terminal / display helpers
// ==============================================================
namespace UI {

void clear() {
#ifdef _WIN32
    system("cls");
#else
    system("clear");
#endif
}

void pause() {
    cout << "\n  Press Enter to continue...";
    cin.ignore(numeric_limits<streamsize>::max(), '\n');
    cin.get();
}

void border(char c = '=', int w = 66) { cout << "  " << string(w, c) << "\n"; }

void title(const string& t) {
    border('=');
    int inner = 62;
    int pad   = max(0, (inner - (int)t.size()) / 2);
    int rpad  = max(0, inner - pad - (int)t.size());
    cout << "  |" << string(pad, ' ') << t << string(rpad, ' ') << "|\n";
    border('=');
}

void success(const string& m) { cout << "\n  [OK]  " << m << "\n"; }
void error  (const string& m) { cout << "\n  [!!]  " << m << "\n"; }
void info   (const string& m) { cout << "\n  [>>]  " << m << "\n"; }
void warn   (const string& m) { cout << "\n  [**]  " << m << "\n"; }

string today() {
    time_t now = time(nullptr);
    char buf[12];
    strftime(buf, sizeof(buf), "%Y-%m-%d", localtime(&now));
    return string(buf);
}

string toLower(const string& s) {
    string r = s;
    transform(r.begin(), r.end(), r.begin(), ::tolower);
    return r;
}

bool containsIgnoreCase(const string& haystack, const string& needle) {
    return toLower(haystack).find(toLower(needle)) != string::npos;
}

} // namespace UI

// ==============================================================
//  INPUT  -  Validated helpers
// ==============================================================
namespace Input {

int getInt(const string& prompt, int minVal = 0, int maxVal = INT_MAX) {
    int v;
    while (true) {
        cout << prompt;
        if (cin >> v && v >= minVal && v <= maxVal) { cin.ignore(); return v; }
        cin.clear();
        cin.ignore(numeric_limits<streamsize>::max(), '\n');
        UI::error("Invalid input. Please try again.");
    }
}

string getString(const string& prompt) {
    string s;
    cout << prompt;
    getline(cin, s);
    return s;
}

string getNonEmpty(const string& prompt) {
    string s;
    while (true) {
        s = getString(prompt);
        if (!s.empty()) return s;
        UI::error("This field cannot be empty.");
    }
}

} // namespace Input

// ==============================================================
//  BOOK  -  Entity class
// ==============================================================
class Book {
private:
    int    bookId;
    string title;
    string author;
    string isbn;
    string genre;
    int    totalCopies;
    int    availableCopies;

public:
    // ---- Constructors -----------------------------------------
    Book() : bookId(0), totalCopies(0), availableCopies(0) {}

    Book(int id, const string& t, const string& a,
         const string& isbnCode, const string& g, int copies)
        : bookId(id), title(t), author(a), isbn(isbnCode),
          genre(g), totalCopies(copies), availableCopies(copies) {}

    // ---- Getters ----------------------------------------------
    int    getId()        const { return bookId;         }
    string getTitle()     const { return title;          }
    string getAuthor()    const { return author;         }
    string getIsbn()      const { return isbn;           }
    string getGenre()     const { return genre;          }
    int    getTotal()     const { return totalCopies;    }
    int    getAvailable() const { return availableCopies;}
    bool   isAvailable()  const { return availableCopies > 0; }

    // ---- Operations -------------------------------------------
    bool issueOne() {
        if (availableCopies <= 0) return false;
        --availableCopies;
        return true;
    }

    bool returnOne() {
        if (availableCopies >= totalCopies) return false;
        ++availableCopies;
        return true;
    }

    // ---- Display ----------------------------------------------
    void printRow() const {
        string avail = to_string(availableCopies) + "/" + to_string(totalCopies);
        string status = availableCopies > 0 ? "Available" : "All Issued";
        cout << "  | "
             << left  << setw(5)  << bookId
             << setw(28) << (title.size() > 27 ? title.substr(0,24) + "..." : title)
             << setw(20) << (author.size() > 19 ? author.substr(0,16) + "..." : author)
             << setw(7)  << avail
             << setw(9)  << status
             << "|\n";
    }

    void printDetail() const {
        UI::border('-');
        cout << "  Book ID    : " << bookId        << "\n"
             << "  Title      : " << title         << "\n"
             << "  Author     : " << author        << "\n"
             << "  ISBN       : " << isbn          << "\n"
             << "  Genre      : " << genre         << "\n"
             << "  Copies     : " << availableCopies << " available / "
                                  << totalCopies   << " total\n"
             << "  Status     : " << (isAvailable() ? "Available" : "All copies issued") << "\n";
        UI::border('-');
    }

    // ---- Persistence ------------------------------------------
    string serialize() const {
        ostringstream o;
        o << bookId << "\n" << title  << "\n" << author << "\n"
          << isbn   << "\n" << genre  << "\n"
          << totalCopies << "\n" << availableCopies << "\n";
        return o.str();
    }

    static Book deserialize(istream& in) {
        Book b;
        string line;
        getline(in, line); b.bookId          = stoi(line);
        getline(in, b.title);
        getline(in, b.author);
        getline(in, b.isbn);
        getline(in, b.genre);
        getline(in, line); b.totalCopies     = stoi(line);
        getline(in, line); b.availableCopies = stoi(line);
        return b;
    }
};

// ==============================================================
//  MEMBER  -  Entity class
// ==============================================================
class Member {
private:
    int    memberId;
    string name;
    string email;
    string phone;
    string memberSince;
    int    activeIssues;   // count of currently borrowed books
    static const int MAX_ISSUES = 3;

public:
    Member() : memberId(0), activeIssues(0) {}

    Member(int id, const string& n, const string& em,
           const string& ph)
        : memberId(id), name(n), email(em), phone(ph),
          memberSince(UI::today()), activeIssues(0) {}

    int    getId()          const { return memberId;    }
    string getName()        const { return name;        }
    string getEmail()       const { return email;       }
    string getPhone()       const { return phone;       }
    string getMemberSince() const { return memberSince; }
    int    getActiveIssues()const { return activeIssues;}
    bool   canBorrow()      const { return activeIssues < MAX_ISSUES; }

    void incrementIssues() { if (activeIssues < MAX_ISSUES) ++activeIssues; }
    void decrementIssues() { if (activeIssues > 0)          --activeIssues; }

    void printRow() const {
        cout << "  | "
             << left << setw(6)  << memberId
             << setw(22) << (name.size() > 21 ? name.substr(0,18) + "..." : name)
             << setw(26) << (email.size() > 25 ? email.substr(0,22) + "..." : email)
             << setw(13) << memberSince
             << setw(2)  << activeIssues << "/" << MAX_ISSUES
             << " |\n";
    }

    void printDetail() const {
        UI::border('-');
        cout << "  Member ID    : " << memberId     << "\n"
             << "  Name         : " << name         << "\n"
             << "  Email        : " << email        << "\n"
             << "  Phone        : " << phone        << "\n"
             << "  Member Since : " << memberSince  << "\n"
             << "  Books Issued : " << activeIssues << " / " << MAX_ISSUES << "\n"
             << "  Can Borrow   : " << (canBorrow() ? "Yes" : "No - limit reached") << "\n";
        UI::border('-');
    }

    string serialize() const {
        ostringstream o;
        o << memberId << "\n" << name   << "\n" << email  << "\n"
          << phone    << "\n" << memberSince << "\n" << activeIssues << "\n";
        return o.str();
    }

    static Member deserialize(istream& in) {
        Member m;
        string line;
        getline(in, line); m.memberId    = stoi(line);
        getline(in, m.name);
        getline(in, m.email);
        getline(in, m.phone);
        getline(in, m.memberSince);
        getline(in, line); m.activeIssues = stoi(line);
        return m;
    }
};

// ==============================================================
//  BORROW RECORD  -  Issue / Return tracking
// ==============================================================
struct BorrowRecord {
    int    recordId;
    int    memberId;
    int    bookId;
    string memberName;
    string bookTitle;
    string issueDate;
    string returnDate;   // empty = not yet returned
    bool   returned;

    BorrowRecord() : recordId(0), memberId(0), bookId(0), returned(false) {}

    BorrowRecord(int rid, int mid, int bid,
                 const string& mname, const string& btitle)
        : recordId(rid), memberId(mid), bookId(bid),
          memberName(mname), bookTitle(btitle),
          issueDate(UI::today()), returned(false) {}

    void printRow() const {
        string retStr = returned ? returnDate : "-- Pending --";
        cout << "  | "
             << left << setw(5)  << recordId
             << setw(7)  << memberId
             << setw(6)  << bookId
             << setw(18) << (memberName.size() > 17 ? memberName.substr(0,14) + "..." : memberName)
             << setw(22) << (bookTitle.size()  > 21 ? bookTitle.substr(0,18)  + "..." : bookTitle)
             << setw(12) << issueDate
             << setw(12) << retStr
             << "|\n";
    }

    string serialize() const {
        ostringstream o;
        o << recordId    << "\n" << memberId   << "\n" << bookId   << "\n"
          << memberName  << "\n" << bookTitle  << "\n"
          << issueDate   << "\n" << returnDate << "\n"
          << (returned ? 1 : 0) << "\n";
        return o.str();
    }

    static BorrowRecord deserialize(istream& in) {
        BorrowRecord r;
        string line;
        getline(in, line); r.recordId   = stoi(line);
        getline(in, line); r.memberId   = stoi(line);
        getline(in, line); r.bookId     = stoi(line);
        getline(in, r.memberName);
        getline(in, r.bookTitle);
        getline(in, r.issueDate);
        getline(in, r.returnDate);
        getline(in, line); r.returned   = (stoi(line) == 1);
        return r;
    }
};

// ==============================================================
//  LIBRARY  -  Orchestrator class (OOP)
// ==============================================================
class Library {
private:
    string libName;
    string booksFile;
    string membersFile;
    string borrowFile;

    vector<Book>         books;
    vector<Member>       members;
    vector<BorrowRecord> records;

    int nextBookId;
    int nextMemberId;
    int nextRecordId;

    // ---- File I/O --------------------------------------------
    void loadBooks() {
        ifstream fin(booksFile);
        if (!fin) return;
        string line;
        getline(fin, line); nextBookId = stoi(line);
        getline(fin, line); int n = stoi(line);
        for (int i = 0; i < n; ++i) books.push_back(Book::deserialize(fin));
    }

    void loadMembers() {
        ifstream fin(membersFile);
        if (!fin) return;
        string line;
        getline(fin, line); nextMemberId = stoi(line);
        getline(fin, line); int n = stoi(line);
        for (int i = 0; i < n; ++i) members.push_back(Member::deserialize(fin));
    }

    void loadRecords() {
        ifstream fin(borrowFile);
        if (!fin) return;
        string line;
        getline(fin, line); nextRecordId = stoi(line);
        getline(fin, line); int n = stoi(line);
        for (int i = 0; i < n; ++i) records.push_back(BorrowRecord::deserialize(fin));
    }

    void saveBooks() const {
        ofstream f(booksFile, ios::trunc);
        f << nextBookId << "\n" << books.size() << "\n";
        for (const auto& b : books) f << b.serialize();
    }

    void saveMembers() const {
        ofstream f(membersFile, ios::trunc);
        f << nextMemberId << "\n" << members.size() << "\n";
        for (const auto& m : members) f << m.serialize();
    }

    void saveRecords() const {
        ofstream f(borrowFile, ios::trunc);
        f << nextRecordId << "\n" << records.size() << "\n";
        for (const auto& r : records) f << r.serialize();
    }

    // ---- Lookup helpers --------------------------------------
    Book*   findBook  (int id) { for (auto& b : books)   if (b.getId() == id) return &b;   return nullptr; }
    Member* findMember(int id) { for (auto& m : members) if (m.getId() == id) return &m; return nullptr; }

    // ---- Table headers ---------------------------------------
    void printBookHeader() const {
        UI::border('-');
        cout << "  | "
             << left << setw(5)  << "ID"
             << setw(28) << "Title"
             << setw(20) << "Author"
             << setw(7)  << "Avail"
             << setw(9)  << "Status"
             << "|\n";
        UI::border('-');
    }

    void printMemberHeader() const {
        UI::border('-');
        cout << "  | "
             << left << setw(6)  << "ID"
             << setw(22) << "Name"
             << setw(26) << "Email"
             << setw(13) << "Since"
             << "Issued |\n";
        UI::border('-');
    }

    void printRecordHeader() const {
        UI::border('-');
        cout << "  | "
             << left << setw(5)  << "Rec"
             << setw(7)  << "Mem"
             << setw(6)  << "Book"
             << setw(18) << "Member"
             << setw(22) << "Book Title"
             << setw(12) << "Issued"
             << setw(12) << "Returned"
             << "|\n";
        UI::border('-');
    }

public:
    // ---- Constructor -----------------------------------------
    Library(const string& name)
        : libName(name),
          booksFile("lib_books.txt"),
          membersFile("lib_members.txt"),
          borrowFile("lib_borrow.txt"),
          nextBookId(1001), nextMemberId(2001), nextRecordId(3001) {
        loadBooks();
        loadMembers();
        loadRecords();
    }

    ~Library() { saveBooks(); saveMembers(); saveRecords(); }

    const string& getName() const { return libName; }

    // ==========================================================
    //  BOOK OPERATIONS
    // ==========================================================

    // ---- Add Book --------------------------------------------
    void addBook() {
        UI::clear();
        UI::title("  ADD NEW BOOK  ");
        cout << "\n";

        string t  = Input::getNonEmpty("  Title        : ");
        string a  = Input::getNonEmpty("  Author       : ");
        string is = Input::getNonEmpty("  ISBN         : ");
        string g  = Input::getNonEmpty("  Genre        : ");
        int    cp = Input::getInt     ("  No. of Copies: ", 1, 100);

        books.emplace_back(nextBookId++, t, a, is, g, cp);
        saveBooks();

        UI::success("Book added! ID: " + to_string(books.back().getId()));
        UI::pause();
    }

    // ---- Display All Books -----------------------------------
    void listBooks() {
        UI::clear();
        UI::title("  ALL BOOKS  ");

        if (books.empty()) { UI::info("No books in the library yet."); UI::pause(); return; }

        printBookHeader();
        for (const auto& b : books) b.printRow();
        UI::border('-');
        cout << "  Total: " << books.size() << " book(s)\n";
        UI::pause();
    }

    // ---- View Book Detail ------------------------------------
    void viewBook() {
        UI::clear();
        UI::title("  BOOK DETAIL  ");
        cout << "\n";
        int id = Input::getInt("  Book ID: ");
        Book* b = findBook(id);
        if (!b) { UI::error("Book ID " + to_string(id) + " not found."); UI::pause(); return; }
        cout << "\n";
        b->printDetail();
        UI::pause();
    }

    // ---- Search Books ----------------------------------------
    void searchBooks() {
        UI::clear();
        UI::title("  SEARCH BOOKS  ");
        cout << "\n"
             << "  Search by:\n"
             << "    [1] Title\n"
             << "    [2] Author\n"
             << "    [3] Genre\n"
             << "    [4] ISBN\n\n";
        int ch = Input::getInt("  Choice: ", 1, 4);

        string q = Input::getNonEmpty("  Search term : ");
        vector<const Book*> results;

        for (const auto& b : books) {
            bool match = false;
            switch (ch) {
                case 1: match = UI::containsIgnoreCase(b.getTitle(),  q); break;
                case 2: match = UI::containsIgnoreCase(b.getAuthor(), q); break;
                case 3: match = UI::containsIgnoreCase(b.getGenre(),  q); break;
                case 4: match = UI::containsIgnoreCase(b.getIsbn(),   q); break;
            }
            if (match) results.push_back(&b);
        }

        cout << "\n";
        if (results.empty()) {
            UI::warn("No books found matching \"" + q + "\".");
        } else {
            printBookHeader();
            for (const auto* b : results) b->printRow();
            UI::border('-');
            cout << "  Found: " << results.size() << " result(s)\n";
        }
        UI::pause();
    }

    // ---- Remove Book -----------------------------------------
    void removeBook() {
        UI::clear();
        UI::title("  REMOVE BOOK  ");
        cout << "\n";
        int id = Input::getInt("  Book ID to remove: ");
        Book* b = findBook(id);
        if (!b) { UI::error("Book not found."); UI::pause(); return; }

        // Check no active issues
        bool hasActive = false;
        for (const auto& r : records)
            if (r.bookId == id && !r.returned) { hasActive = true; break; }
        if (hasActive) {
            UI::error("Cannot remove: book has copies currently issued.");
            UI::pause(); return;
        }

        cout << "\n"; b->printDetail();
        cout << "  Confirm removal? (y/n): ";
        char c; cin >> c; cin.ignore();
        if (c != 'y' && c != 'Y') { UI::info("Cancelled."); UI::pause(); return; }

        books.erase(remove_if(books.begin(), books.end(),
            [id](const Book& bk){ return bk.getId() == id; }), books.end());
        saveBooks();
        UI::success("Book removed from library.");
        UI::pause();
    }

    // ==========================================================
    //  MEMBER OPERATIONS
    // ==========================================================

    // ---- Register Member -------------------------------------
    void addMember() {
        UI::clear();
        UI::title("  REGISTER NEW MEMBER  ");
        cout << "\n";

        string n  = Input::getNonEmpty("  Full Name : ");
        string em = Input::getNonEmpty("  Email     : ");
        string ph = Input::getNonEmpty("  Phone     : ");

        members.emplace_back(nextMemberId++, n, em, ph);
        saveMembers();

        UI::success("Member registered! ID: " + to_string(members.back().getId()));
        UI::pause();
    }

    // ---- List All Members ------------------------------------
    void listMembers() {
        UI::clear();
        UI::title("  ALL MEMBERS  ");

        if (members.empty()) { UI::info("No members registered yet."); UI::pause(); return; }

        printMemberHeader();
        for (const auto& m : members) m.printRow();
        UI::border('-');
        cout << "  Total: " << members.size() << " member(s)\n";
        UI::pause();
    }

    // ---- Search Member by Name -------------------------------
    void searchMember() {
        UI::clear();
        UI::title("  SEARCH MEMBER  ");
        cout << "\n";
        string q = Input::getNonEmpty("  Name to search: ");

        vector<const Member*> results;
        for (const auto& m : members)
            if (UI::containsIgnoreCase(m.getName(), q)) results.push_back(&m);

        cout << "\n";
        if (results.empty()) {
            UI::warn("No member found matching \"" + q + "\".");
        } else {
            printMemberHeader();
            for (const auto* m : results) m->printRow();
            UI::border('-');
            cout << "  Found: " << results.size() << " result(s)\n";
        }
        UI::pause();
    }

    // ---- Remove Member ---------------------------------------
    void removeMember() {
        UI::clear();
        UI::title("  REMOVE MEMBER  ");
        cout << "\n";
        int id = Input::getInt("  Member ID to remove: ");
        Member* m = findMember(id);
        if (!m) { UI::error("Member not found."); UI::pause(); return; }

        if (m->getActiveIssues() > 0) {
            UI::error("Cannot remove: member has " +
                      to_string(m->getActiveIssues()) + " book(s) still issued.");
            UI::pause(); return;
        }

        cout << "\n"; m->printDetail();
        cout << "  Confirm removal? (y/n): ";
        char c; cin >> c; cin.ignore();
        if (c != 'y' && c != 'Y') { UI::info("Cancelled."); UI::pause(); return; }

        members.erase(remove_if(members.begin(), members.end(),
            [id](const Member& mb){ return mb.getId() == id; }), members.end());
        saveMembers();
        UI::success("Member removed.");
        UI::pause();
    }

    // ==========================================================
    //  ISSUE / RETURN OPERATIONS
    // ==========================================================

    // ---- Issue Book ------------------------------------------
    void issueBook() {
        UI::clear();
        UI::title("  ISSUE BOOK  ");
        cout << "\n";

        int mid = Input::getInt("  Member ID : ");
        Member* m = findMember(mid);
        if (!m) { UI::error("Member not found."); UI::pause(); return; }
        if (!m->canBorrow()) {
            UI::error("Member has reached the maximum issue limit (3 books).");
            UI::pause(); return;
        }

        cout << "  Member     : " << m->getName() << "\n\n";

        int bid = Input::getInt("  Book ID   : ");
        Book* b = findBook(bid);
        if (!b) { UI::error("Book not found."); UI::pause(); return; }
        if (!b->isAvailable()) {
            UI::error("No copies available for \"" + b->getTitle() + "\".");
            UI::pause(); return;
        }

        cout << "  Book       : " << b->getTitle()
             << " by " << b->getAuthor() << "\n\n";
        cout << "  Confirm issue? (y/n): ";
        char c; cin >> c; cin.ignore();
        if (c != 'y' && c != 'Y') { UI::info("Cancelled."); UI::pause(); return; }

        b->issueOne();
        m->incrementIssues();
        records.emplace_back(nextRecordId++, mid, bid, m->getName(), b->getTitle());

        saveBooks(); saveMembers(); saveRecords();

        UI::success("Book issued successfully!");
        cout << "  Record ID  : " << records.back().recordId << "\n"
             << "  Issue Date : " << records.back().issueDate << "\n";
        UI::pause();
    }

    // ---- Return Book -----------------------------------------
    void returnBook() {
        UI::clear();
        UI::title("  RETURN BOOK  ");
        cout << "\n";

        int rid = Input::getInt("  Borrow Record ID: ");
        BorrowRecord* rec = nullptr;
        for (auto& r : records)
            if (r.recordId == rid) { rec = &r; break; }

        if (!rec) { UI::error("Record ID " + to_string(rid) + " not found."); UI::pause(); return; }
        if (rec->returned) {
            UI::error("This book was already returned on " + rec->returnDate + ".");
            UI::pause(); return;
        }

        cout << "\n  Member  : " << rec->memberName << "\n"
             << "  Book    : " << rec->bookTitle  << "\n"
             << "  Issued  : " << rec->issueDate  << "\n\n"
             << "  Confirm return? (y/n): ";
        char c; cin >> c; cin.ignore();
        if (c != 'y' && c != 'Y') { UI::info("Cancelled."); UI::pause(); return; }

        rec->returned   = true;
        rec->returnDate = UI::today();

        Book*   b = findBook(rec->bookId);
        Member* m = findMember(rec->memberId);
        if (b) b->returnOne();
        if (m) m->decrementIssues();

        saveBooks(); saveMembers(); saveRecords();

        UI::success("Book returned successfully!");
        cout << "  Return Date: " << rec->returnDate << "\n";
        UI::pause();
    }

    // ---- View All Borrow Records ----------------------------
    void listRecords() {
        UI::clear();
        UI::title("  BORROW RECORDS  ");
        cout << "\n"
             << "  Filter:\n"
             << "    [1] All records\n"
             << "    [2] Active (not returned)\n"
             << "    [3] Returned\n\n";
        int ch = Input::getInt("  Choice: ", 1, 3);

        vector<const BorrowRecord*> filtered;
        for (const auto& r : records) {
            if (ch == 1) filtered.push_back(&r);
            else if (ch == 2 && !r.returned) filtered.push_back(&r);
            else if (ch == 3 &&  r.returned) filtered.push_back(&r);
        }

        cout << "\n";
        if (filtered.empty()) { UI::info("No records found."); UI::pause(); return; }

        printRecordHeader();
        for (const auto* r : filtered) r->printRow();
        UI::border('-');
        cout << "  Total: " << filtered.size() << " record(s)\n";
        UI::pause();
    }

    // ---- Records by Member -----------------------------------
    void memberHistory() {
        UI::clear();
        UI::title("  MEMBER BORROW HISTORY  ");
        cout << "\n";
        int mid = Input::getInt("  Member ID: ");
        Member* m = findMember(mid);
        if (!m) { UI::error("Member not found."); UI::pause(); return; }

        cout << "\n  History for: " << m->getName() << "\n\n";

        vector<const BorrowRecord*> hist;
        for (const auto& r : records)
            if (r.memberId == mid) hist.push_back(&r);

        if (hist.empty()) { UI::info("No borrow history found."); UI::pause(); return; }

        printRecordHeader();
        for (const auto* r : hist) r->printRow();
        UI::border('-');
        cout << "  Total: " << hist.size() << " record(s)\n";
        UI::pause();
    }

    // ==========================================================
    //  DASHBOARD SUMMARY
    // ==========================================================
    void dashboard() {
        UI::clear();
        UI::title("  LIBRARY DASHBOARD  ");

        int totalBooks = 0, availBooks = 0;
        for (const auto& b : books) {
            totalBooks += b.getTotal();
            availBooks += b.getAvailable();
        }
        int issuedBooks = totalBooks - availBooks;

        int activeRecords = 0, returnedRecords = 0;
        for (const auto& r : records)
            r.returned ? ++returnedRecords : ++activeRecords;

        UI::border('-');
        cout << "\n"
             << "  BOOKS\n"
             << "  ------\n"
             << "  Titles registered  : " << books.size()   << "\n"
             << "  Total copies       : " << totalBooks      << "\n"
             << "  Currently available: " << availBooks      << "\n"
             << "  Currently issued   : " << issuedBooks     << "\n"
             << "\n"
             << "  MEMBERS\n"
             << "  --------\n"
             << "  Registered members : " << members.size() << "\n"
             << "\n"
             << "  TRANSACTIONS\n"
             << "  -------------\n"
             << "  Active issues      : " << activeRecords   << "\n"
             << "  Total returned     : " << returnedRecords << "\n"
             << "  All-time records   : " << records.size()  << "\n\n";
        UI::border('-');
        UI::pause();
    }
};

// ==============================================================
//  MENUS
// ==============================================================
void bookMenu(Library& lib) {
    int ch;
    do {
        UI::clear();
        UI::title("  BOOK MANAGEMENT  ");
        cout << "\n"
             << "  [1]  Add New Book\n"
             << "  [2]  View All Books\n"
             << "  [3]  View Book Details\n"
             << "  [4]  Search Books\n"
             << "  [5]  Remove Book\n"
             << "  [0]  Back to Main Menu\n\n";
        UI::border('-');
        ch = Input::getInt("  Choice: ", 0, 5);
        switch (ch) {
            case 1: lib.addBook();    break;
            case 2: lib.listBooks();  break;
            case 3: lib.viewBook();   break;
            case 4: lib.searchBooks();break;
            case 5: lib.removeBook(); break;
        }
    } while (ch != 0);
}

void memberMenu(Library& lib) {
    int ch;
    do {
        UI::clear();
        UI::title("  MEMBER MANAGEMENT  ");
        cout << "\n"
             << "  [1]  Register New Member\n"
             << "  [2]  View All Members\n"
             << "  [3]  Search Member\n"
             << "  [4]  Member Borrow History\n"
             << "  [5]  Remove Member\n"
             << "  [0]  Back to Main Menu\n\n";
        UI::border('-');
        ch = Input::getInt("  Choice: ", 0, 5);
        switch (ch) {
            case 1: lib.addMember();      break;
            case 2: lib.listMembers();    break;
            case 3: lib.searchMember();   break;
            case 4: lib.memberHistory();  break;
            case 5: lib.removeMember();   break;
        }
    } while (ch != 0);
}

void issueMenu(Library& lib) {
    int ch;
    do {
        UI::clear();
        UI::title("  ISSUE & RETURN  ");
        cout << "\n"
             << "  [1]  Issue Book to Member\n"
             << "  [2]  Return Book\n"
             << "  [3]  View Borrow Records\n"
             << "  [0]  Back to Main Menu\n\n";
        UI::border('-');
        ch = Input::getInt("  Choice: ", 0, 3);
        switch (ch) {
            case 1: lib.issueBook();    break;
            case 2: lib.returnBook();   break;
            case 3: lib.listRecords();  break;
        }
    } while (ch != 0);
}

void mainMenu(Library& lib) {
    int ch;
    do {
        UI::clear();
        UI::title(" " + lib.getName() + " ");
        cout << "\n"
             << "  [1]  Book Management\n"
             << "  [2]  Member Management\n"
             << "  [3]  Issue / Return\n"
             << "  [4]  Dashboard\n"
             << "  [0]  Exit\n\n";
        UI::border('-');
        ch = Input::getInt("  Choice: ", 0, 4);
        switch (ch) {
            case 1: bookMenu(lib);    break;
            case 2: memberMenu(lib);  break;
            case 3: issueMenu(lib);   break;
            case 4: lib.dashboard();  break;
            case 0:
                UI::clear();
                UI::title("  GOODBYE - VISIT AGAIN  ");
                cout << "\n  Data saved to lib_books.txt, lib_members.txt, lib_borrow.txt\n\n";
                break;
        }
    } while (ch != 0);
}

// ==============================================================
//  ENTRY POINT
// ==============================================================
int main() {
#ifdef _WIN32
    SetConsoleOutputCP(CP_UTF8);
#endif

    Library lib("NOVA PUBLIC LIBRARY - Management System");
    mainMenu(lib);
    return 0;
}