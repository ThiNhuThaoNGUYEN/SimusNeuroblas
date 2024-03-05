/** ctreesvg.c 
 * print tabular/TGF tree to svg
 * compile:
 * gcc ctreesvg.c -o ctreesvg.out
 * run with a TGF file:
 * ./ctreesvg.out treelist.tgf 1 >tree.svg && rsvg-convert tree.svg >tree.png
 * 
 **/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

struct cell_node {
  int   id;
  float birth;
  float death;

  size_t nbr_children;

  struct cell_node *mother;
  struct cell_node **children;
};

float _max_time;
float _hor_scale = 2.0;
int   _offset = 4;
int   _vert_skip = 25;

struct cell_node*  new_cell_node(struct cell_node *mother, float time, int id)
{
  struct cell_node *child = malloc(sizeof(struct cell_node));
  child->id = id;
  child->birth = time;
  child->death = INFINITY;
  child->nbr_children = 0;
  child->children = (struct cell_node**)NULL;
  child->mother = mother;

  if ( mother != NULL )
  {
    mother->nbr_children++;
    mother->children = realloc(mother->children,mother->nbr_children*sizeof(struct cell_node*));
    mother->children[mother->nbr_children-1] = child;
  }
  return child;
}

void link_child(struct cell_node *mother, struct cell_node *child)
{
  if ( mother != NULL )
  {
    mother->nbr_children++;
    mother->children = realloc(mother->children,mother->nbr_children*sizeof(struct cell_node*));
    mother->children[mother->nbr_children-1] = child;
  }
  else
  {
    fprintf(stderr,"Error, mother is NULL, in %s at line %d.",__func__,__LINE__);
  }
}

struct cell_node* get_node_from_id(struct cell_node *root, int id)
{
  struct cell_node *node;
  if ( root->id == id )
  {
    return root;
  }
  else
  {
    for ( int i = 0; i < root->nbr_children; i++ )
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

void nbr_descendants(struct cell_node *root, int *s)
{
  *s += 1; 
  for ( int i = 0; i < root->nbr_children; i++ )
  {
    nbr_descendants(root->children[i], s); 
  }
}

void print_tree(struct cell_node *root, int *y)
{
  for ( int i = root->nbr_children; i--; )
  {
    int s1 = 0; 
    int start = (int)(_hor_scale*(root->children[i])->birth) + 4*_offset;
    int stop;

    /* get the number of nodes to the right of the tree */
    for ( int k = i+1; k < root->nbr_children; k++)
    {
      nbr_descendants(root->children[k],&s1);
    }

    /* print horizontal connector */
    printf("<path d='M%d %d C%d %d, %d %d, %d %d S%d %d, %d %d' stroke='white' fill='transparent'/>\n",
        start+3*_offset,*y,              /* lower-right end */
        start+3*_offset,*y-_vert_skip/2,  /* lower-right end control point */
        start+2*_offset,*y-_vert_skip/2,  /* inflexion control point */
        start+_offset,*y-_vert_skip/2,    /* inflexion point */
        start,*y-_vert_skip/2,           /* upper-left end control point */
        start,*y-_vert_skip*(s1+1));     /* upper-left end */


    /* print the horizontal lifespan of the node */
    stop = start + (int)(_hor_scale*((root->children[i])->death  - (root->children[i])->birth)); 
    if ( stop > (start + 3*_offset) )
      printf("<line x1='%d' x2='%d' y1='%d' y2='%d' stroke='white'/>\n",start+3*_offset,stop,*y,*y);

    /* print connector ends */
    printf("<circle cx='%d' cy='%d' r='%d' fill='white'/>\n",start+3*_offset,*y,_offset/2);
    printf("<circle cx='%d' cy='%d' r='%d' fill='white'/>\n",start,*y-_vert_skip*(s1+1),_offset/2);

    /* print node id */
    printf("<text x='%d' y='%d' stroke='white' text-anchor='end' font-family='Arial, Helvetica, sans-serif'>%d</text>\n",start+2*_offset,*y,root->children[i]->id);
    printf("<text x='%d' y='%d' fill='white' font-family='Arial, Helvetica, sans-serif'>%d</text>\n",stop+_offset,*y+_offset,root->children[i]->id);

    /* print tip */
    printf("<line x1='%d' x2='%d' y1='%d' y2='%d' stroke='white'/>\n",stop,stop,*y-_offset,*y+_offset);      

    *y += _vert_skip;
    print_tree(root->children[i], y);
  }
}

/* produce a newick tree */
void print_newick(struct cell_node *node)
{ 
  /* len: length of the branch in the newick tree 
   * default value for childless nodes 
   * is the lifetime
   */
  float len = node->death - node->birth; 

  /* first iterate over all children 
   * successive children are located in 
   * nested subtrees
   * If the tree has nodes
   *    -1 -> 0
   *     0 -> 1
   *     0 -> 2
   * then the expected tree is
   * 
   *    |------------ -1
   *    | 
   *    |-1         |---- 0
   *    |      |----|0   
   *    |------|0   |---- 2
   *           |------------ 1
   *
   * this means that (2,0)0 is a subtree of 0 in
   * of (1,0)0, i.e. (1,(2,0)0)0
   *
   */
  for ( int i = 0; i < node->nbr_children; i++ ) /* print all childless nodes */
  {
    printf("(");
    print_newick(node->children[i]); 
  }

  /* second, iterate back on the parent node to close the subtree
   * of descendants of the parent.
   *
   */
  for ( int i = node->nbr_children; i--; ) /* create backward bifurcation with the parent */
  {

    /* The length of each branch are the intervals between 
     * successive birth of the parent node, the births of the children nodes 
     * and the death of the parent:
     *
     *   Bparent   Bchild0   Bchild1 ... Bchild(n-1)   Dparent
     *           |         |                         |
     *          len0      len1                      lenn
     *
     */
    if ( i == node->nbr_children - 1 )
    {
      /* duration between last child and death */
      len = node->death - node->children[i]->birth;
    }
    else 
    {
      /* duration between two births */
      len = node->children[i+1]->birth - node->children[i]->birth;
    }
    printf("%d:%4.2f)",node->id,len);

    /* this will be used only outside the for loop
     * if the node has no children, len has its initial value
     * if the node has children, then len will be set to the value 
     * below
     */
    len = node->children[0]->birth - node->birth;
  }
  /* Either a leaf, or the initial branch of a node with 
   * children. 
   */
  printf("%d:%4.2f,", node->id, len); 
}

void delete_tree(struct cell_node *root)
{
  for ( int i = 0; i < root->nbr_children; i++ )
  {
    delete_tree(root->children[i]);
  }
  free(root);
}

int not_a_child(struct cell_node *root, int id)
{
  for ( int i = 0; i < root->nbr_children; i++ )
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
  int stop, parent, event, child;
  int max_id = -1;
  float time, tip;
  struct cell_node *node = NULL;
  int read = 0; /* 0: read nodes, 1: read edges */
  struct cell_node *node_list = new_cell_node(NULL,0.0,0);
  size_t nbr_nodes = 0;
  int add_node = 0;
  int root_id = -1;


  while ( getline(&line,&linecap,fid) > 0 )
  {

    if ( line[strcspn(line,"#")] == '#' )
    {
      read = 1;
      continue;
    }

    if ( read == 0 )
    {
      *descr = 0;
      if ( sscanf(line,"%d %[^\n]",&parent,descr) < 1)
      {
        break;
      }
      if ( strncasecmp(descr,"root",4) == 0 )
      {
        root_id = parent;
      }
      /* printf("new node %d\n",parent); */
      max_id = parent > max_id ? parent : max_id;
      node_list->id = max_id+1; /* change id to unique value */

      /* check if parent is in ids */
      add_node = 1;
      for ( int i = 0; i < node_list->nbr_children; i++ )
      {
        if ( node_list->children[i]->id == parent ) 
        {
          add_node = 0;
          break;
        }
      }
      if ( add_node )
      {
        new_cell_node(node_list,0.0,parent);
      }
    }
    else /* read == 1 */
    {
      if ( sscanf(line,"%d %d %f %f",&parent,&child,&time,&tip) < 3 )
      {
        break;
      }
      _max_time = time > _max_time ? time : _max_time;
      _max_time = tip > _max_time ? tip : _max_time;

      /* printf("new edge %d -> %d at t=%f\n",parent,child,time); */
      max_id = parent > max_id ? parent : max_id;

      /* check if parent, child is in node_list */
      if ( not_a_child(node_list,parent) )
      {
        fprintf(stderr,"Error. %d is not a node.\n",parent);
        exit(1);
      }
      if ( not_a_child(node_list,child) )
      {
        fprintf(stderr,"Error. %d is not a node.\n",child);
        exit(1);
      }

      if ( child == root_id ) /* death/tip */
      {
        node = get_node_from_id(node_list,parent);
        node->death = time;
      }
      else /* birth: add node/edge to root */
      {
        node = get_node_from_id(node_list,child);
        link_child(get_node_from_id(node_list,parent),
             node);
        node->birth = time;
        node->death = tip;
      }
    }
  }
  *root = get_node_from_id(node_list,root_id);
  (*root)->death = _max_time;
  free(node_list->children);
  free(node_list);
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
          new_cell_node(node,time,child);
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

int main ( int argc, char *argv[] )
{

  FILE *fid = NULL;
  struct cell_node *root = (struct cell_node*)NULL; 
  int filetype = 0; /* 0: tabular 1: TGF */
  int y0 = 25;
 


  if ( argc >= 2 )
  {
    if ( ( fid = fopen(argv[1],"r") ) == NULL )
    {
      fprintf(stderr,"could not open %s. exiting\n",argv[1]);
      return 1;
    }
    if ( argc >= 3 )
    {
      filetype = strtol(argv[2],NULL,10); 
    }
    if ( argc >= 4 )
    {
      _hor_scale = strtof(argv[3],NULL); 
    }
    if ( argc >= 5 )
    {
      _offset = strtol(argv[4],NULL,10); 
    }
    if ( argc >= 6 )
    {
      _vert_skip = strtol(argv[5],NULL,10); 
    }
  }
  else
  {
      fprintf(stderr,"usage %s <filename> [<filetype>:0 for tabular, 1 for tgf] [<horizontal scale=2.0>:float]\n" \
                     "  [<offset=4>:int] [<vertical_skip=25>:int]\n",argv[0]);
      return 1;
  }
  
  if ( filetype == 0 )
  {
    root = new_cell_node(NULL,0.0,-1);
    read_tabular_tree_file(root, fid);
  }
  else
  {
    root = (struct cell_node*)NULL;
    read_trivial_graph_format_file(&root, fid);
  }

	printf("<?xml version='1.0'?>\n");
	printf("<svg xmlns='%s' version='1.1'>\n", \
		"http://www.w3.org/2000/svg");
  print_tree(root, &y0);
  printf("</svg>\n");

  delete_tree(root);

  fclose(fid);
  return 0;

}

