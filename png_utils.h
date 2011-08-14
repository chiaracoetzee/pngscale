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

#ifndef _PNG_UTILS_H_
#define _PNG_UTILS_H_

#include <png.h>

struct png_info
{
    png_structp png_ptr;
    png_infop info_ptr;
    FILE* fp;
    int width;
    int height;
    png_byte color_type;
    png_byte bit_depth;
    int number_of_passes;
    int rowbytes;
    int channels;
};

struct png_info open_read_png(const char* read_file_name);
void open_write_png(const char* write_file_name, struct png_info* info);
void close_read_png(struct png_info info);
void close_write_png(struct png_info info);
int get_channels_per_pixel(struct png_info info);

#endif /* #ifndef _PNG_UTILS_H_ */
