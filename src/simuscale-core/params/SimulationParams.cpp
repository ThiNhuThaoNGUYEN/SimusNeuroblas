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
#include "movement/MoveBehaviour.h"
#include "SimulationParams.h"

using std::string;

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
void SimulationParams::addPop(long size,
                              const char* cell_type_str,
                              const char* formalism_str,
                              const char* move_behaviour_str,
                              double doubling_time,
                              double volume_min,
                              const char* init_file) {
  CellType cell_type = StrToCellType(cell_type_str);
  CellFormalism formalism = StrToFormalism(formalism_str);
  MoveBehaviour::Type move_behaviour_type = StrToMBType(move_behaviour_str);

  pop_params_.emplace_back(size, cell_type, formalism, move_behaviour_type,
                           doubling_time, volume_min, init_file);
}

void SimulationParams::setNicheParams(const char* formalism_str,
                                      double internal_radius) {
  niche_params_.cell_formalism_ = StrToFormalism(formalism_str);
  niche_params_.internal_radius_ = internal_radius;
}

CellFormalism SimulationParams::StrToFormalism(const std::string formalism_str)
noexcept(false) {
  try {
    return CellFormalisms().at(formalism_str);
  }
  catch (std::out_of_range& e) {
    throw string("unknown cell formalism " + formalism_str);
  }
}

CellType SimulationParams::StrToCellType(const std::string cell_type_str) {
  auto kval = CellType_Names.begin();
  while ( kval != CellType_Names.end() ) {
    if ( cell_type_str == kval->second ) 
      return kval->first;
    kval++;
  }
  throw string("unknown cell type " + cell_type_str);

}

MoveBehaviour::Type SimulationParams::StrToMBType(const std::string mb_str) {
  if (mb_str == "IMMOBILE") {
    return MoveBehaviour::IMMOBILE;
  }
  else if (mb_str == "MOBILE") {
    return MoveBehaviour::MOBILE;
  }
  else if (mb_str == "MOTILE") {
    return MoveBehaviour::MOTILE;
  }
  else {
    throw string("unknown move behaviour type (use MOBILE/IMMOBILE/MOTILE)");
  }
}

InterCellSignal SimulationParams::StrToInterCellSignal(const std::string signal_str) {  
  auto kval = InterCellSignal_Names.begin();
  while ( kval != InterCellSignal_Names.end() ) {
    if ( signal_str == kval->second ) 
      return kval->first;
    kval++;
  }
  throw string("unknown intercellular signal " + signal_str);

}


/**
 * Map cell formalism param file keywords to classIds
 *
 * NOTE: Use the "Construct On First Use Idiom" to avoid the
 * "static initialization order fiasco"
 */
std::map<std::string, CellFormalism>& SimulationParams::CellFormalisms() {
  static auto* cellFormalisms = new std::map<std::string, CellFormalism>();
  return *cellFormalisms;
}
