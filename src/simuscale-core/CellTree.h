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

#ifndef SIMUSCALE_CELLTREE_H__
#define SIMUSCALE_CELLTREE_H__


// =================================================================
//                              Includes
// =================================================================
#include <cinttypes>
#include <cstdio>
#include <cstdlib>
#include <cfloat>
#include <vector>
#include <zlib.h>

#include <CellType.h>

// ============================================================================
//                         Class declarations, Using etc
// ============================================================================

class TreeNode 
{

  public :
  // =================================================================
  //                             Constructors
  // =================================================================
  TreeNode() = default;
  TreeNode(uint32_t id, float birth, float tip, CellType cell_type, const char *cell_formalism);


  // =================================================================
  //                             Destructor
  // =================================================================
  virtual ~TreeNode() = default;


  // =================================================================
  //                            Public Methods
  // =================================================================
  void AddChild(TreeNode* child) { children_.push_back(child);};
  void set_time_of_tip(float time) { tip_ = time; };

  void Save(gzFile backup_file) const;
  void Load(gzFile backup_file);

  // =================================================================
  //                              Accessors
  // =================================================================

  uint32_t id() const {return id_;};
  float birth() const {return birth_;};
  float tip() const {return tip_;};
  CellType cell_type() const {return cell_type_;};
  const char *cell_formalism() const {return cell_formalism_;}
  uint32_t nbr_children() {return children_.size();};

  const std::vector<TreeNode*>& children() const {return children_;};

 protected :
  // =================================================================
  //                           Protected Methods
  // =================================================================

 private:
  // =================================================================
  //                           Private Methods
  // =================================================================

 protected:
  // =================================================================
  //                              Attributes
  // =================================================================
    
  uint32_t id_;
  
  /** birth: time of birth of the cell */
  float birth_;

  /** tip: time of tip of the cell: either death, end of simulation or 0 if node is root */
  float tip_; 

  /** cell_type_: cell type of the cell at birth */
  CellType cell_type_;

  /** cell_formalism_: cell formalism used */
  const char *cell_formalism_;

  std::vector<TreeNode*> children_;

};

/*!
  \brief A Cell Phylogenic Tree 
*/
class CellTree 
{

  public :
  // =================================================================
  //                             Constructors
  // =================================================================
  CellTree() = default;
  CellTree(float time);

  // =================================================================
  //                             Destructor
  // =================================================================
  virtual ~CellTree() = default;


  // =================================================================
  //                            Public Methods
  // =================================================================
  void PrintNewick(FILE *file) const;
  void PrintTabularTree(FILE *file) const;
  TreeNode* AddTreeNode(TreeNode* mother, float time, float tip, uint32_t id, CellType cell_type, const char *cell_formalism);
  TreeNode* GetNodeFromId(uint32_t id);

  void Save(gzFile backup_file) const;
  void Load(gzFile backup_file);

  // =================================================================
  //                              Accessors
  // =================================================================
  TreeNode* root() const {return root_;};

 protected :
  // =================================================================
  //                           Protected Methods
  // =================================================================

 private:
  // =================================================================
  //                           Private Methods
  // =================================================================
  TreeNode* GetNodeFromIdRecursive(TreeNode* node,uint32_t id);
  void PrintNewickRecursive(TreeNode* node, FILE *file) const;
  void PrintCellListRecursive(TreeNode* node, FILE *file) const;
  void PrintTabularTreeRecursive(TreeNode* node, FILE *file) const;

 protected:
  // =================================================================
  //                              Attributes
  // =================================================================
  TreeNode* root_;

};


#endif // SIMUSCALE_CELLTREE_H__
