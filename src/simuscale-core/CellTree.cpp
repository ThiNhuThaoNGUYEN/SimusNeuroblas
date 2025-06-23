// ****************************************************************************
//
//              SiMuScale - Multi-scale simulation framework
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


// =================================================================
//                              Includes
// =================================================================
#include <cinttypes>
#include <cstdio>
#include <cstdlib>
#include <iostream>
#include <cfloat>
#include <cstring>

#include <CellTree.h>

using std::cout;
using std::endl;

// =================================================================
//                             Constructors
// =================================================================
CellTree::CellTree(float time) {
  root_ = AddTreeNode((TreeNode*)NULL,time,0.0,0,CellType::ROOT,NULL); // node at time=0.0, with id = 0.
}

TreeNode::TreeNode(uint32_t id, float birth, float tip, CellType cell_type, const char *cell_formalism) :
    id_(id), birth_(birth), tip_(tip), cell_type_(cell_type), cell_formalism_(cell_formalism) {
}


// =================================================================
//                            Public Methods
// =================================================================

void TreeNode::Save(gzFile backup_file) const {

  gzwrite(backup_file,&id_,sizeof(id_));
  gzwrite(backup_file,&birth_,sizeof(birth_));
  gzwrite(backup_file,&tip_,sizeof(tip_));
  int8_t cell_type = static_cast<int8_t>(cell_type_);
  gzwrite(backup_file, &cell_type, sizeof(cell_type));
  uint32_t len;
  if ( cell_formalism_ != NULL ) {
    len = strlen(cell_formalism_);
  }
  else {
    len = 0;
  }
  gzwrite(backup_file, &len, sizeof(len));
  gzwrite(backup_file, cell_formalism_, len*sizeof(char));
  uint32_t size = children_.size();
  gzwrite(backup_file,&size,sizeof(size));
  for ( auto child : children_ ) {
    child->Save(backup_file); 
  }

}

void TreeNode::Load(gzFile backup_file) {

  gzread(backup_file,&id_,sizeof(id_));
  gzread(backup_file,&birth_,sizeof(birth_));
  gzread(backup_file,&tip_,sizeof(tip_));
  int8_t cell_type;
  gzread(backup_file, &cell_type, sizeof(cell_type));
  cell_type_ = static_cast<CellType>(cell_type);
  uint32_t len;
  gzread(backup_file, &len, sizeof(len));
  char *cell_formalism = new char[len];
  gzread(backup_file, cell_formalism, len*sizeof(char));
  cell_formalism_ = cell_formalism;
  uint32_t size;
  gzread(backup_file,&size,sizeof(size));
  for ( uint32_t i = 0; i < size; ++i ) {
    TreeNode* child = new TreeNode();
    child->Load(backup_file);
    AddChild(child); 
  }
}

TreeNode* CellTree::AddTreeNode(TreeNode* mother, float time, float tip, uint32_t id, CellType cell_type, const char *cell_formalism) {  
  TreeNode* child = new TreeNode(id,time,tip,cell_type,cell_formalism);
  if ( mother != NULL ) {
    mother->AddChild(child);
  }
  return child; 
  
}

TreeNode* CellTree::GetNodeFromId(uint32_t id) {
  return GetNodeFromIdRecursive(root_,id);
}

void CellTree::Save(gzFile backup_file) const {
  root_->Save(backup_file);
}

void CellTree::Load(gzFile backup_file) {
  root_->Load(backup_file);
}

TreeNode* CellTree::GetNodeFromIdRecursive(TreeNode* root, uint32_t id) {
  TreeNode *node;
  if ( root->id() == id ) {
    return root;
  }
  else {
    for (auto child : root->children() ) { 
      node = GetNodeFromIdRecursive(child,id); 
      if ( node != NULL) { 
        return node;
      } 
    }
  }
  return NULL;
}

void CellTree::PrintNewick(FILE *file) const {
  fprintf(file,"\n");
  PrintNewickRecursive(root_,file);
  /* fprintf(file,"\033[D;\n"); */
  fseek(file,-1,SEEK_CUR);
  fprintf(file,";\n");
}

void CellTree::PrintTabularTree(FILE *file) const {
  fprintf(file, "%d %s %s %d\n",root_->id(), CellType_Names.at(root_->cell_type()).c_str(), 
      root_->cell_formalism(), root_->id());
  PrintCellListRecursive(root_,file);
  fprintf(file,"#\n");
  PrintTabularTreeRecursive(root_,file);
}

void CellTree::PrintNewickRecursive(TreeNode* node,FILE *file) const {

   /* len: length of the branch in the newick tree
   * default value for childless nodes
   * is the lifetime
   */
  float len = node->tip() - node->birth();
  char nodestr[32];

  /* first iterate over all children
   * successive children are located in
   * nested subtrees
   *
   */
  for ( auto child : node->children() ) /* print all childless nodes */
  {
    fprintf(file,"(");
    PrintNewickRecursive(child, file);
  }

  /* second, iterate back on the parent node to close the subtree
   * of descendants of the parent.
   *
   */
  for ( uint32_t i = node->nbr_children(); i--; ) /* create backward bifurcation with the parent */
  {

    /* The length of each branch are the intervals between
     * successive birth of the parent node, the births of the children nodes
     * and the tip of the parent:
     *
     *   Bparent   Bchild0   Bchild1 ... Bchild(n-1)   Dparent
     *           |         |                         |
     *          len0      len1                      lenn
     *
     */
    if ( i == node->nbr_children() - 1 )
    {
      /* duration between last child and tip */
      len = node->tip() - node->children().at(i)->birth();
    }
    else
    {
      /* duration between two births */
      len = node->children().at(i+1)->birth() - node->children().at(i)->birth();
    }
    if ( node->id() == 0 ) /* root */
      snprintf(nodestr,5,"root");
    else
      snprintf(nodestr,31,"%d",node->id());
    fprintf(file,"%s:%4.2f)%d>",nodestr,len,node->children().at(i)->id());

    /* this will be used only outside the for loop
     * if the node has no children, len has its initial value
     * if the node has children, then len will be set to the value
     * below
     */
    len = node->children().at(0)->birth() - node->birth();
  }
  /* Either a leaf, or the initial branch of a node with
   * children.
   */
  if ( node->id() == 0 ) /* root */
    snprintf(nodestr,5,"root");
  else
    snprintf(nodestr,31,"%d",node->id());
  fprintf(file,"%s:%4.2f,", nodestr, len);

}


void CellTree::PrintTabularTreeRecursive(TreeNode* node, FILE *file) const {
  
  for ( auto child : node->children() ) /* print all childless nodes */
  {
    fprintf(file, "%d %d %.2f %.2f\n",node->id(),child->id(),child->birth(),child->tip());
  }
  for ( auto child : node->children() ) /* print all childless nodes */
  {
    PrintTabularTreeRecursive(child, file);
  }

}

void CellTree::PrintCellListRecursive(TreeNode* node, FILE *file) const {
  
  for ( auto child : node->children() ) /* print all child nodes */
  {
    fprintf(file, "%d %s %s %d\n",child->id(), CellType_Names.at(child->cell_type()).c_str(), 
        child->cell_formalism(),child->id());
  }
  for ( auto child : node->children() ) 
  {
    PrintCellListRecursive(child, file);
  }

}
