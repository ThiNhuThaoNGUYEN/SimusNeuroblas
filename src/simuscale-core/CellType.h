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
// This is file CellType.h.in / CellType.h
// CellType.h is automatically generated from CellType.h.in;
// do not edit CellType.h directly.
// If you want to edit the list of possible Cell Types, edit file
// @PROJECT_SOURCE_DIR@/src/simuscale-core/CellType.values.in.
#ifndef SIMUSCALE_CELLTYPE_H__
#define SIMUSCALE_CELLTYPE_H__
// ============================================================================
//                                   Includes
// ============================================================================
#include <cinttypes>
#include <map>
#include <string>
using CellFormalism = uint32_t;
typedef enum {
  STEM,
  CANCER,
  DIFFA,
  DIFFB,
  LYMPHOCYTE,
  APC,
  KILLER,
  NICHE,
  ROOT,
  NONE,
DIFF_S,
} CellType;
static std::map<CellType,std::string> CellType_Names {
  { CellType::STEM, "STEM" },
  { CellType::CANCER, "CANCER" },
  { CellType::DIFFA, "DIFFA" },
  { CellType::DIFFB, "DIFFB" },
  { CellType::LYMPHOCYTE, "LYMPHOCYTE" },
  { CellType::APC, "APC" },
  { CellType::KILLER, "KILLER" },
  { CellType::NICHE, "NICHE" },
  { CellType::ROOT, "ROOT" },
  { CellType::NONE, "NONE" },
 { CellType::DIFF_S, "DIFF_S" },
};
static auto const& nbr_cellypes = CellType_Names.size();
#endif // SIMUSCALE_CELLTYPE_H__
