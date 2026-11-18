#include <iostream>
#include <fstream>
#include <string>
#include <exception>
#include <cmath>
#include <new>
#include <windows.h>

using namespace std;

// =========================================================
// Завдання №2: Ієрархія власних класів-винятків у просторі імен
// =========================================================
namespace AppExceptions {

    // Базовий клас для всіх користувацьких винятків
    class AppException : public std::exception {
    protected:
        string fullMessage;

    public:
        AppException(const string& category, const string& message, const string& context = "") {
            fullMessage = "[" + category + "] " + message;
            if (!context.empty()) {
                fullMessage += " (Контекст: " + context + ")";
            }
        }

        const char* what() const noexcept override {
            return fullMessage.c_str();
        }
    };

    // ---------------------------------------------------------
    // 1. Гілка помилок роботи з пам'яттю
    // ---------------------------------------------------------
    class MemoryException : public AppException {
    public:
        MemoryException(const string& message, const string& context = "")
            : AppException("Помилка пам'яті", message, context) {}
    };

    class OutOfMemoryException : public MemoryException {
    public:
        OutOfMemoryException(const string& context = "")
            : MemoryException("Недостатньо динамічної пам'яті для виділення блоку.", context) {}
    };

    // ---------------------------------------------------------
    // 2. Гілка помилок структур даних (двозв'язний список тощо)
    // ---------------------------------------------------------
    class DataStructureException : public AppException {
    public:
        DataStructureException(const string& message, const string& context = "")
            : AppException("Помилка структури даних", message, context) {}
    };

    class EmptyContainerException : public DataStructureException {
    public:
        EmptyContainerException(const string& operation)
            : DataStructureException("Операція неможлива: контейнер порожній.", operation) {}
    };

    class IndexOutOfBoundsException : public DataStructureException {
    public:
        IndexOutOfBoundsException(size_t index, size_t size, const string& operation)
            : DataStructureException(
                  "Індекс " + to_string(index) + " виходить за допустимі межі (розмір: " + to_string(size) + ").",
                  operation) {}
    };

    // ---------------------------------------------------------
    // 3. Гілка математичних помилок (рівняння, обчислення)
    // ---------------------------------------------------------
    class MathException : public AppException {
    public:
        MathException(const string& message, const string& context = "")
            : AppException("Математична помилка", message, context) {}
    };

    class DivisionByZeroException : public MathException {
    public:
        DivisionByZeroException(const string& expression = "")
            : MathException("Ділення на нуль заборонено.", expression) {}
    };

    class NegativeSquareRootException : public MathException {
    public:
        NegativeSquareRootException(double value)
            : MathException("Неможливо добути дійсний квадратний корінь із від'ємного числа: " + to_string(value)) {}
    };

    // ---------------------------------------------------------
    // 4. Гілка помилок роботи з файлами
    // ---------------------------------------------------------
    class FileException : public AppException {
    public:
        FileException(const string& message, const string& fileName = "")
            : AppException("Файлова помилка", message, fileName) {}
    };

    class FileOpenException : public FileException {
    public:
        FileOpenException(const string& fileName)
            : FileException("Не вдалося відкрити файл (файл відсутній або заблокований).", fileName) {}
    };

    class FileReadWriteException : public FileException {
    public:
        FileReadWriteException(const string& fileName, const string& details)
            : FileException("Помилка читання/запису даних: " + details, fileName) {}
    };
}

// =========================================================
// Завдання №1: Двозв'язний список з обробкою винятків
// =========================================================
template <typename T>
class DoublyLinkedList {
private:
    struct Node {
        T data;
        Node* prev;
        Node* next;

        Node(const T& value) : data(value), prev(nullptr), next(nullptr) {}
    };

    Node* head;
    Node* tail;
    size_t size;

   Node* createNodeSafe(const T& value, const string& context) {
        try {
            Node* newNode = new (std::nothrow) Node(value);
            if (!newNode) {
                throw AppExceptions::OutOfMemoryException(context);
            }
            return newNode;
        } catch (const std::bad_alloc&) {
            throw AppExceptions::OutOfMemoryException(context);
        }
    }

public:
    DoublyLinkedList() : head(nullptr), tail(nullptr), size(0) {}

    // Конструктор копіювання
    DoublyLinkedList(const DoublyLinkedList<T>& other) : head(nullptr), tail(nullptr), size(0) {
        Node* current = other.head;
        while (current != nullptr) {
            pushBack(current->data);
            current = current->next;
        }
    }

    // Оператор присвоєння копіюванням
    DoublyLinkedList<T>& operator=(const DoublyLinkedList<T>& other) {
        if (this == &other) {
            return *this;
        }
        clear();
        Node* current = other.head;
        while (current != nullptr) {
            pushBack(current->data);
            current = current->next;
        }
        return *this;
    }

    ~DoublyLinkedList() {
        clear();
    }

    bool isEmpty() const {
        return size == 0;
    }

    size_t getSize() const {
        return size;
    }

    // Додавання в початок списку
    void pushFront(const T& value) {
        Node* newNode = createNodeSafe(value, "DoublyLinkedList::pushFront");
        if (isEmpty()) {
            head = tail = newNode;
        } else {
            newNode->next = head;
            head->prev = newNode;
            head = newNode;
        }
        size++;
    }

    // Додавання в кінець списку
    void pushBack(const T& value) {
        Node* newNode = createNodeSafe(value, "DoublyLinkedList::pushBack");
        if (isEmpty()) {
            head = tail = newNode;
        } else {
            newNode->prev = tail;
            tail->next = newNode;
            tail = newNode;
        }
        size++;
    }

    // Вставка елемента за індексом
    void insertAt(size_t index, const T& value) {
        if (index > size) {
            throw AppExceptions::IndexOutOfBoundsException(index, size, "DoublyLinkedList::insertAt");
        }
        if (index == 0) {
            pushFront(value);
            return;
        }
        if (index == size) {
            pushBack(value);
            return;
        }

        Node* newNode = createNodeSafe(value, "DoublyLinkedList::insertAt");
        Node* current = head;
        for (size_t i = 0; i < index; i++) {
            current = current->next;
        }

        newNode->prev = current->prev;
        newNode->next = current;
        current->prev->next = newNode;
        current->prev = newNode;
        size++;
    }

    // Видалення з початку списку
    void popFront() {
        if (isEmpty()) {
            throw AppExceptions::EmptyContainerException("DoublyLinkedList::popFront");
        }
        Node* temp = head;
        if (head == tail) {
            head = tail = nullptr;
        } else {
            head = head->next;
            head->prev = nullptr;
        }
        delete temp;
        size--;
    }

    // Видалення з кінця списку
    void popBack() {
        if (isEmpty()) {
            throw AppExceptions::EmptyContainerException("DoublyLinkedList::popBack");
        }
        Node* temp = tail;
        if (head == tail) {
            head = tail = nullptr;
        } else {
            tail = tail->prev;
            tail->next = nullptr;
        }
        delete temp;
        size--;
    }

    // Видалення елемента за індексом
    void removeAt(size_t index) {
        if (isEmpty()) {
            throw AppExceptions::EmptyContainerException("DoublyLinkedList::removeAt");
        }
        if (index >= size) {
            throw AppExceptions::IndexOutOfBoundsException(index, size, "DoublyLinkedList::removeAt");
        }
        if (index == 0) {
            popFront();
            return;
        }
        if (index == size - 1) {
            popBack();
            return;
        }

        Node* current = head;
        for (size_t i = 0; i < index; i++) {
            current = current->next;
        }

        current->prev->next = current->next;
        current->next->prev = current->prev;
        delete current;
        size--;
    }

    // Доступ до першого елемента
    T& front() {
        if (isEmpty()) {
            throw AppExceptions::EmptyContainerException("DoublyLinkedList::front");
        }
        return head->data;
    }

    // Доступ до останнього елемента
    T& back() {
        if (isEmpty()) {
            throw AppExceptions::EmptyContainerException("DoublyLinkedList::back");
        }
        return tail->data;
    }

    // Доступ за індексом
    T& getAt(size_t index) {
        if (isEmpty()) {
            throw AppExceptions::EmptyContainerException("DoublyLinkedList::getAt");
        }
        if (index >= size) {
            throw AppExceptions::IndexOutOfBoundsException(index, size, "DoublyLinkedList::getAt");
        }
        Node* current = head;
        for (size_t i = 0; i < index; i++) {
            current = current->next;
        }
        return current->data;
    }

    // Імітація виділення надвеликого блоку пам'яті для перевірки OutOfMemoryException
    void reserveExcessiveMemory(size_t elementsCount) {
        try {
            T* buffer = new (std::nothrow) T[elementsCount];
            if (!buffer) {
                throw AppExceptions::OutOfMemoryException("DoublyLinkedList::reserveExcessiveMemory");
            }
            delete[] buffer;
        } catch (const std::bad_alloc&) {
            throw AppExceptions::OutOfMemoryException("DoublyLinkedList::reserveExcessiveMemory");
        }
    }

    void clear() {
        while (!isEmpty()) {
            popFront();
        }
    }

    void print() const {
        cout << "Список (розмір = " << size << "): [ ";
        Node* current = head;
        while (current != nullptr) {
            cout << current->data << (current->next ? " <-> " : " ");
            current = current->next;
        }
        cout << "]\n";
    }
};

// =========================================================
// Допоміжні функції для тестування математичних та файлових винятків
// =========================================================
double divideNumbers(double a, double b) {
    if (b == 0.0) {
        throw AppExceptions::DivisionByZeroException(to_string(a) + " / 0");
    }
    return a / b;
}

double calculateDiscriminantRoot(double d) {
    if (d < 0.0) {
        throw AppExceptions::NegativeSquareRootException(d);
    }
    return sqrt(d);
}

string readFirstLineFromFile(const string& fileName) {
    ifstream file(fileName);
    if (!file.is_open()) {
        throw AppExceptions::FileOpenException(fileName);
    }
    string line;
    if (!getline(file, line)) {
        throw AppExceptions::FileReadWriteException(fileName, "файл порожній або пошкоджений");
    }
    return line;
}

// =========================================================
// Тестування обох завдань у main()
// =========================================================
int main() {
    SetConsoleOutputCP(CP_UTF8);
    SetConsoleCP(CP_UTF8);

    DoublyLinkedList<int> list;

    cout << "=== ТЕСТУВАННЯ ЗАВДАННЯ №1 (Двозв'язний список) ===\n\n";

    // 1. Спроба видалення з порожнього списку
    try {
        cout << "1. Спроба popFront() на порожньому списку:\n   ";
        list.popFront();
    } catch (const AppExceptions::EmptyContainerException& ex) {
        cout << "Перехоплено виняток: " << ex.what() << "\n\n";
    }

    // 2. Коректне наповнення та вставка
    list.pushBack(10);
    list.pushBack(30);
    list.pushFront(5);
    list.insertAt(2, 20);
    cout << "2. Після додавання елементів:\n   ";
    list.print();
    cout << '\n';

    // 3. Спроба доступу та видалення за неіснуючим індексом
    try {
        cout << "3. Спроба getAt(10) у списку з 4 елементів:\n   ";
        cout << list.getAt(10) << '\n';
    } catch (const AppExceptions::IndexOutOfBoundsException& ex) {
        cout << "Перехоплено виняток: " << ex.what() << "\n\n";
    }

    // 4. Перевірка нестачі пам'яті (OutOfMemoryException)
    try {
        cout << "4. Спроба виділити критично великий блок пам'яті:\n   ";
        size_t hugeCount = static_cast<size_t>(-1) / sizeof(int);
        list.reserveExcessiveMemory(hugeCount);
    } catch (const AppExceptions::MemoryException& ex) {
        cout << "Перехоплено виняток: " << ex.what() << "\n\n";
    }

    cout << "=== ТЕСТУВАННЯ ЗАВДАННЯ №2 (Ієрархія винятків у просторі імен) ===\n\n";

    // 5. Математичні винятки
    try {
        cout << "5. Спроба ділення 42 на 0:\n   ";
        divideNumbers(42.0, 0.0);
    } catch (const AppExceptions::MathException& ex) {
        cout << "Перехоплено виняток: " << ex.what() << "\n\n";
    }

    try {
        cout << "6. Спроба обчислити корінь із дискримінанта D = -16:\n   ";
        calculateDiscriminantRoot(-16.0);
    } catch (const AppExceptions::MathException& ex) {
        cout << "Перехоплено виняток: " << ex.what() << "\n\n";
    }

    // 6. Файлові винятки
    try {
        cout << "7. Спроба відкрити неіснуючий файл:\n   ";
        readFirstLineFromFile("non_existent_data.txt");
    } catch (const AppExceptions::FileException& ex) {
        cout << "Перехоплено виняток: " << ex.what() << "\n\n";
    }

    // 7. Поліморфне перехоплення через базовий клас AppException
    try {
        cout << "8. Очищення списку та виклик back() через базовий перехоплювач AppException:\n   ";
        list.clear();
        cout << list.back() << '\n';
    } catch (const AppExceptions::AppException& ex) {
        cout << "Перехоплено базовим класом: " << ex.what() << "\n";
    }

    return 0;
}