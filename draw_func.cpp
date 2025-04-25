#include <windows.h>
#include <cmath>
#include <SDL.h>


class draw {

public:

    void DrawCircle(SDL_Renderer* renderer, int x, int y, int radius) {
        for (int w = 0; w < radius * 2; w++) {
            for (int h = 0; h < radius * 2; h++) {
                int dx = radius - w; 
                int dy = radius - h; 
                if ((dx * dx + dy * dy) <= (radius * radius)) {
                    SDL_RenderDrawPoint(renderer, x + dx, y + dy);
                }
            }
        }
    }
    void DrawArc(SDL_Renderer* renderer, int centerX, int centerY, int radius, int startAngle, int endAngle) {
        int segments = 30; 
        double angleStep = (endAngle - startAngle) * M_PI / 180.0 / segments;

        for (int i = 0; i < segments; ++i) {
            double angle1 = startAngle * M_PI / 180.0 + i * angleStep;
            double angle2 = startAngle * M_PI / 180.0 + (i + 1) * angleStep;

            int x1 = int(centerX + radius * cos(angle1));
            int y1 = int(centerY + radius * sin(angle1));
            int x2 = int(centerX + radius * cos(angle2));
            int y2 = int(centerY + radius * sin(angle2));

            SDL_RenderDrawLine(renderer, x1, y1, x2, y2);
        }
    }

   
    void DrawRoundedRect(SDL_Renderer* renderer, SDL_Rect rect, int radius) {
        
        DrawCircle(renderer, rect.x + radius, rect.y + radius, radius); 
        DrawCircle(renderer, rect.x + rect.w - radius, rect.y + radius, radius);
        DrawCircle(renderer, rect.x + radius, rect.y + rect.h - radius, radius);
        DrawCircle(renderer, rect.x + rect.w - radius, rect.y + rect.h - radius, radius);

        // Çàïîëíåíèå ñåðåäèíû
        SDL_Rect fillRect = { rect.x + radius, rect.y, rect.w - radius * 2, rect.h };
        SDL_RenderFillRect(renderer, &fillRect);
        fillRect = { rect.x, rect.y + radius, rect.w, rect.h - radius * 2 };
        SDL_RenderFillRect(renderer, &fillRect);
    }

    void DrawSidebar(SDL_Renderer* renderer, SDL_Rect rect, int radius) {

        DrawCircle(renderer, rect.w - radius, rect.y + radius, radius); 
        DrawCircle(renderer, rect.w - radius, rect.y + rect.h - radius, radius); 

        
        SDL_Rect fillRect = { 0, rect.y, rect.w-radius, rect.h };
        SDL_RenderFillRect(renderer, &fillRect);
        fillRect = { rect.w - radius, rect.y + radius, radius, rect.h - radius * 2 };
        SDL_RenderFillRect(renderer, &fillRect);
    }

    void DrawSidebarB(SDL_Renderer* renderer, SDL_Rect rect, int radius) {

        DrawArc(renderer, rect.w - radius - 1, rect.y + radius, radius, 270, 360); 
        DrawArc(renderer, rect.w - radius - 1, rect.y + rect.h - radius - 1, radius, 0, 90);

        
        SDL_RenderDrawLine(renderer, rect.x + rect.w - 1, rect.y + radius, rect.x + rect.w - 1, rect.y + rect.h - radius - 1); // Ïðàâàÿ ñòîðîíà

        SDL_RenderDrawLine(renderer, 0, rect.y, rect.w - radius - 1, rect.y); 
        SDL_RenderDrawLine(renderer, 0, rect.y + rect.h - 1, rect.w - radius - 1, rect.y + rect.h - 1); 
    }

    
    void DRRwB(SDL_Renderer* renderer, SDL_Rect rect, int radius) {
        
        DrawArc(renderer, rect.x + radius, rect.y + radius, radius, 180, 270); 
        DrawArc(renderer, rect.x + rect.w - radius - 1, rect.y + radius, radius, 270, 360); 
        DrawArc(renderer, rect.x + radius, rect.y + rect.h - radius - 1, radius, 90, 180);
        DrawArc(renderer, rect.x + rect.w - radius - 1, rect.y + rect.h - radius - 1, radius, 0, 90);

        
        SDL_RenderDrawLine(renderer, rect.x + radius, rect.y, rect.x + rect.w - radius - 1, rect.y); 
        SDL_RenderDrawLine(renderer, rect.x + radius, rect.y + rect.h - 1, rect.x + rect.w - radius - 1, rect.y + rect.h - 1); 
        SDL_RenderDrawLine(renderer, rect.x, rect.y + radius, rect.x, rect.y + rect.h - radius - 1); 
        SDL_RenderDrawLine(renderer, rect.x + rect.w - 1, rect.y + radius, rect.x + rect.w - 1, rect.y + rect.h - radius - 1); 

    }

    

    
};

static void SetRoundedRegion(SDL_Window* window, int radius, bool bordered) {
    HWND hwnd = GetActiveWindow(); // Получаем HWND окна SDL
    if (!hwnd) return;
    if (bordered) {
        // Возвращаем стандартную прямоугольную область (убираем скругление)
        SetWindowRgn(hwnd, NULL, TRUE);
    }
    else {
        RECT rect;
        GetClientRect(hwnd, &rect);
        int width = rect.right;
        int height = rect.bottom;

        HRGN hRegion = CreateRoundRectRgn(0, 0, width, height, radius, radius);
        SetWindowRgn(hwnd, hRegion, TRUE);
    }
}
