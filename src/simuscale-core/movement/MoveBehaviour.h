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

#ifndef SIMUSCALE_MOVEBEHAVIOUR_H
#define SIMUSCALE_MOVEBEHAVIOUR_H

// ============================================================================
//                                   Includes
// ============================================================================
#include <zlib.h>

// ============================================================================
//                         Class declarations, Using etc
// ============================================================================
class Cell;

/**
 * Strategy pattern for cell movement: base class
 */
class MoveBehaviour {
 public:
  typedef enum {
    IMMOBILE,
    MOBILE,
    MOTILE /* implementation plugin-dependent */
  } Type;

  // ==========================================================================
  //                               Constructors
  // ==========================================================================
  MoveBehaviour(void) = default; //< Default ctor
  MoveBehaviour(const MoveBehaviour&) = delete; //< Copy ctor
  MoveBehaviour(MoveBehaviour&&) = delete; //< Move ctor

  // ==========================================================================
  //                                Destructor
  // ==========================================================================
  virtual ~MoveBehaviour(void) = default; //< Destructor

  // ==========================================================================
  //                                Operators
  // ==========================================================================
  MoveBehaviour& operator=(const MoveBehaviour& other) = delete; //< Copy assign
  MoveBehaviour& operator=(MoveBehaviour&& other) = delete; //< Move assign

  // ==========================================================================
  //                              Public Methods
  // ==========================================================================
  static MoveBehaviour& instance(Type type);
  virtual void Move(Cell* cell, const double& dt) const = 0;
  virtual Type type() const = 0;
  virtual void Save(gzFile backup_file) const;
  static MoveBehaviour* Load(gzFile backup_file);

  // ==========================================================================
  //                                Accessors
  // ==========================================================================





 protected:
  // ==========================================================================
  //                            Protected Methods
  // ==========================================================================

  // ==========================================================================
  //                               Attributes
  // ==========================================================================
};

#endif // SIMUSCALE_MOVEBEHAVIOUR_H
