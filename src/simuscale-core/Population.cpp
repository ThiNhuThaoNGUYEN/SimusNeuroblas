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
#include "Population.h"

#include <cstdio>
#include <cstdlib>
#include <cmath>
#include <params/CellParams.h>

#include <iostream>

#include "InterCellSignal.h"
#include "params/PopulationParams.h"
#include "params/NicheParams.h"
#include "Alea.h"
#include "WorldSize.h"
#include "movement/Immobile.h"
#include "movement/Mobile.h"

using std::cout;
using std::endl;


// =================================================================
//                    Definition of static attributes
// =================================================================

// =================================================================
//                             Constructors
// =================================================================


// =================================================================
//                             Destructor
// =================================================================
Population::~Population(void) {
  for (Cell* cell : cell_list_) {
    delete cell;
  }
}


// =================================================================
//                            Public Methods
// =================================================================
void Population::GenerateNiche(const NicheParams& niche_params) {
  // Cells will be placed on an hexagonal mesh
  // Cells on the same row will be placed on the same x-axis, i.e. will
  // have the same y and z values. Two successive cells will hence have
  // their x value distant by 2 * internal_radius
  // Cells on row r+1 will have their x shifted by internal_radius and
  // their y shifted by internal_radius * sqrt(3) w.r.t. row r
  auto internal_radius = niche_params.internal_radius();

//  Coordinates<double> world_total_size = WorldSize::total_size();
  double initial_volume = 4 * M_PI / 3 * pow(internal_radius / 0.9, 3);
  int16_t nb_rows = static_cast<int16_t>(
      floor(WorldSize::size().y / internal_radius / sqrt(3)) - 1);
  int16_t nb_cols = static_cast<int16_t>(
      floor(WorldSize::size().x / internal_radius / 2) - 1);

  Coordinates<double> init_pos(3.0 * internal_radius / 2,
                               3.0 * internal_radius / 2,
                               0.0);
  Coordinates<double> pos = init_pos;

  for (int16_t i = 0 ; i < nb_rows ; i++) {
    if (i % 2 != 0) {
      pos.x += internal_radius;
    }
    for (int16_t j = 0 ; j < nb_cols ; j++) {
      try {
        cell_list_.push_back(Cell::MakeCell(niche_params.formalism(),
                                            Immobile::instance(),
                                            NICHE,
                                            pos, initial_volume, 1.0, 0.0));
      }
      catch (const std::exception& e) {
        cout << e.what() << endl;
        std::terminate();
      }

      pos.x += 2.0 * internal_radius;
    }
    pos.x = init_pos.x;
    pos.y += internal_radius * sqrt(3);
  }
}

/**
 * Add cells of the given type and formalism to the population
 *
 * Can be called several times to setup a mixed population
 */
void Population::AddCells(const PopulationParams& popParams,
                          const CellParams&) {
  double floor_z = 2.0; // TODO(dpa) Magic number !
  FILE* init_fid;
  bool use_init_file = false;

  if ( popParams.init_file().length() ) {
    if ( (init_fid = fopen(popParams.init_file().c_str(),"r")) == NULL ) {
      printf("ERROR : couldn't open file %s\n",
             popParams.init_file().c_str());
      exit(EXIT_FAILURE);
    }
    use_init_file = true;
  }

  for (int32_t i = 0; i < popParams.nb_cells(); i++) {

    // Determine the initial volume of the cell.
    // Value in [vmin, 2*vmin)
    CellSize size(popParams.volume_min() * (1 + Alea::random()),
                  popParams.volume_min());
    // Determine the initial position of the cell
    // Center cannot be within internal_radius of a wall
    Coordinates<double> pos =
        WorldSize::RandomPos(size.internal_radius(), floor_z);

    if ( use_init_file ) {
      int32_t id; // cell index, unused
      float r;    // cell radius
      int32_t c;  // color index, unused
      fscanf(init_fid,"%d %lf %lf %lf %f %d\n",&id, &(pos.x), &(pos.y), &(pos.z), &r, &c);
      size.set_volume(size.compute_volume(r));
    }

    try {
      cell_list_.push_back(
          Cell::MakeCell(popParams.formalism(),
                        MoveBehaviour::instance(popParams.mb_type()),
                        popParams.cell_type(),
                        pos, size.volume(),
                        popParams.volume_min(),
                        popParams.doubling_time()));
    }
    catch (const std::exception& e) {
      cout << e.what() << endl;
      std::terminate();
    }
  }
}


double Population::getAvgNbInteractions() const {
  if (cell_list_.size() == 0) return -1.0; /* no cell in the population */

  double res = 0;
  for (Cell* cell : cell_list_) {
    res += cell->neighbours().size();
  }

  return res / cell_list_.size();
}

double Population::getAvgSignal(InterCellSignal signal) const {
  if (cell_list_.size() == 0) return -1.0; /* no cell in the population */

  double res = 0;
  for (Cell* cell : cell_list_) {
    res += cell->get_output(signal);
  }
  return res / cell_list_.size();
}

void Population::RemoveCell(Cell* cell) {
  cell_list_.remove(cell);
}

int32_t Population::Size(void) const {
  return cell_list_.size();
}

void Population::Save(gzFile backup_file) const {
  Cell::SaveStatic(backup_file);

  int32_t nbCells = cell_list_.size();
  gzwrite(backup_file, &nbCells, sizeof(nbCells));
  for(Cell* cell : cell_list_) {
    cell->Save(backup_file);
  }
}

void Population::Load(gzFile backup_file) {
  // Load static attributes of class Cell
  Cell::LoadStatic(backup_file);

  // Load the actual cells
  int32_t nbCells;
  gzread(backup_file, &nbCells, sizeof(nbCells));
  for(int32_t i = 0 ; i < nbCells ; i++) {
    try {
      cell_list_.push_back(Cell::LoadCell(backup_file));
    }
    catch (const std::out_of_range& e) {
      cout << "Error while loading cells: unknown cell class" << endl;
      std::terminate();
    }
  }
}

// =================================================================
//                           Protected Methods
// =================================================================
/*
 * Add cells from a list of Cell objects.
 */
void Population::AddCells(list<Cell*> newCells) {
  cell_list_.splice(cell_list_.end(), newCells);
}
