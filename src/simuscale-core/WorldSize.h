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

#ifndef SIMUSCALE_WORLDSIZE_H__
#define SIMUSCALE_WORLDSIZE_H__

// ============================================================================
//                                   Includes
// ============================================================================
#include "Coordinates.h"

/**
 *
 */
class WorldSize {
 public :
  // ==========================================================================
  //                                 Accessors
  // ==========================================================================
  static Coordinates<double> size() { return size_; };
  static Coordinates<double> margin() { return margin_; };
  static Coordinates<double> total_size();

  static void ValidateMovement(Coordinates<double>& dpos,
                               const Coordinates<double>& pos,
                               double radius);

  static Coordinates<double> RandomPos(double radius, double floor_z);


  // ==========================================================================
  //                                 Setters
  // ==========================================================================
  static void set_worldsize(Coordinates<double> size) {
    size_ = size; 
  }
  static void set_worldmargin(Coordinates<double> margin) {
    margin_ = margin; 
  }

 protected :
  // ==========================================================================
  //                                 Attributes
  // ==========================================================================
  /** Size of the inner part of the world (not including the margin) */
  static Coordinates<double> size_;
  /**
   * Margin used to limit border effect
   *
   * Only niche cells can be in the margin, all other cells are guarantied to
   * have the whole of their inner sphere (defined by internal_radius_) included
   * in the inner world
   */
  static Coordinates<double> margin_;

  // ==========================================================================
  //  Static class => Remove ctors and destructor
  // ==========================================================================
  WorldSize() = delete; //< Default ctor
  WorldSize(const WorldSize&) = delete; //< Copy ctor
  WorldSize(WorldSize&&) = delete; //< Move ctor
  virtual ~WorldSize() = delete; //< Destructor
};
#endif //SIMUSCALE_WORLDSIZE_H__
