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

#ifndef SIMUSCALE_WORLDSIZE_PARAMS_H__
#define SIMUSCALE_WORLDSIZE_PARAMS_H__


// =================================================================
//                              Libraries
// =================================================================
#include <inttypes.h>
#include <stdio.h>
#include <stdlib.h>

#include <WorldSize.h>

// =================================================================
//                            Project Files
// =================================================================

// =================================================================
//                          Class declarations
// =================================================================





/*!
  \brief Parameters used to create cells
*/
class WorldSizeParams {
  friend class ParamFileReader;

 public :
  // =================================================================
  //                             Constructors
  // =================================================================
  WorldSizeParams(void) = default;
  WorldSizeParams(const WorldSizeParams &model) = default;

  // =================================================================
  //                             Destructor
  // =================================================================
  virtual ~WorldSizeParams(void) = default;

  // =================================================================
  //                            Public Methods
  // =================================================================

  // =================================================================
  //                              Accessors
  // =================================================================
  Coordinates<double> size() const { return size_; };
  Coordinates<double> margin() const { return margin_; };

 protected :
  // =================================================================
  //                           Protected Methods
  // =================================================================

  // =================================================================
  //                               Attributes
  // =================================================================

  /** Size of the inner part of the world (not including the margin) */
  Coordinates<double> size_ {40.0, 40.0, 40.0};

  
  /**
   * Margin used to limit border effect
   *
   * Only niche cells can be in the margin, all other cells are guaranteed to
   * have the whole of their inner sphere (defined by internal_radius_) included
   * in the inner world
   */
  Coordinates<double> margin_ {2.0, 2.0, 2.0};


  // =================================================================
  //                           Private Methods
  // =================================================================
};

#endif // SIMUSCALE_WORLDSIZE_PARAMS_H__
