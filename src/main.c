#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include "capture.h"
#include "overlay.h"

int main() {
    unsigned char* buffer = NULL;
    int width, height;

    while (1) {
        // просто сделаем скриншот
        if (capture_screen(&buffer, &width, &height) == 0) {
            printf("No code detected!\n");
        }
        // sleep(1);
        exit(0);
    }

    return 0;
}
