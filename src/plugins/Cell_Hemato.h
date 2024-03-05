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
#ifndef SIMUSCALE_CELL_HEMATO_H__
#define SIMUSCALE_CELL_HEMATO_H__


// ============================================================================
//                                   Includes
// ============================================================================
#include "Cell.h"

/**
 * This is a cell formalism plugin boilerplate
 */
// <TODO> Modify the class name </TODO>
class Cell_Hemato : public Cell {

 public :

  typedef enum {
    CY_X, 
    CY_Z,
    CD_DEATH,
  } StateVariable;

  // =================================================================
  //                         Factory functions
  // =================================================================
  template <typename... Args>
  static std::shared_ptr<Cell> MakeCell(Args&&... args) {
    return std::shared_ptr<Cell_Hemato>(new Cell_Hemato(std::forward<Args>(args)...));
  }

  // ==========================================================================
  //                               Constructors
  // ==========================================================================
  Cell_Hemato() = delete; //< no default Default ctor
  Cell_Hemato(const Cell_Hemato &); //< Copy ctor
  Cell_Hemato(Cell_Hemato &&) = delete; //< Move ctor
  Cell_Hemato(CellType cellType,
                   std::unique_ptr<MoveBehaviour>&& move_behaviour,
                   const Coordinates<double>& pos,
                   double initial_volume,
                   double volume_min,
                   double doubling_time);
  Cell_Hemato(BackupManager& backup);

  // ==========================================================================
  //                                Destructor
  // ==========================================================================
  virtual ~Cell_Hemato();

  // ==========================================================================
  //                                Operators
  // ==========================================================================
  /// Copy assignment
  Cell_Hemato &operator=(const Cell_Hemato &other) = delete;

  /// Move assignment
  Cell_Hemato &operator=(Cell_Hemato &&other) = delete;

  // ==========================================================================
  //                              Public Methods
  // ==========================================================================
  void InternalUpdate(const double& dt) override;
  std::shared_ptr<Cell> Divide() override;

  void Save(BackupManager& backup) const;
  void Load(BackupManager& backup);

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
  void ODE_update(const double& dt);
  static int compute_dydt(double t, const double y[], double dydt[],
      void* cell);
  int compute_dydt(double t, const double y[], double dydt[]) const;
  
  void UpdateMitoticStatus();
  void UpdateCyclingStatus() override;
  bool StopCycling() override;
  bool StartCycling() override;

  // ==========================================================================
  //                               Attributes
  // ==========================================================================
 public:
  // <TODO> Modify the class id and keyword </TODO>
  // Class ID, uniquely identifies this class
  static constexpr CellFormalism classId_ = 0xdaa16ede; // CRC32 Hash
  // Keyword to be used in param files
  static constexpr char classKW_[] = "HEMATO";

 protected:
  // <TODO> Add your formalism-specific attributes here </TODO>
  static constexpr uint32_t odesystemsize_ = 3;

  // The internal state of the cell
  // Because we use the GSL, the internal state of the system must be an array
  // of double values.
  double* internal_state_;

  bool isMitotic_ = false;  // distinguishes cells about to divide

  static constexpr double mitotic_threshold_ = 0.5;
  static constexpr double cycling_threshold_ = 0.0001;
  static constexpr double dividing_threshold_ = 0.5;
  static constexpr double death_threshold_ = 1.0;

  // Death/Survival parameters
  static constexpr double k_survival = 10.0; // higher values = higher survival.

  // Cell cycle parameters
  // The values are taken from (Battogtokh & Tyson 2006 PRE)
  static constexpr double cy_j   = 0.05;
  static constexpr double cy_p   = 0.15;
  static constexpr double cy_k1  = 0.002;
  static constexpr double cy_k2  = 0.0795;
  static constexpr double cy_k3  = 0.01;
  static constexpr double cy_k4  = 2.0;
  static constexpr double cy_k5  = 0.05;
  static constexpr double cy_k6  = 0.04;
  static constexpr double cy_k7  = 1.5;
  static constexpr double cy_k8  = 0.19;
  static constexpr double cy_k9  = 0.64;
  static constexpr double cy_k10 = 0.0025;
  static constexpr double cy_k11 = 0.07;
  static constexpr double cy_k12 = 0.08;
  static constexpr double cy_A   = 0.52;

 private:
  /** dummy attribute - allows to register class in Simuscale statically */
  static bool registered_;
};

#endif // SIMUSCALE_CELL_HEMATO_H__
