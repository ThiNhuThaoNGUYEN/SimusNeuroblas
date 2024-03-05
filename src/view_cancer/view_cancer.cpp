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
  float orientation[3];
  float color[3];
  float signal;
  float radius;
  int   clip;
};
typedef struct sCellSphere CellSphere;

struct worldsize {
  float x;
  float y;
  float z;
};

GLfloat _colormap[64][3];
GLfloat _min_signal = 0.0, _max_signal = 100.0;
GLfloat _min_clip_signal = -INFINITY, _max_clip_signal = INFINITY;
GLfloat _font_color[3] = {1.0, 1.0, 1.0}; /* default font color is white */
GLfloat _highlight_color[2][3] = {{1.0,0.9,0.0},{0.2,0.2,0.2}};
FILE * trajFile = NULL;
FILE * logFile  = NULL;
char * _norm_filename;
struct worldsize _worldsize = {0.0, 0.0, 0.0};
CellSphere * _mycells = NULL;
int _popsize = 0;
float _time = 0.0;
int _line = 0;
int _pause = 0;
int _end_of_file = 0;
int _skipframe = 1;
int _background_white = 0;
char _colormap_name = 'j'; /* default colormap: jet */
int _clipcell = 0;
enum guides { GUIDE_NONE, GUIDE_BORDER, GUIDE_VIEWCUT, GUIDE_BORDER_VIEWCUT };
int _draw_guides = GUIDE_BORDER_VIEWCUT; 
int _single_cell_tracking = 0;
int _nbr_cell_tracking = 0;
int _keyboard_input_mode = 0;
int _single_cell = 0;
int _single_cell_index = 0;
char _inputstr[16];
GLuint _CellDlist;
float _speed = 0.02;  /* default is 0.02 frames per ms, i.e. wait 50 ms between two frames */
int _first_signal_col;
int _signal_index; 
int _orientation = 0;
char* _signal_name;
struct { 
  unsigned long *pos; 
  unsigned long i; 
  unsigned long size; 
  unsigned long maxsize; 
}  _filepos = {(unsigned long*)malloc(128*sizeof(unsigned long)), 0, 0, 128};
struct list_signals { unsigned int size; char name[MAX_NBR_SIGNAL][128]; };
struct list_signals _list_signals = { .size = 0 };
char _signal_found = 0;
enum actions { MOVE_EYE, TWIST_EYE, ZOOM, MOVE_NONE, MOVE_PLANE };
GLint _action;
GLdouble _xStart = 0.0, _yStart = 0.0;
GLfloat _nearClip, _farClip, _distance, _twistAngle, _incAngle, _azimAngle;
GLfloat _fovy = 45.0;
GLfloat _x_plane = 0.0 , _y_plane = 0.0 , _z_plane = 0.0;
int _pos_plane = 2; /* 0: x, 1: y, 2: z */
int _move_plane = 0;
GLfloat _positioning_plane_color[3][4] = {{1.0, 0.9, 0.3, 0.2},
                                          {0.3, 1.0, 0.9, 0.2},
                                          {0.9, 0.3, 1.0, 0.2}};

void init_color_map_spring();
void init_color_map_jet();
void init_color_map_neon();
void read_min_max_signals();
void read_header();
void read_file_line();
void animate(int value);
void init();
void display();
void reshape (int w, int h);
void resetView();
void polarView( GLfloat distance, GLfloat azimuth, GLfloat incidence, GLfloat twist);
void keyboard(unsigned char key, int x, int y);
void get_keyboard_input(unsigned char key);
void next_signal();
void previous_signal();
void standstill();
void backtrack( unsigned int nbrframes);
void darken( float *color, float amount );
void lighten( float *color, float amount );
void draw_plane();
void move_plane(GLint x, GLint y);
void saveimage();
int select_cell(GLint x, GLint y);
GLvoid mouse( GLint button, GLint state, GLint x, GLint y );
GLvoid motion( GLint x, GLint y );
GLvoid passive_motion( GLint x, GLint y );
void print_help(char* prog_name);
void on_exit();






int main(int argc, char** argv)
{

  /* Define allowed command-line options */
  const char * options_list = "hf:n:s:i:pwc:v:";
  static struct option long_options_list[] = {
    { "help",     no_argument, NULL, 'h' },
    { "file",     required_argument, NULL, 'f' },
    { "normalization",     required_argument, NULL, 'n' },
    { "speed",    required_argument, NULL, 's' },
    { "signal",   required_argument, NULL, 'i' },
    { "pause",    no_argument, NULL, 'p' },
    { "white",    no_argument, NULL, 'w' },
    { "colormap", required_argument, NULL, 'c' },
    { "viewangle", required_argument, NULL, 'v' },
    { 0, 0, 0, 0 }
  };

  /* Get actual values of the command-line options */
  int option;
  char * traj_filename;
  char traj_filename_specified = 0;
  char norm_filename_specified = 0;
  char signal_specified = 0;
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

        traj_filename = (char *) malloc((strlen(optarg) + 1)*sizeof(char));
        sprintf( traj_filename, "%s", optarg );
        traj_filename_specified = 1;
        break;
      }
      case 'n' :
      {
        if ( strcmp( optarg, "" ) == 0 )
        {
          fprintf(stderr, "ERROR : Option -n or --normalization : missing argument.\n" );
          exit( EXIT_FAILURE );
        }

        _norm_filename = (char *) malloc((strlen(optarg) + 1)*sizeof(char));
        sprintf( _norm_filename, "%s", optarg );
        printf("Normalization file: %s\n", optarg);
        norm_filename_specified = 1;
        break;
      }
      case 's' :
      {
        if ( strcmp( optarg, "" ) == 0 )
          {
            fprintf(stderr,
                "ERROR : Option -s or --speed : missing argument.\n" );
            exit( EXIT_FAILURE );
          }
        _speed = atof(optarg); /* in frames per millisecond */
        break;
      }
      case 'i' :
        if ( strcmp( optarg, "" ) == 0 )
        {
            fprintf(stderr,
                "ERROR : Option -i or --signal : missing argument.\n" );
            exit( EXIT_FAILURE );
        }
        _signal_name = (char *)malloc((strlen(optarg) + 1)*sizeof(char));
        sprintf( _signal_name, "%s", optarg );
        _signal_index = SIGNAL_NOT_FOUND; /* the index will need to be found in the header file */
        signal_specified = 1;
        break;
      case 'p':
        _pause = 1;
        break;
      case 'w':
        _background_white = 1;
        _font_color[0] = 0.0;
        _font_color[1] = 0.0;
        _font_color[2] = 0.0;
        break;
      case 'c':
        if ( strcmp( optarg, "spring" ) == 0 )
        {
          _colormap_name = 's'; 
        }
        else if ( strcmp( optarg, "neon") == 0 )
        {
          _colormap_name = 'n';
        }
        else if ( strcmp( optarg, "jet" ) == 0 )
        {
          _colormap_name = 'j';
        }
        else
        {
          printf("Unknown colormap '%s', using 'jet'.\n",optarg);
        }
      break;
      case 'v':
        _fovy = atof(optarg); /* view angle, in degrees */
        if ( _fovy < 0 )
          _fovy *= -1.0;
        if ( _fovy > 180 )
          _fovy = 180.0;
        break;
    }
  }

  if (!traj_filename_specified) trajFile = fopen("trajectory_cancer.txt", "r");
  else
  {
  	trajFile = fopen(traj_filename, "r");
  	free(traj_filename);
  }
  if (trajFile == NULL)
  {
    fprintf(stderr, "Error, file trajectory.txt is missing.\n");
    exit(EXIT_FAILURE);
  }
  if (!norm_filename_specified) 
  {
    _norm_filename = strdup("normalization_cancer.txt");
  }
  if ( ( logFile = fopen("view.log","w") ) == NULL )
  {
    fprintf(stderr, "Error, could not open view.log.\n");
    exit(EXIT_FAILURE);
  }

  if (!signal_specified)
  {
    _signal_name = (char *)malloc(sizeof(char));
    _signal_name[0] = 0;
    _signal_index = DEFAULT_SIGNAL; /* by default */
  }

  read_header();
  glutInit(&argc, argv);
  glutInitDisplayMode (GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH | GLUT_STENCIL | GLUT_MULTISAMPLE);
  glutInitWindowSize (500, 500);
  glutInitWindowPosition (100, 100);
  glutCreateWindow ( "Simuscale - View cells" );
  init ();

  glutDisplayFunc(display);
  glutReshapeFunc(reshape);
  glutKeyboardFunc(keyboard);
  glutMouseFunc(mouse);
  glutMotionFunc(motion);
  glutPassiveMotionFunc(passive_motion);

  atexit(on_exit);

  glutTimerFunc(0,animate,0); 
  glutMainLoop();

  return 0;
}

void init_color_map_neon()
{
  _colormap[0][0] = 0.000000; _colormap[0][1] = 0.200000; _colormap[0][2] = 0.333333;
_colormap[1][0] = 0.000000; _colormap[1][1] = 0.233333; _colormap[1][2] = 0.337500;
_colormap[2][0] = 0.000000; _colormap[2][1] = 0.266667; _colormap[2][2] = 0.341667;
_colormap[3][0] = 0.000000; _colormap[3][1] = 0.300000; _colormap[3][2] = 0.345833;
_colormap[4][0] = 0.000000; _colormap[4][1] = 0.333333; _colormap[4][2] = 0.350000;
_colormap[5][0] = 0.000000; _colormap[5][1] = 0.366667; _colormap[5][2] = 0.354167;
_colormap[6][0] = 0.000000; _colormap[6][1] = 0.400000; _colormap[6][2] = 0.358333;
_colormap[7][0] = 0.000000; _colormap[7][1] = 0.433333; _colormap[7][2] = 0.362500;
_colormap[8][0] = 0.000000; _colormap[8][1] = 0.466667; _colormap[8][2] = 0.366667;
_colormap[9][0] = 0.000000; _colormap[9][1] = 0.500000; _colormap[9][2] = 0.370833;
_colormap[10][0] = 0.000000; _colormap[10][1] = 0.533333; _colormap[10][2] = 0.375000;
_colormap[11][0] = 0.000000; _colormap[11][1] = 0.566667; _colormap[11][2] = 0.379167;
_colormap[12][0] = 0.000000; _colormap[12][1] = 0.600000; _colormap[12][2] = 0.383333;
_colormap[13][0] = 0.000000; _colormap[13][1] = 0.633333; _colormap[13][2] = 0.387500;
_colormap[14][0] = 0.000000; _colormap[14][1] = 0.666667; _colormap[14][2] = 0.391667;
_colormap[15][0] = 0.000000; _colormap[15][1] = 0.700000; _colormap[15][2] = 0.395833;
_colormap[16][0] = 0.000000; _colormap[16][1] = 0.733333; _colormap[16][2] = 0.400000;
_colormap[17][0] = 0.012500; _colormap[17][1] = 0.737500; _colormap[17][2] = 0.425000;
_colormap[18][0] = 0.025000; _colormap[18][1] = 0.741667; _colormap[18][2] = 0.450000;
_colormap[19][0] = 0.037500; _colormap[19][1] = 0.745833; _colormap[19][2] = 0.475000;
_colormap[20][0] = 0.050000; _colormap[20][1] = 0.750000; _colormap[20][2] = 0.500000;
_colormap[21][0] = 0.062500; _colormap[21][1] = 0.754167; _colormap[21][2] = 0.525000;
_colormap[22][0] = 0.075000; _colormap[22][1] = 0.758333; _colormap[22][2] = 0.550000;
_colormap[23][0] = 0.087500; _colormap[23][1] = 0.762500; _colormap[23][2] = 0.575000;
_colormap[24][0] = 0.100000; _colormap[24][1] = 0.766667; _colormap[24][2] = 0.600000;
_colormap[25][0] = 0.112500; _colormap[25][1] = 0.770833; _colormap[25][2] = 0.625000;
_colormap[26][0] = 0.125000; _colormap[26][1] = 0.775000; _colormap[26][2] = 0.650000;
_colormap[27][0] = 0.137500; _colormap[27][1] = 0.779167; _colormap[27][2] = 0.675000;
_colormap[28][0] = 0.150000; _colormap[28][1] = 0.783333; _colormap[28][2] = 0.700000;
_colormap[29][0] = 0.162500; _colormap[29][1] = 0.787500; _colormap[29][2] = 0.725000;
_colormap[30][0] = 0.175000; _colormap[30][1] = 0.791667; _colormap[30][2] = 0.750000;
_colormap[31][0] = 0.187500; _colormap[31][1] = 0.795833; _colormap[31][2] = 0.775000;
_colormap[32][0] = 0.200000; _colormap[32][1] = 0.800000; _colormap[32][2] = 0.800000;
_colormap[33][0] = 0.241667; _colormap[33][1] = 0.779167; _colormap[33][2] = 0.808333;
_colormap[34][0] = 0.283333; _colormap[34][1] = 0.758333; _colormap[34][2] = 0.816667;
_colormap[35][0] = 0.325000; _colormap[35][1] = 0.737500; _colormap[35][2] = 0.825000;
_colormap[36][0] = 0.366667; _colormap[36][1] = 0.716667; _colormap[36][2] = 0.833333;
_colormap[37][0] = 0.408333; _colormap[37][1] = 0.695833; _colormap[37][2] = 0.841667;
_colormap[38][0] = 0.450000; _colormap[38][1] = 0.675000; _colormap[38][2] = 0.850000;
_colormap[39][0] = 0.491667; _colormap[39][1] = 0.654167; _colormap[39][2] = 0.858333;
_colormap[40][0] = 0.533333; _colormap[40][1] = 0.633333; _colormap[40][2] = 0.866667;
_colormap[41][0] = 0.575000; _colormap[41][1] = 0.612500; _colormap[41][2] = 0.875000;
_colormap[42][0] = 0.616667; _colormap[42][1] = 0.591667; _colormap[42][2] = 0.883333;
_colormap[43][0] = 0.658333; _colormap[43][1] = 0.570833; _colormap[43][2] = 0.891667;
_colormap[44][0] = 0.700000; _colormap[44][1] = 0.550000; _colormap[44][2] = 0.900000;
_colormap[45][0] = 0.741667; _colormap[45][1] = 0.529167; _colormap[45][2] = 0.908333;
_colormap[46][0] = 0.783333; _colormap[46][1] = 0.508333; _colormap[46][2] = 0.916667;
_colormap[47][0] = 0.825000; _colormap[47][1] = 0.487500; _colormap[47][2] = 0.925000;
_colormap[48][0] = 0.866667; _colormap[48][1] = 0.466667; _colormap[48][2] = 0.933333;
_colormap[49][0] = 0.875000; _colormap[49][1] = 0.499020; _colormap[49][2] = 0.887500;
_colormap[50][0] = 0.883333; _colormap[50][1] = 0.531373; _colormap[50][2] = 0.841667;
_colormap[51][0] = 0.891667; _colormap[51][1] = 0.563725; _colormap[51][2] = 0.795833;
_colormap[52][0] = 0.900000; _colormap[52][1] = 0.596078; _colormap[52][2] = 0.750000;
_colormap[53][0] = 0.908333; _colormap[53][1] = 0.628431; _colormap[53][2] = 0.704167;
_colormap[54][0] = 0.916667; _colormap[54][1] = 0.660784; _colormap[54][2] = 0.658333;
_colormap[55][0] = 0.925000; _colormap[55][1] = 0.693137; _colormap[55][2] = 0.612500;
_colormap[56][0] = 0.933333; _colormap[56][1] = 0.725490; _colormap[56][2] = 0.566667;
_colormap[57][0] = 0.941667; _colormap[57][1] = 0.757843; _colormap[57][2] = 0.520833;
_colormap[58][0] = 0.950000; _colormap[58][1] = 0.790196; _colormap[58][2] = 0.475000;
_colormap[59][0] = 0.958333; _colormap[59][1] = 0.822549; _colormap[59][2] = 0.429167;
_colormap[60][0] = 0.966667; _colormap[60][1] = 0.854902; _colormap[60][2] = 0.383333;
_colormap[61][0] = 0.975000; _colormap[61][1] = 0.887255; _colormap[61][2] = 0.337500;
_colormap[62][0] = 0.983333; _colormap[62][1] = 0.919608; _colormap[62][2] = 0.291667;
_colormap[63][0] = 0.991667; _colormap[63][1] = 0.951961; _colormap[63][2] = 0.245833;
}

void init_color_map_spring()
{
  _colormap[0][0] = 0.066667; _colormap[0][1] = 0.400000; _colormap[0][2] = 0.533333;
_colormap[1][0] = 0.096774; _colormap[1][1] = 0.415054; _colormap[1][2] = 0.522581;
_colormap[2][0] = 0.126882; _colormap[2][1] = 0.430108; _colormap[2][2] = 0.511828;
_colormap[3][0] = 0.156989; _colormap[3][1] = 0.445161; _colormap[3][2] = 0.501075;
_colormap[4][0] = 0.187097; _colormap[4][1] = 0.460215; _colormap[4][2] = 0.490323;
_colormap[5][0] = 0.217204; _colormap[5][1] = 0.475269; _colormap[5][2] = 0.479570;
_colormap[6][0] = 0.247312; _colormap[6][1] = 0.490323; _colormap[6][2] = 0.468817;
_colormap[7][0] = 0.277419; _colormap[7][1] = 0.505376; _colormap[7][2] = 0.458065;
_colormap[8][0] = 0.307527; _colormap[8][1] = 0.520430; _colormap[8][2] = 0.447312;
_colormap[9][0] = 0.337634; _colormap[9][1] = 0.535484; _colormap[9][2] = 0.436559;
_colormap[10][0] = 0.367742; _colormap[10][1] = 0.550538; _colormap[10][2] = 0.425806;
_colormap[11][0] = 0.397849; _colormap[11][1] = 0.565591; _colormap[11][2] = 0.415054;
_colormap[12][0] = 0.427957; _colormap[12][1] = 0.580645; _colormap[12][2] = 0.404301;
_colormap[13][0] = 0.458064; _colormap[13][1] = 0.595699; _colormap[13][2] = 0.393548;
_colormap[14][0] = 0.488172; _colormap[14][1] = 0.610753; _colormap[14][2] = 0.382796;
_colormap[15][0] = 0.518280; _colormap[15][1] = 0.625806; _colormap[15][2] = 0.372043;
_colormap[16][0] = 0.548387; _colormap[16][1] = 0.640860; _colormap[16][2] = 0.361290;
_colormap[17][0] = 0.578495; _colormap[17][1] = 0.655914; _colormap[17][2] = 0.350538;
_colormap[18][0] = 0.608602; _colormap[18][1] = 0.670968; _colormap[18][2] = 0.339785;
_colormap[19][0] = 0.638710; _colormap[19][1] = 0.686022; _colormap[19][2] = 0.329032;
_colormap[20][0] = 0.668817; _colormap[20][1] = 0.701075; _colormap[20][2] = 0.318280;
_colormap[21][0] = 0.698925; _colormap[21][1] = 0.716129; _colormap[21][2] = 0.307527;
_colormap[22][0] = 0.729032; _colormap[22][1] = 0.731183; _colormap[22][2] = 0.296774;
_colormap[23][0] = 0.759140; _colormap[23][1] = 0.746237; _colormap[23][2] = 0.286022;
_colormap[24][0] = 0.789247; _colormap[24][1] = 0.761290; _colormap[24][2] = 0.275269;
_colormap[25][0] = 0.819355; _colormap[25][1] = 0.776344; _colormap[25][2] = 0.264516;
_colormap[26][0] = 0.849462; _colormap[26][1] = 0.791398; _colormap[26][2] = 0.253763;
_colormap[27][0] = 0.879570; _colormap[27][1] = 0.806452; _colormap[27][2] = 0.243011;
_colormap[28][0] = 0.909677; _colormap[28][1] = 0.821505; _colormap[28][2] = 0.232258;
_colormap[29][0] = 0.939785; _colormap[29][1] = 0.836559; _colormap[29][2] = 0.221505;
_colormap[30][0] = 0.969892; _colormap[30][1] = 0.851613; _colormap[30][2] = 0.210753;
_colormap[31][0] = 1.000000; _colormap[31][1] = 0.866667; _colormap[31][2] = 0.200000;
_colormap[32][0] = 0.995833; _colormap[32][1] = 0.843750; _colormap[32][2] = 0.214583;
_colormap[33][0] = 0.991667; _colormap[33][1] = 0.820833; _colormap[33][2] = 0.229167;
_colormap[34][0] = 0.987500; _colormap[34][1] = 0.797917; _colormap[34][2] = 0.243750;
_colormap[35][0] = 0.983333; _colormap[35][1] = 0.775000; _colormap[35][2] = 0.258333;
_colormap[36][0] = 0.979167; _colormap[36][1] = 0.752083; _colormap[36][2] = 0.272917;
_colormap[37][0] = 0.975000; _colormap[37][1] = 0.729167; _colormap[37][2] = 0.287500;
_colormap[38][0] = 0.970833; _colormap[38][1] = 0.706250; _colormap[38][2] = 0.302083;
_colormap[39][0] = 0.966667; _colormap[39][1] = 0.683333; _colormap[39][2] = 0.316667;
_colormap[40][0] = 0.962500; _colormap[40][1] = 0.660417; _colormap[40][2] = 0.331250;
_colormap[41][0] = 0.958333; _colormap[41][1] = 0.637500; _colormap[41][2] = 0.345833;
_colormap[42][0] = 0.954167; _colormap[42][1] = 0.614583; _colormap[42][2] = 0.360417;
_colormap[43][0] = 0.950000; _colormap[43][1] = 0.591667; _colormap[43][2] = 0.375000;
_colormap[44][0] = 0.945833; _colormap[44][1] = 0.568750; _colormap[44][2] = 0.389583;
_colormap[45][0] = 0.941667; _colormap[45][1] = 0.545833; _colormap[45][2] = 0.404167;
_colormap[46][0] = 0.937500; _colormap[46][1] = 0.522917; _colormap[46][2] = 0.418750;
_colormap[47][0] = 0.933333; _colormap[47][1] = 0.500000; _colormap[47][2] = 0.433333;
_colormap[48][0] = 0.929167; _colormap[48][1] = 0.477083; _colormap[48][2] = 0.447917;
_colormap[49][0] = 0.925000; _colormap[49][1] = 0.454167; _colormap[49][2] = 0.462500;
_colormap[50][0] = 0.920833; _colormap[50][1] = 0.431250; _colormap[50][2] = 0.477083;
_colormap[51][0] = 0.916667; _colormap[51][1] = 0.408333; _colormap[51][2] = 0.491667;
_colormap[52][0] = 0.912500; _colormap[52][1] = 0.385417; _colormap[52][2] = 0.506250;
_colormap[53][0] = 0.908333; _colormap[53][1] = 0.362500; _colormap[53][2] = 0.520833;
_colormap[54][0] = 0.904167; _colormap[54][1] = 0.339583; _colormap[54][2] = 0.535417;
_colormap[55][0] = 0.900000; _colormap[55][1] = 0.316667; _colormap[55][2] = 0.550000;
_colormap[56][0] = 0.895833; _colormap[56][1] = 0.293750; _colormap[56][2] = 0.564583;
_colormap[57][0] = 0.891667; _colormap[57][1] = 0.270833; _colormap[57][2] = 0.579167;
_colormap[58][0] = 0.887500; _colormap[58][1] = 0.247917; _colormap[58][2] = 0.593750;
_colormap[59][0] = 0.883333; _colormap[59][1] = 0.225000; _colormap[59][2] = 0.608333;
_colormap[60][0] = 0.879167; _colormap[60][1] = 0.202083; _colormap[60][2] = 0.622917;
_colormap[61][0] = 0.875000; _colormap[61][1] = 0.179167; _colormap[61][2] = 0.637500;
_colormap[62][0] = 0.870833; _colormap[62][1] = 0.156250; _colormap[62][2] = 0.652083;
_colormap[63][0] = 0.866667; _colormap[63][1] = 0.133333; _colormap[63][2] = 0.666667;

}


void init_color_map_jet()
{
  _colormap[0][0] = 0.000000; _colormap[0][1] = 0.000000; _colormap[0][2] = 0.562500;
  _colormap[1][0] = 0.000000; _colormap[1][1] = 0.000000; _colormap[1][2] = 0.625000;
  _colormap[2][0] = 0.000000; _colormap[2][1] = 0.000000; _colormap[2][2] = 0.687500;
  _colormap[3][0] = 0.000000; _colormap[3][1] = 0.000000; _colormap[3][2] = 0.750000;
  _colormap[4][0] = 0.000000; _colormap[4][1] = 0.000000; _colormap[4][2] = 0.812500;
  _colormap[5][0] = 0.000000; _colormap[5][1] = 0.000000; _colormap[5][2] = 0.875000;
  _colormap[6][0] = 0.000000; _colormap[6][1] = 0.000000; _colormap[6][2] = 0.937500;
  _colormap[7][0] = 0.000000; _colormap[7][1] = 0.000000; _colormap[7][2] = 1.000000;
  _colormap[8][0] = 0.000000; _colormap[8][1] = 0.062500; _colormap[8][2] = 1.000000;
  _colormap[9][0] = 0.000000; _colormap[9][1] = 0.125000; _colormap[9][2] = 1.000000;
  _colormap[10][0] = 0.000000; _colormap[10][1] = 0.187500; _colormap[10][2] = 1.000000;
  _colormap[11][0] = 0.000000; _colormap[11][1] = 0.250000; _colormap[11][2] = 1.000000;
  _colormap[12][0] = 0.000000; _colormap[12][1] = 0.312500; _colormap[12][2] = 1.000000;
  _colormap[13][0] = 0.000000; _colormap[13][1] = 0.375000; _colormap[13][2] = 1.000000;
  _colormap[14][0] = 0.000000; _colormap[14][1] = 0.437500; _colormap[14][2] = 1.000000;
  _colormap[15][0] = 0.000000; _colormap[15][1] = 0.500000; _colormap[15][2] = 1.000000;
  _colormap[16][0] = 0.000000; _colormap[16][1] = 0.562500; _colormap[16][2] = 1.000000;
  _colormap[17][0] = 0.000000; _colormap[17][1] = 0.625000; _colormap[17][2] = 1.000000;
  _colormap[18][0] = 0.000000; _colormap[18][1] = 0.687500; _colormap[18][2] = 1.000000;
  _colormap[19][0] = 0.000000; _colormap[19][1] = 0.750000; _colormap[19][2] = 1.000000;
  _colormap[20][0] = 0.000000; _colormap[20][1] = 0.812500; _colormap[20][2] = 1.000000;
  _colormap[21][0] = 0.000000; _colormap[21][1] = 0.875000; _colormap[21][2] = 1.000000;
  _colormap[22][0] = 0.000000; _colormap[22][1] = 0.937500; _colormap[22][2] = 1.000000;
  _colormap[23][0] = 0.000000; _colormap[23][1] = 1.000000; _colormap[23][2] = 1.000000;
  _colormap[24][0] = 0.062500; _colormap[24][1] = 1.000000; _colormap[24][2] = 0.937500;
  _colormap[25][0] = 0.125000; _colormap[25][1] = 1.000000; _colormap[25][2] = 0.875000;
  _colormap[26][0] = 0.187500; _colormap[26][1] = 1.000000; _colormap[26][2] = 0.812500;
  _colormap[27][0] = 0.250000; _colormap[27][1] = 1.000000; _colormap[27][2] = 0.750000;
  _colormap[28][0] = 0.312500; _colormap[28][1] = 1.000000; _colormap[28][2] = 0.687500;
  _colormap[29][0] = 0.375000; _colormap[29][1] = 1.000000; _colormap[29][2] = 0.625000;
  _colormap[30][0] = 0.437500; _colormap[30][1] = 1.000000; _colormap[30][2] = 0.562500;
  _colormap[31][0] = 0.500000; _colormap[31][1] = 1.000000; _colormap[31][2] = 0.500000;
  _colormap[32][0] = 0.562500; _colormap[32][1] = 1.000000; _colormap[32][2] = 0.437500;
  _colormap[33][0] = 0.625000; _colormap[33][1] = 1.000000; _colormap[33][2] = 0.375000;
  _colormap[34][0] = 0.687500; _colormap[34][1] = 1.000000; _colormap[34][2] = 0.312500;
  _colormap[35][0] = 0.750000; _colormap[35][1] = 1.000000; _colormap[35][2] = 0.250000;
  _colormap[36][0] = 0.812500; _colormap[36][1] = 1.000000; _colormap[36][2] = 0.187500;
  _colormap[37][0] = 0.875000; _colormap[37][1] = 1.000000; _colormap[37][2] = 0.125000;
  _colormap[38][0] = 0.937500; _colormap[38][1] = 1.000000; _colormap[38][2] = 0.062500;
  _colormap[39][0] = 1.000000; _colormap[39][1] = 1.000000; _colormap[39][2] = 0.000000;
  _colormap[40][0] = 1.000000; _colormap[40][1] = 0.937500; _colormap[40][2] = 0.000000;
  _colormap[41][0] = 1.000000; _colormap[41][1] = 0.875000; _colormap[41][2] = 0.000000;
  _colormap[42][0] = 1.000000; _colormap[42][1] = 0.812500; _colormap[42][2] = 0.000000;
  _colormap[43][0] = 1.000000; _colormap[43][1] = 0.750000; _colormap[43][2] = 0.000000;
  _colormap[44][0] = 1.000000; _colormap[44][1] = 0.687500; _colormap[44][2] = 0.000000;
  _colormap[45][0] = 1.000000; _colormap[45][1] = 0.625000; _colormap[45][2] = 0.000000;
  _colormap[46][0] = 1.000000; _colormap[46][1] = 0.562500; _colormap[46][2] = 0.000000;
  _colormap[47][0] = 1.000000; _colormap[47][1] = 0.500000; _colormap[47][2] = 0.000000;
  _colormap[48][0] = 1.000000; _colormap[48][1] = 0.437500; _colormap[48][2] = 0.000000;
  _colormap[49][0] = 1.000000; _colormap[49][1] = 0.375000; _colormap[49][2] = 0.000000;
  _colormap[50][0] = 1.000000; _colormap[50][1] = 0.312500; _colormap[50][2] = 0.000000;
  _colormap[51][0] = 1.000000; _colormap[51][1] = 0.250000; _colormap[51][2] = 0.000000;
  _colormap[52][0] = 1.000000; _colormap[52][1] = 0.187500; _colormap[52][2] = 0.000000;
  _colormap[53][0] = 1.000000; _colormap[53][1] = 0.125000; _colormap[53][2] = 0.000000;
  _colormap[54][0] = 1.000000; _colormap[54][1] = 0.062500; _colormap[54][2] = 0.000000;
  _colormap[55][0] = 1.000000; _colormap[55][1] = 0.000000; _colormap[55][2] = 0.000000;
  _colormap[56][0] = 0.937500; _colormap[56][1] = 0.000000; _colormap[56][2] = 0.000000;
  _colormap[57][0] = 0.875000; _colormap[57][1] = 0.000000; _colormap[57][2] = 0.000000;
  _colormap[58][0] = 0.812500; _colormap[58][1] = 0.000000; _colormap[58][2] = 0.000000;
  _colormap[59][0] = 0.750000; _colormap[59][1] = 0.000000; _colormap[59][2] = 0.000000;
  _colormap[60][0] = 0.687500; _colormap[60][1] = 0.000000; _colormap[60][2] = 0.000000;
  _colormap[61][0] = 0.625000; _colormap[61][1] = 0.000000; _colormap[61][2] = 0.000000;
  _colormap[62][0] = 0.562500; _colormap[62][1] = 0.000000; _colormap[62][2] = 0.000000;
  _colormap[63][0] = 0.500000; _colormap[63][1] = 0.000000; _colormap[63][2] = 0.000000;
}


void read_min_max_signals()
{
  FILE * minmaxfile = fopen(_norm_filename, "r");
  if (minmaxfile == NULL)
  {
    printf("No file called %s in current directory.\n", _norm_filename);
    printf("Using default values for coloring: min=0.0 and max=100.0.\n");
    return;
  }

  int retval;
  char c;
  char str[128];
  retval = fscanf(minmaxfile, "%f", &_max_signal);
  while (retval != 1)
  {
    if (retval == EOF) {printf("File normalization.txt found but premature end of file.\n");return;}

    if (fscanf(minmaxfile, "%s", str) == 1)
    {
      if (str[0]=='#' || (strncmp(str, _signal_name, 127) != 0 && strlen(_signal_name)) ) 
      {
        /* ignore the whole line */
        /* printf("comment line :\n"); */
        do {retval = fscanf(minmaxfile, "%c", &c); /* printf("%c", c); */}  while ((c != '\n') && (retval != EOF));
        if (retval == EOF) {printf("File normalization.txt found but premature end of file.\n");return;}
      }
      else if ( strncmp(str, _signal_name, 127) == 0 || strlen(_signal_name) == 0 )
      {
        /* nothing to do */
      }
      else
      {
        /* line starting with a letter, for example */
        fprintf(stderr, "Unknown file format.\n");
        return;
      }
    }
    retval = fscanf(minmaxfile, "%f", &_max_signal);
  }
  _max_signal += 1e-6; /* to avoid clipping for the max value */

  retval = fscanf(minmaxfile, "%f", &_min_signal);
  while (retval != 1)
  {
    if (retval == EOF) {printf("File normalization.txt found but premature end of file.\n");return;}

    if (fscanf(minmaxfile, "%c", &c) == 1)
    {
      if (c=='#')
      {
        /* comment line, ignore the whole line */
        /* printf("comment line :\n"); */
        do {retval = fscanf(minmaxfile, "%c", &c); /* printf("%c", c); */}  while ((c != '\n') && (retval != EOF));
        if (retval == EOF) {printf("File normalization.txt found but premature end of file.\n");return;}
      }
      else
      {
        /* line strating with a letter, for example */
        fprintf(stderr, "Unknown file format.\n");
        return;
      }
    }
    retval = fscanf(minmaxfile, "%f", &_min_signal);
  }

  fclose(minmaxfile);
}

void read_header()
{
  int ind;
  int retval;
  char c;
  char str[128];


  while ( ( retval = fscanf(trajFile,"%[!$#]", &c ) == 1 ) )
  {
    switch(c)
    {
      case '#' :
        /* comment line, check if orientation is printed */
        if ( fscanf(trajFile, " %*d : %*[xyz] %s", str) == 1 )
        {
          if ( strncmp("orientation",str,11) == 0 )
          {
            _orientation = 1;
          }
        }
        /* printf("comment line :\n"); */
        break; 
      case '!' :
        /* param line specifying _worldsize */
        fscanf(trajFile, "%f %f %f", &(_worldsize.x), &(_worldsize.y), &(_worldsize.z) );
        break;
      case '$' :
        /* signal name e.g. $ 8 = SIGNAL_1 */
        fscanf(trajFile, "%d = %s", &ind, str); 
        if ( _list_signals.size == 0 ) 
        {
          _first_signal_col = ind;
          /* printf("_first_signal_col = %d\n", _first_signal_col); */
        }
        _list_signals.size++;
        if ( _list_signals.size < MAX_NBR_SIGNAL )
        {
          strncpy(_list_signals.name[_list_signals.size-1], str, 127);
        }
        /* first signal found is used as default signal */ 
        if (!_signal_found)
        {
          if ( strncmp(str,_signal_name,127) == 0 )
          {
            _signal_index = ind; 
            _signal_found = 1;
          }
          if ( _signal_index == DEFAULT_SIGNAL )
          {
            _signal_name = (char*)realloc(_signal_name,(strlen(str) + 1)*sizeof(char));
            _signal_index = ind;
            sprintf(_signal_name, "%s", str);
            _signal_found = 1;
          }
        }
        break;
      default :  
        fprintf(stderr, "Unknown file format.\n");
        exit(EXIT_FAILURE);
    }

    /* go to next line or exit if EOF is reached */
    do {retval = fscanf(trajFile, "%c", &c); /* printf("%c", c); */ }  while ((c != '\n') && (retval != EOF));
    if (retval == EOF) {exit(0);}
    _line ++;

  }

  if ( _signal_index == SIGNAL_NOT_FOUND )
  {
    fprintf(stderr, "Signal %s not found.\n", _signal_name);
    exit(EXIT_FAILURE);
  }

  if ( _worldsize.x == 0.0 ) 
  {
    fprintf(stderr, "worldsize not found.\n");
    exit(EXIT_FAILURE);
  }

  /* printf("line %d time %f pop %d \n", _line, _time, _popsize);  */
  
}

void read_file_line()
{
  float signal;
  int i, col;
  int colorindex;
  int minclip = 0, maxclip = 0;
  int retval;
  char c;

  if ( _filepos.i >= _filepos.maxsize ) /* increase storage for file position markers */
  {
    _filepos.maxsize *= 2;
    _filepos.pos = (unsigned long *)realloc(_filepos.pos,_filepos.maxsize*sizeof(unsigned long));
  }
  _filepos.pos[_filepos.i] = ftell(trajFile);
  /* printf("file: frame i = %lu, pos = %lu\n",_filepos.i,_filepos.pos[_filepos.i]);  */

  retval = fscanf(trajFile, "%f %d", &_time, &_popsize);
  /* if (retval == EOF) {exit(0);} */
  if (retval == EOF) 
  {
    _end_of_file = 1;
    return;
  }
  else
  {
    _end_of_file = 0;
  }

  _filepos.i++;

  fseek(trajFile, _filepos.pos[_filepos.i - 1], SEEK_SET);      
  if (_mycells != NULL) free(_mycells);
  _mycells = (CellSphere *) malloc(_popsize * sizeof(CellSphere));

  i = 0;

  do  
  {
    retval = fscanf(trajFile, "%f %d", &_time, &_popsize);
    if ( retval == EOF )
    {
      fprintf(stderr, "Time %f contains only %d rows, but expects %d.\n", _time, i, _popsize);
      exit( EXIT_FAILURE );
    }

    fscanf(trajFile, "%d %f %f %f", &_mycells[i].id, &_mycells[i].position[0], &_mycells[i].position[1], &_mycells[i].position[2]);

    if ( _orientation )
    {
      fscanf(trajFile, "%f %f %f", &_mycells[i].orientation[0], &_mycells[i].orientation[1], &_mycells[i].orientation[2]); /* read and discard orientation vector */
    }

    fscanf(trajFile, "%f", &_mycells[i].radius);

    col = 8 + 3*_orientation;
    while ( col < _first_signal_col )
    {
      retval = fscanf(trajFile, "%*s"); /* skip to next entry */
      if ( retval == EOF )
      {
        fprintf(stderr, "File contains only %d column, but needs %d.\n", col, _signal_index);
        exit( EXIT_FAILURE );
      }
      ++col;
    }

    while ( col < _signal_index )
    {
      retval = fscanf(trajFile, "%f", &signal); /* skip to next entry */
      if ( retval == EOF )
      {
        fprintf(stderr, "File contains only %d column, but needs %d.\n", col, _signal_index);
        exit( EXIT_FAILURE );
      }
      ++col;
    }
    fscanf(trajFile, "%f", &signal);

    /* compute color from signal */
    colorindex = (int) 64 * (signal - _min_signal) / (_max_signal - _min_signal);
    if (colorindex < 0) 
    {
      colorindex = 0; 
      minclip = 1;
    }
    else if (colorindex > 63) 
    {
      colorindex = 63; 
      maxclip = 1;
    }

    _mycells[i].color[0] = _colormap[colorindex][0];
    _mycells[i].color[1] = _colormap[colorindex][1];
    _mycells[i].color[2] = _colormap[colorindex][2];

    _mycells[i].clip     = ( signal > _max_clip_signal ) || ( signal < _min_clip_signal );

    _mycells[i].signal = signal;

    do {retval = fscanf(trajFile, "%c", &c); /* printf("%c", c); */}  while ((c != '\n') && (retval != EOF));
    if (retval == EOF) {exit(0);}

    i++;

  } while ( i < _popsize );


  /* printf("line %d time %f pop %d \n", _line, _time, _popsize); */
  /* printf("\n"); */

  if (minclip) fprintf(logFile,"Warning, at least one cell had min color clipping at t=%f.\n", _time);
  if (maxclip) fprintf(logFile,"Warning, at least one cell had max color clipping at t=%f.\n", _time);

  _line++;
}


void animate(int)
{
  while ( _skipframe-- )
  {
    read_file_line();
  }
  _skipframe = 1;
  if ( _pause == 0 &&  _end_of_file == 0 ) 
  {
    glutTimerFunc(1.0/_speed, animate, 0);
  }
  glutPostRedisplay();
}


void init(void)
{
  switch(_colormap_name)
  {
    case 's':
      init_color_map_spring();
      break;
    case 'n':
      init_color_map_neon();
      break;
    case 'j':
    default:
      init_color_map_jet();
  }
  read_min_max_signals();

  glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

   if ( _background_white )
     glClearColor (1.0, 1.0, 1.0, 1.0);
   else
     glClearColor (0.0, 0.0, 0.0, 1.0);

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

   glEnable(GL_BLEND);
   glEnable(GL_LIGHTING);
   glEnable(GL_LIGHT0);
   glEnable(GL_RESCALE_NORMAL);
   glEnable(GL_DEPTH_TEST);
   glEnable(GL_COLOR_MATERIAL);
   glEnable(GL_MULTISAMPLE);

   /* transparency */
   glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

   /* Enable stencil operations */
   glEnable(GL_STENCIL_TEST);
   glStencilOp(GL_KEEP, GL_KEEP, GL_REPLACE);

   /* Set up nearClip and farClip so that ( farClip - nearClip ) > maxObjectSize, */
   /* and determine the viewing distance (adjust for zooming) */
   _nearClip = 0.1*_worldsize.x;
   _farClip = _nearClip + 4.0*_worldsize.x;
   resetView();

   /* display list to store the geometry of a sphere */
   _CellDlist = glGenLists(1);
   glNewList(_CellDlist, GL_COMPILE);
   glutSolidSphere(1.0, S_SLICES, S_STACKS);
   glEndList();
}

void display(void)
{
  GLfloat NicheColor[] = {0.5,0.5,0.5,0.1};
  GLfloat orientation_color[] = {1.0,1.0,1.0,0.8};
  char legend_string[128];
  char single_cell_legend_string[256];
  GLubyte raster_rect[16] = {0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff};
  GLubyte raster_left[16] = {0x0f, 0x3f, 0x7f, 0x7f, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x7f, 0x7f, 0x3f, 0x0f};
  GLubyte raster_right[16] = {0xf0, 0xfc, 0xfe, 0xfe, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xfe, 0xfe, 0xfc, 0xf0};
  const char axis_name[3][7] = {" mv[x]", " mv[y]", " mv[z]"};

  /***** Clear color buffer, depth buffer, stencil buffer *****/
  /* depth buffer: used for single cell selection
   * stencil buffer: used to identify the background (default value to 0), 
   *                 non-selected cells (value 1) and 
   *                 the selected cell (value 2)
   */
  glClearStencil(0); /* default value */
  glClear (GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT );


  /*************** Draw the legend texts **********************/
  sprintf(legend_string, "t=%.2f s=%lu N=%d%s%s", _time, _filepos.i - 1, _popsize, \
      _clipcell ? " clip" : "", _move_plane ? axis_name[_pos_plane] : ""); 
  glColor3fv (_font_color);
  glWindowPos2i(15, 13); /* also sets current raster color to _font_color */
  for (size_t i = 0; i < strlen(legend_string); i++) glutBitmapCharacter(GLUT_BITMAP_HELVETICA_12, legend_string[i]);

  glColor3f(0.8 - 0.7*_font_color[0],0.8 - 0.7*_font_color[1],0.8 - 0.7*_font_color[2]);
  glWindowPos2i(10, 10); 
  glBitmap(8, 16, 0.0, 0.0, 8, 0, raster_left);
  for(auto i = 0; i < 26; i++)
  {
    glBitmap(8, 16, 0.0, 0.0, 8, 0, raster_rect);
  }
  glBitmap(8, 16, 0.0, 0.0, 8, 0, raster_right);


  /*************** Draw the signal colorbar *******************/
  for(auto i = 0; i < 64; i++)
  {
    glColor3fv(_colormap[i]);
    glWindowPos2i(240 + 4*i, 10); 
    glBitmap(3, 16, 0.0, 0.0, 0, 0, raster_rect);
  }

  glColor3fv (_font_color);
  sprintf(legend_string, "%.2f", _min_signal);
  glWindowPos2i(240, 30); /* also sets current raster color to _font_color */
  for (size_t i = 0; i < strlen(legend_string); i++)
    glutBitmapCharacter(GLUT_BITMAP_HELVETICA_12, legend_string[i]);

  glColor3fv (_font_color);
  sprintf(legend_string, "%s", _signal_name);
  glWindowPos2i(280, 30); /* also sets current raster color to _font_color */
  if ( strlen(legend_string) > MAX_SIG_STRLEN ) /* print first three chars ... last (MAX_SIG_STRLEN - 6) */
  {
    for (auto i = 0; i < 3; i++)
      glutBitmapCharacter(GLUT_BITMAP_HELVETICA_12, legend_string[i]);
    for (auto i = 3; i < 6; i++)
      glutBitmapCharacter(GLUT_BITMAP_HELVETICA_12, '.');
    for (auto i = strlen(legend_string) - (MAX_SIG_STRLEN - 6); i < strlen(legend_string); i++)
      glutBitmapCharacter(GLUT_BITMAP_HELVETICA_12, legend_string[i]);
  }
  else
  {
    for (size_t i = 0; i < strlen(legend_string); i++)
      glutBitmapCharacter(GLUT_BITMAP_HELVETICA_12, legend_string[i]);
  }

  glColor3fv (_font_color);
  sprintf(legend_string, "%.2f", _max_signal);
  glWindowPos2i(460, 30); /* also sets current raster color to _font_color */
  for (size_t i = 0; i < strlen(legend_string); i++)
    glutBitmapCharacter(GLUT_BITMAP_HELVETICA_12, legend_string[i]);

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
  float world_margins[3] = {0.05f*_worldsize.x, 0.05f*_worldsize.y, 0.05f*_worldsize.z}; 
  float total_world_size[3] = {world_size[0] + 2 * world_margins[0],
                               world_size[1] + 2 * world_margins[1],
                               world_size[2] + 2 * world_margins[2]};

  if ( _draw_guides == GUIDE_BORDER || _draw_guides == GUIDE_BORDER_VIEWCUT )
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
  for (int i = 0; i < _popsize; i++)
  {

    /* if _clipcell = true, do not draw color clipped cells */
    if ( _mycells[i].clip && _clipcell ) /* cell is clipped, do not draw */
    {
      continue;
    }

    /* do not draw cells that are above x-plane, below y-plane
     * and above z-plane */
    if ( ( _mycells[i].position[0] > _x_plane ) &&
         ( _mycells[i].position[1] < _y_plane ) &&
         ( _mycells[i].position[2] > _z_plane ) )
    {
      continue; /* do not draw the cell */
    }

    /******** set stencil index for single cell tracking ********/
    if ( _single_cell_tracking == 1 && _mycells[i].id == _single_cell )
    {
      glStencilFunc(GL_ALWAYS, 2, -1); /* set tracked cell stencil to 2 */
    }
    else
    {
      glStencilFunc(GL_ALWAYS, 1, -1); /* set non tracked cell stencil to 1 */
    }

    /**** draw cell i *****/
    glPushMatrix(); /* save V */
    if (_mycells[i].position[2] > 0) {
            glColor3fv(_mycells[i].color);
        } else {
            glColor3fv(NicheColor);
        }
    glTranslatef(_mycells[i].position[0], _mycells[i].position[1], _mycells[i].position[2]); /* current matrix is VT  */
    glScalef(_mycells[i].radius, _mycells[i].radius, _mycells[i].radius ); /* current matrix is VTS */
    glCallList(_CellDlist);
    
    if ( _orientation == 1 )
    {
      /* Add a small bump along cell orientation */
      glColor4fv(orientation_color);
      glTranslatef(0.7*_mycells[i].orientation[0], 0.7*_mycells[i].orientation[1], 0.7*_mycells[i].orientation[2]); /* current matrix is VT  */
      glScalef(0.5,0.5,0.5);
      glCallList(_CellDlist);
    }
    glPopMatrix(); /* current matrix is restored to V */


    /**************** highlight tracked cell if tracking enabled *************/
    if ( _single_cell_tracking == 1 && _mycells[i].id == _single_cell )
    {
      /*********************** Draw single cell legend ***********************/
      sprintf(single_cell_legend_string, "ID: %d, %s: %f",_single_cell,_signal_name,_mycells[i].signal);
      glColor3fv (_font_color);
      glWindowPos2i(15, glutGet(GLUT_WINDOW_HEIGHT) - 15); /* also sets current raster color to white */
      for (size_t i = 0; i < strlen(single_cell_legend_string); i++) glutBitmapCharacter(GLUT_BITMAP_HELVETICA_12, single_cell_legend_string[i]);

      /************** Draw a yellow circle around tracked cell ****************/
      glStencilFunc(GL_NOTEQUAL, 2, 0xFF); /* tracked cell stencil: 2 */
      glPushMatrix(); /* save V */
      glDisable(GL_LIGHTING);
      glTranslatef(_mycells[i].position[0], _mycells[i].position[1], _mycells[i].position[2]); /* current matrix is VT  */
      glScalef(_mycells[i].radius, _mycells[i].radius, _mycells[i].radius ); /* current matrix is VTS */

      /* highlight color 1 */
      glColor3fv(_highlight_color[0]);
      glutSolidSphere(1.2, 2*S_SLICES, 2*S_STACKS);

      /* highlight color 2 */
      glColor3fv(_highlight_color[1]);
      glutSolidSphere(1.3, 2*S_SLICES, 2*S_STACKS);

      glPopMatrix(); /* current matrix is restored to V */
      glEnable(GL_LIGHTING);
    }

  }

  /******************* Draw positioning plane *******************/
  if ( _draw_guides == GUIDE_VIEWCUT || _draw_guides == GUIDE_BORDER_VIEWCUT )
  {
    draw_plane();
  }

  glutSwapBuffers();
}

void reshape (int w, int h)
{
  glViewport (0, 0, (GLsizei) w, (GLsizei) h);
  glMatrixMode (GL_PROJECTION);
  glLoadIdentity ();
  /*gluPerspective(30.0, (1.0*w)/h, 0.5*_worldsize, 5*_worldsize); */
  /* gluPerspective( 45.0f, (1.0*w)/h, _nearClip, _farClip ); */
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
  /* printf(" incidence %f azimuth %f\n", incidence, azimuth); */
  glTranslatef( -_worldsize.x/2.0, -_worldsize.y/2.0, -distance);
  glRotatef( -twist, 0.0f, 0.0f, 1.0);
  glRotatef( -incidence, 1.0f, 0.0f, 0.0);
  glRotatef( -azimuth, 0.0f, 0.0f, 1.0);
}


void keyboard(unsigned char key, int, int)
{
  if ( _keyboard_input_mode ) /* capture the key */
  {
    get_keyboard_input(key);
    return;
  }

  switch (key)
  {
    case 'r': /* r or R : reset viewpoint */
      resetView();
      glutPostRedisplay();
      break;
    case 'R':
      resetView();
      glutPostRedisplay();
      break;
    case 32: /* space bar = pause the animation */
      if (_pause == 0)
      {_pause = 1;}
      else
      {
        _pause = 0;
        glutTimerFunc(1.0/_speed,animate,0);
      }
      break;
    case 'j': /* decrease max signal */
      _max_signal -= 0.1*(_max_signal - _min_signal); 
      standstill();
      glutTimerFunc(1.0/_speed,animate,0);
      break;
    case 'k': /* increase max signal */
      _max_signal += 0.1*(_max_signal - _min_signal);
      standstill();
      glutTimerFunc(1.0/_speed,animate,0);
      break;
    case 'h': /* decrease min signal */
      _min_signal -= 0.1*(_max_signal - _min_signal); 
      standstill();
      glutTimerFunc(1.0/_speed,animate,0);
      break;
    case 'l': /* increase min signal */
      _min_signal += 0.1*(_max_signal - _min_signal); 
      standstill();
      glutTimerFunc(1.0/_speed,animate,0);
      break;
    case ']': /* display next signal */
      next_signal();
      read_min_max_signals();
      standstill();
      glutTimerFunc(1.0/_speed,animate,0);
      break;
    case '[':
      previous_signal();
      read_min_max_signals();
      standstill();
      glutTimerFunc(1.0/_speed,animate,0);
      break;
    case 'n': /* stop at next time step */
      _pause = 1;
      glutTimerFunc(0,animate,0);
      break;
    case 'b': /* back to previous step */
      _pause = 1;
      backtrack(1);
      glutTimerFunc(1.0/_speed,animate,0);
      break;
    case 'B': /* back to previous step */
      _pause = 1;
      backtrack(50);
      glutTimerFunc(1.0/_speed,animate,0);
      break;
    case 'N':
      _skipframe = 50;
      printf("skipping 50 steps\n");
      glutTimerFunc(1.0/_speed,animate,0);
      break;
    case 'c': /* toggle clip: hide cells that have signal outside min/max signal */
      if ( _clipcell == 0 )
      { 
        _clipcell = 1;  
        _min_clip_signal = _min_signal;
        _max_clip_signal = _max_signal;
      }
      else
      { 
        _clipcell = 0; 
      }
      standstill();
      glutTimerFunc(1.0/_speed,animate,0);
      break;
    case 'w': /* toggle show/hide world border display */
      _draw_guides++;
      _draw_guides %= 4;
      standstill();
      glutTimerFunc(1.0/_speed,animate,0);
      break;
    case 't':
      if ( _single_cell_tracking == 0)
      {
        printf("enter cell ID: ");
        fflush(stdout);
        memset(_inputstr,0,16); 
        _keyboard_input_mode = 1;
      }
      else
      {
        _single_cell_tracking = 0;
      }
      standstill();
      glutTimerFunc(1.0/_speed,animate,0);
      break;
    case 's':
      saveimage();
      break;
    case 'F': /* full screen */
      glutFullScreen();
      break;
    case 'f': /* leave full screen */
      glutReshapeWindow(500,500);
      glutPositionWindow(100,100);
      break;
    case 'm':
      _move_plane = 1 - _move_plane;
      glutPostRedisplay();
      break;
    case 'x': /* toggle x-plane positioning */
      _pos_plane = 0;
      glutPostRedisplay();
      break;
    case 'y': /* toggle y-plane positioning */
      _pos_plane = 1;
      glutPostRedisplay();
      break;
    case 'z': /* toggle z-plane positioning */
      _pos_plane = 2;
      glutPostRedisplay();
      break;
    case 27:  /* escape to quit */
    case 'q': /* q to quit */
    case 'Q': /* Q to quit */
      exit(0);
  }
}

void get_keyboard_input(unsigned char key)
{
  if ( key == 13 ) /* carriage return */
  {
    _keyboard_input_mode = 0;
    _single_cell_tracking = sscanf(_inputstr,"%d",&_single_cell); 
    putchar('\n');
    standstill();
    glutTimerFunc(0,animate,0);
    return;
  }
  if ( key == 8 || key == 16 || key == 127) /* backspace */ 
  {
    if ( strlen(_inputstr) > 0 )
    {
      _inputstr[strlen(_inputstr)-1] = 0;  
      PR_BS_CLEAR; fflush(stdout);
    }
    return;
  }

  /* get key */
  _inputstr[strlen(_inputstr)] = key;  
  putchar(key); fflush(stdout);
  return;
}

void next_signal()
{
      _signal_index++;
      _signal_index = ( ( _signal_index - _first_signal_col ) % _list_signals.size ) + _first_signal_col;
      _signal_name = (char*)realloc(_signal_name,(strlen(_list_signals.name[_signal_index - _first_signal_col]) + 1)*sizeof(char));
      sprintf(_signal_name, "%s", _list_signals.name[_signal_index - _first_signal_col]); 
}

void previous_signal()
{
      _signal_index--;
      _signal_index = ( ( _signal_index - _first_signal_col + _list_signals.size ) % _list_signals.size );
      _signal_index +=  _first_signal_col;
      _signal_name = (char*)realloc(_signal_name,(strlen(_list_signals.name[_signal_index - _first_signal_col]) + 1)*sizeof(char));
      sprintf(_signal_name, "%s", _list_signals.name[_signal_index - _first_signal_col]); 
}

GLvoid mouse( GLint button, GLint state, GLint x, GLint y )
{
  if (state == GLUT_DOWN)
  {
    switch (button)
    {
      case GLUT_LEFT_BUTTON:
        if ( _move_plane )
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

    if ( ( button == GLUT_LEFT_BUTTON ) &&  ( glutGetModifiers() & GLUT_ACTIVE_SHIFT ) )
    {
      _single_cell_tracking = select_cell(x,y);
    }

  }
  else
  {
    _action = MOVE_NONE;
  }

}

int select_cell(GLint x, GLint y)
{
  int window_height = glutGet(GLUT_WINDOW_HEIGHT);
  GLfloat depth;
  GLdouble model[16];
  GLdouble projection[16];
  GLint view[4];
  GLdouble x_coord,y_coord,z_coord;
  GLdouble min_dist = INFINITY;
  GLdouble dist;

  glReadPixels(x, window_height - y - 1, 1, 1, GL_STENCIL_INDEX, GL_UNSIGNED_INT, &_single_cell);
  glReadPixels(x, window_height - y - 1, 1, 1, GL_DEPTH_COMPONENT, GL_FLOAT, &depth);

  /* printf("--stencil: %u\n\n",_single_cell); */

  glGetDoublev(GL_MODELVIEW_MATRIX,model);
  glGetDoublev(GL_PROJECTION_MATRIX,projection);
  glGetIntegerv(GL_VIEWPORT,view);

  /* get approximate nearest object coordinates */
  gluUnProject((GLdouble)x, (GLdouble)(window_height - y - 1),
      depth,model,projection,view,&x_coord,&y_coord,&z_coord);

  printf("object coordinates: (%f,%f,%f), ", x_coord, y_coord, z_coord);

  
  if ( _single_cell == 0 )  /* user clicked on the background */
  {
    printf(" no selection\n\n");
    glutPostRedisplay();
    return 0;
  }

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
      _single_cell = _mycells[i].id;
    }
  }
  printf("ID: %u\n\n",_single_cell);
  glutPostRedisplay();
  return 1;
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
  (void)x;
  (void)y;
}

void backtrack( unsigned int nbrframes )
{
    int newframe = _filepos.i - nbrframes - 1;
    if ( newframe < 0 ) 
      _filepos.i = 0;
    else
      _filepos.i = newframe;
    /* printf("bactrack to %ld\n",_filepos.pos[_filepos.i]); */
    fseek(trajFile, _filepos.pos[_filepos.i], SEEK_SET);      
}

void darken( float *color, float amount )
{
  color[0] *=  amount;
  color[1] *=  amount;
  color[2] *=  amount;
}

void lighten( float *color, float amount )
{
  color[0] += (1.0f - color[0])*amount;
  color[1] += (1.0f - color[0])*amount;
  color[2] += (1.0f - color[0])*amount;
}

void standstill()
{
  /* void standstill()
   * rewinds the trajectory file by one frame, 
   * so that the next time the display is updated,
   * it uses the same data.
   */ 
    if (_filepos.i > 0) 
    { 
      _filepos.i--;
    }
    else 
    { 
      _filepos.i = 0;
    }
    fseek(trajFile, _filepos.pos[_filepos.i], SEEK_SET);      
}

void draw_plane()
{

  float world_size[3] = {_worldsize.x, _worldsize.y, _worldsize.z};

  /******************* Draw positioning plane *******************/
  glDisable(GL_LIGHTING);
  if ( _move_plane ) 
  {
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL); // Mesh
    switch(_pos_plane)
    {
      case 0:
        glBegin(GL_QUADS); /* x-plane */
        glColor4fv (_positioning_plane_color[0]);
        glVertex3d(_x_plane, 0.0, 0.0);
        glVertex3d(_x_plane, 0.0, world_size[2]);
        glVertex3d(_x_plane, world_size[1], world_size[2]);
        glVertex3d(_x_plane, world_size[1], 0.0); 
        glEnd(); /* end x-plane */
        break;
      case 1:
        glBegin(GL_QUADS); /* y-plane */
        glColor4fv (_positioning_plane_color[1]);
        glVertex3d(0.0, _y_plane, 0.0);
        glVertex3d(0.0, _y_plane, world_size[2]);
        glVertex3d(world_size[0], _y_plane, world_size[2]);
        glVertex3d(world_size[0], _y_plane, 0.0); 
        glEnd(); /* end y-plane */
        break;
      case 2:
        glBegin(GL_QUADS); /* z-plane */
        glColor4fv (_positioning_plane_color[2]);
        glVertex3d(0.0, 0.0, _z_plane);
        glVertex3d(world_size[0], 0.0, _z_plane);
        glVertex3d(world_size[0], world_size[1], _z_plane);
        glVertex3d(0.0, world_size[1], _z_plane);
        glEnd(); /* end z-plane */
        break;
    }
  }
  glColor3f(0.4,0.5,0.7);
  glBegin(GL_LINES);
  glVertex3d(0.0, _y_plane, _z_plane);
  glVertex3d(world_size[0], _y_plane, _z_plane);
  glVertex3d(_x_plane, 0.0, _z_plane);
  glVertex3d(_x_plane, world_size[1], _z_plane);
  glVertex3d(_x_plane, _y_plane, 0.0);
  glVertex3d(_x_plane, _y_plane, world_size[2]);
  glEnd();
  glEnable(GL_LIGHTING);

}


void move_plane(GLint x, GLint y)
{

  int window_height = glutGet(GLUT_WINDOW_HEIGHT);
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
    snprintf(filepath,64,"s%06lu_n%d_%s.bmp",_filepos.i-1,_popsize,_signal_name);
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
  fclose(trajFile);
  fclose(logFile);
  free(_norm_filename);
  free(_signal_name);
  free(_filepos.pos);
  printf("bye...\n");
}

void print_help( char* prog_name )
{
  printf( "\n************* Simuscale - Viewing program ************* \n\n" );
  printf( "\n\
Usage : %s -h\n\
   or : %s [options]\n\n\
\t-h or --help       : Display this screen\n\n\
Options (i : integer, d : double, s : string) :\n\n\
\t-f or --file   s    : View the simulation stored in file named s (default: trajectory.txt)\n\
\t-s or --speed  d    : Set the playing speed to d, in frames per ms (default: 0.02)\n\
\t-i or --signal s    : Set the signal to display (default: first signal in file)\n\n\
\t-p or --pause       : Launch in pause mode on (resume with space bar, default: no pause)\n\n\
\t-w or --white       : Set white background (default: black)\n\n\
\t-c or --colormap v  : Set colormap to v (default: jet, or spring)\n\n\
\t-v or --viewangle d : Set perspective viewing angle to d,\n\
\t                      (0 < d < 180 degrees, default: 45)\n\n\
If an option is not set, the program uses the default value for this parameter.\n\n\
While the animation is playing, you can use the following commands:\n\n\
\tmouse\n\
\tleft button \tchange the viewpoint\n\
\tright button\tzoom in or zoom out\n\n\
\tkeyboard\n\
\tr or R  \treset the perspective.\n\
\tspace bar \tpause or resume the animation\n\
\t] and [ \tswitch to next (]) or previous signal ([)\n\
\tb       \tbacktrack to the previous timestep\n\
\tB       \tbacktrack 50 timesteps\n\
\tf       \treset the initial window size, position and view\n\
\tF       \tgo full screen\n\
\th and l \tdecrease (h) or increase (l) min signal\n\
\tj and k \tdecrease (j) or increase (k) max signal\n\
\tm       \ttoggle the viewcut plane\n\
\tn       \tpause at the next timestep\n\
\tN       \tskip over 50 timesteps\n\
\ts       \tsave the current frame as a BMP image\n\
\tt       \ttoggle single cell tracking. Upon toggling,\n\
\t        \tenter cell id to enable single cell tracking\n\
\t        \tPress t to disable tracking\n\
\tw       \ttoggle world border display\n\
\tx, y, z \tselect the viewcut plane\n\n\
\tPress ESC or q to quit.\n\n",\
   prog_name, prog_name );
}

