#include "capture.h"
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <direct.h>
#include <windows.h>

// Структура для хранения информации о мониторе
typedef struct {
    int x;
    int y;
    int width;
    int height;
    int number;
} MonitorInfo;

typedef struct {
    MonitorInfo* monitors;
    int index;
} MonitorEnumData;

// Функция для создания директории, если она не существует
static int ensure_directory_exists(const char* dir) {
    // Проверяем существование директории
    if (_access(dir, 0) == -1) {
        // Создаем директорию
        if (_mkdir(dir) == -1) {
            return 0;
        }
    }
    return 1;
}

// Функция для получения текущей даты/времени в формате YYYYMMDDHHMMSS
static void get_timestamp(char* buffer, size_t size) {
    time_t now = time(NULL);
    struct tm* tm_info = localtime(&now);
    strftime(buffer, size, "%Y%m%d%H%M%S", tm_info);
}

// Функция обратного вызова для перечисления мониторов
static BOOL CALLBACK MonitorEnumProc(HMONITOR hMonitor, HDC hdcMonitor, LPRECT lprcMonitor, LPARAM dwData) {
    MonitorEnumData* data = (MonitorEnumData*)dwData;
    MonitorInfo* monitors = data->monitors;
    int idx = data->index;

    MONITORINFOEXA mi;
    mi.cbSize = sizeof(MONITORINFOEXA);
    GetMonitorInfoA(hMonitor, (LPMONITORINFO)&mi);

    monitors[idx].x = mi.rcMonitor.left;
    monitors[idx].y = mi.rcMonitor.top;
    monitors[idx].width = mi.rcMonitor.right - mi.rcMonitor.left;
    monitors[idx].height = mi.rcMonitor.bottom - mi.rcMonitor.top;
    monitors[idx].number = idx + 1;

    data->index += 1;
    return TRUE;
}

// Функция для получения информации о мониторах
static int get_monitors_info(MonitorInfo** monitors, int* count) {
    *count = GetSystemMetrics(SM_CMONITORS);
    *monitors = (MonitorInfo*)malloc(sizeof(MonitorInfo) * (*count));
    if (!*monitors) {
        return 0;
    }
    MonitorEnumData data = { .monitors = *monitors, .index = 0 };
    if (!EnumDisplayMonitors(NULL, NULL, MonitorEnumProc, (LPARAM)&data)) {
        free(*monitors);
        *monitors = NULL;
        return 0;
    }
    return 1;
}

// Функция для захвата конкретного монитора
static int capture_monitor(const MonitorInfo* monitor, unsigned char** buffer, int* width, int* height) {
    HDC hdcScreen = GetDC(NULL);
    HDC hdcMemDC = CreateCompatibleDC(hdcScreen);
    
    *width = monitor->width;
    *height = monitor->height;
    
    HBITMAP hBitmap = CreateCompatibleBitmap(hdcScreen, *width, *height);
    HBITMAP hOldBitmap = (HBITMAP)SelectObject(hdcMemDC, hBitmap);
    
    // Копируем изображение с экрана
    BitBlt(hdcMemDC, 0, 0, *width, *height, 
           hdcScreen, monitor->x, monitor->y, SRCCOPY);
    
    // Получаем данные изображения
    BITMAPINFO bmi = {0};
    bmi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
    bmi.bmiHeader.biWidth = *width;
    bmi.bmiHeader.biHeight = -*height;  // Отрицательная высота для top-down DIB
    bmi.bmiHeader.biPlanes = 1;
    bmi.bmiHeader.biBitCount = 32;
    bmi.bmiHeader.biCompression = BI_RGB;
    
    *buffer = (unsigned char*)malloc(*width * *height * 4);
    if (!*buffer) {
        SelectObject(hdcMemDC, hOldBitmap);
        DeleteObject(hBitmap);
        DeleteDC(hdcMemDC);
        ReleaseDC(NULL, hdcScreen);
        return 0;
    }
    
    GetDIBits(hdcScreen, hBitmap, 0, *height, *buffer, &bmi, DIB_RGB_COLORS);
    
    // Очистка
    SelectObject(hdcMemDC, hOldBitmap);
    DeleteObject(hBitmap);
    DeleteDC(hdcMemDC);
    ReleaseDC(NULL, hdcScreen);
    
    return 1;
}

// Функция для сохранения изображения в BMP
static int save_to_bmp(const char* filename, unsigned char* buffer, int width, int height) {
    FILE* file = fopen(filename, "wb");
    if (!file) {
        return 0;
    }

    // Подготавливаем заголовки BMP
    BITMAPFILEHEADER fileHeader = {0};
    BITMAPINFOHEADER infoHeader = {0};
    
    // Заполняем заголовок файла
    fileHeader.bfType = 0x4D42;  // 'BM'
    fileHeader.bfSize = sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER) + width * height * 4;
    fileHeader.bfOffBits = sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER);
    
    // Заполняем информационный заголовок
    infoHeader.biSize = sizeof(BITMAPINFOHEADER);
    infoHeader.biWidth = width;
    infoHeader.biHeight = -height;  // Отрицательная высота для top-down DIB
    infoHeader.biPlanes = 1;
    infoHeader.biBitCount = 32;
    infoHeader.biCompression = BI_RGB;
    infoHeader.biSizeImage = width * height * 4;
    
    // Записываем заголовки
    fwrite(&fileHeader, sizeof(BITMAPFILEHEADER), 1, file);
    fwrite(&infoHeader, sizeof(BITMAPINFOHEADER), 1, file);
    
    // Записываем данные изображения
    fwrite(buffer, 1, width * height * 4, file);
    
    fclose(file);
    return 1;
}

int capture_screen(unsigned char** buffer, int* width, int* height) {
    // Создаем директорию для скриншотов
    const char* screenshot_dir = "screenshots";
    if (!ensure_directory_exists(screenshot_dir)) {
        return 0;
    }
    
    // Получаем информацию о мониторах
    MonitorInfo* monitors = NULL;
    int monitor_count = 0;
    if (!get_monitors_info(&monitors, &monitor_count)) {
        return 0;
    }
    
    // Получаем текущее время для имени файла
    char timestamp[15];
    get_timestamp(timestamp, sizeof(timestamp));
    
    // Захватываем каждый монитор
    for (int i = 0; i < monitor_count; i++) {
        unsigned char* monitor_buffer = NULL;
        int monitor_width, monitor_height;
        
        if (capture_monitor(&monitors[i], &monitor_buffer, &monitor_width, &monitor_height)) {
            // Формируем имя файла
            char filename[MAX_PATH];
            snprintf(filename, MAX_PATH, "%s/%s_%d.bmp", 
                    screenshot_dir, timestamp, monitors[i].number);
            
            // Сохраняем в файл
            save_to_bmp(filename, monitor_buffer, monitor_width, monitor_height);
            
            // Освобождаем буфер
            free(monitor_buffer);
            
            // Сохраняем размеры последнего захваченного монитора
            if (i == monitor_count - 1) {
                *width = monitor_width;
                *height = monitor_height;
                *buffer = monitor_buffer;
                monitor_buffer = NULL;  // Предотвращаем освобождение буфера
            }
        }
    }
    
    // Освобождаем память
    free(monitors);
    
    return 1;
}
