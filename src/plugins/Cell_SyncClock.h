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

#ifndef SIMUSCALE_CELL_SYNC_CLOCK_H__
#define SIMUSCALE_CELL_SYNC_CLOCK_H__


// =================================================================
//                              Includes
// =================================================================
#include <inttypes.h>
#include <stdio.h>
#include <stdlib.h>

#include "Cell.h"
#include "CellType.h"


/*!
  This class implements a specific model based on a predefined set of ODEs

  This model is described in the following article:
  Bernard S, Gonze D, Čajavec B, Herzel H, Kramer A (2007)
  Synchronization-Induced Rhythmicity of Circadian Oscillators in the
  Suprachiasmatic Nucleus.
  PLOS Computational Biology 3(4): e68. doi: 10.1371/journal.pcbi.0030068
*/
class Cell_SyncClock : public Cell {
 public :
  typedef enum {
    CY_X, CY_Z,
    CD_DEATH,
    Y1, Y2, Y3, Y4, Y5, Y6, Y7,
    V,
    X1, X2
  } StateVariable;


  // =================================================================
  //                             Constructors
  // =================================================================
  // Ctors are protected because we either encapsulate them into factories
  // or call them directly from within the class
 protected:
  Cell_SyncClock() = delete;
  Cell_SyncClock(const Cell_SyncClock &model);

  Cell_SyncClock(CellType cellType,
           const MoveBehaviour& move_behaviour,
           const Coordinates<double>& pos,
           double initial_volume,
           double volume_min,
           double doubling_time);
  Cell_SyncClock(gzFile backup_file);
 public:

  // =================================================================
  //                             Destructor
  // =================================================================
  virtual ~Cell_SyncClock(void) noexcept;


  // =================================================================
  //                            Public Methods
  // =================================================================
  void InternalUpdate(const double& dt) override;
  Cell* Divide(void) override;

  void Save(gzFile backup_file) const override;
  void Load(gzFile backup_file) override;


  // =================================================================
  //                              Accessors
  // =================================================================
  double get_output(InterCellSignal signal) const override;

  bool isCycling() const override;
  bool isDividing() const override;
  bool isDying() const override;

 protected :
  // =================================================================
  //                           Protected Methods
  // =================================================================
  void ODE_update(const double& dt);
  static int compute_dydt(double t, const double y[], double dydt[],
      void* cell);
  int compute_dydt(double t, const double y[], double dydt[]) const;

  void UpdateMitoticStatus();
  void UpdateCyclingStatus() override;
  bool StopCycling() override;
  bool StartCycling() override;
  Coordinates<double> MotileDisplacement(const double& dt) override;


  // =================================================================
  //                               Attributes
  // =================================================================
 public:
  // Class ID, uniquely identifies this class
  static constexpr CellFormalism classId_ = 59345896; // CRC32 Hash
  // Keyword to be used in param files
  static constexpr char classKW_[] = "SYNC_CLOCK";

 protected:
  static constexpr uint32_t odesystemsize_ = 13;

  // The internal state of the cell
  // Because we use the GSL, the internal state of the system must be an array
  // of double values.
  double* internal_state_;

  bool isMitotic_ = false;  // distinguishes cells about to divide

  static constexpr double mitotic_threshold_ = 0.5;
  static constexpr double cycling_threshold_ = 0.0001;
  static constexpr double dividing_threshold_ = 0.5;
  static constexpr double death_threshold_ = 1.0; // 0.10;

  // Circadian clock parameters
  // The values are taken from Bernard et al. 2007 PLoS Comp Biol.
  static constexpr double v1b  = 9.00;
  static constexpr double k1b  = 1.00;
  static constexpr double k1i  = 0.56;
  static constexpr double k1d  = 0.18;
  static constexpr double k2b  = 0.30;
  static constexpr double k2d  = 0.10;
  static constexpr double k2t  = 0.36;
  static constexpr double k3t  = 0.02;
  static constexpr double k3d  = 0.18;
  static constexpr double v4b  = 1.00;
  static constexpr double k4b  = 2.16;
  static constexpr double k4d  = 1.10;
  static constexpr double k5b  = 0.24;
  static constexpr double k5d  = 0.09;
  static constexpr double k5t  = 0.45;
  static constexpr double k6t  = 0.06;
  static constexpr double k6d  = 0.18;
  static constexpr double k6a  = 0.09;
  static constexpr double k7a  = 0.003;
  static constexpr double k7d  = 0.13;
  static constexpr double k8   = 1.00;
  static constexpr double k8d  = 4.00;
  static constexpr double K    = 1.8;  ///< coupling strength
  static constexpr double kx1  = 3.00;
  static constexpr double x1T  = 15.0;
  static constexpr double kdx1 = 4.00;
  static constexpr double kx2  = 0.25;
  static constexpr double x2T  = 15.0;
  static constexpr double kdx2 = 10.0;
  static constexpr double L0   = 0.00;
  double per_scal_; // Scaling parameter (modifies the "speed")

  // Death/Survival parameters
  static constexpr double k_survival = 1.0; // higher values = higher survival.

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

  double sigma_ = 0.0;

 private:
  /** dummy attribute - allows to register class in Simuscale statically */
  static bool registered_;
};

#endif // SIMUSCALE_CELL_SYNC_CLOCK_H__
