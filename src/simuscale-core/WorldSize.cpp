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
#include "WorldSize.h"

#include "Alea.h"
#include "Coordinates.h"


// =================================================================
//                    Definition of static attributes
// =================================================================
Coordinates<double> WorldSize::size_ {40.0, 40.0, 40.0};
Coordinates<double> WorldSize::margin_ {2.0, 2.0, 0.0};

/**
 * Get the total size of the world (including margins)
 */
Coordinates<double> WorldSize::total_size() {
  return size_ + 2 * margin_;
}

/**
 * Cancel any (x-, y- or z-) component of the movement dpos that would cause
 * the sphere defined by (pos, radius) to extend beyond the world boundaries
 * if they were applied
 */
void WorldSize::ValidateMovement(Coordinates<double>& dpos,
                                 const Coordinates<double>& pos,
                                 double radius) {
  if (pos.x + dpos.x <= radius ||
      pos.x + dpos.x >= size_.x - radius)
    dpos.x = 0.0;
  if (pos.y + dpos.y <= radius ||
      pos.y + dpos.y >= size_.y - radius)
    dpos.y = 0.0;
  if (pos.z + dpos.z <= radius ||
      pos.z + dpos.z >= size_.z - radius)
    dpos.z = 0.0;
}

/**
 * Returns a random position within the world boundaries (margin excluded).
 */
Coordinates<double> WorldSize::RandomPos(double radius, double floor_z) {
  Coordinates<double> pos;
  pos.x = margin_.x + radius + size_.x / 3.0 +
          (size_.x / 3.0 - 2 * radius) * (Alea::random());
  pos.y = margin_.y + radius + size_.y / 3.0 +
          (size_.y / 3.0 - 2 * radius) * (Alea::random());
  pos.z = margin_.z + radius + floor_z;
  return pos;
}
