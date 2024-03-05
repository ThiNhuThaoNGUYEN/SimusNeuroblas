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

#ifndef __POPULATION_PARAMS_H__
#define __POPULATION_PARAMS_H__



// =================================================================
//                              Includes
// =================================================================
#include <inttypes.h>
#include <stdio.h>
#include <stdlib.h>

#include "CellType.h"
#include "movement/MoveBehaviour.h"



/*!
  \brief Parameters used to create a population
*/
class PopulationParams
{
 public :
  // =================================================================
  //                             Constructors
  // =================================================================
  PopulationParams(void) = delete;
  PopulationParams(const PopulationParams & model) = default;
  PopulationParams(PopulationParams && rhs) = default;
  inline PopulationParams(int32_t nb_cells,
                          CellType cell_type,
                          CellFormalism formalism,
                          MoveBehaviour::Type mb_type,
                          double doubling_time,
                          double volume_min,
                          const char* init_file);

  // =================================================================
  //                             Destructor
  // =================================================================
  virtual ~PopulationParams(void) = default;

  // =================================================================
  //                            Public Methods
  // =================================================================

  // =================================================================
  //                        Accessors: Getters
  // =================================================================
  int32_t nb_cells() const { return nb_cells_; };
  const CellType& cell_type() const { return cell_type_; };
  const CellFormalism& formalism() const { return formalism_; };
  const MoveBehaviour::Type& mb_type() const { return mb_type_; };
  double doubling_time() const { return doubling_time_; };
  double volume_min() const { return volume_min_; };
  const std::string init_file() const { return init_file_; };

 protected :
  // =================================================================
  //                           Protected Methods
  // =================================================================

  // =================================================================
  //                              Attributes
  // =================================================================
  /** Number of cells of the population */
  int32_t nb_cells_;
  /** Initial cell type */
  CellType cell_type_;
  /** Formalism used to model the cell's dynamics */
  CellFormalism formalism_;
  /** Move Behaviour type */
  MoveBehaviour::Type mb_type_;
  /** Characteristic volume doubling time */
  double doubling_time_;
  /** Minimum volume of a cell */
  double volume_min_;

  /** initial state file name */
  std::string init_file_ = "";
};


// =====================================================================
//                           Getters' definitions
// =====================================================================

// =====================================================================
//                       Other inline method definitions
// =====================================================================
PopulationParams::PopulationParams(int32_t nb_cells,
                                   CellType cell_type,
                                   CellFormalism formalism,
                                   MoveBehaviour::Type mb_type,
                                   double doubling_time,
                                   double volume_min,
                                   const char* init_file) :
    nb_cells_(nb_cells), cell_type_(cell_type),
    formalism_(formalism), mb_type_(mb_type),
    doubling_time_(doubling_time), volume_min_(volume_min), init_file_(init_file) {
};

#endif // __POPULATION_PARAMS_H__
