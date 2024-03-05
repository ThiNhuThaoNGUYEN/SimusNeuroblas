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

#ifndef SIMUSCALE_OBSERVER_H__
#define SIMUSCALE_OBSERVER_H__


// ============================================================================
//                                   Includes
// ============================================================================
#include "ObservableEvent.h"

class Observable;


/**
 *
 */
class Observer {

 public :
  // ==========================================================================
  //                               Constructors
  // ==========================================================================
  Observer(void) = default; //< Default ctor
  Observer(const Observer&) = delete; //< Copy ctor
  Observer(Observer&&) = delete; //< Move ctor

  // ==========================================================================
  //                                Destructor
  // ==========================================================================
  virtual ~Observer(void) = default; //< Destructor

  // ==========================================================================
  //                                Operators
  // ==========================================================================
  Observer& operator=(const Observer& other) = delete; //< Copy assignment
  Observer& operator=(const Observer&& other) = delete; //< Move assignment

  // ==========================================================================
  //                              Public Methods
  // ==========================================================================
  /// This method is called whenever the observed object is changed.
  virtual void Update(Observable& o, ObservableEvent e, void* arg) = 0;

  // ==========================================================================
  //                                 Getters
  // ==========================================================================

  // ==========================================================================
  //                                 Setters
  // ==========================================================================

 protected :
  // ==========================================================================
  //                            Protected Methods
  // ==========================================================================

  // ==========================================================================
  //                               Attributes
  // ==========================================================================
};


#endif // SIMUSCALE_OBSERVER_H__
