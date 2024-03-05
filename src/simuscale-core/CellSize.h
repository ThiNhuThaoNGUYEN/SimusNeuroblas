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

#ifndef SIMUSCALE_CELLSIZE_H__
#define SIMUSCALE_CELLSIZE_H__


// ============================================================================
//                                   Includes
// ============================================================================
#include <cmath>

#include <zlib.h>

/**
 *
 */
class CellSize {

 public :
  // ==========================================================================
  //                               Constructors
  // ==========================================================================
  CellSize() = default; //< Default ctor
  CellSize(const CellSize& model); //< Copy ctor
  CellSize(CellSize&&) = default; //< Move ctor

  CellSize(double initial_volume, double volume_min);

  // ==========================================================================
  //                                Destructor
  // ==========================================================================
  virtual ~CellSize() = default; //< Destructor

  // ==========================================================================
  //                                Operators
  // ==========================================================================
  CellSize& operator=(const CellSize& other) = delete; //< Copy assignment
  CellSize& operator=(CellSize&& other) = default; //< Move assignment

  // ==========================================================================
  //                              Public Methods
  // ==========================================================================
  void divide();

  void Save(gzFile backup_file) const;
  static CellSize Load(gzFile backup_file);

  static void SaveStatic(gzFile backup_file);
  static void LoadStatic(gzFile backup_file);

  // ==========================================================================
  //                                Accessors
  // ==========================================================================
  double internal_radius() const {return internal_radius_;}
  double external_radius() const {return external_radius_;}
  double volume() const {return volume_;}

  /** Maximum authorized (outer) volume for the cell */
  double volume_max() {return volume_max_min_ratio_ * volume_min_;}
  /** Minimum authorized (outer) volume for the cell */
  double volume_min() const {return volume_min_;}

  void set_volume(double vol);

  static void set_volume_max_min_ratio(double volume_max_min_ratio);
  static void set_radii_ratio(double radii_ratio) {
    radii_ratio_ = radii_ratio;
  }

  /** Whether a cell is voluminous enough to divide */
  bool may_divide() const {return volume_ >= volume_division_threshold_;}

  static double compute_volume(double radius) {
    return k4Pi_3 * pow(radius, 3);
  }

  static double compute_radius(double volume) {
    return pow(volume / k4Pi_3, 1.0 / 3);
  }

 protected :
  // ==========================================================================
  //                            Protected Methods
  // ==========================================================================

  // ==========================================================================
  //                               Attributes
  // ==========================================================================
  /** Ratio between the maximum and minimum authorized volumes of a cell */
  static double volume_max_min_ratio_;
  /** Ratio between the internal and external radii (= int/ext) */
  static double radii_ratio_;

  /** Internal radius */
  double internal_radius_;
  /** External radius */
  double external_radius_;
  /** Outer volume */
  double volume_;

  // TODO(dpa) Should be const (*2)
  /** Minimum (outer) volume of a cell */
  double volume_min_;
  /** Minimum (outer) volume a cell must have to be able to divide */
  double volume_division_threshold_;

  /** Useful value */
  static constexpr double k4Pi_3 = 4 * M_PI / 3;
};
#endif //SIMUSCALE_CELLSIZE_H__
