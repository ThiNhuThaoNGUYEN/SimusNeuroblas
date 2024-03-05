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

#ifndef SIMUSCALE_POPULATION_H__
#define SIMUSCALE_POPULATION_H__

// =================================================================
//                              Includes
// =================================================================
#include <cinttypes>
#include <cstdio>
#include <cstdlib>

#include <list>

#include <zlib.h>

#include "Cell.h"

// =================================================================
//                          Class declarations
// =================================================================
class Simulation;
class PopulationParams;
class CellParams;
class NicheParams;


/*!
  \brief A population of cells.
*/
class Population {
  friend class Simulation;
 public :
  // =================================================================
  //                             Constructors
  // =================================================================
  /* Create a population with no cells */
  Population(void) = default;
  Population(const Population &model) = delete;

  // =================================================================
  //                             Destructor
  // =================================================================
  virtual ~Population(void);


  // =================================================================
  //                            Public Methods
  // =================================================================
  void GenerateNiche(const NicheParams& niche_params);
  void AddCells(const PopulationParams& popParams,
                const CellParams& cellParams);
  void AddCells(std::list<Cell*> newCells);
  void RemoveCell(Cell* cell);

  double getAvgNbInteractions() const;
  double getAvgSignal(InterCellSignal signal) const;
  int32_t Size(void) const;

  void Save(gzFile backup_file) const;
  void Load(gzFile backup_file);


  // =================================================================
  //                              Accessors
  // =================================================================
  const std::list<Cell*>& cell_list() const { // TODO <david.parsons@inria.fr> WARNING : cells are not const !
    return cell_list_;
  }

 protected :
  // =================================================================
  //                           Protected Methods
  // =================================================================


  // =================================================================
  //                              Attributes
  // =================================================================
  std::list<Cell*> cell_list_;
};

#endif // SIMUSCALE_POPULATION_H__
