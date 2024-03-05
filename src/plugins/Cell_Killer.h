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

// <TODO> Modify the include guard </TODO>
#ifndef SIMUSCALE_CELL_KILLER_H__
#define SIMUSCALE_CELL_KILLER_H__


// ============================================================================
//                                   Includes
// ============================================================================
#include "Cell.h"

/**
 * This is a cell formalism plugin boilerplate
 */
// <TODO> Modify the class name </TODO>
class Cell_Killer : public Cell {

 public :
  // ==========================================================================
  //                               Constructors
  // ==========================================================================
  Cell_Killer() = default; //< Default ctor
  Cell_Killer(const Cell_Killer &); //< Copy ctor
  Cell_Killer(Cell_Killer &&) = delete; //< Move ctor
  Cell_Killer(CellType cellType,
                   const MoveBehaviour& move_behaviour,
                   const Coordinates<double>& pos,
                   double initial_volume,
                   double volume_min,
                   double doubling_time);
  Cell_Killer(gzFile backup_file);

  // ==========================================================================
  //                                Destructor
  // ==========================================================================
  virtual ~Cell_Killer();

  // ==========================================================================
  //                                Operators
  // ==========================================================================
  /// Copy assignment
  Cell_Killer &operator=(const Cell_Killer &other) = delete;

  /// Move assignment
  Cell_Killer &operator=(Cell_Killer &&other) = delete;

  // ==========================================================================
  //                              Public Methods
  // ==========================================================================
  void InternalUpdate(const double& dt) override;
  Cell* Divide(void) override;

  void Save(gzFile backup_file) const override;
  void Load(gzFile backup_file) override;

  // ==========================================================================
  //                                Accessors
  // ==========================================================================
  double get_output(InterCellSignal signal) const override;

  bool isCycling() const override;
  bool isDividing() const override;
  bool isDying() const override;

 protected :
  // ==========================================================================
  //                            Protected Methods
  // ==========================================================================
  void UpdateCyclingStatus() override;
  bool StopCycling() override;
  bool StartCycling() override;
  Coordinates<double> MotileDisplacement(const double& dt) override;

  // ==========================================================================
  //                               Attributes
  // ==========================================================================
 public:
  // <TODO> Modify the class id and keyword </TODO>
  // Class ID, uniquely identifies this class
  static constexpr CellFormalism classId_ = 0xff277512; // CRC32 Hash
  // Keyword to be used in param files
  static constexpr char classKW_[] = "KILLER";

 protected:
  // <TODO> Add your formalism-specific attributes here </TODO>

  double toxic_signal_ = 10.0;
  double sigma_ = 1.5;
  int32_t nn_ = 0;

 private:
  /** dummy attribute - allows to register class in Simuscale statically */
  static bool registered_;
};

#endif // SIMUSCALE_CELL_KILLER_H__
