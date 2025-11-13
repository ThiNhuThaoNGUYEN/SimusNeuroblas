// ****************************************************************************
//
//              View - Part of SiMuScale - Multi-scale simulation framework
//
// ****************************************************************************
//
// Copyright: See the AUTHORS file provided with the package
// E-mail: simuscale-contact@lists.gforge.inria.fr
// Original Authors : Samuel Bernard, Carole Knibbe, David Parsons
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU Affero General Public License as
// published by the Free Software Foundation, either version 3 of the
// License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Affero General Public License for more details.
//
// You should have received a copy of the GNU Affero General Public License
// along with this program.  If not, see <http://www.gnu.org/licenses/>.
//
// ****************************************************************************

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <getopt.h>
#include "colormap.h"

#define ansi_esc "\033"
#define bb "1"
#define ul "4"
#define plain ""
#define fmt(style) ansi_esc "[" style "m"

int _print_svg = 0;
char _orientation = 'h'; /* 'h':svg horizontal, 'v': svg vertical */

int main ( int argc, char *argv[] )
{

  /* Define allowed command-line options */
  const char * options_list = "hn:c:so:";
  static struct option long_options_list[] = {
    { "help",     no_argument, NULL, 'h'},
    { "nout",     required_argument, NULL, 'n' },
    { "colors",   required_argument, NULL, 'c' },
    { "svg",      no_argument, NULL, 's' },
    { "orientation", required_argument, NULL, 'o' },
    { 0, 0, 0, 0 }
  };

  unsigned long *color_in = NULL; 
  int nb_color_in = 0;
  int nb_color_out = 64; /* default colormap length */
  char *token;
  char *filename = NULL;

  /* Get actual values of the command-line options */
  int option;
  while ((option = getopt_long(argc, argv,
                               options_list, long_options_list, NULL)) != -1)
  {
    switch ( option )
    {
      case 'h' :
      {
        print_help( argv[0] );
        exit( EXIT_SUCCESS );
      }
      case 'n' :
      {
        nb_color_out = strtol(optarg,NULL,10);
        break;
      }
      case 'c' :
      {
        while ( ( token = strsep(&optarg,", ") ) != NULL )
        {
          color_in = realloc(color_in,(nb_color_in + 1)*sizeof(unsigned long));
          color_in[nb_color_in] = strtol(token,NULL,16);
          nb_color_in++;
        }
        break;
      }
      case 's' :
      {
        _print_svg = 1;
        break;
      }
      case 'o' :
        _orientation = *optarg;
        break;
      default:
        fprintf(stderr,"usage %s -n nb_col_out -c 'hexcolor1 hexcolor2 ...'\n",argv[0]);
    }
  }

  if ( nb_color_in == 0 )
  {
    fprintf(stderr,"usage %s -n nb_col_out -c 'hexcolor1 hexcolor2 ...'\n",argv[0]);
  }

  argc -= optind;
  argv += optind;

  if ( argc == 1 )
  {
    filename = argv[0];
  }

  int s = make_colormap(nb_color_out, color_in, nb_color_in, filename);

  free(color_in);
  return s;

}

int make_colormap(const int nb_color_out, const unsigned long *color_in, const int nb_color_in, const char * filename )
{
  struct rgbf *rgbf_in, rgbf;
  struct rgbi *rgbi_in, rgbi;
  int i, j;
  int segment_size = (nb_color_out - 1)/(nb_color_in - 1);
  int nb_segments = nb_color_in - 1;
  FILE *fsvg;
  FILE *fmap;

  unsigned int binwidth = 25;
  unsigned int rectwidth = 20;

  if ( ( nb_color_out - 1 ) % (nb_color_in - 1 ) )
  {
    fprintf(stderr,"warning: (nb_color_out - 1 = %d) not divisible by (nb_color_in - 1 = %d)\n",nb_color_out-1,nb_color_in-1);
  }

  rgbf_in = malloc(nb_color_in*sizeof(struct rgbf));
  rgbi_in = malloc(nb_color_in*sizeof(struct rgbi));
  for ( i = 0; i < nb_color_in; i++ )
  {
    hex2rgbf(color_in[i],rgbf_in + i);
    hex2rgbi(color_in[i],rgbi_in + i);
  }

  if ( filename != NULL )
  {
    if ( (fmap = fopen(filename,"w")) == NULL )
    {
      fprintf(stderr,"coud not open file %s\n",filename);
      return 1;
    }
  }
  else
  {
    fmap = stdout;
  }

  if ( _print_svg ) 
  {
    if ( (fsvg = fopen("colormap.svg","w")) == NULL )
    {
      fprintf(stderr,"coud not open file colormap.svg\n");
      return 1;
    }

    fprintf(fsvg,"<?xml version='1.0'?>\n");
    if ( _orientation == 'h' )
    {
      fprintf(fsvg,"<svg xmlns='%s' width='%u' height='%u' version='1.1'>\n", \
        "http://www.w3.org/2000/svg",binwidth*nb_color_out,50);
    }
    else
    {
      fprintf(fsvg,"<svg xmlns='%s' width='%u' height='%u' version='1.1'>\n", \
        "http://www.w3.org/2000/svg",50,binwidth*nb_color_out);
    }
  }

  /* print input colors in file header */
  fprintf(fmap,"#");
  for ( i = 0; i < nb_color_in; i++ )
  {
    fprintf(fmap," %#08lX", color_in[i]); // print with format 0XRRGGBB
  }
  fprintf(fmap,"\n");

  for ( i = 0; i < nb_segments; i++ )
  {
    for ( j = 0; j < segment_size; j++)
    {
      rgbf.r = (float)j/segment_size*rgbf_in[i+1].r + (1 - (float)j/segment_size)*rgbf_in[i].r;
      rgbf.g = (float)j/segment_size*rgbf_in[i+1].g + (1 - (float)j/segment_size)*rgbf_in[i].g;
      rgbf.b = (float)j/segment_size*rgbf_in[i+1].b + (1 - (float)j/segment_size)*rgbf_in[i].b;
      rgbi.r = j*rgbi_in[i+1].r/segment_size + (1*rgbi_in[i].r - j*rgbi_in[i].r/segment_size);
      rgbi.g = j*rgbi_in[i+1].g/segment_size + (1*rgbi_in[i].g - j*rgbi_in[i].g/segment_size);
      rgbi.b = j*rgbi_in[i+1].b/segment_size + (1*rgbi_in[i].b - j*rgbi_in[i].b/segment_size);
      fprintf(fmap,"%g %g %g\n", rgbf.r, rgbf.g, rgbf.b);

      if ( _print_svg ) 
      {
        if ( _orientation == 'h' )
        {
          fprintf(fsvg,"  <rect x='%u' y='%u' width='%u' height='50' r='1.2' fill='rgb(%u,%u,%u)'/>\n",
            segment_size*i*binwidth + binwidth*j, 0, rectwidth, rgbi.r, rgbi.g, rgbi.b);
        }
        else 
        {
          fprintf(fsvg,"  <rect x='%u' y='%u' width='50' height='%u' r='1.2' fill='rgb(%u,%u,%u)'/>\n",
            0, binwidth*(nb_color_out - 1) - segment_size*i*binwidth - binwidth*j, rectwidth, rgbi.r, rgbi.g, rgbi.b);
        }
      }
    }

  }
  rgbf.r = rgbf_in[nb_segments].r;
  rgbf.g = rgbf_in[nb_segments].g;
  rgbf.b = rgbf_in[nb_segments].b;
  rgbi.r = rgbi_in[nb_segments].r;
  rgbi.g = rgbi_in[nb_segments].g;
  rgbi.b = rgbi_in[nb_segments].b;
  fprintf(fmap,"%g %g %g\n", rgbf.r, rgbf.g, rgbf.b);
  if ( _print_svg ) 
  {
    if ( _orientation == 'h' )
    {
      fprintf(fsvg,"  <rect x='%u' y='%u' width='%u' height='50' r='1.2' fill='rgb(%u,%u,%u)'/>\n",
        binwidth*(nb_color_out - 1), 0, rectwidth, rgbi.r, rgbi.g, rgbi.b);
    }
    else
    {
      fprintf(fsvg,"  <rect x='%u' y='%u' width='50' height='%u' r='1.2' fill='rgb(%u,%u,%u)'/>\n",
        0, 0, rectwidth, rgbi.r, rgbi.g, rgbi.b);
    }
    fprintf(fsvg, "</svg>\n");
  }

  free(rgbf_in);
  free(rgbi_in);
  if ( _print_svg ) 
  {
    fclose(fsvg);
  }
  fclose(fmap);

  return 0;

}

void hex2rgbf(unsigned long c, struct rgbf *rgb)
{
  unsigned char crgb[3];
  crgb[0] = c >> 16;
  crgb[1] = (c >> 8) % 0x100;
  crgb[2] = c % 0x100;
  rgb->r = (float)crgb[0]/255;
  rgb->g = (float)crgb[1]/255;
  rgb->b = (float)crgb[2]/255;
}

void hex2rgbi(unsigned long c, struct rgbi *rgb)
{
  rgb->r = c >> 16;
  rgb->g = (c >> 8) % 0x100;
  rgb->b = c % 0x100;
}

/* colormap.c
 * 
 *  gcc -g colormap.c -o colormap
 *  ./colormap -n <nbcolors> -c '0x003355 0x00BB66 0x33CCCC 0xDD77EE 0xFFFE22'
 *  rsvg-convert colormap.svg| imgcat
 * 
 *  ./colormap -c '0x003355 0x00BB66 0x33CCCC 0xDD77EE 0xFFFE22' >colormap.txt
 *  awk '{ printf "_colormap[%d][0] = %f; _colormap[%d][1] = %f; _colormap[%d][2] = %f;\n", NR-1, $1, NR-1, $2, NR-1, $3}' colormap.txt | pbcopy
 *
 */


void print_help( char* prog_name )
{
  printf( "\n\
Usage : " fmt(bb) "%s" fmt(plain) " -h\n\
   or : " fmt(bb) "%s" fmt(plain) " [options] " fmt(ul) "file" fmt(plain) "\n\n\
Create an RGB colormap and print it to \
" fmt(ul) "file" fmt(plain) ". \n\
If " fmt(ul) "file" fmt(plain) " argument is missing, the colormap is printed to standard output.\n\
\n\
\tOptions\n\n\
\t" fmt(bb) "-h" fmt(plain) " or " fmt(bb) "--help" fmt(plain) "\tDisplay this screen\n\
\t" fmt(bb) "-n" fmt(plain) " or " fmt(bb) "--nout" fmt(plain) " " fmt(ul) "length" fmt(plain) "\tlength of colormap (defaul: 64)\n\
\t" fmt(bb) "-c" fmt(plain) " or " fmt(bb) "--colors" fmt(plain) " \
" fmt(ul) "'hexcolor1 hexcolor2 ...'" fmt(plain) " \n\
\t                   Specify colors (mandatory option)\n\
\t" fmt(bb) "-s" fmt(plain) " or " fmt(bb) "--svg" fmt(plain)  "\talso print colormap in svg format. Use the command\n\
\t                   rsvg-convert colormap.svg| imgcat\n\
\t                   to print an image of the colormap.\n\n\
\tExample\n\n\
\t%s -n 25 -c '0x004499 0xFF4499 0xFF9944' --svg colormap.txt\n\
\trsvg-convert colormap.svg| imgcat\n\
\n", prog_name, prog_name, prog_name );

}
