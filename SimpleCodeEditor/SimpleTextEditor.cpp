#include <FL/Fl.H>
#include <FL/Fl_Window.H>
#include <FL/Fl_Text_Editor.H>
#include <FL/Fl_Menu_Bar.H>
#include <FL/Fl_File_Chooser.H>
#include <FL/Fl_Button.H> // Для кнопок
#include <FL/Fl_Box.H> // Для отображения информации
#include <FL/Fl_Preferences.H> // Для сохранения настроек
#include <FL/Fl_Choice.H> // для выбора шрифта
#include <FL/fl_draw.H>  // Вместо Fl_Font.H
#include <FL/Fl_Input.H> // для поиска и замены
#include <FL/Fl_Text_Buffer.H>
#include <FL/fl_ask.H>  // Для Fl::alert
#include <FL/Fl_Text_Display.H>
#include <iostream>
#include <fstream>
#include <string>

// 1. Глобальные переменные
Fl_Window* window;
Fl_Text_Editor* editor;
Fl_Text_Buffer* textBuffer;  // Добавляем буфер текста
Fl_File_Chooser* fileChooser; // Диалог выбора файла
std::string currentFilePath;
Fl_Preferences prefs(Fl_Preferences::USER, "MyCompany", "SimpleTextEditor");

// 2. Функции обратного вызова для меню и кнопок
void file_open_cb(Fl_Widget* widget, void* data);
void file_save_cb(Fl_Widget* widget, void* data);
void file_save_as_cb(Fl_Widget* widget, void* data); // Добавлено: Save As
void file_exit_cb(Fl_Widget* widget, void* data);

void edit_cut_cb(Fl_Widget* widget, void* data);
void edit_copy_cb(Fl_Widget* widget, void* data);
void edit_paste_cb(Fl_Widget* widget, void* data);
void edit_undo_cb(Fl_Widget* widget, void* data);
void edit_redo_cb(Fl_Widget* widget, void* data);
void edit_find_cb(Fl_Widget* widget, void* data); // Добавлено: Find
void edit_replace_cb(Fl_Widget* widget, void* data); // Добавлено: Replace

void view_font_cb(Fl_Widget* widget, void* data); // Добавлено: Font

// Дополнительные функции обратного вызова и виджеты для поиска/замены:
void find_replace_ok_cb(Fl_Widget* widget, void* data); // Для кнопки OK в диалоге Find/Replace
void find_replace_cancel_cb(Fl_Widget* widget, void* data); // Для кнопки Cancel в диалоге Find/Replace
Fl_Window* findReplaceWindow = nullptr;
Fl_Input* findInput = nullptr;
Fl_Input* replaceInput = nullptr;
Fl_Button* findReplaceOKButton = nullptr;
Fl_Button* findReplaceCancelButton = nullptr;
Fl_Box* searchStatusBox = nullptr; // для отображения статуса поиска
// 3.  Функция для загрузки текста из файла
bool loadFile(const std::string& filePath);

// 4. Функция для сохранения текста в файл
bool saveFile(const std::string& filePath);

// 5.  Функция для отображения сообщения об ошибке
void showError(const std::string& message);

// 6.  Функция для обновления заголовка окна
void updateWindowTitle();

// 7.  Шрифт по умолчанию
int defaultFont = FL_HELVETICA;

// 8.  Диалог выбора шрифта
Fl_Window* fontWindow = nullptr;
Fl_Choice* fontChoice = nullptr;
Fl_Button* fontOKButton = nullptr;
void font_ok_cb(Fl_Widget* widget, void* data);

//-----------------------
// ОСНОВНАЯ ФУНКЦИЯ
//-----------------------
int main(int argc, char** argv) {
    window = new Fl_Window(800, 600, "Simple Text Editor");

    // Создаем буфер текста
    textBuffer = new Fl_Text_Buffer();
    
    // 10. Меню
    Fl_Menu_Bar* menu = new Fl_Menu_Bar(0, 0, 800, 25);
    menu->add("File/Open", FL_CTRL + 'o', file_open_cb, 0);
    menu->add("File/Save", FL_CTRL + 's', file_save_cb, 0);
    menu->add("File/Save As", FL_CTRL + FL_SHIFT + 's', file_save_as_cb, 0);
    menu->add("File/Exit", FL_CTRL + 'q', file_exit_cb, 0);
    menu->add("Edit/Cut", FL_CTRL + 'x', edit_cut_cb, 0);
    menu->add("Edit/Copy", FL_CTRL + 'c', edit_copy_cb, 0);
    menu->add("Edit/Paste", FL_CTRL + 'v', edit_paste_cb, 0);
    menu->add("Edit/Undo", FL_CTRL + 'z', edit_undo_cb, 0);
    menu->add("Edit/Redo", FL_CTRL + 'y', edit_redo_cb, 0);
    menu->add("Edit/Find...", FL_CTRL + 'f', edit_find_cb, 0);
    menu->add("Edit/Replace...", FL_CTRL + 'h', edit_replace_cb, 0);
    menu->add("View/Font...", 0, view_font_cb, 0);

    // 11. Текстовый редактор
    editor = new Fl_Text_Editor(0, 25, 800, 575);
    editor->buffer(textBuffer);  // Устанавливаем буфер
    editor->textfont(defaultFont);

    window->end();
    updateWindowTitle();
    window->show(argc, argv);

    // 12. Загрузка настроек
    int font;
    prefs.get("font", font, FL_HELVETICA);
    editor->textfont(font);

    return Fl::run();
}

//-----------------------
// РЕАЛИЗАЦИЯ ФУНКЦИЙ ОБРАТНОГО ВЫЗОВА
//-----------------------

void file_open_cb(Fl_Widget* widget, void* data) {
    Fl_File_Chooser chooser(".", "*", Fl_File_Chooser::SINGLE, "Open File");
    chooser.show();
    while (chooser.shown()) {
        Fl::wait();
    }

    if (chooser.value() != nullptr) {
        std::string filePath = chooser.value();
        if (loadFile(filePath)) {
            currentFilePath = filePath;
            updateWindowTitle();
        }
    }
}

void file_save_cb(Fl_Widget* widget, void* data) {
    if (currentFilePath.empty()) {
        file_save_as_cb(widget, data); // Если файл не был открыт, используем "Save As"
    }
    else {
        if (saveFile(currentFilePath)) {
            //  Уведомление об успешном сохранении (можно реализовать)
        }
    }
}

void file_save_as_cb(Fl_Widget* widget, void* data) {
    Fl_File_Chooser chooser(".", "*", Fl_File_Chooser::CREATE, "Save File As");
    chooser.show();
    while (chooser.shown()) {
        Fl::wait();
    }

    if (chooser.value() != nullptr) {
        std::string filePath = chooser.value();
        if (saveFile(filePath)) {
            currentFilePath = filePath;
            updateWindowTitle();
        }
    }
}


void file_exit_cb(Fl_Widget* widget, void* data) {
    exit(0);
}

void edit_cut_cb(Fl_Widget* widget, void* data) {
    Fl_Text_Editor::kf_cut(0, editor);
}

void edit_copy_cb(Fl_Widget* widget, void* data) {
    Fl_Text_Editor::kf_copy(0, editor);
}

void edit_paste_cb(Fl_Widget* widget, void* data) {
    Fl_Text_Editor::kf_paste(0, editor);
}

void edit_undo_cb(Fl_Widget* widget, void* data) {
    Fl_Text_Editor::kf_undo(0, editor);
}

void edit_redo_cb(Fl_Widget* widget, void* data) {
    // В FLTK нет встроенной поддержки redo, поэтому просто обновляем состояние
    editor->redraw();
}


void edit_find_cb(Fl_Widget* widget, void* data) {
    // Создаем окно "Найти"
    if (!findReplaceWindow) { // Создаем только один раз
        findReplaceWindow = new Fl_Window(300, 150, "Find");
        findInput = new Fl_Input(10, 10, 200, 25, "Find:");
        findReplaceOKButton = new Fl_Button(10, 50, 100, 25, "OK");
        findReplaceOKButton->callback(find_replace_ok_cb);
        findReplaceCancelButton = new Fl_Button(120, 50, 100, 25, "Cancel");
        findReplaceCancelButton->callback(find_replace_cancel_cb);
        searchStatusBox = new Fl_Box(10, 90, 280, 20, ""); // Статус поиска
        findReplaceWindow->end();
    }
    findReplaceWindow->show();
}
void edit_replace_cb(Fl_Widget* widget, void* data) {
    // Создаем окно "Заменить" (аналогично "Найти", с добавлением поля "Заменить на")
    if (!findReplaceWindow) { // Создаем только один раз
        findReplaceWindow = new Fl_Window(300, 180, "Replace");
        findInput = new Fl_Input(10, 10, 200, 25, "Find:");
        replaceInput = new Fl_Input(10, 40, 200, 25, "Replace with:");
        findReplaceOKButton = new Fl_Button(10, 80, 100, 25, "OK");
        findReplaceOKButton->callback(find_replace_ok_cb);
        findReplaceCancelButton = new Fl_Button(120, 80, 100, 25, "Cancel");
        findReplaceCancelButton->callback(find_replace_cancel_cb);
        searchStatusBox = new Fl_Box(10, 110, 280, 20, ""); // Статус поиска
        findReplaceWindow->end();
    }
    findReplaceWindow->show();
}

void view_font_cb(Fl_Widget* widget, void* data) {
    if (!fontWindow) {
        fontWindow = new Fl_Window(300, 150, "Choose Font");
        fontChoice = new Fl_Choice(10, 10, 280, 25, "Font:");
        fontChoice->add("Helvetica", 0, 0, 0);
        fontChoice->add("Courier", 0, 0, 0);
        fontChoice->add("Times", 0, 0, 0);
        fontChoice->value(0);
        fontOKButton = new Fl_Button(10, 60, 100, 25, "OK");
        fontOKButton->callback(font_ok_cb);
        fontWindow->end();
    }
    fontWindow->show();
}

//-----------------------
// РЕАЛИЗАЦИЯ ВСПОМОГАТЕЛЬНЫХ ФУНКЦИЙ
//-----------------------
void font_ok_cb(Fl_Widget* widget, void* data) {
    defaultFont = fontChoice->value() == 0 ? FL_HELVETICA :
        fontChoice->value() == 1 ? FL_COURIER :
        FL_TIMES;
    editor->textfont(defaultFont);
    prefs.set("font", defaultFont); // Сохраняем настройку
    fontWindow->hide();
}

void find_replace_ok_cb(Fl_Widget* widget, void* data) {
    std::string findText = findInput->value();
    if (findText.empty()) {
        searchStatusBox->copy_label("Введите текст для поиска");
        return;
    }

    std::string replaceText = (replaceInput != nullptr) ? replaceInput->value() : "";
    if (textBuffer) {
        int start = editor->insert_position();
        int found = textBuffer->search_forward(start, findText.c_str(), &start);
        
        if (found == -1) {
            // Если не найдено после курсора, ищем с начала
            found = textBuffer->search_forward(0, findText.c_str(), &start);
        }

        if (found != -1) {
            textBuffer->select(start, start + findText.length());
            if (replaceInput != nullptr && !replaceText.empty()) {
                textBuffer->replace(start, start + findText.length(), replaceText.c_str());
            }
            searchStatusBox->copy_label("Текст найден");
        } else {
            searchStatusBox->copy_label("Текст не найден");
        }
    }
}

void find_replace_cancel_cb(Fl_Widget* widget, void* data) {
    findReplaceWindow->hide();
}
bool loadFile(const std::string& filePath) {
    try {
        std::ifstream file(filePath, std::ios::binary);
        if (!file.is_open()) {
            showError("Не удалось открыть файл: " + filePath);
            return false;
        }

        std::string content((std::istreambuf_iterator<char>(file)),
            std::istreambuf_iterator<char>());
        
        textBuffer->text(content.c_str());
        file.close();
        return true;
    } catch (const std::exception& e) {
        showError("Ошибка при чтении файла: " + std::string(e.what()));
        return false;
    }
}

bool saveFile(const std::string& filePath) {
    try {
        std::ofstream file(filePath, std::ios::binary);
        if (!file.is_open()) {
            showError("Не удалось сохранить файл: " + filePath);
            return false;
        }

        const char* text = textBuffer->text();
        file << text;
        file.close();
        return true;
    } catch (const std::exception& e) {
        showError("Ошибка при сохранении файла: " + std::string(e.what()));
        return false;
    }
}

void showError(const std::string& message) {
    fl_alert("%s", message.c_str());
}

void updateWindowTitle() {
    std::string title = "Simple Text Editor";
    if (!currentFilePath.empty()) {
        title += " - " + currentFilePath;
    }
    window->label(title.c_str());
}