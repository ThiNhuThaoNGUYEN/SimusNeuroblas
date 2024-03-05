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

#ifndef SIMUSCALE_CELL_PARAMS_H__
#define SIMUSCALE_CELL_PARAMS_H__


// =================================================================
//                              Libraries
// =================================================================
#include <inttypes.h>
#include <stdio.h>
#include <stdlib.h>

// =================================================================
//                            Project Files
// =================================================================

// =================================================================
//                          Class declarations
// =================================================================





/*!
  \brief Parameters used to create cells
*/
class CellParams {
  friend class ParamFileReader;

 public :
  // =================================================================
  //                             Constructors
  // =================================================================
  CellParams(void) = default;
  CellParams(const CellParams &model) = default;

  // =================================================================
  //                             Destructor
  // =================================================================
  virtual ~CellParams(void) = default;

  // =================================================================
  //                            Public Methods
  // =================================================================

  // =================================================================
  //                              Accessors
  // =================================================================
  double radii_ratio() const { return radii_ratio_; };
  double volume_max_min_ratio() const { return volume_max_min_ratio_; };

 protected :
  // =================================================================
  //                           Protected Methods
  // =================================================================

  // =================================================================
  //                               Attributes
  // =================================================================
  /** Ratio between the internal and external radii (= int/ext) */
  double radii_ratio_ = 0.95;

  /** Ratio between the maximal and the minimal volume */
  double volume_max_min_ratio_ = 2.2;

  // =================================================================
  //                           Private Methods
  // =================================================================
};

#endif // SIMUSCALE_CELL_PARAMS_H__
