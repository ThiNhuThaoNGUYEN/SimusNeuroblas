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
// This is file InterCellSignal.h.in / InterCellSignal.h
// InterCellSignal.h is automatically generated from InterCellSignal.h.in;
// do not edit InterCellSignal.h directly.
// If you want to edit the list of possible  Intercellular Signals, edit file
// @PROJECT_SOURCE_DIR@/src/simuscale-core/InterCellSignal.values.in.
#ifndef SIMUSCALE_INTERCELLSIGNAL_H__
#define SIMUSCALE_INTERCELLSIGNAL_H__
// ============================================================================
//                                   Includes
// ============================================================================
#include <cinttypes>
#include <map>
#include <string>
using CellFormalism = uint32_t;
enum class InterCellSignal {
  CYCLE,
  CYCLE_X,
  CYCLE_Z,
  CLOCK,
  DEATH,
  NICHE,
  SPIKY_1,
  LYMPHOCYTE_TYPE,
  LYMPHOCYTE_P1,
  LYMPHOCYTE_P2,
  LYMPHOCYTE_P3,
  LYMPHOCYTE_mRNA1,
  LYMPHOCYTE_mRNA2,
  LYMPHOCYTE_mRNA3,
  APC_CONTACT,
  VOLUME,
  DIFFERENTIATION_STATE,
  KILL,
  BP1,
  RBP1,
CANCER_TYPE,
STEM_CONTACT,
SYP_CONTACT,
CANCER_S,
CANCER_D1,
CANCER_D2,
CANCER_P,
REMEMBER_DIVISION,
CANCER_mRNA_S,
CANCER_mRNA_D1,
CANCER_mRNA_D2,
CANCER_mRNA_P,
S2,
};
static std::map<InterCellSignal,std::string> InterCellSignal_Names {
  { InterCellSignal::CYCLE, "CYCLE" },
  { InterCellSignal::CYCLE_X, "CYCLE_X" },
  { InterCellSignal::CYCLE_Z, "CYCLE_Z" },
  { InterCellSignal::CLOCK, "CLOCK" },
  { InterCellSignal::DEATH, "DEATH" },
  { InterCellSignal::NICHE, "NICHE" },
  { InterCellSignal::SPIKY_1, "SPIKY_1" },
  { InterCellSignal::LYMPHOCYTE_TYPE, "LYMPHOCYTE_TYPE" },
  { InterCellSignal::LYMPHOCYTE_P1, "LYMPHOCYTE_P1" },
  { InterCellSignal::LYMPHOCYTE_P2, "LYMPHOCYTE_P2" },
  { InterCellSignal::LYMPHOCYTE_P3, "LYMPHOCYTE_P3" },
  { InterCellSignal::LYMPHOCYTE_mRNA1, "LYMPHOCYTE_mRNA1" },
  { InterCellSignal::LYMPHOCYTE_mRNA2, "LYMPHOCYTE_mRNA2" },
  { InterCellSignal::LYMPHOCYTE_mRNA3, "LYMPHOCYTE_mRNA3" },
  { InterCellSignal::APC_CONTACT, "APC_CONTACT" },
  { InterCellSignal::VOLUME, "VOLUME" },
  { InterCellSignal::DIFFERENTIATION_STATE, "DIFFERENTIATION_STATE" },
  { InterCellSignal::KILL, "KILL" },
  { InterCellSignal::BP1, "BP1" },
  { InterCellSignal::RBP1, "RBP1" },
  { InterCellSignal::CANCER_TYPE, "CANCER_TYPE" },
 { InterCellSignal::STEM_CONTACT, "STEM_CONTACT" },
  { InterCellSignal::SYP_CONTACT, "SYP_CONTACT" },
  { InterCellSignal::CANCER_S, "CANCER_S" },
  { InterCellSignal::CANCER_D1, "CANCER_D1" },
  { InterCellSignal::CANCER_D2, "CANCER_D2" },
  { InterCellSignal::CANCER_P, "CANCER_P" },
{ InterCellSignal::REMEMBER_DIVISION, "REMEMBER_DIVISION" },
  { InterCellSignal::CANCER_mRNA_S, "CANCER_mRNA_S" },
  { InterCellSignal::CANCER_mRNA_D1, "CANCER_mRNA_D1" },
  { InterCellSignal::CANCER_mRNA_D2, "CANCER_mRNA_D2" },
  { InterCellSignal::CANCER_mRNA_P, "CANCER_mRNA_P" },
  { InterCellSignal::S2, "S2" },
};
static auto const& nbr_signals = InterCellSignal_Names.size();
#endif // SIMUSCALE_INTERCELLSIGNAL_H__
