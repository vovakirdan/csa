CC = gcc
CFLAGS = -Iinclude -Wall -Wextra
LIBS = -lX11 -lopencv_core -lopencv_imgproc

all: app

app: src/main.o src/capture.o src/detect.o src/overlay.o
	$(CC) -o app src/main.o src/capture.o src/detect.o src/overlay.o $(LIBS)

src/%.o: src/%.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f app src/*.o
