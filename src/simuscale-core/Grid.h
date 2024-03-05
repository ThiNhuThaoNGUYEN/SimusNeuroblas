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

#ifndef SIMUSCALE_GRID_H__
#define SIMUSCALE_GRID_H__

// =================================================================
//                              Includes
// =================================================================
#include <inttypes.h>
#include <stdio.h>
#include <stdlib.h>
#include <list>

#include "Cell.h"

// =================================================================
//                          Class declarations
// =================================================================


/**
 * \brief The spatial structure, in the form of a 3D matrix of voxels
*/
class Grid : public Observer {
 public :
  // =================================================================
  //                             Constructors
  // =================================================================
  Grid();
  Grid(const Grid &model) = delete;

  // =================================================================
  //                             Destructor
  // =================================================================
  virtual ~Grid();

  // =================================================================
  //                              Accessors
  // =================================================================
  std::list<Cell*>& getCellsInVoxel(Coordinates<int16_t> gridCoords);

  // =================================================================
  //                            Public Methods
  // =================================================================
  void print(void); // for debug purposes
  void Update(Observable& o, ObservableEvent e, void* arg);
  void MoveCell(Cell* cell, Coordinates<double>& old_pos);
  void RemoveCell(Cell* cell);
  void AddCell(Cell* cell);
  bool IsValid(Coordinates<int16_t> coordinates);

  /*
   * Get the coordinates of the voxel containing the cell
   */
  Coordinates<int16_t> GridCoords(Coordinates<double> realCoords) const;

  void Save(gzFile backup_file) const;
  void Load(gzFile backup_file);



 protected :
  // =================================================================
  //                           Protected Methods
  // =================================================================


  // =================================================================
  //                          Protected Attributes
  // =================================================================
  // Size of a voxel, a 3D cubic volume containing cells
  static constexpr double kVoxelSize = 2.0;
  static constexpr int16_t kNbVoxels = 52;

  // Each voxel has a list of cells
  std::list<Cell*> cells_[kNbVoxels][kNbVoxels][kNbVoxels];
};

#endif // SIMUSCALE_GRID_H__
