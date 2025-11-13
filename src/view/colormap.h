
/* colormap.h
 * 
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

/* rgb color in float format */
struct rgbf {
  float r; 
  float g; 
  float b; 
};

/* rgb color in integer format */
struct rgbi {
  unsigned char r; 
  unsigned char g; 
  unsigned char b;
};

/* convert rgb color from hexadecimal to rgb float rgbf */
void hex2rgbf(unsigned long c, struct rgbf *rgb); 

/* convert rgb color from hexadecimal to rgb integer rgbi */
void hex2rgbi(unsigned long c, struct rgbi *rgb);

/* make the colormap */
int make_colormap(const int nb_color_out, const unsigned long *color_in, const int nb_color_in, const char * filename );

void print_help(char* prog_name);

