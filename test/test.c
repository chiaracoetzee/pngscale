/* Copyright (c) 2011 Derrick Coetzee, Guillaume Cottenceau, and contributors

Permission is hereby granted, free of charge, to any person obtaining
a copy of this software and associated documentation files (the
"Software"), to deal in the Software without restriction, including
without limitation the rights to use, copy, modify, merge, publish,
distribute, sublicense, and/or sell copies of the Software, and to
permit persons to whom the Software is furnished to do so, subject to
the following conditions:

The above copyright notice and this permission notice shall be
included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE
LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION
OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

Based on code distributed by Guillaume Cottenceau and contributors
under MIT/X11 License at http://zarb.org/~gc/html/libpng.html
*/

#include "pngcompare.h"
#include "../utils.h"

#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>

void sys(const char* command) {
    int return_code = system(command);
    if (return_code != 0) {
        abort_("Received nonzero return code %d from command '%s'", return_code, command);
    }
}

void assert_png_approx_equal(const char* filename_1, const char* filename_2, double max_error) {
    double difference = png_compare(filename_1, filename_2);
    if (difference > max_error) {
        abort_("Scaled output files '%s' and '%s' were too different (distance=%f, threshold=%f).", filename_1, filename_2, difference, max_error);
    }
}

void test_basic(const char* filename, int max_width, double max_error) {
    printf("Testing %s at %dpx...", filename, max_width);
    fflush(stdout);
    char buffer[256];
    snprintf(buffer, sizeof(buffer), "./pngscale %s /tmp/out.pngscale.png %d -1", filename, max_width);
    sys(buffer);
    /* Requires ImageMagick convert */
    snprintf(buffer, sizeof(buffer), "convert %s -resize %d /tmp/out.convert.png", filename, max_width);
    sys(buffer);
    assert_png_approx_equal("/tmp/out.pngscale.png", "/tmp/out.convert.png", max_error);
    printf("\n");
}

int main(void) {
    int i;
    int sizes[] = { 1, 50, 150, 200, 220, 300, 400, 1000 };
    for (i=0; i < sizeof(sizes)/sizeof(*sizes); i++) {
        test_basic("test/data/ferriero.png", sizes[i], 5.0);
        test_basic("test/data/antonio.png", sizes[i], 5.0);
    }
    test_basic("test/data/antonio.png", 5000, 5.0);

    /* Different color types and bit depths */
    test_basic("test/data/ferriero_gray.png", 220, 5.0);
    test_basic("test/data/ferriero_16bit.png", 220, 5.0);
    test_basic("test/data/ferriero_palette_bw.png", 220, 5.0);
    test_basic("test/data/ferriero_palette_2.png", 220, 5.0);
    test_basic("test/data/ferriero_palette_4.png", 220, 5.0);
    test_basic("test/data/ferriero_palette_8.png", 220, 5.0);
    test_basic("test/data/ferriero_palette_16.png", 220, 5.0);
    test_basic("test/data/ferriero_palette_32.png", 220, 5.0);
    test_basic("test/data/ferriero_palette_64.png", 220, 5.0);
    test_basic("test/data/ferriero_palette_128.png", 220, 5.0);
    test_basic("test/data/ferriero_palette_256.png", 220, 5.0);

#if 0
    /* Commented out - passes but really slow */
    printf("Creating /tmp/antonio_large.png...\n");
    sys("convert test/data/antonio.png -resize 19203 /tmp/antonio-large.png");
    printf("Doing large image test...\n");
    test_basic("/tmp/antonio_large.png", 220, 5.0);
    unlink("/tmp/antonio_large.png");
#endif

    /* Transparency */
    test_basic("test/data/Abrams-transparent.png", 220, 6.0);
    test_basic("test/data/Abrams-transparent_palette_256.png", 220, 5.0);

    unlink("/tmp/out.pngscale.png");
    unlink("/tmp/out.convert.png");

    printf("\nAll tests passed.\n");
    return 0;
}
