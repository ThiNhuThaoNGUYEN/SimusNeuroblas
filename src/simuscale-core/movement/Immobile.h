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

#ifndef SIMUSCALE_IMMOBILE_H
#define SIMUSCALE_IMMOBILE_H

// ============================================================================
//                                   Includes
// ============================================================================
#include "MoveBehaviour.h"

// ============================================================================
//                         Class declarations, Using etc
// ============================================================================
class Cell;

/**
 * Strategy pattern for cell movement: Immobile cells
 */
class Immobile : public MoveBehaviour {
  // =================================================================
  //                        Singleton management
  // =================================================================
 private:
  static Immobile instance_;
  Immobile(); //< Default ctor
  Immobile (const Immobile&) = delete; //< Copy ctor
  Immobile& operator= (const Immobile&) = delete; //< Copy assign
  virtual ~Immobile(); //< Destructor
 public:
  static Immobile& instance() {return instance_;}

  // ==========================================================================
  //                              Public Methods
  // ==========================================================================
  void Move(Cell*, const double&) const override {return;}
  MoveBehaviour::Type type() const override {return IMMOBILE;}

 protected:
  // ==========================================================================
  //                            Protected Methods
  // ==========================================================================

  // ==========================================================================
  //                               Attributes
  // ==========================================================================
};

#endif // SIMUSCALE_IMMOBILE_H
