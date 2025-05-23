#include "nlohmann/json.hpp"
#include "tasks_func.cpp"
#include "draw_func.cpp"
#include <SDL_image.h>
#include <SDL_ttf.h>
#include <iostream>
#include <string>
#include <cmath>
#include <ctime>    
#include <SDL.h>

using json = nlohmann::ordered_json;

int WINDOW_WIDTH = 800;
int WINDOW_HEIGHT = 600;

static void logSDLError(const std::string& msg) {
    //std::cerr << msg << " Error: " << SDL_GetError() << std::endl;
}
struct TextItem {
    SDL_Texture* texture;
    SDL_Rect rect;
    SDL_Texture* time;
};
struct description {
    SDL_Texture* texture;
    SDL_Rect rect;
    int DesTaskId;
};

static SDL_Texture* CreateTextTextureNW(SDL_Renderer* renderer, TTF_Font* font, const std::string& text, SDL_Color color, SDL_Rect& rect) {
    SDL_Surface* surface = TTF_RenderUTF8_Blended(font, text.c_str(), color);
    if (!surface) {
        SDL_Log("Ошибка рендеринга текста: %s", TTF_GetError());
        return nullptr;
    }

    SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, surface);
    if (!texture) {
        SDL_Log("Ошибка создания текстуры: %s", SDL_GetError());
    }

    // Устанавливаем размер текста
    rect.w = surface->w;
    rect.h = surface->h;

    SDL_FreeSurface(surface);
    return texture;
}

static SDL_Texture* CreateTextTexture(SDL_Renderer* renderer, TTF_Font* font, const std::string& text, SDL_Color color, SDL_Rect& rect, int len_) {
    SDL_Surface* surface = TTF_RenderUTF8_Blended_Wrapped(font, text.c_str(), color, len_);
    if (!surface) {
        SDL_Log("Ошибка рендеринга текста: %s", TTF_GetError());
        return nullptr;
    }

    SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, surface);
    if (!texture) {
        SDL_Log("Ошибка создания текстуры: %s", SDL_GetError());
    }

    // Устанавливаем размер текста
    rect.w = surface->w;
    rect.h = surface->h;

    SDL_FreeSurface(surface);
    return texture;
}

static SDL_HitTestResult HitTestCallback(SDL_Window* win, const SDL_Point* area, void* data) {
    int width, height;
    SDL_GetWindowSize(win, &width, &height);

    // Делаем верхнюю часть (30px) перетаскиваемой, кроме правых 130px (где может быть кнопка закрытия)
    if (area->x <= width - 130) {
        if ((area->x >= 250 && area->y < 30) or area->y < 10) return SDL_HITTEST_DRAGGABLE;
    }

    return SDL_HITTEST_NORMAL;
}
static SDL_HitTestResult HitTestCallback_cs(SDL_Window* win, const SDL_Point* area, void* data) {
    int width, height;
    SDL_GetWindowSize(win, &width, &height);
    if (area->x <= width - 50 and area->y < 50) return SDL_HITTEST_DRAGGABLE;
    return SDL_HITTEST_NORMAL;
}


static SDL_Color show_color_picker(SDL_Color selected_color) {
    SDL_Window* color_window = SDL_CreateWindow(
        "Color Picker",
        SDL_WINDOWPOS_CENTERED,
        SDL_WINDOWPOS_CENTERED,
        400, 300,
        SDL_WINDOW_BORDERLESS
    );
    TTF_Font* font = TTF_OpenFont("data/font.ttf", 24);

    draw draw;

    if (!color_window) {
        return selected_color;
    }
    SetRoundedRegion(color_window, 40, false);
    SDL_SetWindowHitTest(color_window, HitTestCallback_cs, nullptr);

    SDL_Renderer* renderer = SDL_CreateRenderer(color_window, -1, SDL_RENDERER_ACCELERATED);
    if (!renderer) {
        SDL_DestroyWindow(color_window);
        return selected_color;
    }

    // Initial color values
    int r = selected_color.r;
    int g = selected_color.g;
    int b = selected_color.b;

    SDL_Texture* r_val;
    SDL_Texture* g_val;
    SDL_Texture* b_val;
    SDL_Rect r_rect = {330, 140, 0, 0};
    SDL_Rect g_rect = { 330, 190, 0, 0 };
    SDL_Rect b_rect = { 330, 240, 0, 0 };

    SDL_Surface* surface = TTF_RenderUTF8_Blended(font, std::to_string(r).c_str(), { 245, 245, 245, 255 });
    r_val = SDL_CreateTextureFromSurface(renderer, surface);
    SDL_FreeSurface(surface);
    r_rect.w = surface->w;
    r_rect.h = surface->h;

    surface = TTF_RenderUTF8_Blended(font, std::to_string(g).c_str(), { 245, 245, 245, 255 });
    g_val = SDL_CreateTextureFromSurface(renderer, surface);
    SDL_FreeSurface(surface);
    g_rect.w = surface->w;
    g_rect.h = surface->h;

    surface = TTF_RenderUTF8_Blended(font, std::to_string(b).c_str(), { 245, 245, 245, 255 });
    b_val = SDL_CreateTextureFromSurface(renderer, surface);
    SDL_FreeSurface(surface);
    b_rect.w = surface->w;
    b_rect.h = surface->h;


    int last_r = 0;
    int last_g = 0;
    int last_b = 0;

    bool done = false;
    bool result = false;

    SDL_Event e;
    while (!done) {
        int mouseX, mouseY;
        Uint32 mouseState = SDL_GetMouseState(&mouseX, &mouseY);
        while (SDL_PollEvent(&e)) {

            if (e.type == SDL_KEYDOWN) {
                if (e.key.keysym.sym == SDLK_RETURN || e.key.keysym.sym == SDLK_KP_ENTER) {
                    // Save color and close
                    selected_color.r = r;
                    selected_color.g = g;
                    selected_color.b = b;
                    done = true;
                    result = true;
                }
                else if (e.key.keysym.sym == SDLK_ESCAPE) {
                    // Close without saving
                    done = true;
                }
            }
            else if (e.type == SDL_MOUSEMOTION) {
                // You could add mouse interaction for sliders here
            }
            else if (e.type == SDL_MOUSEBUTTONDOWN) {
                if (e.button.button == SDL_BUTTON_LEFT) {
                    if (mouseY < 50 and mouseX > 350 and mouseX < 400) {
                        done = true;
                    }
                }
            }
            else if (e.type == SDL_MOUSEWHEEL) {
                if (e.wheel.y > 0) {
                    if (20 <= mouseX and mouseX <= 295) {
                        if (130 <= mouseY and mouseY <= 180) r++;
                        if (180 <= mouseY and mouseY <= 230) g++;
                        if (230 <= mouseY and mouseY <= 280) b++;
                    }
                }
                else if (e.wheel.y < 0) {
                    if (20 <= mouseX and mouseX <= 295) {
                        if (130 <= mouseY and mouseY <= 180) r--;
                        if (180 <= mouseY and mouseY <= 230) g--;
                        if (230 <= mouseY and mouseY <= 280) b--;
                    }
                }
            }
        }

        if (mouseState & SDL_BUTTON(SDL_BUTTON_LEFT)) {
            if (30 <= mouseX and mouseX <= 285) {
                if (135 <= mouseY and mouseY <= 175) r = mouseX - 30;
                if (185 <= mouseY and mouseY <= 225) g = mouseX - 30;
                if (235 <= mouseY and mouseY <= 275) b = mouseX - 30;
            }
        }

        if (r > 255) r = 255;
        if (g > 255) g = 255;
        if (b > 255) b = 255;
        if (r < 0) r = 0;
        if (g < 0) g = 0;
        if (b < 0) b = 0;
        

        // Clear screen
        SDL_SetRenderDrawColor(renderer, 26, 25, 24, 255);
        SDL_RenderClear(renderer);

        SDL_SetRenderDrawColor(renderer, 234, 224, 200, 255);
        draw.DRRwB(renderer, {1,1,398,298}, 20);
        // Draw color preview
        SDL_SetRenderDrawColor(renderer, r, g, b, 255);
        draw.DrawRoundedRect(renderer, { 30, 30, 255, 90 }, 15);

        SDL_SetRenderDrawColor(renderer, 249, 122, 122, 255);
        draw.DRRwB(renderer, { 30, 140, 270, 30 }, 15);
        draw.DrawCircle(renderer, 37 + r, 154, 15);

        SDL_SetRenderDrawColor(renderer, 122, 249, 142, 255);
        draw.DRRwB(renderer, { 30, 190, 270, 30 }, 15);
        draw.DrawCircle(renderer, 37 + g, 204, 15);

        SDL_SetRenderDrawColor(renderer, 122, 173, 249, 255);
        draw.DRRwB(renderer, { 30, 240, 270, 30 }, 15);
        draw.DrawCircle(renderer, 37 + b, 254, 15);

        if (last_r != r) {
            last_r = r;
            surface = TTF_RenderUTF8_Blended(font, std::to_string(r).c_str(), {245, 245, 245, 255});
            r_val = SDL_CreateTextureFromSurface(renderer, surface);
            SDL_FreeSurface(surface);
            r_rect.w = surface->w;
            r_rect.h = surface->h;
        }
        if (last_g != g) {
            last_g = g;
            surface = TTF_RenderUTF8_Blended(font, std::to_string(g).c_str(), { 245, 245, 245, 255 });
            g_val = SDL_CreateTextureFromSurface(renderer, surface);
            SDL_FreeSurface(surface);
            g_rect.w = surface->w;
            g_rect.h = surface->h;
        }
        if (last_b != b) {
            last_b = b;
            surface = TTF_RenderUTF8_Blended(font, std::to_string(b).c_str(), { 245, 245, 245, 255 });
            b_val = SDL_CreateTextureFromSurface(renderer, surface);
            SDL_FreeSurface(surface);
            b_rect.w = surface->w;
            b_rect.h = surface->h;
        }
        SDL_RenderCopy(renderer, r_val, nullptr, &r_rect);
        SDL_RenderCopy(renderer, g_val, nullptr, &g_rect);
        SDL_RenderCopy(renderer, b_val, nullptr, &b_rect);


        SDL_SetRenderDrawColor(renderer, 128, 62, 60, 255);
        draw.DrawCircle(renderer, 375, 25, 15);

        SDL_Delay(16);
        SDL_RenderPresent(renderer);
    }

    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(color_window);

    selected_color.r = r;
    selected_color.g = g;
    selected_color.b = b;

    return selected_color;
}


int main(int argc, char* argv[]) {
    SetConsoleOutputCP(CP_UTF8);
    SetConsoleCP(CP_UTF8);
    std::string filename = "data/tasks.txt";
    std::vector<Task> tasks = loadTasks(filename);
    filename = "data/targets.txt";
    std::vector<Task> targets = loadTasks(filename);

    std::ifstream inFile("data/tags.json");
    json tagj;
    inFile >> tagj;
    inFile.close();

    std::ifstream inSettFile("data/settings.json");
    json settings;
    inSettFile >> settings;
    inSettFile.close();

    
    // Инициализация SDL2
    if (SDL_Init(SDL_INIT_VIDEO) != 0) {
        logSDLError("SDL_Init");
        return 1;
    }

    // Инициализация SDL_ttf
    if (TTF_Init() != 0) {
        logSDLError("TTF_Init");
        SDL_Quit();
        return 1;
    }

    // Создание окна
    SDL_Window* window = SDL_CreateWindow("2Task",
        SDL_WINDOWPOS_CENTERED,
        SDL_WINDOWPOS_CENTERED,
        WINDOW_WIDTH, WINDOW_HEIGHT,
        SDL_WINDOW_RESIZABLE);
    if (!window) {
        logSDLError("SDL_CreateWindow");
        TTF_Quit();
        SDL_Quit();
        return 1;
    }

    // Создание рендера
    SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
    if (!renderer) {
        logSDLError("SDL_CreateRenderer");
        SDL_DestroyWindow(window);
        TTF_Quit();
        SDL_Quit();
        return 1;
    }
    draw draw;

    // Загрузка шрифта
    TTF_Font* font = TTF_OpenFont("data/font.ttf", settings["font_size"]["sub"]);
    if (!font) {
        logSDLError("TTF_OpenFont");
        SDL_DestroyRenderer(renderer);
        SDL_DestroyWindow(window);
        TTF_Quit();
        SDL_Quit();
        return 1;
    }
    TTF_Font* lfont = TTF_OpenFont("data/lfont.ttf", settings["font_size"]["low"]);
    if (!lfont) {
        logSDLError("TTF_OpenFont");
        SDL_DestroyRenderer(renderer);
        SDL_DestroyWindow(window);
        TTF_Quit();
        SDL_Quit();
        return 1;
    }
    TTF_Font* lowfont = TTF_OpenFont("data/font.ttf", settings["font_size"]["general"]);
    if (!lowfont) {
        logSDLError("TTF_OpenFont");
        SDL_DestroyRenderer(renderer);
        SDL_DestroyWindow(window);
        TTF_Quit();
        SDL_Quit();
        return 1;
    }

    SDL_Surface* icon = IMG_Load("data/icon.png");
    if (icon != nullptr) {
        // Установка иконки для окна
        SDL_SetWindowIcon(window, icon);
        SDL_FreeSurface(icon);
    }
    /*  if (SDL_SetWindowHitTest(window, HitTestCallback, nullptr) != 0) {
          SDL_Log("Could not set hit test: %s", SDL_GetError());
          SDL_DestroyWindow(window);
          SDL_Quit();
          return 1;
      }*/

      // Основной цикл приложения
      //SDL_SetWindowResizable(window, SDL_TRUE);
    SDL_SetWindowMinimumSize(window, 420, 220);

    bool PlanTasks = 0;
    bool target_deep = 0;
    double sidebar_anim = 40.00;
    double last_sidebar_anim_ = -11.00;
    int start_index = 0;
    int targ_start_index = 0;
    bool sidebar_opened = false;
    bool running = true;
    bool isBordered = true;
    bool created_tt = false;
    int NearestTaskId = 0;
    bool TaskTimeChange = true;
    bool EditMenu = false;
    bool CreateNewTask = false;
    bool ChangeTag = false;
    bool static_img = false;
    Uint32 static_img_timer = 0;
    int static_posX = 0;
    int static_posY = 0;
    int EditItem_inTask = 0;
    Task EditNewTask_;
    EditNewTask_.tag = 0;
    int IndexEditTask = -1;
    bool NoSavedChanges = false;
    bool crutch = false;
    int TimeNearestTask = 0;
    int NearestTagrId = -1;
    int NearestSUBTagrId = -1;
    int CurrTagrId = 0; 
    start_index = getNearestTask(tasks);
    TimeNearestTask = start_index;
    if (TaskTimeChange == true) NearestTaskId = TimeNearestTask;
    std::vector<TextItem> textItems;
    description desc;
    desc.DesTaskId = -1;
    SDL_Event event;
    std::string text = "план      задачи";
    int choice = 0;
    Uint32 timer = 0;
    SDL_Color textColor = { Uint8(settings["font_color"]["default"][0]), Uint8(settings["font_color"]["default"][1]), Uint8(settings["font_color"]["default"][2]), 255};
    SDL_Texture* MainCanvasTxtr;
    SDL_Rect ContentTypeRect = {300, 20, 10, 10};
    SDL_Texture* ContentTypeTxtr = CreateTextTextureNW(renderer, lowfont, (std::string)"Название", textColor, ContentTypeRect);
    SDL_Rect MainCanvasRect;
    SDL_Rect MainCanvasDRAWRect; 
    SDL_Surface* surface = TTF_RenderUTF8_Blended(font, text.c_str(), { 108, 106, 102, 255 });
    SDL_Texture* PlanTasksChoise = SDL_CreateTextureFromSurface(renderer, surface);
    text = "назад"; //76 28
    surface = TTF_RenderUTF8_Blended(font, text.c_str(), { 108, 106, 102, 255 });
    SDL_Texture* BackTestButt = SDL_CreateTextureFromSurface(renderer, surface);
    SDL_FreeSurface(surface);
    text = "save";//87 27
    surface = TTF_RenderUTF8_Blended(font, text.c_str(), { 108, 106, 102, 255 });
    SDL_Texture* SaveChangesButt = SDL_CreateTextureFromSurface(renderer, surface);
    //int ii = surface->w;
    //std::cout << ii << std::endl;
    //ii = surface->h;
    //std::cout << ii << std::endl;
    SDL_FreeSurface(surface);
    text = "Здесь пока что ничего нет";
    MainCanvasTxtr = CreateTextTexture(renderer, lowfont, text.c_str(), textColor, MainCanvasRect, WINDOW_WIDTH - 80 - int(sidebar_anim));
    MainCanvasRect.x = int(sidebar_anim) + 40;
    MainCanvasRect.y = 40;

    while (running) {

        int mouseX, mouseY;
        SDL_GetMouseState(&mouseX, &mouseY);
        //std::cout << mouseX << " " << mouseY << std::endl;
        if (mouseX != static_posX and mouseY != static_posY) {
            static_posX = mouseX;
            static_posY = mouseY;
            static_img_timer = SDL_GetTicks();
            static_img = false;
        }
        else if (SDL_GetTicks() - static_img_timer > 5000 and static_img == false) static_img = true;
        // Обработка событий
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) {
                running = false;
            }
            else  if (event.type == SDL_WINDOWEVENT) {
                static_img_timer = SDL_GetTicks();
                static_img = false;
                last_sidebar_anim_ = -40.00;
                if (event.window.event == SDL_WINDOWEVENT_RESIZED) {
                    WINDOW_HEIGHT = event.window.data2;
                    WINDOW_WIDTH = event.window.data1;
                    created_tt = false;
                    for (auto& item : textItems) {
                        SDL_DestroyTexture(item.texture);
                        if (PlanTasks == 0)SDL_DestroyTexture(item.time);
                    }
                    textItems.clear();

                }
            }
            else if (event.type == SDL_MOUSEBUTTONDOWN) {
                //std::cout << mouseX << " - WINDOW_HEIGHT-" << WINDOW_HEIGHT-mouseY << std::endl;
                //std::cout << "WINDOW_WIDTH - " << WINDOW_WIDTH - mouseX << " " << mouseY << std::endl;
                if (event.button.button == SDL_BUTTON_LEFT) {
                    static_posX = mouseX;
                    static_posY = mouseY;
                    static_img_timer = SDL_GetTicks();
                    static_img = false;

                    if (mouseY < 35) {
                        if (WINDOW_WIDTH - 130 <= mouseX && mouseX <= WINDOW_WIDTH - 90) {
                            isBordered = !isBordered;
                            SDL_SetWindowBordered(window, isBordered ? SDL_TRUE : SDL_FALSE);
                            SetRoundedRegion(window, 80, isBordered);
                            SDL_SetWindowResizable(window, SDL_TRUE);

                            if (isBordered) SDL_SetWindowHitTest(window, nullptr, nullptr); // Отключаем HitTest
                            else SDL_SetWindowHitTest(window, HitTestCallback, nullptr);

                        }else if (WINDOW_WIDTH - 80 <= mouseX && mouseX <= WINDOW_WIDTH - 30) running = false;
                    }
                    if (mouseY > WINDOW_HEIGHT - 44 and 10 <= mouseX && mouseX <= 100 and NoSavedChanges) {
                        NoSavedChanges = false;
                        filename = "data/tasks.txt";
                        saveTasks(filename, tasks);
                        filename = "data/targets.txt";
                        saveTasks(filename, targets);
                        std::ofstream output_file("data/tags.json");
                        if (!output_file.is_open()) return 1;
                        output_file << tagj.dump(4);
                        output_file.close();
                    }
                    if (mouseX < sidebar_anim) {
                        if (!sidebar_opened) sidebar_opened = true;
                        if (sidebar_anim > 200) {
                            if (mouseY < 50) {
                                created_tt = false;
                                if (mouseX < 130) {
                                    PlanTasks = 0;
                                    for (auto& item : textItems) {
                                        SDL_DestroyTexture(item.texture);
                                    }
                                }
                                else {
                                    PlanTasks = 1; targ_start_index = 0;
                                    for (auto& item : textItems) {
                                        SDL_DestroyTexture(item.texture);
                                        //SDL_DestroyTexture(item.time);
                                    }
                                }
                                textItems.clear();
                            }
                            else if (mouseY > WINDOW_HEIGHT - 40 and sidebar_anim - 175 <= mouseX && mouseX <= sidebar_anim - 125 and EditMenu) {
                                CreateNewTask = true;
                                if (EditItem_inTask == 0) {
                                    last_sidebar_anim_ = -40.00;
                                    EditItem_inTask = 1;

                                    EditNewTask_.name = "...";
                                    EditNewTask_.description = "*****";

                                    text = EditNewTask_.name;
                                    ContentTypeTxtr = CreateTextTextureNW(renderer, lowfont, (std::string)"Название", textColor, ContentTypeRect);
                                }
                                else {
                                    CreateNewTask = false;
                                    EditItem_inTask = 0;
                                    last_sidebar_anim_ = -40.00;
                                }
                                if (PlanTasks == 0) {
                                    EditNewTask_.timeInMinutes = getCurrentTimeInMinutes();
                                    EditNewTask_.type = 0;
                                }
                                else if (target_deep == 0) {
                                    if (tasks[NearestTaskId].type != 0) {
                                        int index_offset = 0;
                                        int max_ind = 0;
                                        while (index_offset < targets.size()) {
                                            if (tasks[NearestTaskId].type == int(targets[index_offset].type / 10000) and targets[index_offset].type % 10000 > max_ind) max_ind = targets[index_offset].type % 10000;
                                            index_offset++;
                                        }
                                        max_ind += 1;
                                        EditNewTask_.timeInMinutes = 0;
                                        EditNewTask_.type = (tasks[NearestTaskId].type * 10000) + max_ind;
                                    }
                                    else {
                                        CreateNewTask = false;
                                        EditItem_inTask = 0;
                                        last_sidebar_anim_ = -40.00;
                                    }
                                }
                                else {
                                    int index_offset = 0;
                                    int max_ind = 0;
                                    while (index_offset < targets.size()) {
                                        if (targets[index_offset].type == targets[CurrTagrId].type and targets[index_offset].timeInMinutes > max_ind) max_ind = targets[index_offset].timeInMinutes;
                                        index_offset++;
                                    }
                                    max_ind += 1;
                                    EditNewTask_.timeInMinutes = max_ind;
                                    EditNewTask_.type = targets[CurrTagrId].type;
                                }
                            }
                            else if (mouseY > WINDOW_HEIGHT - 40 and sidebar_anim - 30 <= mouseX && mouseX <= sidebar_anim and EditMenu and PlanTasks) {
                                ChangeTag = true;
                                if (EditItem_inTask == 0) {
                                    last_sidebar_anim_ = -40.00;
                                    EditItem_inTask = 1;

                                    EditNewTask_.name = "0";
                                    EditNewTask_.description = "*****";

                                    text = EditNewTask_.name;
                                    ContentTypeTxtr = CreateTextTextureNW(renderer, lowfont, (std::string)"Номер тега", textColor, ContentTypeRect);
                                }
                                else {
                                    ChangeTag = false;
                                    EditItem_inTask = 0;
                                    last_sidebar_anim_ = -40.00;
                                }
                            }
                            else if (PlanTasks == 0) {
                                bool NewNearTask = true;
                                int draw_offset = 50;
                                int index_offset = 0;
                                while (NewNearTask) {
                                    if (draw_offset < WINDOW_HEIGHT - 90) {
                                        if (index_offset + start_index >= tasks.size()) break;
                                        if (draw_offset <= mouseY && mouseY <= draw_offset + 60) {

                                            if (EditMenu and mouseX < 80) {
                                                if (mouseX < 40) {
                                                    //std::cout << "Удаление задачи" << std::endl;
                                                    NoSavedChanges = true;
                                                    tasks.erase(tasks.begin() + start_index + index_offset);
                                                    TimeNearestTask = getNearestTask(tasks);
                                                    if (TaskTimeChange == true) NearestTaskId = TimeNearestTask;
                                                    for (auto& item : textItems) {
                                                        SDL_DestroyTexture(item.texture);
                                                        if (PlanTasks == 0)SDL_DestroyTexture(item.time);
                                                    } created_tt = false;
                                                }
                                                else {
                                                    //std::cout << "Изменение задачи" << std::endl;
                                                    if (EditItem_inTask == 0) {
                                                        IndexEditTask = start_index + index_offset;
                                                        last_sidebar_anim_ = -40.00;
                                                        EditItem_inTask = 1;
                                                        ContentTypeTxtr = CreateTextTextureNW(renderer, lowfont, (std::string)"Название", textColor, ContentTypeRect);

                                                        text = tasks[IndexEditTask].name;
                                                    }
                                                    else {
                                                        CreateNewTask = false;
                                                        EditItem_inTask = 0;
                                                        last_sidebar_anim_ = -40.00;
                                                    }
                                                }

                                            }
                                            else NearestTaskId = start_index + index_offset;
                                            target_deep = 0;
                                            targ_start_index = 0;
                                            NearestTagrId = -1;
                                            NewNearTask = false;
                                            TaskTimeChange = false;

                                            created_tt = false;
                                            for (auto& item : textItems) {
                                                SDL_DestroyTexture(item.texture);
                                                if (PlanTasks == 0)SDL_DestroyTexture(item.time);
                                            }
                                            textItems.clear();
                                        }
                                        draw_offset += 75;
                                        index_offset++;
                                    }
                                    else {
                                        NewNearTask = false;
                                        if (sidebar_opened) sidebar_opened = false;
                                    }
                                }
                                if (NearestTaskId == TimeNearestTask) TaskTimeChange = true;
                            }
                            else {
                                if (EditMenu and mouseX < 80 and NearestTagrId != -1) {
                                    if (mouseX < 80) {
                                        if (mouseX > 40) {
                                            if (EditItem_inTask == 0) {
                                                CreateNewTask = false;
                                                IndexEditTask = NearestTagrId;
                                                last_sidebar_anim_ = -40.00;
                                                EditItem_inTask = 1;
                                                ContentTypeTxtr = CreateTextTextureNW(renderer, lowfont, (std::string)"Название", textColor, ContentTypeRect);

                                                text = targets[IndexEditTask].name;
                                            }
                                            else {
                                                CreateNewTask = false;
                                                EditItem_inTask = 0;
                                                last_sidebar_anim_ = -40.00;
                                            }
                                        }
                                        else {
                                            NoSavedChanges = true;
                                            std::vector<int> deleting;
                                            if (target_deep == 0) {
                                                for (size_t i = 0; i < targets.size(); ++i) {
                                                    if (targets[i].type == targets[NearestTagrId].type) {
                                                        deleting.push_back(i);
                                                    }
                                                }
                                                std::reverse(deleting.begin(), deleting.end());
                                                for (size_t i = 0; i < deleting.size(); ++i) {
                                                    targets.erase(targets.begin() + deleting[i]);
                                                }
                                                deleting.clear();
                                                deleting.shrink_to_fit();

                                            }
                                            else targets.erase(targets.begin() + NearestTagrId);
                                            NearestSUBTagrId = -1;
                                            targ_start_index = 0;
                                            NearestTagrId = -1;
                                            for (auto& item : textItems) {
                                                SDL_DestroyTexture(item.texture);
                                                if (PlanTasks == 0)SDL_DestroyTexture(item.time);
                                            }
                                            textItems.clear();
                                            created_tt = false;
                                        }

                                    }
                                }
                                else if (target_deep == 0) {
                                    if (NearestTagrId > -1) {
                                        target_deep = 1;
                                        targ_start_index = 0;
                                        CurrTagrId = NearestTagrId;
                                        for (auto& item : textItems) {
                                            SDL_DestroyTexture(item.texture);
                                            if (PlanTasks == 0)SDL_DestroyTexture(item.time);
                                        }
                                        textItems.clear();
                                        created_tt = false;
                                    }
                                }
                                else {
                                    if (NearestTagrId > -1) {
                                        NearestSUBTagrId = NearestTagrId;
                                        last_sidebar_anim_ = 0;
                                    }
                                    if (mouseY > WINDOW_HEIGHT - 47 and 160 <= mouseX && mouseX <= 246) {
                                        target_deep = 0;
                                        targ_start_index = 0;
                                        NearestTagrId = -1;
                                        for (auto& item : textItems) {
                                            SDL_DestroyTexture(item.texture);
                                            if (PlanTasks == 0)SDL_DestroyTexture(item.time);
                                        }
                                        textItems.clear();
                                        created_tt = false;
                                    }
                                }

                                //int(sidebar_anim - 190), WINDOW_HEIGHT - 44, 86, 30
                            }
                        }
                    }
                    else if (sidebar_opened) sidebar_opened = false;
                }
                else if (event.button.button == SDL_BUTTON_RIGHT) {
                    if (sidebar_anim > 200 and mouseX < 270 and 50 <= mouseY) {
                        EditMenu = !EditMenu;
                    }
                }

            }
            else if (event.type == SDL_MOUSEWHEEL) {
                static_img_timer = SDL_GetTicks();
                static_img = false;
                if (mouseX < sidebar_anim) {
                    if (event.wheel.y > 0) {
                        if (PlanTasks == 0 and start_index > 0) start_index--;
                        if (PlanTasks == 1 and targ_start_index > 0) targ_start_index--;
                    }
                    else if (event.wheel.y < 0) {
                        if (PlanTasks == 0 and start_index + 1 < tasks.size() - (WINDOW_HEIGHT - 110) / 75) start_index++;
                        if (PlanTasks == 1) targ_start_index++;
                    }
                    //TaskTimeChange = false;
                    created_tt = false;
                    for (auto& item : textItems) {
                        SDL_DestroyTexture(item.texture);
                        if (PlanTasks == 0)SDL_DestroyTexture(item.time);
                    }
                    textItems.clear();
                    desc.DesTaskId = -1;
                }
            }
            if (EditItem_inTask != 0) {
                if (event.type == SDL_TEXTINPUT) {
                    if (!ChangeTag) {
                        static_img_timer = SDL_GetTicks();
                        static_img = false;
                        if (EditItem_inTask <= 2) {
                            text += event.text.text;
                        }
                        else {
                            char ch = event.text.text[0]; // Получаем введенный символ
                            if (ch >= '0' && ch <= '9') {
                                bool sum_this_ = false;
                                if (text == "") text = "0";
                                //std::cout << std::stoi(text) << "  -  " << (ch - '0') << std::endl;
                                if (PlanTasks) sum_this_ = true;
                                if (EditItem_inTask == 3 and std::stoi(text) * 10 + (ch - '0') < 7) sum_this_ = true;
                                if (EditItem_inTask == 4 and std::stoi(text) * 10 + (ch - '0') < 24) sum_this_ = true;
                                if (EditItem_inTask == 5 and std::stoi(text) * 10 + (ch - '0') < 60) sum_this_ = true;
                                if (EditItem_inTask == 6 and std::stoi(text) * 10 + (ch - '0') < 10) sum_this_ = true;
                                if (sum_this_) { if (text == "0") text = ch; else text += ch; }

                            }
                        }
                        last_sidebar_anim_ = -40.00;
                    }
                    else {
                        if (EditItem_inTask < 3) {
                            if (EditItem_inTask == 1) {
                                char ch = event.text.text[0]; // Получаем введенный символ
                                if (ch >= '0' && ch <= '9') {
                                    if (text == "0") text = ch;
                                    else text += ch;
                                }
                            }
                            else if (EditItem_inTask == 2) {
                                text += event.text.text;
                            }
                            last_sidebar_anim_ = -40.00;
                        }
                    }
                }
                else if (event.type == SDL_KEYDOWN) {
                    static_img_timer = SDL_GetTicks();
                    static_img = false;
                    if (event.key.keysym.sym == SDLK_BACKSPACE && !text.empty()) {
                        if (EditItem_inTask != 0) {
                            if (ChangeTag) {
                                if (EditItem_inTask < 3) {
                                    pop_back_utf8(text);
                                    last_sidebar_anim_ = -40.00;
                                }
                            }
                            else {
                                pop_back_utf8(text);
                                last_sidebar_anim_ = -40.00;
                            }
                        }
                    }
                    else if (event.key.keysym.sym == SDLK_RETURN) {
                        if (ChangeTag) {
                            if (text == "") {
                                if (EditItem_inTask == 0) text = "0";
                                else text = "Вы ничего не ввели";
                            }

                            if (EditItem_inTask == 1) {
                                EditNewTask_.name = text;
                                if (!tagj.contains(EditNewTask_.name)) tagj[EditNewTask_.name]["name"] = "*****";
                                text = tagj[EditNewTask_.name]["name"];
                                ContentTypeTxtr = CreateTextTextureNW(renderer, lowfont, (std::string)"Описание", textColor, ContentTypeRect);
                            }
                            else if (EditItem_inTask == 2) {
                                SDL_Color tag_color = { 128, 128, 128, 255 }; // Начальный цвет
                                /*if (tagj.contains(EditNewTask_.name)) {
                                    tag_color.r = Uint8(tagj[EditNewTask_.name]["color"][0]);
                                    tag_color.g = Uint8(tagj[EditNewTask_.name]["color"][1]);
                                    tag_color.b = Uint8(tagj[EditNewTask_.name]["color"][2]);
                                }*/
                                tagj[EditNewTask_.name]["name"] = text;
                                ContentTypeTxtr = CreateTextTextureNW(renderer, lowfont, (std::string)"Цвет тега", textColor, ContentTypeRect);
                                text = "Настройте цвет в открывшемся окне, а после нажмите Enter";
                                
                                SDL_SetRenderDrawColor(renderer, 32, 31, 30, 255);
                                draw.DrawRoundedRect(renderer, { int(sidebar_anim + 20), 20, WINDOW_WIDTH - 40 - int(sidebar_anim), WINDOW_HEIGHT - 40 }, 40);
                                SDL_SetRenderDrawColor(renderer, 234, 224, 200, 255);
                                draw.DRRwB(renderer, { int(sidebar_anim + 30), 30, WINDOW_WIDTH - 60 - int(sidebar_anim), WINDOW_HEIGHT - 60 }, 30);
                                SDL_DestroyTexture(MainCanvasTxtr);
                                MainCanvasTxtr = CreateTextTexture(renderer, lowfont, text.c_str(), textColor, MainCanvasRect, WINDOW_WIDTH - 100 - int(sidebar_anim));
                                MainCanvasRect.y = 50;
                                MainCanvasRect.x = int(sidebar_anim) + 50;

                                SDL_SetRenderDrawColor(renderer, 36, 35, 34, 255);
                                draw.DrawRoundedRect(renderer, { ContentTypeRect.x - 7, 18, ContentTypeRect.w + 14 , 30 }, 10);
                                SDL_SetRenderDrawColor(renderer, 100, 100, 100, 255);
                                draw.DRRwB(renderer, { ContentTypeRect.x - 7, 18, ContentTypeRect.w + 14 , 30 }, 10);
                                SDL_RenderCopy(renderer, ContentTypeTxtr, nullptr, &ContentTypeRect);

                                if (MainCanvasRect.h >= WINDOW_HEIGHT - 90) {
                                    MainCanvasDRAWRect = { 0, 0, MainCanvasRect.w, WINDOW_HEIGHT - 90 };
                                    MainCanvasRect.h = WINDOW_HEIGHT - 90;
                                    SDL_RenderCopy(renderer, MainCanvasTxtr, &MainCanvasDRAWRect, &MainCanvasRect);
                                }
                                else SDL_RenderCopy(renderer, MainCanvasTxtr, nullptr, &MainCanvasRect);

                                SDL_RenderPresent(renderer);
                                tag_color = show_color_picker(tag_color);
                                tagj[EditNewTask_.name]["color"][0] = tag_color.r;
                                tagj[EditNewTask_.name]["color"][1] = tag_color.g;
                                tagj[EditNewTask_.name]["color"][2] = tag_color.b;
                                
                            }
                            else if (EditItem_inTask == 3) {
                                SDL_Color tag_color = { 128, 128, 128, 255 }; // Начальный цвет
                                /*if (tagj.contains(EditNewTask_.name)) {
                                    tag_color.r = Uint8(tagj[EditNewTask_.name]["text_color"][0]);
                                    tag_color.g = Uint8(tagj[EditNewTask_.name]["text_color"][1]);
                                    tag_color.b = Uint8(tagj[EditNewTask_.name]["text_color"][2]);
                                }*/
                                ContentTypeTxtr = CreateTextTextureNW(renderer, lowfont, (std::string)"Цвет текста", textColor, ContentTypeRect);
                                text = "Настройте цвет текста в открывшемся окне, а после нажмите Enter";
                                
                                SDL_SetRenderDrawColor(renderer, 32, 31, 30, 255);
                                draw.DrawRoundedRect(renderer, { int(sidebar_anim + 20), 20, WINDOW_WIDTH - 40 - int(sidebar_anim), WINDOW_HEIGHT - 40 }, 40);
                                SDL_SetRenderDrawColor(renderer, 234, 224, 200, 255);
                                draw.DRRwB(renderer, { int(sidebar_anim + 30), 30, WINDOW_WIDTH - 60 - int(sidebar_anim), WINDOW_HEIGHT - 60 }, 30);
                                SDL_DestroyTexture(MainCanvasTxtr);
                                MainCanvasTxtr = CreateTextTexture(renderer, lowfont, text.c_str(), textColor, MainCanvasRect, WINDOW_WIDTH - 100 - int(sidebar_anim));
                                MainCanvasRect.y = 50;
                                MainCanvasRect.x = int(sidebar_anim) + 50;

                                SDL_SetRenderDrawColor(renderer, 36, 35, 34, 255);
                                draw.DrawRoundedRect(renderer, { ContentTypeRect.x - 7, 18, ContentTypeRect.w + 14 , 30 }, 10);
                                SDL_SetRenderDrawColor(renderer, 100, 100, 100, 255);
                                draw.DRRwB(renderer, { ContentTypeRect.x - 7, 18, ContentTypeRect.w + 14 , 30 }, 10);
                                SDL_RenderCopy(renderer, ContentTypeTxtr, nullptr, &ContentTypeRect);

                                if (MainCanvasRect.h >= WINDOW_HEIGHT - 90) {
                                    MainCanvasDRAWRect = { 0, 0, MainCanvasRect.w, WINDOW_HEIGHT - 90 };
                                    MainCanvasRect.h = WINDOW_HEIGHT - 90;
                                    SDL_RenderCopy(renderer, MainCanvasTxtr, &MainCanvasDRAWRect, &MainCanvasRect);
                                }
                                else SDL_RenderCopy(renderer, MainCanvasTxtr, nullptr, &MainCanvasRect);

                                SDL_RenderPresent(renderer);
                                tag_color = show_color_picker(tag_color);
                                tagj[EditNewTask_.name]["text_color"][0] = tag_color.r;
                                tagj[EditNewTask_.name]["text_color"][1] = tag_color.g;
                                tagj[EditNewTask_.name]["text_color"][2] = tag_color.b;
                            }
                            EditItem_inTask++;
                            last_sidebar_anim_ = -40.00;
                            if (EditItem_inTask > 3) {
                                EditItem_inTask = 0;
                                ChangeTag = false;
                                NoSavedChanges = true;
                                last_sidebar_anim_ = -40.00;
                                for (auto& item : textItems) {
                                    SDL_DestroyTexture(item.texture);
                                    if (PlanTasks == 0)SDL_DestroyTexture(item.time);
                                }
                                textItems.clear();
                                created_tt = false;
                            }
                        }
                        else {

                            if (text == "") {
                                if (EditItem_inTask > 2) text = "0";
                                else text = "Вы ничего не ввели";
                            }

                            if (CreateNewTask == false) {
                                if (PlanTasks == 0) {
                                    if (EditItem_inTask == 1) {
                                        tasks[IndexEditTask].name = text; text = tasks[IndexEditTask].description; ContentTypeTxtr = CreateTextTextureNW(renderer, lowfont, (std::string)"Описание", textColor, ContentTypeRect);
                                    }
                                    else if (EditItem_inTask == 2) {
                                        tasks[IndexEditTask].description = text; text = std::to_string(int(floor(tasks[IndexEditTask].timeInMinutes / 1440))); ContentTypeTxtr = CreateTextTextureNW(renderer, lowfont, (std::string)"День недели", textColor, ContentTypeRect);
                                    }

                                    else if (EditItem_inTask == 3) {
                                        tasks[IndexEditTask].timeInMinutes = (std::stoi(text) * 1440) + (tasks[IndexEditTask].timeInMinutes % 1440);
                                        text = std::to_string(int(floor((tasks[IndexEditTask].timeInMinutes % 1440) / 60)));
                                        ContentTypeTxtr = CreateTextTextureNW(renderer, lowfont, (std::string)"Часы", textColor, ContentTypeRect);
                                    }
                                    else if (EditItem_inTask == 4) {
                                        tasks[IndexEditTask].timeInMinutes = tasks[IndexEditTask].timeInMinutes - tasks[IndexEditTask].timeInMinutes % 1440 + (std::stoi(text) * 60) + tasks[IndexEditTask].timeInMinutes % 60;
                                        text = std::to_string(tasks[IndexEditTask].timeInMinutes % 60);
                                        ContentTypeTxtr = CreateTextTextureNW(renderer, lowfont, (std::string)"Минуты", textColor, ContentTypeRect);
                                    }
                                    else if (EditItem_inTask == 5) {
                                        tasks[IndexEditTask].timeInMinutes = tasks[IndexEditTask].timeInMinutes - tasks[IndexEditTask].timeInMinutes % 60 + std::stoi(text);  text = std::to_string(tasks[IndexEditTask].type); ContentTypeTxtr = CreateTextTextureNW(renderer, lowfont, (std::string)"Тип", textColor, ContentTypeRect);
                                    }


                                    else if (EditItem_inTask == 6) { tasks[IndexEditTask].type = std::stoi(text); }
                                    EditItem_inTask++;
                                    if (EditItem_inTask > 6) {
                                        NoSavedChanges = true;
                                        EditItem_inTask = 0;
                                        EditMenu = false;
                                        EditNewTask_ = tasks[IndexEditTask];
                                        tasks.erase(tasks.begin() + IndexEditTask);

                                        bool exit_ = false;
                                        for (size_t i = 0; i < tasks.size() - 1; ++i) {
                                            if (EditNewTask_.timeInMinutes > tasks[i].timeInMinutes and EditNewTask_.timeInMinutes <= tasks[i + 1].timeInMinutes) {
                                                tasks.insert(tasks.begin() + i + 1, EditNewTask_);
                                                exit_ = true;
                                                break;
                                            }
                                        }
                                        if (exit_ == false) tasks.push_back(EditNewTask_);
                                        TimeNearestTask = getNearestTask(tasks);
                                    }
                                    last_sidebar_anim_ = -40.00;
                                    for (auto& item : textItems) {
                                        SDL_DestroyTexture(item.texture);
                                        if (PlanTasks == 0)SDL_DestroyTexture(item.time);
                                    }
                                    textItems.clear();
                                    created_tt = false;
                                }
                                else {
                                    //if (target_deep == 0) {
                                    if (EditItem_inTask == 1) { targets[IndexEditTask].name = text; text = targets[IndexEditTask].description; ContentTypeTxtr = CreateTextTextureNW(renderer, lowfont, (std::string)"Описание", textColor, ContentTypeRect); }
                                    else if (EditItem_inTask == 2) { targets[IndexEditTask].description = text; text = std::to_string(targets[IndexEditTask].tag); ContentTypeTxtr = CreateTextTextureNW(renderer, lowfont, (std::string)"Номер тега", textColor, ContentTypeRect); }
                                    else if (EditItem_inTask == 3) targets[IndexEditTask].tag = std::stoi(text);
                                    EditItem_inTask++;
                                    if (EditItem_inTask > 3) {
                                        NoSavedChanges = true;
                                        EditItem_inTask = 0;
                                        EditMenu = false;
                                    }
                                    last_sidebar_anim_ = -40.00;
                                    for (auto& item : textItems) {
                                        SDL_DestroyTexture(item.texture);
                                        if (PlanTasks == 0)SDL_DestroyTexture(item.time);
                                    }
                                    textItems.clear();
                                    created_tt = false;

                                    //}
                                }

                            }
                            else {
                                if (PlanTasks == 0) {
                                    if (EditItem_inTask == 1) { EditNewTask_.name = text; text = EditNewTask_.description; ContentTypeTxtr = CreateTextTextureNW(renderer, lowfont, (std::string)"Описание", textColor, ContentTypeRect); }
                                    else if (EditItem_inTask == 2) { EditNewTask_.description = text; text = std::to_string(int(floor(EditNewTask_.timeInMinutes / 1440))); ContentTypeTxtr = CreateTextTextureNW(renderer, lowfont, (std::string)"День недели", textColor, ContentTypeRect); }

                                    else if (EditItem_inTask == 3) {
                                        EditNewTask_.timeInMinutes = (std::stoi(text) * 1440) + (EditNewTask_.timeInMinutes % 1440);
                                        text = std::to_string(int(floor((EditNewTask_.timeInMinutes % 1440) / 60)));
                                        ContentTypeTxtr = CreateTextTextureNW(renderer, lowfont, (std::string)"Часы", textColor, ContentTypeRect);
                                    }
                                    else if (EditItem_inTask == 4) {
                                        EditNewTask_.timeInMinutes = EditNewTask_.timeInMinutes - EditNewTask_.timeInMinutes % 1440 + (std::stoi(text) * 60) + EditNewTask_.timeInMinutes % 60;
                                        text = std::to_string(EditNewTask_.timeInMinutes % 60);
                                        ContentTypeTxtr = CreateTextTextureNW(renderer, lowfont, (std::string)"Минуты", textColor, ContentTypeRect);
                                    }
                                    else if (EditItem_inTask == 5) { EditNewTask_.timeInMinutes = EditNewTask_.timeInMinutes - EditNewTask_.timeInMinutes % 60 + std::stoi(text); text = std::to_string(EditNewTask_.type); ContentTypeTxtr = CreateTextTextureNW(renderer, lowfont, (std::string)"Тип", textColor, ContentTypeRect); }


                                    else if (EditItem_inTask == 6) { EditNewTask_.type = std::stoi(text); }
                                    EditItem_inTask++;
                                    if (EditItem_inTask > 6) {
                                        NoSavedChanges = true;
                                        EditItem_inTask = 0;
                                        EditMenu = false;
                                        CreateNewTask = false;

                                        bool exit_ = false;
                                        for (size_t i = 0; i < tasks.size() - 1; ++i) {
                                            if (EditNewTask_.timeInMinutes > tasks[i].timeInMinutes and EditNewTask_.timeInMinutes <= tasks[i + 1].timeInMinutes) {
                                                tasks.insert(tasks.begin() + i + 1, EditNewTask_);
                                                exit_ = true;
                                                break;
                                            }
                                        }
                                        if (exit_ == false) tasks.push_back(EditNewTask_);
                                        TimeNearestTask = getNearestTask(tasks);
                                        if (TaskTimeChange == true) NearestTaskId = TimeNearestTask;
                                    }
                                    last_sidebar_anim_ = -40.00;
                                    for (auto& item : textItems) {
                                        SDL_DestroyTexture(item.texture);
                                        if (PlanTasks == 0)SDL_DestroyTexture(item.time);
                                    }
                                    textItems.clear();
                                    created_tt = false;
                                }
                                else {

                                    if (EditItem_inTask == 1) { EditNewTask_.name = text; text = EditNewTask_.description; ContentTypeTxtr = CreateTextTextureNW(renderer, lowfont, (std::string)"Описание", textColor, ContentTypeRect); }
                                    else if (EditItem_inTask == 2) { EditNewTask_.description = text; text = std::to_string(EditNewTask_.tag); ContentTypeTxtr = CreateTextTextureNW(renderer, lowfont, (std::string)"Номер тега", textColor, ContentTypeRect); }
                                    else if (EditItem_inTask == 3) EditNewTask_.tag = std::stoi(text);
                                    EditItem_inTask++;
                                    if (EditItem_inTask > 3) {
                                        EditItem_inTask = 0;
                                        EditMenu = false;
                                        CreateNewTask = false;
                                        for (auto& item : textItems) {
                                            SDL_DestroyTexture(item.texture);
                                        }
                                        textItems.clear();
                                        created_tt = false;
                                        NoSavedChanges = true;
                                        targets.push_back(EditNewTask_);
                                    }
                                    last_sidebar_anim_ = -40.00;

                                }

                            }
                        }
                    }
                }
            }


        }

        if (sidebar_opened == false) {
            if (mouseX < 40 && 20 <= mouseY && mouseY <= WINDOW_HEIGHT - 20 and !(mouseY > WINDOW_HEIGHT - 44 and NoSavedChanges)) sidebar_anim += (80 - sidebar_anim) * 0.18;
            else sidebar_anim += (15 - sidebar_anim) * 0.22;
        }
        else sidebar_anim += (280 - sidebar_anim) * 0.18;
        // Очистка экрана
        SDL_SetRenderDrawColor(renderer, 26, 25, 24, 255);
        SDL_RenderClear(renderer);

        SDL_SetRenderDrawColor(renderer, 64, 62, 60, 255);
        draw.DrawRoundedRect(renderer, { WINDOW_WIDTH - 130, 9, 40, 10 }, 4);

        if (!isBordered) {
            SDL_SetRenderDrawColor(renderer, 128, 62, 60, 255);
            draw.DrawRoundedRect(renderer, { WINDOW_WIDTH - 80, 9, 40, 10 }, 4);
        }

        if (SDL_GetTicks() - timer >= 45000) {
            //std::cout << "Прошла минута, сообщение выводится!" << std::endl;
            TimeNearestTask = getNearestTask(tasks);
            if (TaskTimeChange == true) NearestTaskId = TimeNearestTask;
            timer = SDL_GetTicks();
            for (auto& item : textItems) {
                SDL_DestroyTexture(item.texture);
                if (PlanTasks == 0)SDL_DestroyTexture(item.time);
            }
            created_tt = false;
            textItems.shrink_to_fit();
            std::vector<TextItem>().swap(textItems);
        }

        SDL_SetRenderDrawColor(renderer, 32, 31, 30, 255);
        draw.DrawSidebar(renderer, { 0, 20, int(sidebar_anim), WINDOW_HEIGHT - 40 }, 40);
        draw.DrawRoundedRect(renderer, { int(sidebar_anim + 20), 20, WINDOW_WIDTH - 40 - int(sidebar_anim), WINDOW_HEIGHT - 40 }, 40);

        SDL_SetRenderDrawColor(renderer, 148, 70, 76, 255);
        draw.DrawSidebarB(renderer, { 0, 30, int(sidebar_anim - 10), WINDOW_HEIGHT - 60 }, 30);

        SDL_SetRenderDrawColor(renderer, 234, 224, 200, 255);
        draw.DRRwB(renderer, { int(sidebar_anim + 30), 30, WINDOW_WIDTH - 60 - int(sidebar_anim), WINDOW_HEIGHT - 60 }, 30);


        SDL_SetRenderDrawColor(renderer, 36, 35, 34, 255);
        draw.DrawRoundedRect(renderer, { int(sidebar_anim - 270), 10, 230, 30 }, 10);
        SDL_SetRenderDrawColor(renderer, 84, 81, 78, 255);
        if (!PlanTasks) draw.DRRwB(renderer, { int(sidebar_anim - 270), 10, 110, 30}, 10);
        else draw.DRRwB(renderer, { int(sidebar_anim - 150), 10, 110, 30 }, 10);
        SDL_Rect asd = { int(sidebar_anim - 245), 9, 194, 28 };
        SDL_RenderCopy(renderer, PlanTasksChoise, nullptr, &asd);


        //ОТРИСОВКА ТЕКСТА ПОДЗАДАЧИ
        if (fabs(last_sidebar_anim_ - sidebar_anim) >= 30) {
            SDL_DestroyTexture(MainCanvasTxtr);
            if (EditItem_inTask == 0) {
                if (NearestSUBTagrId != -1) MainCanvasTxtr = CreateTextTexture(renderer, lowfont, targets[NearestSUBTagrId].description.c_str(), textColor, MainCanvasRect, WINDOW_WIDTH - 100 - int(sidebar_anim));
                else {
                    text = "Здесь пока что ничего нет";
                    MainCanvasTxtr = CreateTextTexture(renderer, lowfont, text.c_str(), textColor, MainCanvasRect, WINDOW_WIDTH - 100 - int(sidebar_anim));
                }
            }
            else MainCanvasTxtr = CreateTextTexture(renderer, lowfont, text.c_str(), textColor, MainCanvasRect, WINDOW_WIDTH - 100 - int(sidebar_anim));

            last_sidebar_anim_ = sidebar_anim;
            MainCanvasRect.y = 50;
        }
        MainCanvasRect.x = int(sidebar_anim) + 50;
        //MainCanvasRect.w = WINDOW_WIDTH - 90 - int(sidebar_anim);
        if (MainCanvasRect.h >= WINDOW_HEIGHT - 90) {
            MainCanvasDRAWRect = { 0, 0, MainCanvasRect.w, WINDOW_HEIGHT - 90 };
            MainCanvasRect.h = WINDOW_HEIGHT - 90;
            SDL_RenderCopy(renderer, MainCanvasTxtr, &MainCanvasDRAWRect, &MainCanvasRect);
        }
        else SDL_RenderCopy(renderer, MainCanvasTxtr, nullptr, &MainCanvasRect);

        ContentTypeRect.x = int(sidebar_anim) + 50;
        if (EditItem_inTask != 0) {
            SDL_SetRenderDrawColor(renderer, 36, 35, 34, 255);
            draw.DrawRoundedRect(renderer, { ContentTypeRect.x - 7, 18, ContentTypeRect.w + 14 , 30 }, 10);
            SDL_SetRenderDrawColor(renderer, 100, 100, 100, 255);
            draw.DRRwB(renderer, { ContentTypeRect.x - 7, 18, ContentTypeRect.w + 14 , 30 }, 10);
            SDL_RenderCopy(renderer, ContentTypeTxtr, nullptr, &ContentTypeRect);
        } //ТУТА КАНЕЦ



        if (PlanTasks == 0) { // -=-=-=-=-=-=-=-=-=-=-=-=-==-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-  PlanTasks == 0  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=--
            SDL_SetRenderDrawColor(renderer, 84, 81, 78, 255);
            bool draw_tasks = true;
            int draw_offset = 50;
            int index_offset = 0;
            bool DescDraw = false;
            NearestTagrId = -1;
            while (draw_tasks) {
                if (draw_offset < WINDOW_HEIGHT - 90) {
                    if (index_offset + start_index >= tasks.size()) break;
                    if (created_tt == false) {
                        TextItem item;
                        item.texture = CreateTextTexture(renderer, lowfont, tasks[start_index + index_offset].name.c_str(), textColor, item.rect, 150);
                        item.rect.x = 0;
                        item.rect.y = draw_offset + 28;
                        SDL_Surface* surface = TTF_RenderUTF8_Blended(font, convertToHHMM(tasks[start_index + index_offset].timeInMinutes).c_str(), { 128, 111, 111, 255 });
                        //int ii = surface->h;
                        item.time = SDL_CreateTextureFromSurface(renderer, surface);
                        //std::cout << ii << std::endl;
                        SDL_FreeSurface(surface);

                        if (item.texture) {
                            textItems.push_back(item);
                        }
                    }

                    if (start_index + index_offset == TimeNearestTask) SDL_SetRenderDrawColor(renderer, 138, 114, 99, 255);
                    if (start_index + index_offset == NearestTaskId) {
                        SDL_SetRenderDrawColor(renderer, 228, 228, 228, 255);
                        if (TaskTimeChange) SDL_SetRenderDrawColor(renderer, 148, 70, 76, 255);
                    }
                    draw.DrawSidebarB(renderer, { 0, draw_offset, int(sidebar_anim - 30), 60 }, 20);
                    SDL_Rect text_rect = { textItems[index_offset].rect.x + int(sidebar_anim) - 195, textItems[index_offset].rect.y - textItems[index_offset].rect.h / 2, textItems[index_offset].rect.w,  textItems[index_offset].rect.h };
                    SDL_Rect time_rect = { textItems[index_offset].rect.x + int(sidebar_anim) - 270, textItems[index_offset].rect.y - 12, 62,  27 };
                    SDL_RenderCopy(renderer, textItems[index_offset].texture, nullptr, &text_rect);
                    SDL_RenderCopy(renderer, textItems[index_offset].time, nullptr, &time_rect);
                    


                    if (draw_offset <= mouseY && mouseY <= draw_offset + 60 and mouseX < sidebar_anim and sidebar_opened) {

                        NearestTagrId = start_index + index_offset;
                        if (EditMenu) {
                            SDL_SetRenderDrawColor(renderer, 31, 30, 29, 255);
                            draw.DrawRoundedRect(renderer, { int(sidebar_anim - 275), draw_offset + 10, 70, 40 }, 20);
                            SDL_SetRenderDrawColor(renderer, 100, 100, 100, 255);
                            draw.DRRwB(renderer, { int(sidebar_anim - 275), draw_offset + 10, 70, 40 }, 20);

                            SDL_SetRenderDrawColor(renderer, 180, 110, 100, 255);
                            draw.DrawCircle(renderer, int(sidebar_anim - 255), draw_offset + 29, 12);
                            SDL_SetRenderDrawColor(renderer, 180, 175, 100, 255);
                            draw.DrawCircle(renderer, int(sidebar_anim - 227), draw_offset + 29, 12);

                        }

                        if (desc.DesTaskId != index_offset) {
                            if (desc.DesTaskId >= 0) SDL_DestroyTexture(desc.texture);
                            SDL_Surface* surface = TTF_RenderUTF8_Blended_Wrapped(lowfont, tasks[start_index + index_offset].description.c_str(), { 190, 190, 190, 255 }, 180);
                            desc.texture = SDL_CreateTextureFromSurface(renderer, surface);

                            // Устанавливаем размер текста
                            desc.rect.w = surface->w;
                            desc.rect.h = surface->h;
                            desc.rect.x = mouseX;
                            desc.rect.y = draw_offset + 50;

                            SDL_FreeSurface(surface);
                            desc.DesTaskId = index_offset;
                        }
                        else if (EditMenu == false) DescDraw = true;
                    }

                    SDL_SetRenderDrawColor(renderer, 84, 81, 78, 255);

                    index_offset++;
                    draw_offset += 75;
                }

                else draw_tasks = false;
            }
            if (DescDraw) {
                SDL_SetRenderDrawColor(renderer, 28, 27, 26, 255);
                draw.DrawRoundedRect(renderer, { desc.rect.x - 10, desc.rect.y - 10, desc.rect.w + 20, desc.rect.h + 20 }, 20);
                SDL_SetRenderDrawColor(renderer, 100, 100, 100, 255);
                draw.DRRwB(renderer, { desc.rect.x - 10, desc.rect.y - 10, desc.rect.w + 20, desc.rect.h + 20 }, 20);
                SDL_RenderCopy(renderer, desc.texture, nullptr, &desc.rect);
                SDL_SetRenderDrawColor(renderer, 84, 81, 78, 255);
            }
            if (created_tt == false) created_tt = true;
        }
        else { // -=-=-=-=-=-=-=-=-=-=-=-=-==-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-  PlanTasks == 1  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=--
            SDL_SetRenderDrawColor(renderer, 84, 81, 78, 255);
            bool draw_tasks = true;
            int draw_offset = 50;
            int index_offset = 0;
            int index_offset_unr = 0;
            NearestTagrId = -1;
            //TargFullSc = false;
            draw_offset = 50 - 75 * targ_start_index;
            if (target_deep == 0) {
                while (draw_tasks) {
                    if (draw_offset < WINDOW_HEIGHT - 90) {
                        if (index_offset >= targets.size()) break;
                        if (tasks[NearestTaskId].type != 0 and int(targets[index_offset].type / 10000) == tasks[NearestTaskId].type) {

                            if (targets[index_offset].timeInMinutes == 0) {
                                if (draw_offset >= 50) {
                                    std::string tag_key = std::to_string(targets[index_offset].tag);

                                    if (created_tt == false) {
                                        TextItem item;
                                        if (tagj.contains(tag_key)) item.texture = CreateTextTexture(renderer, lowfont, targets[index_offset].name.c_str(), { Uint8(tagj[tag_key]["text_color"][0]), Uint8(tagj[tag_key]["text_color"][1]), Uint8(tagj[tag_key]["text_color"][2]), 255 }, item.rect, 210);
                                        else item.texture = CreateTextTexture(renderer, lowfont, targets[index_offset].name.c_str(), textColor, item.rect, 210);
                                        item.rect.x = 0;
                                        item.rect.y = draw_offset + 28;

                                        if (item.texture) {
                                            textItems.push_back(item);
                                        }
                                    }


                                    if (tagj.contains(tag_key)) SDL_SetRenderDrawColor(renderer, Uint8(tagj[tag_key]["color"][0]), Uint8(tagj[tag_key]["color"][1]), Uint8(tagj[tag_key]["color"][2]), 255);
                                    else SDL_SetRenderDrawColor(renderer, 84, 81, 78, 255);

                                    draw.DrawSidebarB(renderer, { 0, draw_offset, int(sidebar_anim - 30), 60 }, 20);
                                    //SDL_SetRenderDrawColor(renderer, 84, 81, 78, 255);
                                    SDL_Rect text_rect;
                                    if (settings["centered"]["deep0"]) text_rect = { textItems[index_offset_unr].rect.x + int(sidebar_anim) - 160 - textItems[index_offset_unr].rect.w/2, textItems[index_offset_unr].rect.y - textItems[index_offset_unr].rect.h / 2, textItems[index_offset_unr].rect.w,  textItems[index_offset_unr].rect.h };
                                    else text_rect = { int(sidebar_anim) - 265, textItems[index_offset_unr].rect.y - textItems[index_offset_unr].rect.h / 2, textItems[index_offset_unr].rect.w,  textItems[index_offset_unr].rect.h };
                                    SDL_RenderCopy(renderer, textItems[index_offset_unr].texture, nullptr, &text_rect);

                                    if (draw_offset <= mouseY && mouseY <= draw_offset + 60 and mouseX < sidebar_anim and sidebar_opened) {
                                        NearestTagrId = index_offset;
                                        if (EditMenu) {
                                            SDL_SetRenderDrawColor(renderer, 31, 30, 29, 255);
                                            draw.DrawRoundedRect(renderer, { int(sidebar_anim - 275), draw_offset + 10, 70, 40 }, 20);
                                            SDL_SetRenderDrawColor(renderer, 100, 100, 100, 255);
                                            draw.DRRwB(renderer, { int(sidebar_anim - 275), draw_offset + 10, 70, 40 }, 20);

                                            SDL_SetRenderDrawColor(renderer, 180, 110, 100, 255);
                                            draw.DrawCircle(renderer, int(sidebar_anim - 255), draw_offset + 29, 12);
                                            SDL_SetRenderDrawColor(renderer, 180, 175, 100, 255);
                                            draw.DrawCircle(renderer, int(sidebar_anim - 227), draw_offset + 29, 12);
                                            SDL_SetRenderDrawColor(renderer, 84, 81, 78, 255);

                                        }
                                    }

                                    index_offset_unr++;
                                }
                                draw_offset += 75;

                            }
                        }
                        index_offset++;
                    }

                    else draw_tasks = false;
                }
                //if (draw_offset < WINDOW_HEIGHT - 90 - 75) TargFullSc = true;
                if (created_tt == false) created_tt = true;
            }
            else {
                while (draw_tasks) {
                    if (draw_offset < WINDOW_HEIGHT - 90) {
                        if (index_offset >= targets.size()) break;
                        if (targets[index_offset].type == targets[CurrTagrId].type and targets[index_offset].timeInMinutes != 0)
                        {
                            if (draw_offset >= 50) {
                                std::string tag_key = std::to_string(targets[index_offset].tag);
                                if (created_tt == false) {
                                    TextItem item;
                                    if (tagj.contains(tag_key)) item.texture = CreateTextTexture(renderer, lowfont, targets[index_offset].name.c_str(), { Uint8(tagj[tag_key]["text_color"][0]), Uint8(tagj[tag_key]["text_color"][1]), Uint8(tagj[tag_key]["text_color"][2]), 255 }, item.rect, 210);
                                    else item.texture = CreateTextTexture(renderer, lowfont, targets[index_offset].name.c_str(), textColor, item.rect, 210);
                                    item.rect.x = item.rect.w / 2;
                                    item.rect.y = draw_offset + 28;

                                    if (item.texture) {
                                        textItems.push_back(item);
                                    }
                                }

                                if (tagj.contains(tag_key)) SDL_SetRenderDrawColor(renderer, Uint8(tagj[tag_key]["color"][0]), Uint8(tagj[tag_key]["color"][1]), Uint8(tagj[tag_key]["color"][2]), 255);
                                else SDL_SetRenderDrawColor(renderer, 84, 81, 78, 255);
                                draw.DrawSidebarB(renderer, { 0, draw_offset, int(sidebar_anim - 30), 60 }, 20);
                                //SDL_SetRenderDrawColor(renderer, 84, 81, 78, 255);
                                SDL_Rect text_rect;
                                if (settings["centered"]["deep1"]) text_rect = { int(sidebar_anim) - 160 - textItems[index_offset_unr].rect.x, textItems[index_offset_unr].rect.y - textItems[index_offset_unr].rect.h / 2, textItems[index_offset_unr].rect.w,  textItems[index_offset_unr].rect.h };
                                else text_rect = { int(sidebar_anim) - 265, textItems[index_offset_unr].rect.y - textItems[index_offset_unr].rect.h / 2, textItems[index_offset_unr].rect.w,  textItems[index_offset_unr].rect.h };
                                SDL_RenderCopy(renderer, textItems[index_offset_unr].texture, nullptr, &text_rect);
                                if (draw_offset <= mouseY && mouseY <= draw_offset + 60 and mouseX < sidebar_anim and sidebar_opened) {
                                    NearestTagrId = index_offset;
                                    if (EditMenu) {
                                        SDL_SetRenderDrawColor(renderer, 31, 30, 29, 255);
                                        draw.DrawRoundedRect(renderer, { int(sidebar_anim - 275), draw_offset + 10, 70, 40 }, 20);
                                        SDL_SetRenderDrawColor(renderer, 100, 100, 100, 255);
                                        draw.DRRwB(renderer, { int(sidebar_anim - 275), draw_offset + 10, 70, 40 }, 20);

                                        SDL_SetRenderDrawColor(renderer, 180, 110, 100, 255);
                                        draw.DrawCircle(renderer, int(sidebar_anim - 255), draw_offset + 29, 12);
                                        SDL_SetRenderDrawColor(renderer, 180, 175, 100, 255);
                                        draw.DrawCircle(renderer, int(sidebar_anim - 227), draw_offset + 29, 12);
                                        SDL_SetRenderDrawColor(renderer, 84, 81, 78, 255);

                                    }
                                }
                                index_offset_unr++;
                            }
                            draw_offset += 75;
                        }

                        index_offset++;
                    }
                    else draw_tasks = false;
                }
                if (created_tt == false) created_tt = true;
                SDL_SetRenderDrawColor(renderer, 36, 35, 34, 255);
                draw.DrawRoundedRect(renderer, { int(sidebar_anim - 120), WINDOW_HEIGHT - 44, 86, 30 }, 10);
                //SDL_SetRenderDrawColor(renderer, 84, 81, 78, 255);
                asd = { int(sidebar_anim - 115), WINDOW_HEIGHT - 45, 76, 28 };
                SDL_RenderCopy(renderer, BackTestButt, nullptr, &asd);
            }
        }
        if (EditMenu and sidebar_opened) {
            SDL_SetRenderDrawColor(renderer, 31, 30, 29, 255);
            draw.DrawRoundedRect(renderer, { int(sidebar_anim - 170), WINDOW_HEIGHT - 50, 40 , 40}, 20);
            SDL_SetRenderDrawColor(renderer, 100, 100, 100, 255);
            draw.DRRwB(renderer, { int(sidebar_anim - 170), WINDOW_HEIGHT - 50, 40, 40 }, 20);
            SDL_SetRenderDrawColor(renderer, 110, 180, 100, 255);

            draw.DrawCircle(renderer, int(sidebar_anim - 151), WINDOW_HEIGHT - 31, 12);
            if (PlanTasks) {
                SDL_SetRenderDrawColor(renderer, 56, 54, 52, 255);
                draw.DrawCircle(renderer, int(sidebar_anim) - 15, WINDOW_HEIGHT - 31, 10);
            }
            
            /*SDL_SetRenderDrawColor(renderer, 31, 30, 29, 255);
            draw.DrawRoundedRect(renderer, { int(sidebar_anim - 190), WINDOW_HEIGHT - 50, 80 , 40 }, 20);
            SDL_SetRenderDrawColor(renderer, 100, 100, 100, 255);
            draw.DRRwB(renderer, { int(sidebar_anim - 190), WINDOW_HEIGHT - 50, 80, 40 }, 20);
            SDL_SetRenderDrawColor(renderer, 110, 180, 100, 255);

            draw.DrawCircle(renderer, int(sidebar_anim - 151), WINDOW_HEIGHT - 31, 12);

            SDL_SetRenderDrawColor(renderer, 98, 98, 98, 255);
            draw.DrawCircle(renderer, int(sidebar_anim - 126), WINDOW_HEIGHT - 31, 10);
            SDL_SetRenderDrawColor(renderer, 65, 65, 65, 255);
            draw.DrawCircle(renderer, int(sidebar_anim - 175), WINDOW_HEIGHT - 31, 10);*/

        }
        if (NoSavedChanges) {
            SDL_SetRenderDrawColor(renderer, 36, 35, 34, 255);
            draw.DrawRoundedRect(renderer, { 10, WINDOW_HEIGHT - 44, 90 , 30 }, 10);

            asd = { 27, WINDOW_HEIGHT - 45, 55, 27 };
            SDL_RenderCopy(renderer, SaveChangesButt, nullptr, &asd);
        }

        // Обновление экрана
        SDL_RenderPresent(renderer);
        SDL_Delay(29);

        if (static_img) SDL_Delay(200);


    }
    // Очистка ресурсов
    for (auto& item : textItems) {
        SDL_DestroyTexture(item.texture);
        if (PlanTasks == 0)SDL_DestroyTexture(item.time);
    }
    SDL_DestroyTexture(PlanTasksChoise);
    SDL_DestroyTexture(BackTestButt);
    SDL_DestroyTexture(SaveChangesButt);
    SDL_DestroyTexture(MainCanvasTxtr);
    SDL_DestroyTexture(ContentTypeTxtr);
    TTF_CloseFont(font);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    TTF_Quit();
    SDL_Quit();

    return 0;
}
