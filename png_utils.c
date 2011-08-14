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

struct png_info open_read_png(const char* file_name)
{
    struct png_info result;
    unsigned char header[8];    /* 8 is the maximum size that can be checked */

    /* open file and test for it being a png */
    result.fp = fopen(file_name, "rb");
    if (!result.fp) {
        abort_("File %s could not be opened for reading", file_name);
    }
    if (fread(header, 1, 8, result.fp) < 8 || png_sig_cmp(header, 0, 8)) {
        abort_("File %s is not recognized as a PNG file", file_name);
    }

    /* initialize stuff */
    result.png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
    if (!result.png_ptr) {
        abort_("png_create_read_struct failed while opening %s for reading", file_name);
    }

    result.info_ptr = png_create_info_struct(result.png_ptr);
    if (!result.info_ptr) {
        abort_("png_create_info_struct failed while opening %s for reading", file_name);
    }

    if (setjmp(png_jmpbuf(result.png_ptr))) {
        abort_("Error during init_io while opening %s for reading", file_name);
    }

    png_init_io(result.png_ptr, result.fp);
    png_set_sig_bytes(result.png_ptr, 8);

    png_read_info(result.png_ptr, result.info_ptr);

    /* Expand any grayscale, RGB, or palette images to RGBA */
    png_set_expand(result.png_ptr);

    /* Reduce any 16-bits-per-sample images to 8-bits-per-sample */
    png_set_strip_16(result.png_ptr);

    /* Must be before reading fields from result.info_ptr */
    png_read_update_info(result.png_ptr, result.info_ptr);

    result.width = png_get_image_width(result.png_ptr, result.info_ptr);
    result.height = png_get_image_height(result.png_ptr, result.info_ptr);
    result.color_type = png_get_color_type(result.png_ptr, result.info_ptr);
    result.bit_depth = png_get_bit_depth(result.png_ptr, result.info_ptr);
    result.number_of_passes = png_set_interlace_handling(result.png_ptr);
    result.rowbytes = png_get_rowbytes(result.png_ptr, result.info_ptr);
    result.channels = png_get_channels(result.png_ptr, result.info_ptr);

    /* read file */
    if (setjmp(png_jmpbuf(result.png_ptr))) {
        abort_("Error while reading PNG image");
    }

    return result;
}

void open_write_png(const char* file_name, struct png_info* info)
{
    /* create output file */
    info->fp = fopen(file_name, "wb");
    if (!info->fp) {
        abort_("File %s could not be opened for writing", file_name);
    }

    /* initialize stuff */
    info->png_ptr = png_create_write_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);

    if (!info->png_ptr) {
        abort_("png_create_write_struct failed while opening %s for writing", file_name);
    }

    info->info_ptr = png_create_info_struct(info->png_ptr);
    if (!info->info_ptr) {
        abort_("png_create_info_struct failed while opening %s for writing", file_name);
    }

    if (setjmp(png_jmpbuf(info->png_ptr))) {
        abort_("Error during init_io while opening %s for writing", file_name);
    }

    png_init_io(info->png_ptr, info->fp);

    /* write header */
    if (setjmp(png_jmpbuf(info->png_ptr))) {
        abort_("Error writing header while opening %s for writing", file_name);
    }

    png_set_IHDR(info->png_ptr, info->info_ptr, info->width, info->height,
                 info->bit_depth, info->color_type, PNG_INTERLACE_NONE,
                 PNG_COMPRESSION_TYPE_BASE, PNG_FILTER_TYPE_BASE);

    png_read_update_info(info->png_ptr, info->info_ptr);
    info->rowbytes = png_get_rowbytes(info->png_ptr, info->info_ptr);
    info->channels = png_get_channels(info->png_ptr, info->info_ptr);
    png_write_info(info->png_ptr, info->info_ptr);

    /* set jmpbuf */
    if (setjmp(png_jmpbuf(info->png_ptr))) {
        abort_("Error while writing PNG image");
    }
}

int get_channels_per_pixel(struct png_info info)
{
    int result = 0;
    if (info.color_type & PNG_COLOR_MASK_COLOR) {
        result += 3; /* red, green, blue */
    } else {
        result += 1; /* gray */
    }
    if (info.color_type & PNG_COLOR_MASK_ALPHA) {
        result += 1; /* alpha */
    }
    return result;
}

void close_read_png(struct png_info info) {
    if (setjmp(png_jmpbuf(info.png_ptr))) {
        abort_("Error during png_read_end");
    }
    png_read_end(info.png_ptr, NULL);
    png_destroy_read_struct(&info.png_ptr, &info.info_ptr, NULL);
    fclose(info.fp);
}

void close_write_png(struct png_info info) {
    if (setjmp(png_jmpbuf(info.png_ptr))) {
        abort_("Error during png_write_end");
    }
    png_write_end(info.png_ptr, NULL);
    png_destroy_write_struct(&info.png_ptr, &info.info_ptr);
    fclose(info.fp);
}

