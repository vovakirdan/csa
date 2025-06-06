# Компилятор и флаги для Windows
CC = gcc
CFLAGS = -Iinclude -Wall -Wextra
# Библиотеки для Windows (используем MinGW)
LIBS = -lgdi32

# Используем Windows-совместимые пути
SRC_DIR = src
OBJ_DIR = obj
BIN_DIR = bin
SCREENSHOTS_DIR = screenshots

# Создаем директории если их нет
$(shell mkdir $(OBJ_DIR) $(BIN_DIR))

# Список объектных файлов
OBJS = $(OBJ_DIR)/main.o $(OBJ_DIR)/capture.o $(OBJ_DIR)/overlay.o

all: $(BIN_DIR)/app.exe

$(BIN_DIR)/app.exe: $(OBJS)
	$(CC) -o $@ $^ $(LIBS)

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c
	$(CC) $(CFLAGS) -c $< -o $@

# Команда очистки для Windows
clean:
	if exist $(OBJ_DIR)\*.o del /Q $(OBJ_DIR)\*.o
	if exist $(BIN_DIR)\app.exe del $(BIN_DIR)\app.exe

# Создаем директории при очистке
clean-dirs:
	if exist $(OBJ_DIR) rmdir /S /Q $(OBJ_DIR)
	if exist $(BIN_DIR) rmdir /S /Q $(BIN_DIR)
	if exist $(SCREENSHOTS_DIR) rmdir /S /Q $(SCREENSHOTS_DIR)

.PHONY: all clean clean-dirs
