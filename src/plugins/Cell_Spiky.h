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
#ifndef SIMUSCALE_CELL_SPIKY_H__
#define SIMUSCALE_CELL_SPIKY_H__


// ============================================================================
//                                   Includes
// ============================================================================
#include "Cell.h"

/**
 * This is a cell formalism plugin boilerplate
 */
// <TODO> Modify the class name </TODO> -- DONE
class Cell_Spiky : public Cell {

 public :
  // ==========================================================================
  //                               Constructors
  // ==========================================================================
  Cell_Spiky() = default; //< Default ctor
  Cell_Spiky(const Cell_Spiky &); //< Copy ctor
  Cell_Spiky(CellType cellType,
                   const MoveBehaviour& move_behaviour,
                   const Coordinates<double>& pos,
                   double initial_volume,
                   double volume_min,
                   double doubling_time);
  Cell_Spiky(gzFile backup_file);

  // ==========================================================================
  //                                Destructor
  // ==========================================================================
  virtual ~Cell_Spiky();

  // ==========================================================================
  //                                Operators
  // ==========================================================================
  /// Copy assignment
  Cell_Spiky &operator=(const Cell_Spiky &other) = delete;

  /// Move assignment
  Cell_Spiky &operator=(Cell_Spiky &&other) = delete;

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
  // <TODO> Modify the class id and keyword </TODO> -- DONE
  // Class ID, uniquely identifies this class
  static constexpr CellFormalism classId_ = 0x5d861934; // CRC32 Hash
  // Keyword to be used in param files
  static constexpr char classKW_[] = "SPIKY";

 protected:
  // <TODO> Add your formalism-specific attributes here </TODO> - DONE
  double x1_; // first intracellular variable
  double x2_; // 2nd   intracellular variable
  double x3_; // 3rd   intracellular variable

  static constexpr double max_age_     = 48.0;  // lifespan of a cell
  static constexpr double production_  = 0.1;   // protein production rate
  static constexpr double degradation_ = 1.0;   // protein degradation rate
  static constexpr double sigma_       = 0.0;   // std of random movement

 private:
  /** dummy attribute - allows to register class in Simuscale statically */
  static bool registered_;
};

#endif // SIMUSCALE_CELL_SPIKY_H__
