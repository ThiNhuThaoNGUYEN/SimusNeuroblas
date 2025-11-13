// ****************************************************************************
//
//              Populate - Part of SiMuScale - Multi-scale simulation framework
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


#define GL_GLEXT_PROTOTYPES
#define GL_SILENCE_DEPRECATION

#ifdef __linux__
#include <GL/glut.h>
#endif

#if defined(__APPLE__) && defined(__MACH__) 
#include <GLUT/glut.h>
#endif

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <getopt.h>

#define DEFAULT_SIGNAL -1 
#define SIGNAL_NOT_FOUND -1
#define MAX_SIG_STRLEN 16
#define MAX_NBR_SIGNAL 1024

#define ansi_esc "\033"
#define bb "1"
#define ul "4"
#define plain ""
#define fmt(style) ansi_esc "[" style "m"
#define num_opt fmt(plain)fmt(ul) "num" fmt(plain)
#define name_opt fmt(plain)fmt(ul) "name" fmt(plain)
#define file_opt fmt(plain)fmt(ul) "file" fmt(plain)
#define options_opt fmt(plain)fmt(ul) "options" fmt(plain)
#define list_opt fmt(plain)fmt(ul) "list" fmt(plain)

#define S_SLICES 32
#define S_STACKS 32

#define MODE_ADD_BULK         0x01
#define MODE_REMOVE           0x02
#define MODE_PLANE            0x04
#define MODE_WORLD_BORDER     0x08
#define MODE_ADD_BULK_SELECT  0x10

#define NBR_SPHEROID_IN       64

#define DELTA_ANGLE           0.05

/******* structures *******/

struct scell_sphere
{
  int id;
  float position[3];
  float color[3];
  float radius;
  int   colorindex;
};
typedef struct scell_sphere cell_sphere;

struct worldsize {
  float x;
  float y;
  float z;
};

/******* functions *******/

void init();
void init_graphical_interface();
void init_colormap();
void read_file();
void display();
void reshape (int w, int h);
void reset_view();
void polar_view( GLfloat distance, GLfloat azimuth, GLfloat incidence, GLfloat twist);
void keyboard(unsigned char key, int x, int y);
void special_keyboard(int key, int x, int y);
void mouse_on_plane( GLint x, GLint y);
void saveimage();
int add_cell(GLint x, GLint y);
void add_bulk();
void add_spheroid(int i);
void draw_bulk_area();
void draw_mouse_shadow();
int remove_cell(GLint x, GLint y);
void remove_cell_index(int to_remove);
int move_plane(GLint x, GLint y);
GLvoid mouse( GLint button, GLint state, GLint x, GLint y );
GLvoid motion( GLint x, GLint y );
GLvoid passive_motion( GLint x, GLint y );
void print_help(char* prog_name);
void on_exit();

/************ constants *************/
GLubyte _raster_rect[72] = { 0x00, 0x00, 0x00, 
                              0x00, 0x00, 0x00, 
                              0x0f, 0xff, 0xf0, 
                              0x1f, 0xff, 0xf8, 
                              0x3f, 0xff, 0xfc, 
                              0x3f, 0xff, 0xfc, 
                              0x3f, 0xff, 0xfc, 
                              0x3f, 0xff, 0xfc, 
                              0x3f, 0xff, 0xfc, 
                              0x3f, 0xff, 0xfc, 
                              0x3f, 0xff, 0xfc, 
                              0x3f, 0xff, 0xfc, 
                              0x3f, 0xff, 0xfc, 
                              0x3f, 0xff, 0xfc, 
                              0x3f, 0xff, 0xfc, 
                              0x3f, 0xff, 0xfc, 
                              0x3f, 0xff, 0xfc, 
                              0x3f, 0xff, 0xfc, 
                              0x3f, 0xff, 0xfc, 
                              0x3f, 0xff, 0xfc, 
                              0x1f, 0xff, 0xf8, 
                              0x0f, 0xff, 0xf0, 
                              0x00, 0x00, 0x00, 
                              0x00, 0x00, 0x00};
GLubyte _raster_border[72] = {0x0f, 0xff, 0xf0, 
                               0x3f, 0xff, 0xfc, 
                               0x70, 0x00, 0x0e, 
                               0xe0, 0x00, 0x07, 
                               0xc0, 0x00, 0x03, 
                               0xc0, 0x00, 0x03, 
                               0xc0, 0x00, 0x03, 
                               0xc0, 0x00, 0x03, 
                               0xc0, 0x00, 0x03, 
                               0xc0, 0x00, 0x03, 
                               0xc0, 0x00, 0x03, 
                               0xc0, 0x00, 0x03, 
                               0xc0, 0x00, 0x03, 
                               0xc0, 0x00, 0x03, 
                               0xc0, 0x00, 0x03, 
                               0xc0, 0x00, 0x03, 
                               0xc0, 0x00, 0x03, 
                               0xc0, 0x00, 0x03, 
                               0xc0, 0x00, 0x03, 
                               0xc0, 0x00, 0x03, 
                               0xe0, 0x00, 0x07, 
                               0x70, 0x00, 0x0e, 
                               0x3f, 0xff, 0xfc, 
                               0x0f, 0xff, 0xf0};

