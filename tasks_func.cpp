#include <windows.h>
#include <iostream>
#include <sstream>
#include <fstream>
#include <iomanip>
#include <vector>
#include <string>

struct Task {
    int timeInMinutes; // Время в минутах от начала недели (0:00 воскресенья)
    int type;
    int tag;
    std::string name;
    std::string description;

    void display() const {
        std::cout << name << "|" << description << "|" << timeInMinutes << "|" << type << std::endl;
    }
};

/* для  задач:
* 
    std::string name;  - название подзадачи (кратко)
    std::string description;  - весь длинный текст что конкретно нужно сделать
    int timeInMinutes;  - номер подзадачи
    int type;  - тип задачи (уникальный) 1xx для развлечений 2xx для разработки

    если номер задачи равен 0:
        std::string name;  - название задачи
        std::string description;  - описание заадчи
        int timeInMinutes = 0;
        int type;  - тип задачи для типо для одного приложения один тип

*/

// Функция для чтения задач из файла
static std::vector<Task> loadTasks(const std::string& filename) {
    std::vector<Task> tasks;
    std::ifstream file(filename);

    if (!file.is_open()) {
        return tasks;
    }

    std::string line;
    while (std::getline(file, line)) {
        std::stringstream ss(line);
        Task task;
        std::getline(ss, task.name, '|');
        std::getline(ss, task.description, '|');
        ss >> task.timeInMinutes;
        ss.ignore(); // Пропускаем разделитель '|'
        ss >> task.type;
        ss.ignore(); // Пропускаем разделитель '|'
        ss >> task.tag;
        tasks.push_back(task);
    }

    file.close();
    return tasks;
}

static std::string convertToHHMM(int timeInMinutes) {
    int minutesInDay = timeInMinutes % 1440; // Убираем дни недели
    int hours = int(floor(minutesInDay / 60));
    int minutes = minutesInDay % 60;

    // Форматируем строку в HH:MM
    std::ostringstream oss;
    oss << std::setw(2) << std::setfill('0') << hours << ":"
        << std::setw(2) << std::setfill('0') << minutes;

    return oss.str();
}
// Функция для записи задач в файл
static void saveTasks(const std::string& filename, const std::vector<Task>& tasks) {
    std::ofstream file(filename, std::ios::trunc);

    if (!file.is_open()) {
        return;
    }

    for (const Task& task : tasks) {
        file << task.name << "|" << task.description << "|" << task.timeInMinutes << "|" << task.type << "|" << task.tag << std::endl;
    }

    file.close();
}

// Функция для вычисления текущего времени в минутах от начала недели
static int getCurrentTimeInMinutes() {
    SYSTEMTIME systemTime;
    GetLocalTime(&systemTime);

    // День недели (0 - воскресенье, 1 - понедельник, ..., 6 - суббота)
    int dayOfWeek = systemTime.wDayOfWeek;
    int hours = systemTime.wHour;
    int minutes = systemTime.wMinute;

    // Время в минутах от начала недели
    return dayOfWeek * 1440 + hours * 60 + minutes;
}

// Функция для отображения всех задач

// Функция для добавления новой задачи
//static void addTask(std::vector<Task>& tasks) {
//    Task newTask;
//    std::cout << "Введите название задачи: ";
//    std::getline(std::cin, newTask.name);
//    std::cout << "Введите описание задачи: ";
//    std::getline(std::cin, newTask.description);
//    std::cout << "Введите время выполнения задачи в минутах от начала недели (0-10079): ";
//    std::cin >> newTask.timeInMinutes;
//    std::cout << "Введите тип задачи (целое число): ";
//    std::cin >> newTask.type;
//    std::cin.ignore(); // Очистка буфера
//
//
//    for (size_t i = 0; i < tasks.size(); ++i) {
//        int difference = newTask.timeInMinutes - tasks[i].timeInMinutes;
//        if (difference < 0) {
//            tasks.insert(tasks.begin() + i, newTask);
//            break;
//        }
//    }
//}

static int getNearestTask(const std::vector<Task>& tasks) {
    if (tasks.empty()) return 0;

    int currentTimeInMinutes = getCurrentTimeInMinutes();
    int counter = -1;

    for (const Task& task : tasks) {
        int difference = task.timeInMinutes - currentTimeInMinutes;
        if (difference > 0) {
            break;
        }
        counter++;
    }
    if (counter < 0) counter = int(tasks.size()+counter);
    return counter;
}

static void pop_back_utf8(std::string& str) {
    if (str.empty()) return;
    auto it = str.end();
    do {
        --it;
    } while (it != str.begin() && ((*it & 0xC0) == 0x80)); // Убираем continuation bytes
    str.erase(it, str.end());
}