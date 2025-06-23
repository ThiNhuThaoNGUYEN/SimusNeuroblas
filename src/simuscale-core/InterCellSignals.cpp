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

using real_type = InterCellSignals::real_type_;

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

std::vector<real_type> InterCellSignals::gaussian_field(InterCellSignal inSignal) const {
  try {
    return gaussian_fields_.at(static_cast<int>(inSignal));
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


void InterCellSignals::AddGaussianField(InterCellSignal signal, std::vector<real_type>& value) {
  try {
      /*int IntSignal =static_cast<int>(signal);
      if (IntSignal == 6){
      std::cout << "Signal" << IntSignal << "of init value" << signals_[IntSignal] << "is added " << value << std::endl;
      std::cout << signals_[static_cast<int> (signal)] + value << std::endl;}*/
    for ( auto v : value ) {
      gaussian_fields_[static_cast<int>(signal)].push_back(v);
    }

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
  for (auto& fields : gaussian_fields_) {
    fields.clear();
  }
}

void InterCellSignals::Load(gzFile backup_file) {
  for ( unsigned long i = 0; i < nbr_signals; ++i) {
    gzread(backup_file, &signals_[i], sizeof(signals_[i]));
  }
  for ( unsigned long i = 0; i < nbr_signals; ++i) {
    unsigned long size;
    gzread(backup_file,&size,sizeof(size));
    for ( unsigned long j = 0; j < size; ++j) {
      real_type val;
      gzread(backup_file, &val, sizeof(val));
      gaussian_fields_[i].push_back(val);
    }
  }
}

void InterCellSignals::Save(gzFile backup_file) const {
  double tmp;
  std::vector<real_type> tmp_v;
  for ( unsigned long i = 0; i < nbr_signals; ++i) {
    tmp = getInSignal(static_cast<InterCellSignal>(i));
    gzwrite(backup_file, &tmp, sizeof(tmp));
  }
  for ( unsigned long i = 0; i < nbr_signals; ++i) {
    tmp_v = gaussian_field(static_cast<InterCellSignal>(i));
    unsigned long size = tmp_v.size();
    gzwrite(backup_file,&size,sizeof(size));
    for ( auto t : tmp_v ) {
      gzwrite(backup_file, &t, sizeof(t));
    }
  }
}
