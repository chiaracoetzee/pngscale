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

#include "png_utils.h"
#include "utils.h"

#include <stdlib.h> /* abort */
#include <stdint.h> /* uint64_t */
#include <limits.h> /* INT_MAX */
#include <math.h>
#include <assert.h>

#include <png.h>

#define ROUND_DIV(x,y) (((x) + (y)/2)/(y))
#define SWAP(x,y,type)  do { type temp = x; x = y; y = temp; } while(0)

static void scale_png_up(struct png_info read, struct png_info write);
static void scale_png_down(struct png_info read, struct png_info write);
static struct png_info compute_write_info(struct png_info read, int max_width, int max_height);
int main(int argc, char **argv);

void scale_png_up(struct png_info read, struct png_info write)
{
    int x, y, c;

    /* Read and write pixels */
    png_bytep read_row_pointer = (png_byte*) malloc(read.rowbytes);
    png_bytep read_next_row_pointer = (png_byte*) malloc(read.rowbytes);
    if (!read_row_pointer || !read_next_row_pointer) {
        abort_("Failed to allocate memory to hold two rows of input PNG image");
    }
    png_bytep write_row_pointer = (png_byte*) malloc(write.rowbytes);
    if (!write_row_pointer) {
        abort_("Failed to allocate memory to hold one row of output PNG image");
    }
    
    if (read.height == 1) {
        abort_("Not yet handling upscaling of images one pixel high");
    }

    /* Using floating point in this procedure because performance isn't a
       concern - upscaling is rare and generally involves small images. */
    png_read_row(read.png_ptr, read_row_pointer, NULL);
    png_read_row(read.png_ptr, read_next_row_pointer, NULL);
    /* Subtracting 1 because our read pixels are conceptually being
       sampled at the upper-left corner of each pixel, so the bottom-
       right corners have no value. */
    double x_scale = (double)write.width / (read.width - 1); 
    double y_scale = (double)write.height / (read.height - 1);
    double int_part_y = 0.0;
    for (y=0; y < write.height; y++) {
        double old_int_part_y = int_part_y;
        double fraction_from_above_row = 1.0 - modf(y/y_scale, &int_part_y);
        double fraction_from_below_row = 1.0 - fraction_from_above_row;
        if (int_part_y != old_int_part_y) {
            int i;
            for (i=0; i < int_part_y - old_int_part_y; i++) {
                SWAP(read_row_pointer, read_next_row_pointer, png_bytep);
                png_read_row(read.png_ptr, read_next_row_pointer, NULL);
            }
        }
        for (x=0; x < write.width; x++) {
            double read_x_dbl;
            double fraction_from_left_col = 1.0 - modf(x/x_scale, &read_x_dbl);
            int read_x = (int)read_x_dbl;
            double fraction_from_right_col = 1.0 - fraction_from_left_col;

            png_byte* read_above_left_ptr = &(read_row_pointer[read_x*read.channels]);
            png_byte* read_below_left_ptr = &(read_next_row_pointer[read_x*read.channels]);
            png_byte* read_above_right_ptr = &(read_row_pointer[(read_x + 1)*read.channels]);
            png_byte* read_below_right_ptr = &(read_next_row_pointer[(read_x + 1)*read.channels]);
            png_byte* write_ptr = &(write_row_pointer[x*write.channels]);
            for (c=0; c < write.channels; c++) {
                double val = read_above_left_ptr[c]  * fraction_from_above_row * fraction_from_left_col + 
                             read_above_right_ptr[c] * fraction_from_above_row * fraction_from_right_col + 
                             read_below_left_ptr[c]  * fraction_from_below_row * fraction_from_left_col + 
                             read_below_right_ptr[c] * fraction_from_below_row * fraction_from_right_col;
                write_ptr[c] = (int)round(val);
            }
        }

        png_write_row(write.png_ptr, write_row_pointer);
    }

    close_read_png(read);
    close_write_png(write);
}

void scale_png_down(struct png_info read, struct png_info write)
{
    int x, y, c;

    /* Read and write pixels */
    png_bytep read_row_pointer = (png_byte*) malloc(read.rowbytes);
    if (!read_row_pointer) {
        abort_("Failed to allocate memory to hold one row of input PNG image");
    }

    uint64_t* write_row_sums_pointer = (uint64_t*) malloc(sizeof(uint64_t) * write.width * write.channels);
    uint64_t* write_next_row_sums_pointer = (uint64_t*) malloc(sizeof(uint64_t) * write.width * write.channels);
    uint64_t* read_areas = (uint64_t*) malloc(sizeof(uint64_t) * write.width * write.channels);
    uint64_t* read_areas_next_row = (uint64_t*) malloc(sizeof(uint64_t) * write.width * write.channels);
    if (!write_row_sums_pointer || !write_next_row_sums_pointer || !read_areas || !read_areas_next_row) {
        abort_("Failed to allocate memory - need enough to hold nine rows of output PNG image");
    }

    png_bytep write_row_pointer = (png_byte*) malloc(write.rowbytes);
    if (!write_row_pointer) {
        abort_("Failed to allocate memory to hold one row of output PNG image");
    }
    
    memset(write_row_sums_pointer, 0, sizeof(uint64_t) * write.width * write.channels);
    memset(write_next_row_sums_pointer, 0, sizeof(uint64_t) * write.width * write.channels);
    memset(read_areas, 0, sizeof(uint64_t) * write.width * write.channels);
    memset(read_areas_next_row, 0, sizeof(uint64_t) * write.width * write.channels);
    int y_frac = 0;
    for (y=0; y < read.height; y++) {
        png_read_row(read.png_ptr, read_row_pointer, NULL);

        int end_of_row = 0;
        unsigned int fraction_in_current_row = write.height; /* Proportion represented by integer between 0 and write.height */
        unsigned int fraction_in_next_row = 0;
        y_frac += write.height;
        if (y_frac >= read.height) {
            /* We've reached a boundary between output image rows. */
            end_of_row = 1;
            y_frac -= read.height;
            fraction_in_current_row = write.height - y_frac;
            fraction_in_next_row = y_frac;
        }

        int write_x = 0;
        int x_frac = 0;
        for (x=0; x < read.width; x++) {
            int end_of_col = 0;
            unsigned int fraction_in_current_col = write.width; /* Proportion represented by integer between 0 and write.width */
            unsigned int fraction_in_next_col = 0;
            x_frac += write.width;
            if (x_frac >= read.width) {
                /* We've reached a boundary between output image columns. */
                end_of_col = 1;
                x_frac -= read.width;
                fraction_in_current_col = write.width - x_frac;
                fraction_in_next_col = x_frac;
            }

            png_byte* read_ptr = &(read_row_pointer[x*read.channels]);
            for (c=0; c < write.channels; c++) {
                uint64_t value = read_ptr[c];
                uint64_t alpha = 255;
                if (read.channels >= 4 && c < 3) {
                    alpha = read_ptr[3];
                }
                write_row_sums_pointer[write.channels*write_x + c] +=
                    value * fraction_in_current_col * fraction_in_current_row * alpha / 255;
                read_areas[write.channels*write_x + c] += fraction_in_current_col * fraction_in_current_row * alpha / 255;
                if (fraction_in_next_col) {
                    write_row_sums_pointer[write.channels*(write_x + 1) + c] +=
                        value * fraction_in_next_col * fraction_in_current_row * alpha / 255;
                    read_areas[write.channels*(write_x + 1) + c] += fraction_in_next_col * fraction_in_current_row * alpha / 255;
                }
                if (fraction_in_next_row) {
                    write_next_row_sums_pointer[write.channels*write_x + c] +=
                        value * fraction_in_current_col * fraction_in_next_row * alpha / 255;
                    read_areas_next_row[write.channels*write_x + c] += fraction_in_current_col * fraction_in_next_row * alpha / 255;
                }
                if (fraction_in_next_col && fraction_in_next_row) {
                    write_next_row_sums_pointer[write.channels*(write_x + 1) + c] +=
                        value * fraction_in_next_col * fraction_in_next_row * alpha / 255;
                    read_areas_next_row[write.channels*(write_x + 1) + c] += fraction_in_next_col * fraction_in_next_row * alpha / 255;
                }
            }

            if (end_of_col) {
                write_x++;
                assert (write_x < write.width || x == read.width - 1);
            }
        }

        if (end_of_row) {
            for (x=0; x < write.width; x++) {
                png_byte* write_ptr = &(write_row_pointer[x*write.channels]);
                uint64_t* write_sums_ptr = &(write_row_sums_pointer[x*write.channels]);
                uint64_t* read_areas_ptr = &(read_areas[x*write.channels]);
                for (c=0; c < write.channels; c++) {
                    if (read_areas_ptr[c] == 0) {
                        /* Fully transparent pixel, value is irrelevant */
                        write_ptr[c] = 0;
                    } else {
                        write_ptr[c] = ROUND_DIV(write_sums_ptr[c], read_areas_ptr[c]);
                    }
                }
            }

            png_write_row(write.png_ptr, write_row_pointer);
            SWAP(write_row_sums_pointer, write_next_row_sums_pointer, uint64_t*);
            memset(write_next_row_sums_pointer, 0, sizeof(uint64_t) * write.width * write.channels);
            SWAP(read_areas, read_areas_next_row, uint64_t*);
            memset(read_areas_next_row, 0, sizeof(uint64_t) * write.width * write.channels);
        }
    }

    close_read_png(read);
    close_write_png(write);
}

struct png_info compute_write_info(struct png_info read, int width, int height)
{
    struct png_info write;

    /* If either width or height is -1, user is requesting
       us to preserve the aspect ratio:

       Set write.width, write.height so that:
       1. read.width/read.height approx= write.width/write.height
       2. write.width <= width
       3. write.height <= height
       4. Image is large as possible */
    if (width == -1 && height > 0) {
        write.height = height;
        write.width = ROUND_DIV(write.height * read.width, read.height);
    } else if (width > 0 && height == -1) {
        write.width = width;
        write.height = ROUND_DIV(write.width * read.height, read.width);
    } else if (width <= 0 || height <= 0) {
        abort_("Invalid width/height");
    } else {
        write.width = width;
        write.height = height;
    }

    if (write.width == 0) {
        write.width = 1;
    }
    if (write.height == 0) {
        write.height = 1;
    }
    write.bit_depth = 8;
    write.color_type = read.color_type & ~PNG_COLOR_MASK_PALETTE;
    return write;
}

int main(int argc, char **argv)
{
    if (argc != 5) {
        printf("Usage: pngscale <input file> <output file> <width px> <height px>\n"
               "Set either width or height to -1 to choose other to preserve aspect ratio.\n");
        return 1;
    }

    int width = atoi(argv[3]);
    int height = atoi(argv[4]);

    struct png_info read = open_read_png(argv[1]);
    struct png_info write = compute_write_info(read, width, height);
    open_write_png(argv[2], &write);
    if (write.width > read.width || write.height > read.height) {
        scale_png_up(read, write);
    } else {
        scale_png_down(read, write);
    }

    return 0;
}
