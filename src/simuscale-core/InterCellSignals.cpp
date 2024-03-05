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
#include "InterCellSignals.h"
#include "InterCellSignal.h"

#include <iostream>
#include <stdexcept>
#include <typeinfo>
// ============================================================================
//                       Definition of static attributes
// ============================================================================

// ============================================================================
//                                Constructors
// ============================================================================

// ============================================================================
//                                 Destructor
// ============================================================================

// ============================================================================
//                                   Methods
// ============================================================================

// ============================================================================
//                            Non inline accessors
// ============================================================================
double InterCellSignals::getInSignal(InterCellSignal inSignal) const {
  try {
    return signals_.at(static_cast<int>(inSignal));
  }
  catch (const std::out_of_range& oor) {
    std::cerr << "An exception was caught:\n" << oor.what();
    throw;
  }
}

void InterCellSignals::Add(InterCellSignal signal, double value) {
  try {
      /*int IntSignal =static_cast<int>(signal);
      if (IntSignal == 6){
      std::cout << "Signal" << IntSignal << "of init value" << signals_[IntSignal] << "is added " << value << std::endl;
      std::cout << signals_[static_cast<int> (signal)] + value << std::endl;}*/
    signals_[static_cast<int>(signal)] += value;

  }
  catch (std::out_of_range& oor) {
    std::cerr << "An exception was caught:\n" << oor.what();
    throw;
  }
}

void InterCellSignals::Reset() {
  for (auto& val : signals_) {
    val = 0;
  }
}

void InterCellSignals::Load(gzFile backup_file) {
  for ( int i = 0; i < nbr_signals; ++i) {
    gzread(backup_file, &signals_[i], sizeof(signals_[i]));
  }
}

void InterCellSignals::Save(gzFile backup_file) const {
  double tmp;
  for ( int i = 0; i < nbr_signals; ++i) {
    tmp = getInSignal(static_cast<InterCellSignal>(i));
    gzwrite(backup_file, &tmp, sizeof(tmp));
  }
}
