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

#define GL_GLEXT_PROTOTYPES
#define GL_SILENCE_DEPRECATION

#ifdef __linux__
#include <GL/glut.h>
#endif

#if defined(__APPLE__) && defined(__MACH__) 
#include <GLUT/glut.h>
#endif

#define ansi_esc "\033"
#define bb "1"
#define ul "4"
#define plain ""
#define fmt(style) ansi_esc "[" style "m"
#define num_opt fmt(plain)fmt(ul) "num" fmt(plain)
#define name_opt fmt(plain)fmt(ul) "name" fmt(plain)
#define file_opt fmt(plain)fmt(ul) "file" fmt(plain)
#define options_opt fmt(plain)fmt(ul) "options" fmt(plain)

#define DEFAULT_SIGNAL -2 
#define SIGNAL_NOT_FOUND -1
#define MAX_SIG_STRLEN 16
#define MAX_NBR_SIGNAL 1024
#define INITIAL_TS_RANGE 10.0f

#define PR_BS_CLEAR printf("\033[D\033[J")

#define S_SLICES 10
#define S_STACKS 10


/******* structures *******/

struct scell_sphere
{
  int id;
  float position[3];
  float orientation[3];
  float color[4];
  char  cell_tag[32];
  char  living_status;
  float signal;
  float radius;
  int   clip;
};
typedef struct scell_sphere cell_sphere;

struct worldsize {
  float x;
  float y;
  float z;
};

struct striangle
{
  GLfloat v1[3];
  GLfloat v2[3];
  GLfloat v3[3];
  GLfloat normal[3];
};
typedef struct striangle triangle;

struct stimeseries {
  GLfloat *t;
  GLfloat *y;
  GLfloat *y2;
  GLuint  *step;
  GLfloat tmin;
  GLfloat tmax;
  GLfloat ymin;
  GLfloat ymax;
  size_t  size;
  size_t  maxsize;
  size_t  id;
};
typedef struct stimeseries timeseries;

struct srgbacolor {
  GLfloat c[4];
};
typedef struct srgbacolor rgbacolor;

/******* enums *******/

enum actions { MOVE_EYE, TWIST_EYE, ZOOM, MOVE_NONE, MOVE_PLANE, MOVE_PLANE_FINE };
enum guides { GUIDE_NONE, GUIDE_BORDER, GUIDE_VIEWCUT, GUIDE_BORDER_VIEWCUT };
enum color_models { COLOR_MODEL_NORMAL, COLOR_MODEL_INVERSE, COLOR_MODEL_FLAT };
enum stencils { STENCIL_BACKGROUND, STENCIL_BORDER, STENCIL_LEGEND, STENCIL_CELL, STENCIL_TRACK, STENCIL_HOOVER, STENCIL_TAG };

/******* functions *******/

void init_color_map_spring();
void init_color_map_jet();
void init_color_map_neon();
void init_color_map_custom(char *filename);
void read_min_max_signals();
void read_header();
void read_file_line();
void animate(int value);
void init();
void display();
void drawcell(GLint i);
void auxdisplay();
void statsdisplay();
void draw_ts(timeseries *ts, size_t current);
void reshape (int w, int h);
void reset_view();
void polar_view( GLfloat distance, GLfloat azimuth, GLfloat incidence, GLfloat twist);
void keyboard(unsigned char key, int x, int y);
void special_keyboard(int key, int x, int y);
void get_keyboard_input(unsigned char key);
void next_signal();
void previous_signal();
void standstill();
void backtrack( unsigned int nbrframes);
void darken( float *color, float amount );
void lighten( float *color, float amount );
GLfloat distsq3v( GLfloat *p1, GLfloat *p2);
GLfloat normsq3v( GLfloat *v);
void assign3v(const GLfloat *source, size_t size_source, GLfloat *target, size_t size_target);
void copy_triangle(const triangle source, triangle *target);
void rotate3v(const GLfloat *axis, const GLfloat *angle, GLfloat *points, const size_t nbr_points);
void translate3v(const GLfloat *shift, GLfloat *points, size_t nbr_points);
void normal3v( GLfloat *p1, GLfloat *p2, GLfloat *p3, GLfloat *nv );
void dead_cell_geometry();
void triangularize(triangle **trgl, size_t nbr_triangles);
void draw_plane();
void move_plane(GLint x, GLint y);
void save_image();
void print_snapshot();
int select_cell(GLint x, GLint y);
int hoover_cell(GLint x, GLint y);
GLvoid mouse( GLint button, GLint state, GLint x, GLint y );
GLvoid motion( GLint x, GLint y );
GLvoid passive_motion( GLint x, GLint y );
void print_help(char* prog_name);
void on_exit();

/* constants */
const GLfloat _white_light[] = {1.0, 1.0, 1.0, 1.0};


