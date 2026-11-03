#include <iostream>
#include <iomanip>
#include <sstream>
#include <string>
#include <cctype>
#include <cstdlib>
using namespace std;

const string STORE_NAME = "YOUR PERFUME SHOP";   // <-- change to your business name
const int    LOW_STOCK  = 5;                      // stock at/below this shows "LOW"

// -----------------------------------------------------------------------------
//  Product record (as required by the project specification)
// -----------------------------------------------------------------------------
struct Product {
    int    id;             // Unique Identifier
    string name;           // Product Title
    string category;       // Category (Eau de Parfum, Eau de Toilette, ...)
    string brand;          // Official Partner Brand (Dior, Chanel, ...)
    double price;          // Unit Price (PHP)
    int    stockQuantity;  // Available Inventory Level
};

// -----------------------------------------------------------------------------
//  Helpers
// -----------------------------------------------------------------------------
static string toLower(string s) {
    for (size_t i = 0; i < s.size(); i++) s[i] = (char)tolower((unsigned char)s[i]);
    return s;
}
static bool containsCI(const string& hay, const string& needle) {
    return toLower(hay).find(toLower(needle)) != string::npos;
}
static string fit(const string& s, size_t w) {
    return s.size() <= w ? s : s.substr(0, w - 2) + "..";
}
static string money(double v) {
    ostringstream o;
    o << "PHP " << fixed << setprecision(2) << v;
    return o.str();
}
static string center(const string& s, size_t w) {
    if (s.size() >= w) return s.substr(0, w);
    size_t left = (w - s.size()) / 2;
    return string(left, ' ') + s + string(w - s.size() - left, ' ');
}
static string readLine(const string& prompt) {
    cout << prompt;
    string s;
    if (!getline(cin, s)) { cout << "\n"; exit(0); }
    size_t a = s.find_first_not_of(" \t\r\n");
    if (a == string::npos) return "";
    size_t b = s.find_last_not_of(" \t\r\n");
    return s.substr(a, b - a + 1);
}
static string readNonEmpty(const string& prompt) {
    while (true) {
        string s = readLine(prompt);
        if (!s.empty()) return s;
        cout << "  This field cannot be empty.\n";
    }
}
static int readInt(const string& prompt, int lo, int hi) {
    while (true) {
        istringstream in(readLine(prompt));
        int v; char extra;
        if ((in >> v) && !(in >> extra) && v >= lo && v <= hi) return v;
        cout << "  Invalid input. Enter a whole number from " << lo << " to " << hi << ".\n";
    }
}
static double readDouble(const string& prompt, double lo) {
    while (true) {
        istringstream in(readLine(prompt));
        double v; char extra;
        if ((in >> v) && !(in >> extra) && v >= lo) return v;
        cout << "  Invalid input. Enter a number >= " << lo << ".\n";
    }
}
static string readPhone(const string& prompt) {          // contact number: 7-15 digits, + - and spaces allowed
    while (true) {
        string s = readNonEmpty(prompt);
        int digits = 0; bool ok = true;
        for (size_t i = 0; i < s.size(); i++) {
            char c = s[i];
            if (isdigit((unsigned char)c)) digits++;
            else if (!(c == ' ' || c == '-' || (c == '+' && i == 0))) ok = false;
        }
        if (ok && digits >= 7 && digits <= 15) return s;
        cout << "  Invalid contact number. Use 7-15 digits (e.g. 09171234567).\n";
    }
}
static void pause() { readLine("\nPress Enter to continue..."); }

// =============================================================================
//  LAB 1 : Dynamic catalog (array of Product* that grows and shrinks)
// =============================================================================
class Catalog {
    Product** items;
    int count, capacity;

    void resize(int newCap) {
        Product** n = new Product*[newCap];
        for (int i = 0; i < count; i++) n[i] = items[i];
        delete[] items;
        items = n;
        capacity = newCap;
    }
public:
    Catalog() : items(new Product*[4]), count(0), capacity(4) {}
    ~Catalog() {
        for (int i = 0; i < count; i++) delete items[i];
        delete[] items;
    }
    int size() const { return count; }
    int cap()  const { return capacity; }
    Product* at(int i) const { return items[i]; }

    void add(Product* p) {
        if (count == capacity) resize(capacity * 2);          // grow
        items[count++] = p;
    }
    bool remove(Product* p) {                                  // does not delete p
        for (int i = 0; i < count; i++) {
            if (items[i] == p) {
                for (int j = i; j < count - 1; j++) items[j] = items[j + 1];
                count--;
                if (capacity > 4 && count <= capacity / 4) resize(capacity / 2);  // shrink
                return true;
            }
        }
        return false;
    }
    // Copy of the pointer array so we can sort for display without touching the catalog
    Product** snapshot() const {
        Product** copy = new Product*[count > 0 ? count : 1];
        for (int i = 0; i < count; i++) copy[i] = items[i];
        return copy;
    }
};

// =============================================================================
//  LAB 6 : Binary Search Tree indexed by Product ID
// =============================================================================
struct BSTNode {
    int id; Product* p; BSTNode *l, *r;
    BSTNode(Product* pp) : id(pp->id), p(pp), l(0), r(0) {}
};
class ProductIndex {
    BSTNode* root;
    static BSTNode* ins(BSTNode* n, Product* p) {
        if (!n) return new BSTNode(p);
        if (p->id < n->id) n->l = ins(n->l, p);
        else if (p->id > n->id) n->r = ins(n->r, p);
        return n;
    }
    static BSTNode* del(BSTNode* n, int id) {
        if (!n) return 0;
        if (id < n->id) n->l = del(n->l, id);
        else if (id > n->id) n->r = del(n->r, id);
        else {
            if (!n->l) { BSTNode* r = n->r; delete n; return r; }
            if (!n->r) { BSTNode* l = n->l; delete n; return l; }
            BSTNode* s = n->r;                       // in-order successor
            while (s->l) s = s->l;
            n->id = s->id; n->p = s->p;
            n->r = del(n->r, s->id);
        }
        return n;
    }
    static void destroy(BSTNode* n) {
        if (!n) return;
        destroy(n->l); destroy(n->r); delete n;
    }
    // Insert the middle element first so the tree starts balanced
    void buildBalanced(Product** a, int lo, int hi) {
        if (lo > hi) return;
        int mid = (lo + hi) / 2;
        insert(a[mid]);
        buildBalanced(a, lo, mid - 1);
        buildBalanced(a, mid + 1, hi);
    }
public:
    ProductIndex() : root(0) {}
    ~ProductIndex() { destroy(root); }
    void insert(Product* p) { root = ins(root, p); }
    void remove(int id)     { root = del(root, id); }
    Product* find(int id, int& steps) const {
        steps = 0;
        BSTNode* n = root;
        while (n) {
            steps++;
            if (id == n->id) return n->p;
            n = (id < n->id) ? n->l : n->r;
        }
        return 0;
    }
    void buildFrom(const Catalog& c) {                // catalog must be in ascending ID order
        Product** a = c.snapshot();
        buildBalanced(a, 0, c.size() - 1);
        delete[] a;
    }
};

// =============================================================================
//  LAB 2 : Shopping cart as a DOUBLY linked list
// =============================================================================
struct CartNode {
    int id; string name, brand; double price; int qty;
    CartNode *next, *prev;
    CartNode(const Product& p, int q)
        : id(p.id), name(p.name), brand(p.brand), price(p.price), qty(q), next(0), prev(0) {}
};
static void freeChain(CartNode* h) {
    while (h) { CartNode* n = h->next; delete h; h = n; }
}
class Cart {
    CartNode *head, *tail, *cur;
    int count;
public:
    Cart() : head(0), tail(0), cur(0), count(0) {}
    ~Cart() { clear(); }
    bool empty() const { return count == 0; }
    CartNode* findItem(int id) const {
        for (CartNode* n = head; n; n = n->next) if (n->id == id) return n;
        return 0;
    }
    int qtyOf(int id) const { CartNode* n = findItem(id); return n ? n->qty : 0; }
    void add(const Product& p, int qty) {
        CartNode* n = findItem(p.id);
        if (n) { n->qty += qty; cur = n; return; }
        n = new CartNode(p, qty);
        if (!tail) head = tail = n;
        else { tail->next = n; n->prev = tail; tail = n; }
        cur = n; count++;
    }
    bool next() { if (cur && cur->next) { cur = cur->next; return true; } return false; }
    bool prev() { if (cur && cur->prev) { cur = cur->prev; return true; } return false; }
    void removeCurrent() {
        if (!cur) return;
        CartNode* n = cur;
        if (n->prev) n->prev->next = n->next; else head = n->next;
        if (n->next) n->next->prev = n->prev; else tail = n->prev;
        cur = n->next ? n->next : n->prev;
        delete n; count--;
    }
    void clear() { freeChain(head); head = tail = cur = 0; count = 0; }
    CartNode* detachAll() {                 // hand the whole list to an order
        CartNode* h = head;
        for (CartNode* n = h; n; n = n->next) n->prev = 0;
        head = tail = cur = 0; count = 0;
        return h;
    }
    double total() const {
        double t = 0;
        for (CartNode* n = head; n; n = n->next) t += n->price * n->qty;
        return t;
    }
    void display() const {
        if (!head) { cout << "  (cart is empty)\n"; return; }
        int i = 1, pos = 0;
        cout << "  " << left << setw(4) << "" << setw(5) << "#" << setw(28) << "ITEM"
             << setw(18) << "BRAND" << right << setw(5) << "QTY" << setw(16) << "SUBTOTAL" << "\n";
        for (CartNode* n = head; n; n = n->next, i++) {
            if (n == cur) pos = i;
            cout << "  " << left << setw(4) << (n == cur ? ">>" : "") << setw(5) << i
                 << setw(28) << fit(n->name, 27) << setw(18) << fit(n->brand, 17)
                 << right << setw(5) << n->qty << setw(16) << money(n->price * n->qty) << "\n";
        }
        cout << "  Viewing item " << pos << " of " << count << "   |   Cart total: "
             << money(total()) << "\n";
    }
};

// =============================================================================
//  LAB 5 : Customer checkout queue (FIFO)
// =============================================================================
struct Order { int no; string customer; string address; string contact; CartNode* items; Order* next; };
class OrderQueue {
    Order *front, *rear;
    int count;
public:
    OrderQueue() : front(0), rear(0), count(0) {}
    ~OrderQueue() {
        while (Order* o = dequeue()) { freeChain(o->items); delete o; }
    }
    int size() const { return count; }
    Order* peek() const { return front; }
    void enqueue(int no, const string& cust, const string& address, const string& contact, CartNode* items) {
        Order* o = new Order{no, cust, address, contact, items, 0};
        if (!rear) front = rear = o; else { rear->next = o; rear = o; }
        count++;
    }
    Order* dequeue() {
        if (!front) return 0;
        Order* o = front;
        front = front->next;
        if (!front) rear = 0;
        count--;
        return o;
    }
};

// =============================================================================
//  LAB 4 : Admin undo history (stack, LIFO)
// =============================================================================
enum ActionType { ACT_ADD, ACT_REMOVE, ACT_RESTOCK };
struct Action { ActionType type; Product snap; int oldStock; Action* next; };
class UndoStack {
    Action* top; int count;
public:
    UndoStack() : top(0), count(0) {}
    ~UndoStack() { Action a; while (pop(a)) {} }
    int size() const { return count; }
    void push(ActionType t, const Product& p, int oldStock) {
        top = new Action{t, p, oldStock, top};
        count++;
    }
    bool pop(Action& out) {
        if (!top) return false;
        Action* n = top;
        out = *n;
        top = top->next;
        delete n; count--;
        return true;
    }
};

// =============================================================================
//  LAB 3 : Circular linked list of partner brands (banner showcase)
// =============================================================================
struct BrandNode {
    string brand; int count; double lo, hi; BrandNode* next;
};
class BrandRing {
    BrandNode* tail;   // tail->next is the head
    int n;
public:
    BrandRing() : tail(0), n(0) {}
    ~BrandRing() {
        if (!tail) return;
        BrandNode* c = tail->next;
        tail->next = 0;
        while (c) { BrandNode* t = c->next; delete c; c = t; }
    }
    int size() const { return n; }
    BrandNode* head() const { return tail ? tail->next : 0; }
    void add(const Product& p) {
        if (tail) {
            BrandNode* c = tail->next;
            do {
                if (toLower(c->brand) == toLower(p.brand)) {
                    c->count++;
                    if (p.price < c->lo) c->lo = p.price;
                    if (p.price > c->hi) c->hi = p.price;
                    return;
                }
                c = c->next;
            } while (c != tail->next);
        }
        BrandNode* b = new BrandNode{p.brand, 1, p.price, p.price, 0};
        if (!tail) { b->next = b; tail = b; }
        else { b->next = tail->next; tail->next = b; tail = b; }
        n++;
    }
};

// =============================================================================
//  Store = everything the program owns
// =============================================================================
struct Store {
    Catalog      catalog;
    ProductIndex index;
    UndoStack    undo;
    Cart         cart;
    OrderQueue   orders;
    int nextId, nextOrderNo;
    Store() : nextId(101), nextOrderNo(1001) {}
};

// =============================================================================
//  LAB 8 : Sorting
// =============================================================================
static int cmpBrand(Product* a, Product* b) {
    int c = toLower(a->brand).compare(toLower(b->brand));
    return c != 0 ? c : toLower(a->name).compare(toLower(b->name));
}
static void insertionSortBrand(Product** a, int n) {
    for (int i = 1; i < n; i++) {
        Product* key = a[i];
        int j = i - 1;
        while (j >= 0 && cmpBrand(a[j], key) > 0) { a[j + 1] = a[j]; j--; }
        a[j + 1] = key;
    }
}
enum SortKey { KEY_PRICE, KEY_STOCK };
static int cmpKey(Product* a, Product* b, SortKey k) {
    if (k == KEY_PRICE) return a->price < b->price ? -1 : (a->price > b->price ? 1 : 0);
    return a->stockQuantity - b->stockQuantity;
}
static void quickSort(Product** a, int lo, int hi, SortKey k, bool asc) {
    if (lo >= hi) return;
    Product* pivot = a[hi];
    int i = lo - 1;
    for (int j = lo; j < hi; j++) {
        int c = cmpKey(a[j], pivot, k);
        if (asc ? c <= 0 : c >= 0) { i++; Product* t = a[i]; a[i] = a[j]; a[j] = t; }
    }
    Product* t = a[i + 1]; a[i + 1] = a[hi]; a[hi] = t;
    quickSort(a, lo, i, k, asc);
    quickSort(a, i + 2, hi, k, asc);
}

// =============================================================================
//  Display
// =============================================================================
static void printTable(Product** a, int n) {
    const string line(103, '-');
    cout << line << "\n" << left
         << setw(6) << "ID" << setw(30) << "PRODUCT NAME" << setw(17) << "CATEGORY"
         << setw(20) << "BRAND" << right << setw(14) << "PRICE" << setw(8) << "STOCK"
         << "  " << left << "STATUS" << "\n" << line << "\n";
    if (n == 0) cout << "  No products found.\n";
    for (int i = 0; i < n; i++) {
        Product* p = a[i];
        string status = p->stockQuantity == 0 ? "OUT" : (p->stockQuantity <= LOW_STOCK ? "LOW" : "OK");
        cout << left << setw(6) << p->id << setw(30) << fit(p->name, 29)
             << setw(17) << fit(p->category, 16) << setw(20) << fit(p->brand, 19)
             << right << setw(14) << money(p->price) << setw(8) << p->stockQuantity
             << "  " << left << status << "\n";
    }
    cout << line << "\n" << n << " product(s) listed.\n";
}
static void printHeader(const string& title) {
    cout << "\n" << string(60, '=') << "\n" << center(title, 60) << "\n" << string(60, '=') << "\n";
}

// =============================================================================
//  Menu operations
// =============================================================================

// 1. View complete catalog
static void viewCatalog(Store& s) {
    printHeader("COMPLETE PERFUME CATALOG");
    Product** a = s.catalog.snapshot();
    printTable(a, s.catalog.size());
    delete[] a;
    cout << "(Dynamic array: " << s.catalog.size() << " used / " << s.catalog.cap() << " capacity)\n";
}

// 2. Filter by official brand
static void filterByBrand(Store& s) {
    printHeader("FILTER BY OFFICIAL BRAND");
    BrandRing ring;
    for (int i = 0; i < s.catalog.size(); i++) ring.add(*s.catalog.at(i));
    if (ring.size() == 0) { cout << "  Catalog is empty.\n"; return; }
    cout << "Partner brands:\n";
    BrandNode* b = ring.head();
    for (int i = 1; i <= ring.size(); i++, b = b->next)
        cout << "  " << i << ". " << b->brand << " (" << b->count << ")\n";
    string in = readNonEmpty("Enter brand name or number: ");
    string target = in;
    istringstream ss(in); int num; char extra;
    if ((ss >> num) && !(ss >> extra) && num >= 1 && num <= ring.size()) {
        b = ring.head();
        for (int i = 1; i < num; i++) b = b->next;
        target = b->brand;
    }
    Product** hits = new Product*[s.catalog.size()];
    int n = 0;
    for (int i = 0; i < s.catalog.size(); i++)             // linear scan
        if (containsCI(s.catalog.at(i)->brand, target)) hits[n++] = s.catalog.at(i);
    cout << "\nResults for brand \"" << target << "\":\n";
    printTable(hits, n);
    delete[] hits;
}

// 3. Keyword search (linear)
static void searchKeyword(Store& s) {
    printHeader("SEARCH BY KEYWORD (LINEAR SEARCH)");
    string kw = readNonEmpty("Keyword (name, brand or category): ");
    Product** hits = new Product*[s.catalog.size() > 0 ? s.catalog.size() : 1];
    int n = 0, compared = 0;
    for (int i = 0; i < s.catalog.size(); i++) {
        Product* p = s.catalog.at(i);
        compared++;
        if (containsCI(p->name, kw) || containsCI(p->brand, kw) || containsCI(p->category, kw))
            hits[n++] = p;
    }
    cout << "\nScanned " << compared << " record(s), " << n << " match(es) for \"" << kw << "\":\n";
    printTable(hits, n);
    delete[] hits;
}

// 4. Price range (sort + binary search)
static void filterByPrice(Store& s) {
    printHeader("FILTER BY PRICE RANGE (BINARY SEARCH)");
    double lo = readDouble("Minimum price (PHP): ", 0);
    double hi = readDouble("Maximum price (PHP): ", 0);
    if (lo > hi) { double t = lo; lo = hi; hi = t; }
    int n = s.catalog.size();
    Product** a = s.catalog.snapshot();
    quickSort(a, 0, n - 1, KEY_PRICE, true);               // binary search needs sorted data

    int l = 0, h = n;                                      // first index with price >= lo
    while (l < h) { int m = (l + h) / 2; if (a[m]->price < lo) l = m + 1; else h = m; }
    int first = l;
    l = first; h = n;                                      // first index with price > hi
    while (l < h) { int m = (l + h) / 2; if (a[m]->price <= hi) l = m + 1; else h = m; }
    int last = l;

    cout << "\nPrice range " << money(lo) << " - " << money(hi)
         << "  (binary search: indexes " << first << " to " << (last - 1) << ")\n";
    printTable(a + first, last - first);
    delete[] a;
}

// 5. Shopping cart
static void cartMenu(Store& s) {
    while (true) {
        printHeader("SHOPPING CART (DOUBLY LINKED LIST)");
        s.cart.display();
        cout << "\n  1. Add item by Product ID\n  2. Next item  ->\n  3. Previous item  <-\n"
             << "  4. Remove highlighted item\n  5. Place order (send to checkout queue)\n"
             << "  0. Back to main menu\n";
        int c = readInt("Choice: ", 0, 5);
        if (c == 0) return;
        if (c == 1) {
            Product** a = s.catalog.snapshot();
            printTable(a, s.catalog.size());
            delete[] a;
            int id = readInt("Product ID to add: ", 1, 999999);
            int steps;
            Product* p = s.index.find(id, steps);          // BST lookup
            if (!p) { cout << "  Product ID " << id << " not found (BST checked " << steps << " node(s)).\n"; pause(); continue; }
            int room = p->stockQuantity - s.cart.qtyOf(id);
            if (room <= 0) { cout << "  Sorry, no more stock available for " << p->name << ".\n"; pause(); continue; }
            cout << "  Found: " << p->name << " (" << p->brand << ") in " << steps << " BST step(s). Available: " << room << "\n";
            int q = readInt("Quantity: ", 1, room);
            s.cart.add(*p, q);
            cout << "  Added to cart.\n";
        } else if (c == 2) {
            if (!s.cart.next()) cout << "  Already at the last item.\n";
        } else if (c == 3) {
            if (!s.cart.prev()) cout << "  Already at the first item.\n";
        } else if (c == 4) {
            if (s.cart.empty()) cout << "  Cart is empty.\n";
            else { s.cart.removeCurrent(); cout << "  Item removed.\n"; }
        } else if (c == 5) {
            if (s.cart.empty()) { cout << "  Cart is empty.\n"; continue; }
            string name = readNonEmpty("Customer name: ");
            string address = readNonEmpty("Customer address: ");
            string contact = readPhone("Contact number: ");
            int no = s.nextOrderNo++;
            s.orders.enqueue(no, name, address, contact, s.cart.detachAll());
            cout << "  Order #" << no << " placed. Customers waiting in queue: " << s.orders.size() << "\n";
            pause();
            return;
        }
    }
}

// 6. Process checkout queue
static void processCheckout(Store& s) {
    printHeader("PROCESS CUSTOMER CHECKOUT QUEUE (FIFO)");
    cout << "Orders waiting: " << s.orders.size() << "\n";
    Order* o = s.orders.dequeue();
    if (!o) { cout << "  No orders to process.\n"; return; }
    cout << "\nNow serving Order #" << o->no << " - " << o->customer << "\n"
         << "Deliver to: " << o->address << "\n"
         << "Contact:    " << o->contact << "\n" << string(60, '-') << "\n";
    double total = 0;
    for (CartNode* n = o->items; n; n = n->next) {
        int steps;
        Product* p = s.index.find(n->id, steps);
        if (!p) { cout << "  [SKIPPED] " << n->name << " is no longer in the catalog.\n"; continue; }
        int q = n->qty;
        if (p->stockQuantity < q) {
            if (p->stockQuantity == 0) { cout << "  [SKIPPED] " << n->name << " is out of stock.\n"; continue; }
            cout << "  [PARTIAL] " << n->name << ": only " << p->stockQuantity << " left (wanted " << q << ").\n";
            q = p->stockQuantity;
        }
        p->stockQuantity -= q;
        double sub = p->price * q;
        total += sub;
        cout << "  " << left << setw(30) << fit(p->name, 29) << right << setw(3) << q << " x "
             << setw(14) << money(p->price) << setw(16) << money(sub) << "\n";
    }
    cout << string(60, '-') << "\n  TOTAL PAID: " << money(total) << "\n  Thank you, " << o->customer << "!\n";
    freeChain(o->items);
    delete o;
    cout << "Orders still waiting: " << s.orders.size() << "\n";
}

// 7. Undo (stack pop)
static void undoAction(Store& s) {
    printHeader("ADMIN UNDO (LIFO STACK POP)");
    Action a;
    if (!s.undo.pop(a)) { cout << "  Nothing to undo.\n"; return; }
    int steps;
    Product* p = s.index.find(a.snap.id, steps);
    if (a.type == ACT_ADD) {
        if (p) { s.index.remove(p->id); s.catalog.remove(p); delete p; }
        cout << "  Undid ADD: removed \"" << a.snap.name << "\" from the catalog.\n";
    } else if (a.type == ACT_REMOVE) {
        Product* np = new Product(a.snap);
        s.catalog.add(np);
        s.index.insert(np);
        cout << "  Undid REMOVE: restored \"" << a.snap.name << "\" (stock " << a.snap.stockQuantity << ").\n";
    } else {
        if (p) {
            int delta = a.snap.stockQuantity - a.oldStock;  // the stock that was added
            p->stockQuantity -= delta;
            if (p->stockQuantity < 0) p->stockQuantity = 0;
            cout << "  Undid RESTOCK: \"" << p->name << "\" is now at " << p->stockQuantity << " in stock.\n";
        } else cout << "  Product no longer exists; nothing to restore.\n";
    }
    cout << "  Remaining undo history: " << s.undo.size() << "\n";
}

// 8. Brand banner showcase (circular list)
static void bannerShowcase(Store& s) {
    printHeader("PARTNER BRAND BANNER SHOWCASE (CIRCULAR LIST)");
    BrandRing ring;
    for (int i = 0; i < s.catalog.size(); i++) ring.add(*s.catalog.at(i));
    if (ring.size() == 0) { cout << "  No brands to show.\n"; return; }
    BrandNode* cur = ring.head();
    int k = 1;
    while (true) {
        const size_t W = 52;
        string bar = "+" + string(W, '=') + "+";
        cout << "\n" << bar << "\n"
             << "|" << center("*  *  *  OFFICIAL PARTNER BRAND  *  *  *", W) << "|\n"
             << "|" << string(W, ' ') << "|\n"
             << "|" << center(toLower(cur->brand) == cur->brand ? cur->brand : cur->brand, W) << "|\n"
             << "|" << string(W, ' ') << "|\n"
             << "|" << center(to_string(cur->count) + " fragrance(s) available", W) << "|\n"
             << "|" << center(money(cur->lo) + " - " + money(cur->hi), W) << "|\n"
             << bar << "\n" << "  Banner " << k << " of " << ring.size() << "\n";
        string in = readLine("[Enter] next banner   [Q] quit: ");
        if (toLower(in) == "q") return;
        cur = cur->next;                       // wraps around automatically
        k = (k % ring.size()) + 1;
    }
}

                        // 9. kanan Sort display

static void sortDisplay(Store& s) {
    printHeader("SORT INVENTORY DISPLAY");
    cout << "  1. Brand A-Z            (Insertion Sort)\n  2. Price: low to high   (Quick Sort)\n"
         << "  3. Price: high to low   (Quick Sort)\n  4. Stock: low to high   (Quick Sort)\n"
         << "  5. Stock: high to low   (Quick Sort)\n";
    int c = readInt("Sort by: ", 1, 5);
    int n = s.catalog.size();
    Product** a = s.catalog.snapshot();
    if (c == 1) insertionSortBrand(a, n);
    else quickSort(a, 0, n - 1, (c <= 3 ? KEY_PRICE : KEY_STOCK), (c == 2 || c == 4));
    printTable(a, n);
    delete[] a;
}

// 11. kanan Admin panel

static Product* askExistingProduct(Store& s) {
    Product** a = s.catalog.snapshot();
    printTable(a, s.catalog.size());
    delete[] a;
    int id = readInt("Product ID: ", 1, 999999);
    int steps;
    Product* p = s.index.find(id, steps);
    if (!p) cout << "  Product ID " << id << " not found.\n";
    return p;
}
static void adminPanel(Store& s) {
    while (true) {
        printHeader("ADMIN: INVENTORY MANAGEMENT");
        cout << "  1. Add new perfume\n  2. Restock existing perfume\n  3. Remove perfume\n"
             << "  0. Back   (undo history: " << s.undo.size() << ")\n";
        int c = readInt("Choice: ", 0, 3);
        if (c == 0) return;
        if (c == 1) {
            Product* p = new Product;
            p->id = s.nextId++;
            p->name = readNonEmpty("Perfume name (e.g. Sauvage 100ml): ");
            const char* cats[] = {"Eau de Parfum", "Eau de Toilette", "Parfum", "Cologne", "Body Mist"};
            for (int i = 0; i < 5; i++) cout << "  " << (i + 1) << ". " << cats[i] << "\n";
            int ci = readInt("Category (1-5): ", 1, 5);
            p->category = cats[ci - 1];
            p->brand = readNonEmpty("Brand: ");
            p->price = readDouble("Price (PHP): ", 0.01);
            p->stockQuantity = readInt("Stock quantity: ", 0, 1000000);
            s.catalog.add(p);
            s.index.insert(p);
            s.undo.push(ACT_ADD, *p, 0);
            cout << "  Added \"" << p->name << "\" with ID " << p->id << ".\n";
        } else if (c == 2) {
            Product* p = askExistingProduct(s);
            if (!p) continue;
            int add = readInt("Quantity to add: ", 1, 1000000);
            int old = p->stockQuantity;
            p->stockQuantity += add;
            s.undo.push(ACT_RESTOCK, *p, old);
            cout << "  " << p->name << " stock: " << old << " -> " << p->stockQuantity << "\n";
        } else {
            Product* p = askExistingProduct(s);
            if (!p) continue;
            string yn = readLine("Remove \"" + p->name + "\"? (y/n): ");
            if (toLower(yn) != "y") { cout << "  Cancelled.\n"; continue; }
            s.undo.push(ACT_REMOVE, *p, 0);
            s.index.remove(p->id);
            s.catalog.remove(p);
            cout << "  Removed \"" << p->name << "\".\n";
            delete p;
        }
    }
}

// -----------------------------------------------------------------------------
//  Sample data (replace with your real stock via the Admin menu)
// -----------------------------------------------------------------------------

static void seed(Store& s) {
    struct Row { const char *name, *cat, *brand; double price; int stock; };
    Row rows[] = {
        {"Sauvage 100ml",            "Eau de Toilette", "Dior",                 6200, 20},
        {"J'adore 100ml",            "Eau de Parfum",   "Dior",                 7400, 12},
        {"Miss Dior 50ml",           "Eau de Parfum",   "Dior",                 5600,  8},
        {"Bleu de Chanel 100ml",     "Eau de Parfum",   "Chanel",               7900, 10},
        {"Coco Mademoiselle 50ml",   "Eau de Parfum",   "Chanel",               7200,  6},
        {"Chance Eau Tendre 50ml",   "Eau de Toilette", "Chanel",               6800,  4},
        {"Eros 100ml",               "Eau de Toilette", "Versace",              4300, 25},
        {"Bright Crystal 90ml",      "Eau de Toilette", "Versace",              3900, 14},
        {"CK One 100ml",             "Eau de Toilette", "Calvin Klein",         2500, 30},
        {"Eternity 100ml",           "Eau de Parfum",   "Calvin Klein",         2800,  3},
        {"Acqua di Gio 100ml",       "Eau de Toilette", "Giorgio Armani",       5100, 18},
        {"Si 50ml",                  "Eau de Parfum",   "Giorgio Armani",       5900,  9},
        {"Black Opium 50ml",         "Eau de Parfum",   "Yves Saint Laurent",   5700, 11},
        {"Libre 50ml",               "Eau de Parfum",   "Yves Saint Laurent",   5800,  7},
        {"Vanilla Mist 236ml",       "Body Mist",       "Victoria's Secret",    1100, 40},
    };
    for (size_t i = 0; i < sizeof(rows) / sizeof(rows[0]); i++) {
        Product* p = new Product{s.nextId++, rows[i].name, rows[i].cat, rows[i].brand,
                                 rows[i].price, rows[i].stock};
        s.catalog.add(p);
    }
    s.index.buildFrom(s.catalog);
}

// -----------------------------------------------------------------------------
//  Main menu
// -----------------------------------------------------------------------------
static void showMenu() {
    string eq(64, '=');
    cout << "\n" << eq << "\n"
         << center(STORE_NAME, 64) << "\n"
         << center("BRAND-AFFILIATED PERFUME INVENTORY & SEARCH SYSTEM", 64) << "\n" << eq << "\n"
         << " 1. View Complete Catalog (Tabular View with Brand Affiliations)\n"
         << " 2. Filter Products by Official Brand (e.g., Dior, Chanel, Versace)\n"
         << " 3. Search Products by Keyword (Linear Search)\n"
         << " 4. Filter Products by Price Range (Binary Search)\n"
         << " 5. Add Item to Shopping Cart (Doubly Linked List Navigation)\n"
         << " 6. Process Customer Checkout Queue (FIFO Order Queue)\n"
         << " 7. Perform Admin Undo Action (LIFO Stack Pop)\n"
         << " 8. Display Partner Brand Banner Showcase (Circular List Cycle)\n"
         << " 9. Sort Inventory Display (By Price, Stock, or Brand)\n"
         << "10. Exit\n"
         << "11. [Admin] Add / Restock / Remove Perfumes\n"
         << eq << "\n";
}

int main() {
    Store store;
    seed(store);
    while (true) {
        showMenu();
        int c = readInt("Enter choice: ", 1, 11);
        switch (c) {
            case 1:  viewCatalog(store);     pause(); break;
            case 2:  filterByBrand(store);   pause(); break;
            case 3:  searchKeyword(store);   pause(); break;
            case 4:  filterByPrice(store);   pause(); break;
            case 5:  cartMenu(store);                 break;
            case 6:  processCheckout(store); pause(); break;
            case 7:  undoAction(store);      pause(); break;
            case 8:  bannerShowcase(store);           break;
            case 9:  sortDisplay(store);     pause(); break;
            case 10: cout << "\nThank you for using the system. Goodbye!\n"; return 0;
            case 11: adminPanel(store);               break;
        }
    }
}
