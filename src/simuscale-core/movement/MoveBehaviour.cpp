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
#include "MoveBehaviour.h"
#include <cstdio>
#include <cstdlib> 
#include "Immobile.h"
#include "Mobile.h"
#include "Motile.h"

// ============================================================================
//                                   Methods
// ============================================================================
MoveBehaviour& MoveBehaviour::instance(Type type) {
  switch (type) {
    case IMMOBILE :
      return Immobile::instance();
    case MOBILE :
      return Mobile::instance();
    case MOTILE :
      return Motile::instance();
    default :
      printf("%s:%d: error: case not implemented.\n", __FILE__, __LINE__);
      exit(EXIT_FAILURE);
  };
}

void MoveBehaviour::Save(gzFile backup_file) const {
  int8_t type = static_cast<int8_t>(this->type());
  gzwrite(backup_file, &type, sizeof(type));
}

MoveBehaviour* MoveBehaviour::Load(gzFile backup_file) {
  int8_t type;
  gzread(backup_file, &type, sizeof(type));
  return &instance(static_cast<Type>(type));
}
