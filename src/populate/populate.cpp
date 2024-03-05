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

#include <GLUT/glut.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <getopt.h>

#define DEFAULT_SIGNAL -1 
#define SIGNAL_NOT_FOUND -1
#define MAX_SIG_STRLEN 16
#define MAX_NBR_SIGNAL 1024

#define PR_BS_CLEAR printf("\033[D\033[J")

#define S_SLICES 12
#define S_STACKS 12

inline GLfloat sign(GLfloat x) 
{
  if ( x > 0.0f ) return 1.0f;
  if ( x < 0.0f ) return -1.0f;
  return 0.0f;
}

inline void clamp(GLfloat *v, GLfloat vmin, GLfloat vmax)
{
  if (*v > vmax)
  {
    *v = vmax;
  }
  else if ( *v < vmin )
  {
    *v = vmin;
  }
}

struct sCellSphere
{
  int id;
  float position[3];
  float color[3];
  float radius;
  int   colorindex;
};
typedef struct sCellSphere CellSphere;

struct worldsize {
  float x;
  float y;
  float z;
};

GLfloat _colormap[64][3];
const int _colormap_size = 5;
GLfloat _min_signal = 0.0, _max_signal = 100.0;
GLfloat _min_clip_signal = -INFINITY, _max_clip_signal = INFINITY;
GLfloat _font_color[3] = {1.0, 1.0, 1.0}; /* default font color is white */
GLfloat _x_plane = 1.0 , _y_plane = 1.0 , _z_plane = 1.0;
int _pos_plane = 2; /* 0: x, 1: y, 2: z */
GLfloat _radius = 1.0;
GLdouble _mouse_x, _mouse_y, _mouse_z;
int _move_plane = 0;
int _remove_cell = 0;
int _within_world = 0;
GLfloat _mouse_shadow_color[4] = {0.0,0.0,0.0,0.3};
GLfloat _mouse_overlay_color[4] = {0.8,0.8,0.8,0.8};
GLfloat _positioning_plane_color[3][4] = {{1.0, 0.9, 0.3, 0.3},
                                          {0.3, 1.0, 0.9, 0.3},
                                          {0.9, 0.3, 1.0, 0.3}}; 
int _cell_to_remove;
int _current_color = 0;
FILE * _init_file = NULL;
FILE * logFile  = NULL;
char _write_mode[3] = "w";
struct worldsize _worldsize = {0.0, 0.0, 0.0};
CellSphere * _mycells = NULL;
int _popsize = 0;
int _mycellscap = 0;
int _line = 0;
int _background_white = 0;
int _clipcell = 0;
int _draw_world_border = 1;
char _inputstr[16];
char * _init_filename;
GLuint _SphereDList;
enum actions { MOVE_EYE, TWIST_EYE, ZOOM, MOVE_NONE, MOVE_PLANE };
GLint _action;
GLdouble _xStart = 0.0, _yStart = 0.0;
GLfloat _nearClip, _farClip, _distance, _twistAngle, _incAngle, _azimAngle;
GLfloat _fovy = 45.0;

void init();
void read_file();
void display();
void reshape (int w, int h);
void resetView();
void polarView( GLfloat distance, GLfloat azimuth, GLfloat incidence, GLfloat twist);
void keyboard(unsigned char key, int x, int y);
void saveimage();
int add_cell(GLint x, GLint y);
int remove_cell(GLint x, GLint y);
int move_plane(GLint x, GLint y);
GLvoid mouse( GLint button, GLint state, GLint x, GLint y );
GLvoid motion( GLint x, GLint y );
GLvoid passive_motion( GLint x, GLint y );
void print_help(char* prog_name);
void on_exit();

void init_colormap()
{
  _colormap[0][0] = 1.000000; _colormap[0][1] = 0.200000; _colormap[0][2] = 0.000000;
  _colormap[1][0] = 1.000000; _colormap[1][1] = 0.666667; _colormap[1][2] = 0.000000;
  _colormap[2][0] = 0.019608; _colormap[2][1] = 0.800000; _colormap[2][2] = 0.200000;
  _colormap[3][0] = 0.000000; _colormap[3][1] = 0.666667; _colormap[3][2] = 1.000000;
  _colormap[4][0] = 0.666667; _colormap[4][1] = 0.200000; _colormap[4][2] = 1.000000;
}

int main(int argc, char** argv)
{

  /* Define allowed command-line options */
  const char * options_list = "hf:e";
  static struct option long_options_list[] = {
    { "help",     no_argument, NULL, 'h' },
    { "file",     required_argument, NULL, 'f' },
    { "edit",   no_argument, NULL, 'e' },
    { 0, 0, 0, 0 }
  };

  /* Get actual values of the command-line options */
  int option;

  /* default file name */
  _init_filename = (char *) malloc(12*sizeof(char));
  sprintf(_init_filename,"initpop.txt");

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
      case 'f' :
      {
        if ( strcmp( optarg, "" ) == 0 )
        {
          fprintf(stderr, "ERROR : Option -f or --file : missing argument.\n" );
          exit( EXIT_FAILURE );
        }

        _init_filename = (char *) realloc(_init_filename,(strlen(optarg) + 1)*sizeof(char));
        sprintf( _init_filename, "%s", optarg );
        break;
      }
      case 'e':
        snprintf(_write_mode,3,"r+");
        break;
    }
  }

  glutInit(&argc, argv);
  glutInitDisplayMode (GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH | GLUT_STENCIL | GLUT_MULTISAMPLE);
  glutInitWindowSize (500, 500);
  glutInitWindowPosition (100, 100);
  glutCreateWindow ( "Simuscale - Populate cells" );
  init ();

  glutDisplayFunc(display);
  glutReshapeFunc(reshape);
  glutKeyboardFunc(keyboard);
  glutMouseFunc(mouse);
  glutMotionFunc(motion);
  glutPassiveMotionFunc(passive_motion);

  atexit(on_exit);

  glutPostRedisplay();
  glutMainLoop();

  return 0;
}


void init(void)
{

  init_colormap();
  
  _worldsize.x = 40;
  _worldsize.y = 40;
  _worldsize.z = 40;

  _mycells = (CellSphere*)malloc(sizeof(CellSphere));
  _mycellscap = 1;

  glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

   if ( _background_white )
     glClearColor (1.0, 1.0, 1.0, 1.0);
   else
     glClearColor (0.0, 0.0, 0.0, 0.0);

   glShadeModel (GL_SMOOTH);
   /* glShadeModel (GL_FLAT); */

   /* color of the light */
   GLfloat white_light[] = {1.0, 1.0, 1.0, 1.0};
   glLightfv(GL_LIGHT0, GL_AMBIENT, white_light);
   glLightfv(GL_LIGHT0, GL_DIFFUSE, white_light);
   glLightfv(GL_LIGHT0, GL_SPECULAR, white_light);

   /* position of the light */
   GLfloat light_position[] = {0.0, 0.0, 0.0, 1.0}; /* will move with the viewpoint */
   /* w=0 thus parallel rays like the sun, called directional light source */
   /* The direction (along z axis here) is transformed by the current modelview matrix */
   /* There is no attenuation for a directional light source */
   glLightfv(GL_LIGHT0, GL_POSITION, light_position);

   /* lighting model */
   GLfloat global_ambient_light[] = {0.3, 0.3, 0.3, 1.0}; /* to see objects even if no light source */
   glLightModelfv(GL_LIGHT_MODEL_AMBIENT, global_ambient_light);
   glLightModeli(GL_LIGHT_MODEL_LOCAL_VIEWER, GL_FALSE); /* infinite viewpoint */
   glLightModeli(GL_LIGHT_MODEL_TWO_SIDE, GL_FALSE); /* back faces are inside the spheres, never seen */

   /* material for the objects */
   GLfloat mat_specular[] = {0.0, 0.0, 0.0, 1.0};
   GLfloat mat_ambient_refl[] = {0.2, 0.2, 0.2, 1.0};
   GLfloat mat_shininess[] = {0.0};
   GLfloat mat_diffuse_color[] = {0.5, 0.5, 0.5, 1.0};
   // GLfloat mat_emission[] = {0.05, 0.05, 0.05, 0.0};
   glMaterialfv(GL_FRONT, GL_AMBIENT, mat_ambient_refl);
   glMaterialfv(GL_FRONT, GL_SPECULAR, mat_specular);
   glMaterialfv(GL_FRONT, GL_SHININESS, mat_shininess);
   glMaterialfv(GL_FRONT, GL_DIFFUSE, mat_diffuse_color);
   // glMaterialfv(GL_FRONT, GL_EMISSION, mat_emission);
   glColorMaterial(GL_FRONT, GL_DIFFUSE); /* now glColor changes diffusion color */

   glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);

   glEnable(GL_LIGHT0);
   glEnable(GL_BLEND);
   glEnable(GL_RESCALE_NORMAL);
   glEnable(GL_DEPTH_TEST);
   glEnable(GL_COLOR_MATERIAL);
   glEnable(GL_MULTISAMPLE);

   /* Enable stencil operations */
   glEnable(GL_STENCIL_TEST);
   glStencilOp(GL_KEEP, GL_KEEP, GL_REPLACE);

   /* Set up nearClip and farClip so that ( farClip - nearClip ) > maxObjectSize, */
   /* and determine the viewing distance (adjust for zooming) */
   _nearClip = 0.1*_worldsize.x;
   _farClip = _nearClip + 4.0*_worldsize.x;
   resetView();

   /* display list to store the geometry of a sphere */
   _SphereDList = glGenLists(1);
   glNewList(_SphereDList, GL_COMPILE);
   glutSolidSphere(1.0, S_SLICES, S_STACKS);
   glEndList();

   if ( ( logFile = fopen("populate.log","w") ) == NULL )
   {
     fprintf(stderr, "Error, could not open populate.log.\n");
     exit(EXIT_FAILURE);
   }

   if ( _write_mode[0] == 'r' ) 
   { /* try reading existing data */ 
     read_file();
   }

}

void read_file()
{
  int id;
  float x,y,z;
  float r;

  if ( ( _init_file = fopen(_init_filename, _write_mode) ) == NULL)
  {
    fprintf(stderr, "Error, file %s is missing.\n", _init_filename);
    exit(EXIT_FAILURE);
  }
  while ( fscanf(_init_file,"%d %f %f %f %f %d\n",&id,&x,&y,&z,&r,&_current_color) != EOF )
  {
    if ( _mycellscap <= _popsize )
    {
      _mycells = (CellSphere*)realloc(_mycells,2*_mycellscap*sizeof(CellSphere));
      _mycellscap *= 2;    
    }

    _mycells[_popsize].id = id;
    _mycells[_popsize].position[0] = x; 
    _mycells[_popsize].position[1] = y; 
    _mycells[_popsize].position[2] = z; 
    _mycells[_popsize].radius = r;
    _mycells[_popsize].color[0] = _colormap[_current_color][0]; 
    _mycells[_popsize].color[1] = _colormap[_current_color][1]; 
    _mycells[_popsize].color[2] = _colormap[_current_color][2];
    _mycells[_popsize].colorindex = _current_color;
    _popsize++;
    
  }
  fclose(_init_file);

}

void display(void)
{

  char legend_string[128];
  /* GLubyte raster_rect[16] = {0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff}; */
  GLubyte raster_rect[32] = {0x00, 0x00, 0x3f, 0xfc, 0x3f, 0xfc, 0x3f, 0xfc, 0x3f, 0xfc, 0x3f, 0xfc, 0x3f, 0xfc, 0x3f, 0xfc, 0x3f, 0xfc, 0x3f, 0xfc, 0x3f, 0xfc, 0x3f, 0xfc, 0x3f, 0xfc, 0x3f, 0xfc, 0x3f, 0xfc, 0x00, 0x00};
  GLubyte raster_border[32] = {0xff, 0xff, 0xc0, 0x03, 0xc0, 0x03, 0xc0, 0x03, 0xc0, 0x03, 0xc0, 0x03, 0xc0, 0x03, 0xc0, 0x03, 0xc0, 0x03, 0xc0, 0x03, 0xc0, 0x03, 0xc0, 0x03, 0xc0, 0x03, 0xc0, 0x03, 0xc0, 0x03, 0xff, 0xff};
  /* GLubyte raster_vert[2] = {0xc0, 0x03}; */
  /* GLubyte raster_horz[2] = {0xff, 0xff}; */

  /***** Clear color buffer, depth buffer, stencil buffer *****/
  glClearStencil(0); /* default value */
  glClear (GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT );

  /*************** Draw the text and legends ******************/
  sprintf(legend_string, "r=%.2f, x=%.2f, y=%.2f, z=%.2f %s %s", _radius,_mouse_x,_mouse_y,_mouse_z,_move_plane ? "move plane" : "", _remove_cell ? "remove" : ""); 
  glColor3fv (_font_color);
  glWindowPos2i(15, 15); /* also sets current raster color to white */
  for (size_t i = 0; i < strlen(legend_string); i++) glutBitmapCharacter(GLUT_BITMAP_HELVETICA_12, legend_string[i]);


  glColor3fv(_colormap[_current_color]);
  glWindowPos2i(300, 10);
  glBitmap(16, 16, 0.0, 0.0, 0, 0, raster_rect);
  glColor3f(1.0,1.0,1.0);
  glWindowPos2i(300, 10);
  glBitmap(16, 16, 0.0, 0.0, 0, 0, raster_border);
  sprintf(legend_string, "color index=%d",_current_color); 
  glColor3fv (_font_color);
  glWindowPos2i(320, 15); /* also sets current raster color to white */
  for (size_t i = 0; i < strlen(legend_string); i++) glutBitmapCharacter(GLUT_BITMAP_HELVETICA_12, legend_string[i]);

  /*********************** Set the view **************************/
  glLoadIdentity ();             /* clear the matrix */
  /* viewing transformation  */
  /* gluLookAt (-1.5*_worldsize, -1.5*_worldsize, 1.5*_worldsize, _worldsize, _worldsize, 0.0, _worldsize, _worldsize, 1.5*_worldsize); */
  polarView( _distance, _azimAngle, _incAngle, _twistAngle );

  /******************** Draw the world borders ********************/
  glColor3f (1.0, 1.0, 1.0); // White
  glLineWidth(1.0);
  glPolygonMode(GL_FRONT_AND_BACK, GL_LINE); // Mesh

  float world_size[3] = {_worldsize.x, _worldsize.y, _worldsize.z};
  float world_margins[3] = {2.0, 2.0, 0.0};
  float total_world_size[3] = {world_size[0] + 2 * world_margins[0],
                               world_size[1] + 2 * world_margins[1],
                               world_size[2] + 2 * world_margins[2]};

  if ( _draw_world_border )
  {
    // Draw outer world borders
    glBegin(GL_QUAD_STRIP);
    glVertex3d(-world_margins[0], -world_margins[1], -world_margins[2]);
    glVertex3d(-world_margins[0], -world_margins[1] + total_world_size[1], -world_margins[2]);
    glVertex3d(-world_margins[0] + total_world_size[0], -world_margins[1], -world_margins[2]);
    glVertex3d(-world_margins[0] + total_world_size[0], -world_margins[1] + total_world_size[1], -world_margins[2]);
    glVertex3d(-world_margins[0] + total_world_size[0], -world_margins[1], -world_margins[2] + total_world_size[2]);
    glVertex3d(-world_margins[0] + total_world_size[0], -world_margins[1] + total_world_size[1], -world_margins[2] + total_world_size[2]);
    glVertex3d(-world_margins[0], -world_margins[1], -world_margins[2] + total_world_size[2]);
    glVertex3d(-world_margins[0], -world_margins[1] + total_world_size[1], -world_margins[2] + total_world_size[2]);
    glVertex3d(-world_margins[0], -world_margins[1], -world_margins[2]);
    glVertex3d(-world_margins[0], -world_margins[1] + total_world_size[1], -world_margins[2]);
    glEnd();

    // Draw inner world borders
    glBegin(GL_QUAD_STRIP);
    glVertex3d(0, 0, 0);
    glVertex3d(0, world_size[1], 0);
    glVertex3d(world_size[0], 0, 0);
    glVertex3d(world_size[0], world_size[1], 0);
    glVertex3d(world_size[0], 0, world_size[2]);
    glVertex3d(world_size[0], world_size[1], world_size[2]);
    glVertex3d(0, 0, world_size[2]);
    glVertex3d(0, world_size[1], world_size[2]);
    glVertex3d(0, 0, 0);
    glVertex3d(0, world_size[1], 0);
    glEnd();
  }
  glPolygonMode(GL_FRONT_AND_BACK, GL_FILL); // Back to fill mode

  /******************* Draw the cells ***************************/
  glEnable(GL_LIGHTING);
  for (int i = 0; i < _popsize; i++)
  {

    /**** draw cell i *****/
    glPushMatrix(); /* save V */
    glColor3fv(_mycells[i].color);
    glTranslatef(_mycells[i].position[0], _mycells[i].position[1], _mycells[i].position[2]); /* current matrix is VT  */
    glScalef(_mycells[i].radius, _mycells[i].radius, _mycells[i].radius ); /* current matrix is VTS */
    /* glCallList(_SphereDList); */
    glutSolidSphere(1.0, S_SLICES, S_STACKS);
    glPopMatrix(); /* current matrix is restored to V */

  }
  glDisable(GL_LIGHTING);

  /******************* Draw positioning plane *******************/
  glBegin(GL_QUADS);
  if ( _pos_plane == 0 ) /* x-plane x = _x_plane */ 
  {
    glColor4fv (_positioning_plane_color[0]); 
    glVertex3d(_x_plane, -world_margins[1], -world_margins[2]);
    glVertex3d(_x_plane, -world_margins[1], -world_margins[2] + total_world_size[2]);
    glVertex3d(_x_plane, -world_margins[1] + total_world_size[1], -world_margins[2] + total_world_size[2]);
    glVertex3d(_x_plane, -world_margins[1] + total_world_size[1], -world_margins[2]);
  }
  else if ( _pos_plane == 1 ) /* y-plane y = _y_plane */
  {
    glColor4fv (_positioning_plane_color[1]); 
    glVertex3d(-world_margins[0], _y_plane, -world_margins[2]);
    glVertex3d(-world_margins[0], _y_plane, -world_margins[2] + total_world_size[2]);
    glVertex3d(-world_margins[0] + total_world_size[0], _y_plane, -world_margins[2] + total_world_size[2]);
    glVertex3d(-world_margins[0] + total_world_size[0], _y_plane, -world_margins[2]);
  }
  else if ( _pos_plane == 2 ) /* z-plane z = _z_plane */
  {
    glColor4fv (_positioning_plane_color[2]); 
    glVertex3d(-world_margins[0], -world_margins[1], _z_plane);
    glVertex3d(-world_margins[0] + total_world_size[0], -world_margins[1], _z_plane);
    glVertex3d(-world_margins[0] + total_world_size[0], -world_margins[1] + total_world_size[1], _z_plane);
    glVertex3d(-world_margins[0], -world_margins[1] + total_world_size[1], _z_plane);


  }
  glEnd();


  /* square at mouse position */
  if ( _within_world ) /* if mouse points to coordinate inside world */ 
  {
    glDepthMask(GL_FALSE);
    glBegin(GL_QUADS);
    if ( _pos_plane == 0 ) /* x-plane x = _x_plane */ 
    {
      glColor4fv (_mouse_overlay_color);
      glVertex3d(_mouse_x + 0.15, _mouse_y - 0.8, _mouse_z - 0.8); 
      glVertex3d(_mouse_x + 0.15, _mouse_y - 0.8, _mouse_z + 0.8); 
      glVertex3d(_mouse_x + 0.15, _mouse_y + 0.8, _mouse_z + 0.8); 
      glVertex3d(_mouse_x + 0.15, _mouse_y + 0.8, _mouse_z - 0.8); 
      glColor4fv (_mouse_shadow_color);
      glVertex3d(_x_plane + 0.1, _mouse_y - 0.8, _mouse_z - 0.8);
      glVertex3d(_x_plane + 0.1, _mouse_y - 0.8, _mouse_z + 0.8);
      glVertex3d(_x_plane + 0.1, _mouse_y + 0.8, _mouse_z + 0.8);
      glVertex3d(_x_plane + 0.1, _mouse_y + 0.8, _mouse_z - 0.8);
    }
    else if ( _pos_plane == 1 ) /* y-plane y = _y_plane */
    {
      glColor4fv (_mouse_overlay_color);
      glVertex3d(_mouse_x - 0.8, _mouse_y - 0.15, _mouse_z - 0.8); 
      glVertex3d(_mouse_x - 0.8, _mouse_y - 0.15, _mouse_z + 0.8); 
      glVertex3d(_mouse_x + 0.8, _mouse_y - 0.15, _mouse_z + 0.8); 
      glVertex3d(_mouse_x + 0.8, _mouse_y - 0.15, _mouse_z - 0.8); 
      glColor4fv (_mouse_shadow_color);
      glVertex3d(_mouse_x - 0.8, _y_plane - 0.1, _mouse_z - 0.8);
      glVertex3d(_mouse_x - 0.8, _y_plane - 0.1, _mouse_z + 0.8);
      glVertex3d(_mouse_x + 0.8, _y_plane - 0.1, _mouse_z + 0.8);
      glVertex3d(_mouse_x + 0.8, _y_plane - 0.1, _mouse_z - 0.8);
    }
    else if ( _pos_plane == 2 ) /* z-plane z = _z_plane */
    {
      glColor4fv (_mouse_overlay_color);
      glVertex3d(_mouse_x - 0.8, _mouse_y - 0.8, _mouse_z + 0.15); 
      glVertex3d(_mouse_x - 0.8, _mouse_y + 0.8, _mouse_z + 0.15); 
      glVertex3d(_mouse_x + 0.8, _mouse_y + 0.8, _mouse_z + 0.15); 
      glVertex3d(_mouse_x + 0.8, _mouse_y - 0.8, _mouse_z + 0.15); 
      glColor4fv (_mouse_shadow_color);
      glVertex3d(_mouse_x - 0.8, _mouse_y - 0.8, _z_plane + 0.1);
      glVertex3d(_mouse_x - 0.8, _mouse_y + 0.8, _z_plane + 0.1);
      glVertex3d(_mouse_x + 0.8, _mouse_y + 0.8, _z_plane + 0.1);
      glVertex3d(_mouse_x + 0.8, _mouse_y - 0.8, _z_plane + 0.1);
    }
    glEnd();
    glDepthMask(GL_TRUE);
  }

  glutSwapBuffers();
}

void reshape (int w, int h)
{
  glViewport (0, 0, (GLsizei) w, (GLsizei) h);
  glMatrixMode (GL_PROJECTION);
  glLoadIdentity ();
  gluPerspective( _fovy, (1.0*w)/h, _nearClip, _farClip );
  glMatrixMode (GL_MODELVIEW);
}


void resetView()
{
  _distance = _nearClip + (_farClip - _nearClip) / 2.0;
  _twistAngle = 0.0;	/* rotation of viewing volume (camera) */
  _incAngle = 85.0;
  _azimAngle = 30.0;
}


void polarView( GLfloat distance, GLfloat azimuth, GLfloat incidence, GLfloat twist)
{
  glTranslatef( -_worldsize.x/2.0, -_worldsize.y/2.0, -distance);
  glRotatef( -twist, 0.0f, 0.0f, 1.0);
  glRotatef( -incidence, 1.0f, 0.0f, 0.0);
  glRotatef( -azimuth, 0.0f, 0.0f, 1.0);
}


void keyboard(unsigned char key, int, int)
{

  switch (key)
  {
    case 'r': /* r or R : reset viewpoint */
      resetView();
      break;
    case 'R':
      resetView();
      break;
    case 'w': /* toggle show/hide world border display */
      if ( _draw_world_border == 0 )
      {
        _draw_world_border = 1;
      }
      else
      {
        _draw_world_border = 0;
      }
      break;
    case 'x': /* toggle x-plane positioning */
      _pos_plane = 0;
      break;
    case 'y': /* toggle y-plane positioning */
      _pos_plane = 1;
      break;
    case 'z': /* toggle z-plane positioning */
      _pos_plane = 2;
      break;
    case 'm':
      _move_plane = 1 - _move_plane;
      break;
    case 'd':
      _remove_cell = 1 - _remove_cell;
      break;
    case '+':
      _radius += 0.1;
      break;
    case '-':
      _radius -= 0.1;
      break;
    case ']':
      _current_color++;
      _current_color %= _colormap_size;
      break;
    case '[':
      _current_color += _colormap_size - 1;
      _current_color %= _colormap_size;
      break;
    case 's':
      saveimage();
      break;
    case 27:  /* escape to quit */
    case 'q': /* q to quit */
    case 'Q': /* Q to quit */
      exit(0);
  }
  glutPostRedisplay();
}

GLvoid mouse( GLint button, GLint state, GLint x, GLint y )
{
  int modifiers = glutGetModifiers();
  if (state == GLUT_DOWN)
  {
    switch (button)
    {
      case GLUT_LEFT_BUTTON:
        if ( _remove_cell ) 
        {
          remove_cell(x,y);
        }
        else if ( modifiers & GLUT_ACTIVE_SHIFT  )
        {
          add_cell(x,y); 
        }
        else if ( _move_plane )
        {
          _action = MOVE_PLANE;
        }
        else
        {
          _action = MOVE_EYE;
        }
        break;
      case GLUT_MIDDLE_BUTTON:
        _action = TWIST_EYE;
        break;
      case GLUT_RIGHT_BUTTON:
        _action = ZOOM;
        break;
    }

    /* Update the saved mouse position */
    _xStart = x;
    _yStart = y;

  }
  else
  {
    _action = MOVE_NONE;
  }

}

int add_cell(GLint x, GLint y)
{
  int window_height = glutGet(GLUT_WINDOW_HEIGHT);
  GLfloat depth;
  GLint stencil;
  GLdouble model[16];
  GLdouble projection[16];
  GLint view[4];
  GLdouble x_coord,y_coord,z_coord;
  GLdouble min_dist = INFINITY;
  GLdouble dist;

  glReadPixels(x, window_height - y - 1, 1, 1, GL_STENCIL_INDEX, GL_UNSIGNED_INT, &stencil);
  glReadPixels(x, window_height - y - 1, 1, 1, GL_DEPTH_COMPONENT, GL_FLOAT, &depth);

  glGetDoublev(GL_MODELVIEW_MATRIX,model);
  glGetDoublev(GL_PROJECTION_MATRIX,projection);
  glGetIntegerv(GL_VIEWPORT,view);

  /* get approximate nearest object coordinates */
  gluUnProject((GLdouble)x, (GLdouble)(window_height - y - 1),
      depth,model,projection,view,&x_coord,&y_coord,&z_coord);


  if ( _mycellscap <= _popsize )
  {
    _mycells = (CellSphere*)realloc(_mycells,2*_mycellscap*sizeof(CellSphere));
    _mycellscap *= 2;    
  }

  _mycells[_popsize].id = _popsize+1;
  _mycells[_popsize].position[0] = x_coord; 
  _mycells[_popsize].position[1] = y_coord; 
  _mycells[_popsize].position[2] = z_coord; 
  _mycells[_popsize].radius = _radius;
  _mycells[_popsize].color[0] = _colormap[_current_color][0]; 
  _mycells[_popsize].color[1] = _colormap[_current_color][1]; 
  _mycells[_popsize].color[2] = _colormap[_current_color][2];
  _mycells[_popsize].colorindex = _current_color;

  _popsize++;


  glutPostRedisplay();
  
  return 0;
}

int remove_cell(GLint x, GLint y)
{
  int window_height = glutGet(GLUT_WINDOW_HEIGHT);
  GLfloat depth;
  GLdouble model[16];
  GLdouble projection[16];
  GLint view[4];
  GLdouble x_coord,y_coord,z_coord;
  GLdouble min_dist = INFINITY;
  GLdouble dist;

  glReadPixels(x, window_height - y - 1, 1, 1, GL_DEPTH_COMPONENT, GL_FLOAT, &depth);

  glGetDoublev(GL_MODELVIEW_MATRIX,model);
  glGetDoublev(GL_PROJECTION_MATRIX,projection);
  glGetIntegerv(GL_VIEWPORT,view);

  /* get approximate nearest object coordinates */
  gluUnProject((GLdouble)x, (GLdouble)(window_height - y - 1),
      depth,model,projection,view,&x_coord,&y_coord,&z_coord);

  /* find cell with center closest to mouse coordinates */
  /* this will return a cell ID even if user clicked far away */
  for ( int i = 0; i < _popsize ; i++ )
  {
    dist = (_mycells[i].position[0] - x_coord)*(_mycells[i].position[0] - x_coord) +
         (_mycells[i].position[1] - y_coord)*(_mycells[i].position[1] - y_coord) +
         (_mycells[i].position[2] - z_coord)*(_mycells[i].position[2] - z_coord);
    if ( dist < min_dist )
    {
      min_dist = dist; 
      _cell_to_remove = i;
    }
  }

  for ( int i = _cell_to_remove; i < _popsize -  1 ; i++ )
  {
    /* shift cell i+1 -> cell i */
    _mycells[i].position[0] = _mycells[i+1].position[0];
    _mycells[i].position[1] = _mycells[i+1].position[1]; 
    _mycells[i].position[2] = _mycells[i+1].position[2];
    _mycells[i].radius = _mycells[i+1].radius;
    _mycells[i].color[0] = _mycells[i+1].color[0]; 
    _mycells[i].color[1] = _mycells[i+1].color[1];
    _mycells[i].color[2] = _mycells[i+1].color[2];
    _mycells[i].colorindex = _mycells[i+1].colorindex;
  }
  _popsize--;


  glutPostRedisplay();
  return 1;

}

int move_plane(GLint x, GLint y)
{

  int window_height = glutGet(GLUT_WINDOW_HEIGHT);
  int window_width = glutGet(GLUT_WINDOW_WIDTH);
  GLfloat depth;
  GLdouble model[16];
  GLdouble projection[16];
  GLint view[4];
  GLdouble new_x, new_y, new_z;
  static GLdouble old_x, old_y, old_z;

  glReadPixels(x, window_height - y - 1, 1, 1, GL_DEPTH_COMPONENT, GL_FLOAT, &depth);

  glGetDoublev(GL_MODELVIEW_MATRIX,model);
  glGetDoublev(GL_PROJECTION_MATRIX,projection);
  glGetIntegerv(GL_VIEWPORT,view);

  /* get approximate nearest object coordinates */
  gluUnProject((GLdouble)x, (GLdouble)(window_height - y - 1),
      0.0,model,projection,view,&new_x,&new_y,&new_z);

  switch(_pos_plane)
  {
    case 0:
      _x_plane += 0.02*_worldsize.x*sign(new_x - old_x)*(fabs(new_x - old_x) < 0.02*_worldsize.x);
      clamp(&_x_plane,0,_worldsize.x);
      break;
    case 1:
      _y_plane += 0.02*_worldsize.y*sign(new_y - old_y)*(fabs(new_y - old_y) < 0.02*_worldsize.y);
      clamp(&_y_plane,0,_worldsize.y);
      break;
    case 2:
      _z_plane += 0.02*_worldsize.z*sign(new_z - old_z)*(fabs(new_z - old_z) < 0.02*_worldsize.z);
      clamp(&_z_plane,0,_worldsize.z);
      break;
  }


  old_x = new_x;
  old_y = new_y;
  old_z = new_z;

  glutPostRedisplay();
  
  return 0;
}

GLvoid motion( GLint x, GLint y )
{
  switch (_action)
  {
    case MOVE_EYE:
      /* Adjust the eye position based on the mouse position */
      _azimAngle += (GLdouble) (x - _xStart);
      _incAngle -= (GLdouble) (y - _yStart);
      break;
    case TWIST_EYE:
      /* Adjust the eye twist based on the mouse position */
      _twistAngle = fmod(_twistAngle+(x - _xStart), 360.0);
      break;
    case ZOOM:
      /* Adjust the eye distance based on the mouse position */
      _distance -= (GLdouble) (y - _yStart)/10.0;
      break;
    case MOVE_PLANE:
      move_plane(x,y);
      break;
  }

  /* Update the stored mouse position for later use */
  _xStart = x;
  _yStart = y;

  glutPostRedisplay();
}

GLvoid passive_motion( GLint x, GLint y)
{
  int window_height = glutGet(GLUT_WINDOW_HEIGHT);
  int window_width = glutGet(GLUT_WINDOW_WIDTH);
  GLfloat depth;
  GLdouble model[16];
  GLdouble projection[16];
  GLint view[4];

  if ( x < 1 | x > window_width | y < 1 | y > window_height )
  { return; }

  glReadPixels(x, window_height - y - 1, 1, 1, GL_DEPTH_COMPONENT, GL_FLOAT, &depth);

  glGetDoublev(GL_MODELVIEW_MATRIX,model);
  glGetDoublev(GL_PROJECTION_MATRIX,projection);
  glGetIntegerv(GL_VIEWPORT,view);

  /* get approximate nearest object coordinates */
  gluUnProject((GLdouble)x, (GLdouble)(window_height - y - 1),
      depth,model,projection,view,&_mouse_x,&_mouse_y,&_mouse_z);

  if ( _mouse_x >= 0 && _mouse_x <= _worldsize.x && 
       _mouse_y >= 0 && _mouse_y <= _worldsize.y && 
       _mouse_z >= 0 && _mouse_z <= _worldsize.z )
  {
    _within_world = 1;
  }
  else
  {
    _within_world = 0;
  }

  glutPostRedisplay();
}


void saveimage() 
{
    int window_height = glutGet(GLUT_WINDOW_HEIGHT);
    int window_width  = glutGet(GLUT_WINDOW_WIDTH);
    char *buffer;
    char filepath[64];
    FILE *fid;
    char mn[] = "BM";
    char creator[] = "SCVW";
    unsigned int offset = 0x36;  /* = 54: header=14 and DIB=40 */
    unsigned int dibsize = 0x28; /* = 40 */
    unsigned int zero = 0x0;     /* = 00 00 00 00 */
    unsigned int resolution = 2835; /* points per meter = 72dpi */
    GLsizei nrChannels = 3;         
    unsigned int image_size;
    unsigned int bufferSize = window_width * window_height * nrChannels; /* size in bytes */
    image_size = window_width * window_height * nrChannels + offset; /* total image size on disk */
    buffer = (char*)malloc(bufferSize * sizeof(char));
    glPixelStorei(GL_PACK_ALIGNMENT, 4);
    glReadBuffer(GL_FRONT);
    glReadPixels(0, 0, window_width, window_height, GL_BGR, GL_UNSIGNED_BYTE, buffer);

    /* filepath: sSTEPNUMBER_nPOPSIZE_SIGNAL.bmp */
    snprintf(filepath,64,"init.bmp");
    if ( ( fid = fopen(filepath,"w") ) == NULL )
    {
          fprintf(stderr, "ERROR : could not open file '%s'.\n", filepath );
    }
    printf("image: %s\n",filepath);
    printf("  window_size: %u x %u\n",window_width,window_height);
    printf("  buffer size: %u bytes\n",bufferSize);
    printf("  total size:  %u bytes\n",image_size);

    /* bitmap header: 14 bytes */
    fwrite(mn,1,2,fid);             /* bitmap image type: BM */
    fwrite(&image_size,4,1,fid);    /* total image size */
    fwrite(creator,1,4,fid);        /* user-defined bytes */
    fwrite(&offset,4,1,fid);        /* total offset, including DIB */

    /* DIB - BITMAPINFOHEADER */
    fwrite(&dibsize,4,1,fid);          /* DIB size: 40 bytes - 4 bytes */
    fwrite(&window_width,4,1,fid);     /* image width, in pixels  - 4 bytes */
    fwrite(&window_height,4,1,fid);    /* image height, in pixels - 4 bytes */
    fputc(1,fid);                      /* number of color planes: 1 */ 
    fputc(0,fid);                      /* pad with zeros to fill 2 bytes - 2 bytes */
    fputc(0x18,fid);                   /* number of bits/pixel n = 24 */
    fputc(0,fid);                      /* pad with zeros to fill 2 bytes - 2 bytes */
    fwrite(&zero,4,1,fid);             /* compression mode: 0 = no compression - 4 bytes */
    fwrite(&zero,4,1,fid);             /* image size: 0 when no compression - 4 bytes */
    fwrite(&resolution,4,1,fid);       /* res horiz - 4 bytes */
    fwrite(&resolution,4,1,fid);       /* res vert  - 4 bytes */
    fwrite(&zero,4,1,fid);             /* default number colors in palette: 0 to defaults to n^24 - 4 bytes */
    fwrite(&zero,4,1,fid);             /* default important colors: 0 - 4 bytes */

    /* image raw data, not memory aligned */
    fwrite(buffer,1,bufferSize,fid);   /* write image raw data */

    /* clean up */
    free(buffer);
    fclose(fid);
}

void on_exit() 
{
  
  if ( ( _init_file = fopen(_init_filename, "w") ) == NULL)
  {
    fprintf(stderr, "Error, could not write to output file %s.\n", _init_filename);
    /* exit(EXIT_FAILURE); */
    _init_file = stdout;
  }
  for ( int i = 0; i < _popsize ; i++ )
  {
    fprintf(_init_file, "%d %f %f %f %f %d\n", _mycells[i].id,
                                           _mycells[i].position[0],
                                           _mycells[i].position[1],
                                           _mycells[i].position[2],
                                           _mycells[i].radius,
                                           _mycells[i].colorindex);
  }
  if ( _init_file != stdout )
    fclose(_init_file);
  fclose(logFile);
  printf("%d lines written in file %s\n",_popsize,_init_filename);
  free(_init_filename);
  free(_mycells);
}

void print_help( char* prog_name )
{
  printf( "\n************* Simuscale - Populate ************* \n\n" );
  printf( "\n\
Usage : %s -h\n\
   or : %s [options]\n\n\
\tOptions\n\n\
\t-h or --help       Display this screen\n\
\t-f or --file s     Set output file to s (defaults\n\
\t                   to initpop.txt)\n\
\t-e or --edit       If output file exists, load existing\n\
\t                   data.\n\
\n\
\tRuntime commands\n\
\n\
\tr or R             Reset 3D view\n\
\tw                  Toggle worldborder\n\
\tx, y, or z         Activate x, y, or z positioning plane\n\
\tm                  Toggle plane moving mode\n\
\td                  Toggle cell delete mode\n\
\t-,+                Decrease, increase radius\n\
\ts                  Save current frame as an image (bitmap)\n\
\n\
\tmouse + left click          In normal mode: move view,\n\
\t                            In cell delete mode: delete cell\n\
\t                            In plane moving mode: move plane along normal\n\
\tmouse + shift + left click  Add a new cell\n\
\n\
\tPress ESC or q to quit.\n\n",\
   prog_name, prog_name );
}

