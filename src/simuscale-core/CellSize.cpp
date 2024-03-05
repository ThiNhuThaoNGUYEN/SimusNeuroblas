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
#include "CellSize.h"

#include <cassert>
#include <cmath>

#include <algorithm>

// ============================================================================
//                       Definition of static attributes
// ============================================================================
double CellSize::volume_max_min_ratio_; // = 2.2; seems this is never assigned
double CellSize::radii_ratio_; // = 0.95; seems this is never assigned

// ============================================================================
//                                Constructors
// ============================================================================
CellSize::CellSize(const CellSize& model) :
    volume_min_(model.volume_min()),
    volume_division_threshold_(2.0 * model.volume_min()) {
  set_volume(model.volume());
}

CellSize::CellSize(double initial_volume, double volume_min) :
    volume_min_(volume_min),
    volume_division_threshold_(2.0 * volume_min) {
  set_volume(initial_volume);
}

// ============================================================================
//                                 Destructor
// ============================================================================

// ============================================================================
//                                   Methods
// ============================================================================

// ============================================================================
//                                  Accessors
// ============================================================================
void CellSize::set_volume(double vol) {
  // Once the cell volume reached its maximum, it does not grow anymore
  volume_ = std::min(vol, volume_max());
  external_radius_ = compute_radius(volume_);
  internal_radius_ = radii_ratio_ * external_radius_;
}

void CellSize::set_volume_max_min_ratio(double volume_max_min_ratio) {
  assert(volume_max_min_ratio >= 2.0);
  volume_max_min_ratio_ = volume_max_min_ratio;
}

void CellSize::divide() {
  set_volume(volume_ / 2.0);
}

void CellSize::Save(gzFile backup_file) const {
  gzwrite(backup_file, &volume_, sizeof(volume_));
  gzwrite(backup_file, &volume_min_, sizeof(volume_min_));
}

CellSize CellSize::Load(gzFile backup_file) {
  double volume, volume_min;
  gzread(backup_file, &volume, sizeof(volume));
  gzread(backup_file, &volume_min, sizeof(volume_min));

  return CellSize(volume, volume_min);
}

void CellSize::SaveStatic(gzFile backup_file) {
  gzwrite(backup_file, &volume_max_min_ratio_, sizeof(volume_max_min_ratio_));
  gzwrite(backup_file, &radii_ratio_, sizeof(radii_ratio_));
}

void CellSize::LoadStatic(gzFile backup_file) {
  gzread(backup_file, &volume_max_min_ratio_, sizeof(volume_max_min_ratio_));
  gzread(backup_file, &radii_ratio_, sizeof(radii_ratio_));
}
