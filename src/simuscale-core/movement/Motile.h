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

#ifndef SIMUSCALE_MOTILE_H__
#define SIMUSCALE_MOTILE_H__


// ============================================================================
//                                   Includes
// ============================================================================
#include "MoveBehaviour.h"

// ============================================================================
//                         Class declarations, Using etc
// ============================================================================
class Cell;

/**
 * Strategy pattern for cell movement: Passively mobile cells
 */
class Motile : public MoveBehaviour {
  // =================================================================
  //                        Singleton management
  // =================================================================
 private:
  static Motile instance_;
  Motile(); //< Default ctor
  Motile (const Motile&) = delete; //< Copy ctor
  Motile& operator= (const Motile&) = delete; //< Copy assign
  virtual ~Motile(); //< Destructor
 public:
  static Motile& instance() {return instance_;};

  // ==========================================================================
  //                              Public Methods
  // ==========================================================================
  void Move(Cell* cell, const double& dt) const override;
  MoveBehaviour::Type type() const override {return MOTILE;}

 protected:
  // ==========================================================================
  //                            Protected Methods
  // ==========================================================================

  // ==========================================================================
  //                               Attributes
  // ==========================================================================
};

#endif //SIMUSCALE_MOTILE_H__
