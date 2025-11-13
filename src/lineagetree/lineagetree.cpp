/** lineagetree.c 
 * print tabular/TGF tree to svg
 * compile:
 * gcc lineagetree.c -o lineagetree.out
 * usage: see print_help
 * 
 **/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <getopt.h>

#define PRINT(x) printf("%g\n",x);

struct cell_node {
  unsigned int    id;           /* unique cell id non-negative integer */
  float           birth;        /* time of birth */ 
  float           death;        /* time of death */
  char            color[8];     /* branch color for output */
  unsigned int    lca;          /* last common ancestor */

  char            tag[32];

  size_t nbr_children;

  struct cell_node *mother;
  struct cell_node **children;
};

struct table {
  struct cell_node **node;
  size_t size;
};


float _max_time;
float _hor_scale = 2.0;
int   _offset = 4;
int   _vert_skip = 25;
size_t _total_nbr_nodes;
float _angle = 0;
float _circle_coverage = 0.98;
int   _radial_center_offset;
unsigned int   _lca_id = 0; /* last common ancestor to plot: default root 0 */
unsigned int   _limit_nbr_nodes = 100000;

int _cflag = 1;
char _tree_color[7] = "000000";

int _view_box = 2000;
int _view_center = 1000;

FILE *_fout = NULL;
FILE *_ftraj = NULL;

float _font_size = 1.0; /* relative size in 'em' */

char _style_text[] = "<style>\n"\
            "  text {\n"\
            "    font-family: Arial, Helvetica, sans-serif;\n"\
            "  }\n"\
            "  .branch:hover text {\n"\
            "    font-size: larger;\n"\
            "  }\n"\
            "  .branch:hover line {\n"\
            "    stroke-width: 3px;\n"\
            "  }\n"\
            " .tooltiptext {\n"\
            "    visibility: hidden;\n"\
            "  }\n"\
            "  .branch:hover + .tooltiptext {\n"\
            "    visibility: visible;\n"\
            "    font-size: larger;\n"\
            "  }\n"\
            "</style>\n";

enum treetype { TT_LINEAR, TT_RADIAL }; 
enum treetype _tree_type = TT_RADIAL;

enum filetype { FT_TGF, FT_TABULAR };

struct cell_node*  new_cell_node(struct cell_node *mother, float time, const char descr[32], unsigned int id);
void link_child(struct cell_node *mother, struct cell_node *child);
void add_child(struct cell_node *mother, struct cell_node *child, float birth, float tip);
struct cell_node* get_node_from_id(struct cell_node *root, unsigned int id);
void nbr_descendants(struct cell_node *root, size_t *s);
void print_tree(struct cell_node *root, int *y);
void print_radial_tree(struct cell_node *root);
void print_radial_branch(struct cell_node *root, size_t i, size_t s1);
void delete_tree(struct cell_node *root);
int not_a_child(struct cell_node *root, unsigned int id);
void read_trivial_graph_format_file(struct cell_node** root, FILE* fid);
void read_tabular_tree_file(struct cell_node* root, FILE* fid);
void gencolor(char color[8], const char descr[32]);
void set_view();
void get_node_tag(unsigned int id, char *tag );
void print_help(const char *prog_name);


int main ( int argc, char *argv[] )
{

  FILE *fid = NULL;
  char * stringp = NULL;
  char * progname = argv[0];

  struct cell_node *root = (struct cell_node*)NULL; 
  enum filetype filetype = FT_TGF; /* tabular or tgf */
  int y0 = 0; /* 25; */

  int option;
  const char * options_list = "hj:g:t:s:v:l:c:o:m:b:";
  static struct option long_options_list[] = {
    { "help",     no_argument, NULL, 'h' },
    { "trajectory", required_argument, NULL, 'j'},
    { "format",   required_argument, NULL, 'g' },
    { "treetype", required_argument, NULL, 't' },
    { "horscale", required_argument, NULL, 's' },
    { "vertskip", required_argument, NULL, 'v' },
    { "lca",      required_argument, NULL, 'l' },
    { "color",    required_argument, NULL, 'c' },
    { "output-file",    required_argument, NULL, 'o' },
    { "font-size",    required_argument, NULL, 'm' },
    { "offset",    required_argument, NULL, 'b' },
    { 0, 0, 0, 0 }
  };

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
      case 'j':
      {
        if ( ( _ftraj = fopen(optarg,"r") ) == NULL )
        {
          fprintf(stderr, "Error, could not open trajectory file %s.\n", optarg);
          exit(EXIT_FAILURE);
        }
        break;
      }
      case 'g':
        if ( strcmp( optarg, "tgf" ) == 0 )
        {
          filetype = FT_TGF;
        }
        else if ( strcmp( optarg, "tabular") == 0 )
        {
          filetype = FT_TABULAR;
        }
        else
        {
          fprintf(stderr,"Unknown graph format type '%s'\n",optarg);
        }
        break;
      case 't':
        if ( strcmp( optarg, "radial" ) == 0 )
        {
          _tree_type = TT_RADIAL;
        }
        else if ( strcmp( optarg, "linear") == 0 )
        {
          _tree_type = TT_LINEAR;
        }
        else
        {
          fprintf(stderr,"Unknown graph structure '%s'\n",optarg);
        }
        break;
      case 's':
        _hor_scale = strtof(optarg,NULL);
        break;
      case 'v':
        _vert_skip = strtol(optarg,NULL,10);
        break;
      case 'l':
        _lca_id = strtol(optarg,NULL,10);
        break;
      case 'c':
        _cflag = 0;
        snprintf( _tree_color, 7, "%s", optarg );
        break;
      case 'o':
        stringp = optarg; 
        break;
      case 'm':
        _font_size = strtof(optarg,NULL);
        break;
      case 'b':
        _offset = strtol(optarg,NULL,10);
        break;
      default:
          print_help( progname );
          exit(EXIT_FAILURE);
    }
  }

  argc -= optind; /* discard all options */
  argv += optind; /* shift pointer argv to nonoption arguments */
  if ( argc == 0 )
  {
      fprintf(stderr, "Error, missing input file name.\n");
      print_help( progname );
      exit(EXIT_FAILURE);
  }

  /* open input filename in argv[0] */
  if ( ( fid = fopen(argv[0],"r") ) == NULL )
  {
    fprintf(stderr, "Error, could not open %s.\n", argv[0]);
    exit(EXIT_FAILURE);
  }
  printf("reading file %s\n", argv[0]);

  if ( stringp == NULL ) /* -o option was not used */
  {
    stringp = argv[0] + strcspn(argv[0],".");
    *(++stringp) = 's';
    *(++stringp) = 'v';
    *(++stringp) = 'g';
    *(++stringp) = 0;
    stringp = argv[0];
  }

  /* open output file */
  printf("output file: %s\n",stringp);
  if ( ( _fout = fopen(stringp,"w") ) == NULL )
  {
    fprintf(stderr, "Error, could not open output file %s.\n", stringp);
    exit(EXIT_FAILURE);
  }

  if ( filetype == FT_TABULAR )
  {
    root = new_cell_node(NULL,0.0, "root", -1);
    read_tabular_tree_file(root, fid);
  }
  else if ( filetype == FT_TGF )
  {
    root = (struct cell_node*)NULL;

    read_trivial_graph_format_file(&root, fid);
  }

  nbr_descendants(get_node_from_id(root,_lca_id),&_total_nbr_nodes);
  printf("total number of nodes: %zu\n",_total_nbr_nodes);

  set_view();

	fprintf(_fout,"<?xml version='1.0'?>\n");

  if ( _tree_type == TT_RADIAL )
  {
    fprintf(_fout,"<svg viewBox='0 0 %d %d' xmlns='%s' version='1.1'>\n", \
      _view_box, _view_box, "http://www.w3.org/2000/svg");
    fprintf(_fout,"<rect width='100%%' height='100%%' fill='white'/>\n");
    _radial_center_offset = root->nbr_children * _offset;
    _hor_scale = 0.9*(_view_center - _radial_center_offset)/_max_time;
    fprintf(_fout,"%s", _style_text);
    print_radial_tree(root);
  }
  else
  {
    fprintf(_fout,"<svg viewBox='0 %d %d %lu' xmlns='%s' version='1.1'>\n", \
       -2*_vert_skip, 4*_offset + (int)(_max_time*_hor_scale), (_total_nbr_nodes+1)*_vert_skip, "http://www.w3.org/2000/svg");
    fprintf(_fout,"<rect width='100%%' height='100%%' fill='white'/>\n");
    fprintf(_fout,"%s", _style_text);
    print_tree(root, &y0);
  }
  fprintf(_fout,"</svg>\n");

  delete_tree(root);

  fclose(fid);
  fclose(_fout);
  fclose(_ftraj);
  return 0;

}


struct cell_node*  new_cell_node(struct cell_node *mother, float time, const char descr[32], unsigned int id)
{
  char col[8];
  struct cell_node *child = (struct cell_node *)malloc(sizeof(struct cell_node));
  child->id = id;
  child->birth = time;
  child->death = INFINITY;
  child->nbr_children = 0;
  child->children = (struct cell_node**)NULL;
  child->mother = mother;
  gencolor(col,descr);
  strncpy(child->color,col,sizeof(child->color));
  child->lca = 0;
  strncpy(child->tag,descr,31);

  if ( mother != NULL )
  {
    mother->nbr_children++;
    mother->children = (struct cell_node **)realloc(mother->children,mother->nbr_children*sizeof(struct cell_node*));
    mother->children[mother->nbr_children-1] = child;
  }

  return child;
}

void link_child(struct cell_node *mother, struct cell_node *child)
{
  if ( mother != NULL )
  {
    mother->nbr_children++;
    mother->children = (struct cell_node **)realloc(mother->children,mother->nbr_children*sizeof(struct cell_node*));
    mother->children[mother->nbr_children-1] = child;
  }
  else
  {
    fprintf(stderr,"Error, mother is NULL, in %s at line %d.",__func__,__LINE__);
  }
}

void add_child(struct cell_node *mother, struct cell_node *child, float birth, float tip)
{
  child->birth = birth;
  child->death = tip;
  if (child->id == _lca_id )
  {
    child->lca = _lca_id;
  }
  else
  {
    child->lca = mother->lca;
  }
  link_child(mother,child);

}

struct cell_node* get_node_from_id(struct cell_node *root, unsigned int id)
{
  struct cell_node *node;
  if ( root->id == id )
  {
    return root;
  }
  else
  {
    for ( size_t i = 0; i < root->nbr_children; i++ )
    {
      node = get_node_from_id(root->children[i],id);
      if ( node != NULL)
      {
        return node;
      }
    }
  }
  return NULL;
}

void nbr_descendants(struct cell_node *root, size_t *s)
{
  *s += 1; 
  for ( size_t i = 0; i < root->nbr_children; i++ )
  {
    nbr_descendants(root->children[i], s); 
  }
}

void print_tree(struct cell_node *root, int *y)
{
  for ( size_t i = root->nbr_children; i--; )
  {
    size_t s1 = 0; 
    int start = (int)(_hor_scale*(root->children[i])->birth) + 4*_offset;
    int stop;

    /* get the number of nodes to the right of the tree */
    for ( size_t k = i+1; k < root->nbr_children; k++)
    {
      nbr_descendants(root->children[k],&s1);
    }

    /* print horizontal connector */
    fprintf(_fout,"<path d='M%d %d C%d %d, %d %d, %d %d S%d %d, %d %d' stroke='black' fill='transparent'/>\n",
        start+3*_offset,*y,              /* lower-right end */
        start+3*_offset,*y-_vert_skip/2,  /* lower-right end control point */
        start+2*_offset,*y-_vert_skip/2,  /* inflexion control point */
        start+_offset,*y-_vert_skip/2,    /* inflexion point */
        start,*y-_vert_skip/2,           /* upper-left end control point */
        start,*y-_vert_skip*((int)s1+1));     /* upper-left end */


    /* print the horizontal lifespan of the node */
    stop = start + (int)(_hor_scale*((root->children[i])->death  - (root->children[i])->birth)); 
    if ( stop > (start + 3*_offset) )
      fprintf(_fout,"<line x1='%d' x2='%d' y1='%d' y2='%d' stroke='black'/>\n",start+3*_offset,stop,*y,*y);

    /* print connector ends */
    fprintf(_fout,"<circle cx='%d' cy='%d' r='%d' fill='black'/>\n",start+3*_offset,*y,_offset/2);
    fprintf(_fout,"<circle cx='%d' cy='%d' r='%d' fill='black'/>\n",start,*y-_vert_skip*((int)s1+1),_offset/2);

    /* print node id */
    fprintf(_fout,"<text x='%d' y='%d' stroke='black' text-anchor='end'>%u</text>\n",start+2*_offset,*y,root->children[i]->id);
    fprintf(_fout,"<text x='%d' y='%d' fill='black'>%u</text>\n",stop+_offset,*y+_offset,root->children[i]->id);

    /* print tip */
    fprintf(_fout,"<line x1='%d' x2='%d' y1='%d' y2='%d' stroke='black'/>\n",stop,stop,*y-_offset,*y+_offset);      

    *y += _vert_skip;
    print_tree(root->children[i], y);
  }
}

void print_radial_tree(struct cell_node *root)
{
  size_t s1;

  for ( size_t i = root->nbr_children; i--; )
  {
    if ( root->children[i]->lca != _lca_id ) goto next_node; /* skip the body of the loop except 
                                                           for the recursive call to 
                                                           print_radial_tree */

    s1 = 0; 

    /* get the number of nodes to the right of the tree */
    for ( size_t k = i+1; k < root->nbr_children; k++)
    {
      nbr_descendants(root->children[k],&s1);
    }
 
    print_radial_branch(root, i, s1);
    _angle += _circle_coverage*360.0/_total_nbr_nodes;

    next_node: print_radial_tree(root->children[i]);
  }
}

void print_radial_branch(struct cell_node *root, size_t i, size_t s1  )
{
    float cc, ss, cc2, ss2;
    int y = 0;
    int start, stop;
    char* stroke;
    cc = cos(_circle_coverage*2*M_PI*(s1+1)/_total_nbr_nodes);
    ss = sin(_circle_coverage*2*M_PI*(s1+1)/_total_nbr_nodes);
    cc2 = cos(_circle_coverage*M_PI*(s1+1)/_total_nbr_nodes);
    ss2 = sin(_circle_coverage*M_PI*(s1+1)/_total_nbr_nodes);
    start = (int)(_hor_scale*(root->children[i])->birth) + _radial_center_offset;
    if ( _cflag )
    {
      stroke = (root->children[i])->color;
    }
    else
    {
      stroke = _tree_color;
    }
    fprintf(_fout,"<g class='branch' font-size='%gem' transform='rotate(%f, %d, %d) translate(%d %d)'>\n",
        _font_size,_angle,_view_center,_view_center,_view_center,_view_center);

    /* print arc connector
     *  unless root had id 0 or
     *  child is lca root->children[i]->id == root->children[i]->lca ) 
     */
    if ( root->id > 0 && root->children[i]->id != root->children[i]->lca ) 
    {
      fprintf(_fout,"<path d='M%d %d A %d %d 0 %d 0 %d %d' " \
          "stroke='#%s' fill='transparent'/>\n",
          start,y,     /* lower-right end */
          start,start,  /* radius x, radius y */
          (int)(-ss*(start)-cc*(y)) > 0, /* is angle > 180 deg ? then draw the long arc*/
          (int)(cc*(start)-ss*(y)),(int)(-ss*(start)-cc*(y)), /* upper-left end  */
          stroke 
          );     

      /* print birth time */
      fprintf(_fout,"<text x='%d' y='%d' fill='#55BB77' " \
          "transform='rotate(%f, %d, %d)' font-size='0.75em'>%.1f</text>\n",
          (int)(cc2*(start)-ss2*(y))-3*_offset,(int)(-ss2*(start)-cc2*(y))-_offset, /* upper-left end  */
          -180*_circle_coverage*(s1+1)/_total_nbr_nodes + 90, (int)(cc2*(start)-ss2*(y)),(int)(-ss2*(start)-cc2*(y)),
          root->children[i]->birth);
    }


    /* print the horizontal lifespan of the node */
    stop = start + (int)(_hor_scale*((root->children[i])->death  - (root->children[i])->birth)); 
    fprintf(_fout,"<line x1='%d' x2='%d' y1='%d' y2='%d' stroke='#%s'/>\n",start,stop,y,y,stroke);

    /* print connector ends 
     * at parent branch if root is not 0
     * at child branch if root is 0 or lca
     * lca root->children[i]->id == root->children[i]->lca ) 
     */
    if ( root->id > 0 && root->children[i]->id != root->children[i]->lca ) 
    {
      fprintf(_fout,"<circle cx='%d' cy='%d' r='%d' fill='#%s'/>\n",
          (int)(cc*(start)-ss*(y)),(int)(-ss*(start)-cc*(y)),_offset/2,stroke);
    }
    else 
    {
      fprintf(_fout,"<circle cx='%d' cy='%d' r='%d' fill='#%s'/>\n",start,y,_offset/2,stroke); 
    }

    /* print node id */

    /* print id at root */
    fprintf(_fout,"<text x='%d' y='%d' stroke='black' text-anchor='end' " \
        " " \
        ">%d</text>\n",start-_offset,y+_offset,root->children[i]->id);

    /* print id at tip */
    fprintf(_fout,"<text x='%d' y='%d' fill='black'>%d</text>\n", 
        stop+_offset,y+_offset,
        root->children[i]->id);

    /* print birth/death */
    if ( ( stop - start ) > 100 )
    {
      fprintf(_fout,"<text x='%d' y='%d' fill='#AAAAAA' font-size='0.75em'>%.1f</text>\n", 
        (start + stop)/2,y-_offset/2,
        root->children[i]->death - root->children[i]->birth);
    }

    /* print tip */
    fprintf(_fout,"<line x1='%d' x2='%d' y1='%d' y2='%d' stroke='#%s'/>\n",
        stop,stop,y-_offset,y+_offset,stroke);      

    fprintf(_fout,"</g>\n");

    fprintf(_fout,"<text x='%d' y='%d' class='tooltiptext'>%d %s</text>\n",\
        0,50,root->children[i]->id,root->children[i]->tag);

}

void delete_tree(struct cell_node *root)
{
  for ( size_t i = 0; i < root->nbr_children; i++ )
  {
    delete_tree(root->children[i]);
  }
  free(root);
}

int not_a_child(struct cell_node *root, unsigned int id)
{
  for ( size_t i = 0; i < root->nbr_children; i++ )
  {
    if ( root->children[i]->id == id )
      return 0;
  }
  return 1;
}

void read_trivial_graph_format_file(struct cell_node** root, FILE* fid)
{
  char *line = NULL;
  char descr[32];
  size_t linecap = 0;
  unsigned int parent_id, child_id, node_id, root_id = 0;
  unsigned int max_id = 0;
  float time, tip;
  struct cell_node *child_node = NULL;
  struct cell_node *parent_node = NULL;
  int read = 0; /* 0: read nodes, 1: read edges */
  struct cell_node *node_list = new_cell_node(NULL,0.0,"node_list",0); 
  struct table node_id_table = {NULL,0};
  int add_node = 0;
  unsigned int line_number = 0;


  while ( getline(&line,&linecap,fid) > 0 )
  {
    line_number++;

    if ( line[strcspn(line,"#")] == '#' )
    {
      read = 1;
      fprintf(stderr,"  list of nodes done\n");
      continue;
    }

    if ( read == 0 )
    {
      *descr = 0;
      if ( sscanf(line,"%u %[^\n]",&node_id,descr) < 1)
      {
        break;
      }
      if ( node_id > _limit_nbr_nodes )
      {
        fprintf(stderr,"Error. In TGF file on line %u, node id is either too large: %u, or negative: %d.\n",line_number,node_id,(int)node_id);
        fprintf(stderr,"Node id must be between 0 and %u\n", _limit_nbr_nodes);
        exit(EXIT_FAILURE);
      }
      if ( strncasecmp(descr,"root",4) == 0 )
      {
        root_id = node_id;
      }
      max_id = node_id > max_id ? node_id : max_id;
      node_list->id = max_id+1; /* change id to unique value */

      /* check if node_id is in ids */
      add_node = 1;
#if 0
      for ( size_t i = 0; i < node_list->nbr_children; i++ )
      {
        if ( node_list->children[i]->id == node_id ) 
        {
          add_node = 0;
          break;
        }
      }
#endif
      if ( add_node )
      {
        struct cell_node *new_node = new_cell_node(node_list,0.0, descr, node_id); 
        if ( node_id >= node_id_table.size )
        {
          node_id_table.node = (struct cell_node **)realloc(node_id_table.node,(2*node_id+1)*sizeof(struct cell_node **));
          node_id_table.size = (2*node_id_table.size + 1);
        }
        node_id_table.node[(size_t)node_id] = new_node;
      }
    }
    else /* read == 1 */
    {
      if ( sscanf(line,"%u %u %f %f",&parent_id,&child_id,&time,&tip) < 3 )
      {
        break;
      }
      _max_time = time > _max_time ? time : _max_time;
      _max_time = tip > _max_time ? tip : _max_time;

      /* check if parent_id, child_id is in node_list */
      if ( not_a_child(node_list,parent_id) )
      {
        fprintf(stderr,"Error. %d is not a node.\n",parent_id);
        exit(1);
      }
      if ( not_a_child(node_list,child_id) )
      {
        fprintf(stderr,"Error. %d is not a node.\n",child_id);
        exit(1);
      }

      /* birth: add node/edge to parent */
      /* child_node = get_node_from_id(node_list,child); */
      /* parent_node = get_node_from_id(node_list,parent); */
      child_node = node_id_table.node[child_id];
      parent_node = node_id_table.node[parent_id];
      add_child(parent_node,child_node,time,tip);
      /* fprintf(stderr,"node %d with lca %d.\n",child_node->id,child_node->lca);  */
    }
  }
  *root = get_node_from_id(node_list,root_id);
  (*root)->death = _max_time;
  free(node_list->children);
  free(node_list);
  free(node_id_table.node);
}

void read_tabular_tree_file(struct cell_node* root, FILE* fid)
{
  char *line = NULL;
  size_t linecap = 0;
  int stop, parent, event, child;
  int max_id = -1;
  float time;
  struct cell_node *node = NULL;
  while ( getline(&line,&linecap,fid) > 0 )
  {

    if ( sscanf(line,"%f %*f %*d %d %d %d %d",&time,&stop,&parent,&event,&child) )
    {
      if ( event )
      {
        max_id = child > max_id ? child : max_id;
        _max_time = time > _max_time ? time : _max_time;

        if ( event == 1 )
        {
          node = get_node_from_id(root,parent);
          new_cell_node(node,time, "000000", child);
        }
        if ( event == -1 )
        {
          node = get_node_from_id(root,parent);
          node->death = time;
        }
        

      }
    }
  }
}

void gencolor(char color[8], const char descr[32])
{
  (void)descr;
  /* abcdef */
  char c;
  memset(color,0x30,8);
  color[6] = 0;
  for ( int i = 0; i < 6; i++ )
  {
    c = descr[i] % 16;
    if ( c < 10 )
    {
      color[i] = c + 0x30;
    }
    else
    {
      color[i] = (c - 0xA) + 0x41; 
    }
  }
}

void set_view()
{
  if ( _total_nbr_nodes > 100 )
  {
    _view_box = 3000;
    _view_center = 1500;
  }
  if ( _total_nbr_nodes > 200 )
  {
    _view_box = 4000;
    _view_center = 2000;
  }
  if ( _total_nbr_nodes > 500 )
  {
    _view_box = 6000;
    _view_center = 3000;
  }
  if ( _total_nbr_nodes > 1000 )
  {
    _view_box = 10000;
    _view_center = 5000;
  }
}

void get_node_tag(unsigned int id, char *tag)
{
  int retval; 
  unsigned int cid;
  char c;

  if (_ftraj == NULL ) return;

  rewind(_ftraj);
  while ( ( retval = fscanf(_ftraj,"%[!$#]", &c ) ) == 1  )
  {
    /* go to next line or exit if EOF is reached */
    do {retval = fscanf(_ftraj, "%c", &c); }  while ((c != '\n') && (retval != EOF));
  }

  while ( ( retval = fscanf(_ftraj,"%*f %*d %d", &cid ) == 1 ) )
  {
    if ( cid == id ) break;  
    /* go to next line or exit if EOF is reached */
    do {retval = fscanf(_ftraj, "%c", &c); }  while ((c != '\n') && (retval != EOF));
  }

  if ( retval == EOF ) return;

  fscanf(_ftraj,"%*[^a-zA-Z] %s", tag ); 
  fprintf(stderr,"id: %d, tag: %s\n", id, tag);

}



void print_help(const char *prog_name)
{
  printf( "\n************* lineagetree ************* \n" );
  printf( "\n\
Usage : %s -h\n\
   or : %s [-g format] [-t type] [-s hor_scale] [-v vert_skip] [-l lca] [-c color] [-o outputfile] [-m font_size] file\n\n\
\t-h or --help       : Display this screen\n\n\
Options (i : integer, d : double, s : string) :\n\n\
\t-g   --format      s  : Graph file format (tfg or tabular, default=tgf)\n\
\t-j   --trajectory  s  : Trajectory file name\n\
\t-t   --treetype    s  : Tree type to print (linear or radial, default=radial)\n\
\t-s   --horscale    d  : Horizontal stretch (linear tree only, default=2.0)\n\
\t-v   --vertskip    i  : Vertical skip (linear tree only, default=25)\n\
\t-l   --lca         i  : Last common ancestor (default=0)\n\
\t-c   --color       s  : Branch colors, RRGGBB (default is color coded by cell tag)\n\
\t-o   --output-file s  : Output file name (svg, default=tree.svg)\n\
\t-m   --font-size   d  : Relative font size (in em, default=1.0)\n\
\t-b   --offset      i  : Offset controls the layout of the tree\n\n\
Convert the svg output to png:\n\
\trsvg-convert tree.svg >tree.png\n\
\n\n",\
   prog_name, prog_name );
}


