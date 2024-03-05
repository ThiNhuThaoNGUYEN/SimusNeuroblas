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

// ============================================================================
//                                   Includes
// ============================================================================
#include "Motile.h"

#include "Cell.h"
#include "Coordinates.h"

// ============================================================================
//                       Definition of static attributes
// ============================================================================
// The singleton instance
Motile Motile::instance_;

// ============================================================================
//                                Constructors
// ============================================================================
Motile::Motile() {
}

// ============================================================================
//                                 Destructor
// ============================================================================
Motile::~Motile() {
}

// ============================================================================
//                                   Methods
// ============================================================================
void Motile::Move(Cell* cell, const double& dt) const {
  // Compute the displacement in space.
  Coordinates<double> dpos{dt * cell->mechanical_force_.x,
                           dt * cell->mechanical_force_.y,
                           dt * cell->mechanical_force_.z };

  dpos += cell->MotileDisplacement(dt);

  // Apply displacement
  cell->Move(dpos);
}
