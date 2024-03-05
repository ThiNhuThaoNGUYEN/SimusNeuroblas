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

#ifndef BOILERPLATE_CLASS_H__
#define BOILERPLATE_CLASS_H__

// ============================================================================
//                                   Includes
// ============================================================================
#include <cinttypes>
#include <cstdio>
#include <cstdlib>
#include <cassert>



// ============================================================================
//                         Class declarations, Using etc
// ============================================================================






class BoilerplateClass
{
 public :
  // ==========================================================================
  //                               Constructors
  // ==========================================================================
  BoilerplateClass(void) = default; //< Default ctor
  BoilerplateClass(const BoilerplateClass&) = delete; //< Copy ctor
  BoilerplateClass(BoilerplateClass&&) = delete; //< Move ctor

  // ==========================================================================
  //                                Destructor
  // ==========================================================================
  virtual ~BoilerplateClass(void) = default; //< Destructor

  // ==========================================================================
  //                                Operators
  // ==========================================================================
  BoilerplateClass& operator=(const BoilerplateClass& other); //< Copy assign
  BoilerplateClass& operator=(const BoilerplateClass&& other); //< Move assign

  // ==========================================================================
  //                              Public Methods
  // ==========================================================================

  // ==========================================================================
  //                                Accessors
  // ==========================================================================





 protected :
  // ==========================================================================
  //                            Protected Methods
  // ==========================================================================

  // ==========================================================================
  //                               Attributes
  // ==========================================================================
};

#endif // BOILERPLATE_CLASS_H__
