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

#include "../png_utils.h"
#include "../utils.h"

#include <stdlib.h> /* abort */
#include <stdint.h> /* uint64_t */
#include <limits.h> /* INT_MAX */
#include <math.h>
#include <assert.h>

#include <png.h>

#define ROUND_DIV(x,y) (((x) + (y)/2)/(y))
#define SWAP(x,y,type)  do { type temp = x; x = y; y = temp; } while(0)

static uint64_t squared_difference(struct png_info read_1, struct png_info read_2);
int main(int argc, char **argv);

uint64_t squared_difference(struct png_info read_1, struct png_info read_2)
{
    int x, y, c;

    /* Expand any grayscale, RGB, or palette images to RGBA */
    png_set_expand(read_1.png_ptr);
    png_set_expand(read_2.png_ptr);

    /* Reduce any 16-bits-per-sample images to 8-bits-per-sample */
    png_set_strip_16(read_1.png_ptr);
    png_set_strip_16(read_2.png_ptr);

    /* Compute bytes per row */
    int channels_per_pixel_1 = get_channels_per_pixel(read_1);
    int channels_per_pixel_2 = get_channels_per_pixel(read_2);
    if (read_1.width != read_2.width ||
        read_1.height != read_2.height ||
        channels_per_pixel_1 != channels_per_pixel_2)
    {
        /* Don't attempt to compare images of different sizes or color types */
        return ULLONG_MAX;
    }

    int read_rowbytes_1 = read_1.width*channels_per_pixel_1*read_1.bit_depth/8;
    int read_rowbytes_2 = read_2.width*channels_per_pixel_2*read_2.bit_depth/8;

    /* Read and write pixels */
    png_bytep read_row_pointer_1 = (png_byte*) malloc(read_rowbytes_1);
    png_bytep read_row_pointer_2 = (png_byte*) malloc(read_rowbytes_2);
    
    uint64_t result = 0;
    for (y=0; y < read_1.height; y++) {
        png_read_row(read_1.png_ptr, read_row_pointer_1, NULL);
        png_read_row(read_2.png_ptr, read_row_pointer_2, NULL);
        for (x=0; x < read_1.width; x++) {
            png_byte* read_ptr_1 = &(read_row_pointer_1[x*channels_per_pixel_1]);
            png_byte* read_ptr_2 = &(read_row_pointer_2[x*channels_per_pixel_2]);
            if (channels_per_pixel_1 == 4 && (read_ptr_1[3] == 0 || read_ptr_2[3] == 0)) {
                /* If one of them is fully transparent, assume RGB channels match */
                int diff = read_ptr_1[3] - read_ptr_2[3];
                result += diff*diff;
            } else {
                for (c=0; c < channels_per_pixel_1; c++) {
                    int diff = read_ptr_1[c] - read_ptr_2[c];
                    result += diff*diff;
                }
            }
        }
    }

    close_read_png(read_1);
    close_read_png(read_2);
    return result;
}

double png_compare(const char* filename_1, const char* filename_2)
{
    struct png_info read_1 = open_read_png(filename_1);
    struct png_info read_2 = open_read_png(filename_2);
    int width = read_1.width;
    int height = read_1.height;
    return sqrt((double)squared_difference(read_1, read_2))/sqrt(width*width + height*height);
}
