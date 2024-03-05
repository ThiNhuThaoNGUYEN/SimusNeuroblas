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
#include "Grid.h"

#include <cstdio>
#include <cstdlib>
#include <cmath>
#include <cinttypes>
#include <cassert>

#include <algorithm>
#include <functional>
#include <iostream>

#include "Coordinates.h"

using std::cout;
using std::endl;



// =================================================================
//                    Definition of static attributes
// =================================================================

// =================================================================
//                             Constructors
// =================================================================
Grid::Grid() = default;


// =================================================================
//                             Destructor
// =================================================================
Grid::~Grid() = default;


// =================================================================
//                            Public Methods
// =================================================================
void Grid::print() {
  for(int32_t x=0; x < kNbVoxels; x++)
    for(int32_t y = 0; y < kNbVoxels; y++)
      for(int32_t z = 0; z < kNbVoxels; z++)
        if (! (cells_[x][y][z].empty()))
          cout << "There are " << cells_[x][y][z].size() <<
              "cells in voxel (" << x << ", " << y << ", " << z << ")" << endl;
}

/**
 * Called whenever an observed object notifies us
 *
 * Here when a cell has moved
 */
void Grid::Update(Observable& o, ObservableEvent e, void* arg) {
  assert(e == CELL_MOVED);
  MoveCell(static_cast<Cell*>(&o), *static_cast<Coordinates<double>*>(arg));
}

void Grid::MoveCell(Cell* cell, Coordinates<double>& old_pos) {
  // Update grid if there was a movement
  Coordinates<int16_t> newGridCoords{
      static_cast<int16_t>(floor(cell->pos_x() / kVoxelSize)),
      static_cast<int16_t>(floor(cell->pos_y() / kVoxelSize)),
      static_cast<int16_t>(floor(cell->pos_z() / kVoxelSize))
  };
  Coordinates<int16_t> oldGridCoords{
      static_cast<int16_t>(floor(old_pos.x / kVoxelSize)),
      static_cast<int16_t>(floor(old_pos.y / kVoxelSize)),
      static_cast<int16_t>(floor(old_pos.z / kVoxelSize))
  };
  assert((newGridCoords.x >= 0) && (newGridCoords.x < kNbVoxels));
  assert((newGridCoords.y >= 0) && (newGridCoords.y < kNbVoxels));
  assert((newGridCoords.z >= 0) && (newGridCoords.z < kNbVoxels));

  // Change voxel the cell is in
  if (newGridCoords != oldGridCoords) {
    list<Cell*>& oldList =
        cells_[oldGridCoords.x][oldGridCoords.y][oldGridCoords.z];
    list<Cell*>& newList =
        cells_[newGridCoords.x][newGridCoords.y][newGridCoords.z];

    // Move cell from oldList to newList
    list<Cell*>::iterator it = find(oldList.begin(), oldList.end(), cell);
    // If the cell is not in the list, it's a new cell that has not been placed
    // on the grid yet => nothing to do
    if (it == oldList.end()) return;

    // Actually move cell from oldList to newList
    newList.splice(newList.end(), oldList, it);

    // Sort the list of cells in the "new" voxel
    newList.sort([] (Cell* first, Cell* second) {
        return first->id() < second->id();
    });
  }
}

void Grid::RemoveCell(Cell* cell) {
  auto cellCoords = GridCoords(cell->pos());
  cells_[cellCoords.x][cellCoords.y][cellCoords.z].remove(cell);
}

void Grid::AddCell(Cell* cell) {
  Coordinates<int16_t> gridCoords{
      static_cast<int16_t>(floor(cell->pos_x() / kVoxelSize)),
      static_cast<int16_t>(floor(cell->pos_y() / kVoxelSize)),
      static_cast<int16_t>(floor(cell->pos_z() / kVoxelSize))
  };
  assert((gridCoords.x >= 0) && (gridCoords.x < kNbVoxels));
  assert((gridCoords.y >= 0) && (gridCoords.y < kNbVoxels));
  assert((gridCoords.z >= 0) && (gridCoords.z < kNbVoxels));
  cells_[gridCoords.x][gridCoords.y][gridCoords.z].push_back(cell);

  // Subscribe as an observer of the cell
  cell->AddObserver(this, CELL_MOVED);
}

void Grid::Save(gzFile) const {
}

void Grid::Load(gzFile) {
}


// =================================================================
//                           Protected Methods
// =================================================================





// =================================================================
//                          Non inline accessors
// =================================================================
list<Cell*>& Grid::getCellsInVoxel(Coordinates<int16_t> gridCoords) {
  assert (gridCoords.x >= 0 and gridCoords.x <= kNbVoxels and
          gridCoords.y >= 0 and gridCoords.y <= kNbVoxels and
          gridCoords.z >= 0 and gridCoords.z <= kNbVoxels);
  return cells_[gridCoords.x][gridCoords.y][gridCoords.z];
}

Coordinates<int16_t> Grid::GridCoords(Coordinates<double> realCoords) const {
  return Coordinates<int16_t>{
      static_cast<int16_t>(floor(realCoords.x / kVoxelSize)),
      static_cast<int16_t>(floor(realCoords.y / kVoxelSize)),
      static_cast<int16_t>(floor(realCoords.z / kVoxelSize))};
}

bool Grid::IsValid(Coordinates<int16_t> coordinates) {
  return (coordinates.x >= 0) && (coordinates.x < kNbVoxels) &&
      (coordinates.y >= 0) && (coordinates.y < kNbVoxels) &&
      (coordinates.z >= 0) && (coordinates.z < kNbVoxels);
}
