#include "pngcompare.h"
#include "../utils.h"

#include <stdlib.h>
#include <stdio.h>

void sys(const char* command) {
    int return_code = system(command);
    if (return_code != 0) {
        abort_("Received nonzero return code %d from command '%s'", return_code, command);
    }
}

void assert_png_approx_equal(const char* filename_1, const char* filename_2) {
    static double threshold = 5;
    double difference = png_compare(filename_1, filename_2);
    if (difference > threshold) {
        abort_("Scaled output files '%s' and '%s' were too different (distance=%f, threshold=%f).", filename_1, filename_2, difference, threshold);
    }
}

void test_success(void) {
    printf(".");
    fflush(stdout);
}

void test_basic(const char* filename, int max_width) {
    char buffer[256];
    snprintf(buffer, sizeof(buffer), "./pngscale %s /tmp/out.pngscale.png %d -1", filename, max_width);
    sys(buffer);
    /* Requires ImageMagick convert */
    snprintf(buffer, sizeof(buffer), "convert %s -resize %d /tmp/out.convert.png", filename, max_width);
    sys(buffer);
    assert_png_approx_equal("/tmp/out.pngscale.png", "/tmp/out.convert.png");
    test_success();
}

int main(void) {
    int i;
    int sizes[] = { 1, 50, 150, 200, 220, 300, 400, 1000 };
    for (i=0; i < sizeof(sizes)/sizeof(*sizes); i++) {
        test_basic("test/data/ferriero.png", sizes[i]);
        test_basic("test/data/antonio.png", sizes[i]);
    }

    /* Different color types and bit depths */
    test_basic("test/data/ferriero_gray.png", 220);
    test_basic("test/data/ferriero_16bit.png", 220);
    test_basic("test/data/ferriero_palette_bw.png", 220);
    test_basic("test/data/ferriero_palette_2.png", 220);
    test_basic("test/data/ferriero_palette_4.png", 220);
    test_basic("test/data/ferriero_palette_8.png", 220);
    test_basic("test/data/ferriero_palette_16.png", 220);
    test_basic("test/data/ferriero_palette_32.png", 220);
    test_basic("test/data/ferriero_palette_64.png", 220);
    test_basic("test/data/ferriero_palette_128.png", 220);
    test_basic("test/data/ferriero_palette_256.png", 220);
    test_basic("test/data/antonio_large.png", 220);

#if 0
    /* Commented out - passes but really slow */
    printf("Creating /tmp/antonio_large.png...\n");
    sys("convert test/data/antonio.png -resize 19203 /tmp/antonio-large.png");
    printf("Doing large image test...\n");
    test_basic("/tmp/antonio_large.png", 220);
    unlink("/tmp/antonio_large.png");
#endif
    
    unlink("/tmp/out.pngscale.png");
    unlink("/tmp/out.convert.png");

    printf("\nAll tests passed.\n");
    return 0;
}
