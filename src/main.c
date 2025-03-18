#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "capture.h"
#include "detect.h"
#include "overlay.h"

int main() {
    unsigned char* buffer = NULL;
    int width, height;

    while (1) {
        if (capture_screen(&buffer, &width, &height) == 0) {
            if (detect_code(buffer, width, height)) {
                printf("Code detected!\n");
                show_overlay();
            }
        }
        sleep(1);
    }

    return 0;
}
