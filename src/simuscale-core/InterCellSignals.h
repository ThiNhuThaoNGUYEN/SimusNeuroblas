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

#ifndef SIMUSCALE_INTERCELLSIGNALS_H__
#define SIMUSCALE_INTERCELLSIGNALS_H__


// ============================================================================
//                                   Includes
// ============================================================================
#include <vector>

#include <zlib.h>

#include "InterCellSignal.h"

/**
 *
 */
class InterCellSignals {

 public :
  // ==========================================================================
  //                               Constructors
  // ==========================================================================
  InterCellSignals() = default; //< Default ctor
  InterCellSignals(const InterCellSignals&) = default; //< Copy ctor // TODO <david.parsons@inria.fr> tmp
  InterCellSignals(InterCellSignals&&) = default; //< Move ctor

  // ==========================================================================
  //                                Destructor
  // ==========================================================================
  virtual ~InterCellSignals() = default; //< Destructor

  // ==========================================================================
  //                                Operators
  // ==========================================================================
  /// Copy assignment
  InterCellSignals& operator=(const InterCellSignals& other) = delete;

  /// Move assignment
  InterCellSignals& operator=(InterCellSignals&& other) = delete;

  // ==========================================================================
  //                              Public Methods
  // ==========================================================================
  void Reset();
  void Add(InterCellSignal signal, double value);
  void Load(gzFile backup_file);
  void Save(gzFile backup_file) const;

  // ==========================================================================
  //                                Accessors
  // ==========================================================================
  double getInSignal(InterCellSignal inSignal) const;
  const std::vector<double>& signals() const {
    return signals_;
  }


 protected :
  // ==========================================================================
  //                            Protected Methods
  // ==========================================================================

 protected:
// ==========================================================================
  //                               Attributes
  // ==========================================================================
  std::vector<double> signals_ = std::vector<double>(nbr_signals);
};
#endif //SIMUSCALE_INTERCELLSIGNALS_H__
