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
#include "Cell_Hemato.h"

#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <cassert>

#include <memory>

#include <gsl/gsl_errno.h>
#include <gsl/gsl_matrix.h>
#include <gsl/gsl_odeiv2.h>

#include "Simulation.h"
#include "Alea.h"
#include "Coordinates.h"
#include "io/BackupManager.h"

using std::unique_ptr;
using std::shared_ptr;

// ============================================================================
//                       Definition of static attributes
// ============================================================================
constexpr CellFormalism Cell_Hemato::classId_;
constexpr char Cell_Hemato::classKW_[];

constexpr uint32_t Cell_Hemato::odesystemsize_;

// ============================================================================
//                                Constructors
// ============================================================================
Cell_Hemato::Cell_Hemato(const Cell_Hemato &model):
    Cell(model),
    isMitotic_(model.isMitotic_) {
  internal_state_ = new double[odesystemsize_];
  memcpy(internal_state_, model.internal_state_, odesystemsize_ * sizeof(*internal_state_));
}

/**
 * Create a new object with the provided constitutive elements and values
 */
Cell_Hemato::Cell_Hemato(CellType cellType,
                                   unique_ptr<MoveBehaviour>&& move_behaviour,
                                   const Coordinates<double>& pos,
                                   double initial_volume,
                                   double volume_min,
                                   double doubling_time) :
    Cell(cellType, std::move(move_behaviour),
         pos, CellSize(initial_volume, volume_min),
         doubling_time) {
  internal_state_ = new double[odesystemsize_];
  for (u_int32_t j = 0 ; j < odesystemsize_; j++)
    internal_state_[j] = 0.05 * Alea::random();
}

/**
 * Restore an object that was backed-up in backup_file
 */
Cell_Hemato::Cell_Hemato(BackupManager& backup) :
    Cell(backup) {
  Load(backup);
}

// ============================================================================
//                                 Destructor
// ============================================================================
Cell_Hemato::~Cell_Hemato() noexcept {
  delete [] internal_state_;
}

// ============================================================================
//                                   Methods
// ============================================================================
void Cell_Hemato::InternalUpdate(const double& dt) {
  // <TODO>
  // Describe what happens inside the cell when taking a time step

  // Update the intracellular state of the cell
  if (cell_type_ != CellType::NICHE) {
    ODE_update(dt);
  }

  // If the cell is dying, don't take any further actions
  if (isDying()) return;

  UpdateCyclingStatus();

  // Make the cell grow if it's cycling (and not dying)
  if (isCycling_) Grow(dt);

  UpdateMitoticStatus();
  // </TODO>
}

shared_ptr<Cell> Cell_Hemato::Divide() {
  // <TODO> Determine how the cell should divide </TODO>
  assert(move_behaviour_->type() != MoveBehaviour::IMMOBILE);

  // Reset mitotic status
  isMitotic_ = false;

  // Create a copy of the cell with no mechanical forces nor chemical stimuli
  // applied to it
  auto newCell = MakeCell(*this);

  move_behaviour_->SeparateDividingCells(shared_from_this(), newCell);
  return newCell;

  // return nullptr;
}

void Cell_Hemato::Save(BackupManager& backup) const {
  // Write my classId
  backup.write(classId_);

  // Write the generic cell stuff
  Cell::Save(backup);

  // Write the specifics of this class
  // <TODO> Save your specific attributes </TODO>
  for (u_int32_t i = 0 ; i < odesystemsize_; i++)
    backup.write(internal_state_[i]);
}

void Cell_Hemato::Load(BackupManager& backup) {
  // <TODO> Load your specific attributes </TODO>
}

double Cell_Hemato::get_output(InterCellSignal signal) const {
  // <TODO> Determine the amount of <signal> the cell is secreting </TODO>
  switch (signal) {
    case InterCellSignal::DIFFERENTIATION_STATE:
      return   1.0*(cell_type_ == CellType::CANCER) 
             + 0.5*(cell_type_ == CellType::STEM);
    case InterCellSignal::CYCLE_X:
      return internal_state_[CY_X];
    case InterCellSignal::CYCLE_Z:
      return internal_state_[CY_Z];
    case InterCellSignal::DEATH:
      return internal_state_[CD_DEATH];
    default:
      return 0;
  }
}

bool Cell_Hemato::isCycling() const {
  // <TODO> Is the cell currently cycling ? </TODO>
  return isCycling_;
}

bool Cell_Hemato::isDividing() const {
  // <TODO> Should the cell divide now ? </TODO>
  return (cell_type_ != CellType::NICHE) &&
      isMitotic_ &&
      internal_state_[CY_X] < dividing_threshold_ &&
      size_.may_divide();
}

bool Cell_Hemato::isDying() const {
  // <TODO> Should the cell die now ? </TODO>
  return (cell_type_ != CellType::NICHE) &&
         (internal_state_[CD_DEATH] > death_threshold_);
}

void Cell_Hemato::UpdateMitoticStatus() {
  // Once a cell enters the mitotic state, it remains mitotic until it
  // dies or divides
  if (not isMitotic_)
    isMitotic_ = isCycling_ && (internal_state_[CY_X] > mitotic_threshold_);
}

void Cell_Hemato::UpdateCyclingStatus() {
  if (isCycling_) {
    if (StopCycling()) isCycling_ = false;
  }
  else if (StartCycling()) {
    isCycling_ = true;
  }
}

bool Cell_Hemato::StopCycling() {
  // <TODO> Should the cell stop cycling now ? </TODO>
  return (cell_type_ == CellType::NICHE or internal_state_[CY_X] <= cycling_threshold_);
}

bool Cell_Hemato::StartCycling() {
  // <TODO> Should the cell start cycling now ? </TODO>
  return cell_type_ != CellType::NICHE &&
      internal_state_[CY_X] > cycling_threshold_ &&
      ((cell_type_ == CellType::STEM && getInSignal(InterCellSignal::NICHE) > 0) ||
          cell_type_ == CellType::CANCER);
}


void Cell_Hemato::ODE_update(const double& dt) {
  // RK-Cash-Karp seems faster than rkf45 (runge-kutta-fehlberg)
  gsl_odeiv2_step*    s  = gsl_odeiv2_step_alloc(gsl_odeiv2_step_rkck,
                                                 odesystemsize_);
  gsl_odeiv2_control* c  = gsl_odeiv2_control_y_new(1e-6, 0.0); // first parameter is the tolerance for ODE system
  gsl_odeiv2_evolve*  e  = gsl_odeiv2_evolve_alloc(odesystemsize_);
  gsl_odeiv2_system  sys = {compute_dydt, NULL, odesystemsize_, this};

  double t = 0.0, t1 = dt;
  double h = 1e-3;

  // Let y(t) = internal_state_(t)
  // The following loop computes f(y) = dy/dt in order to solve the system until time t + DT, where DT is the "big" time step.
  // DT is cut into several small time steps, with an adaptative small time step that may vary at each small time step:
  // t, t + h, t + h + h', t + h + h' + h'', ... t + DT. Each small timestep is computed by gsl_odeiv2_evolve_apply.
  // The function gsl_odeiv2_evolve_apply advances the system dy/dt = f(y) from time t and state y using
  // the function step with an adaptative method.

  // The last argument to gsl_odeiv2_evolve_apply , here internal_state_, is both an input (state at time t) and an output of the
  // routine (state at time t+h).
  // The h given as the 7th parameter is also both an input and an output: it is the initial guess for the
  // small time step.  The routine will make several calls to func before finding the best value of h.
  // Each call to func advances the system to time t+h. If the resulting error is too large, this candidate new state
  // is rejected, the system goes back to time t and a new, smaller, h is tested. Once the error is smaller
  // than the tolerance specified in argument c, the new state is accepted and the system advances to time t+h.

  // Note that the error is controlled at each timestep. Thus the "right" h value may change at each
  // time step: with the above notations, the successive "right" values would be h, then h', then h'', and so on.
  // With the RK-Cash-Karp method, there are roughly 6 calls to func to compute the candidate new state and its error.
  // If the candidate new state is rejected, in the worst case, 6 more calls to func will be necessary to find the
  // new candidate state corresponding to the smaller h. There are thus 6 * (number of candidate values for h) calls
  // to func at each small timestep.

  // When the right h is found, the candidate new state is accepted.
  // For the next timestep, a slightly larger h may be tried first, if the next error is likely to be smaller
  // than the tolerance.

  while (t < t1)
  {
    int status = gsl_odeiv2_evolve_apply (e, c, s,
                                          &sys,
                                          &t, t1,    /* t will be updated several times until t1=t+DT is reached */
                                          &h,        /* the small time step can be updated several times too (adaptative time step) */
                                          internal_state_); /* internal_state_ will be updated several times */


    if (status != GSL_SUCCESS)
    {
      fprintf(stderr, "Warning: error in ODE update\n");
      exit(EXIT_FAILURE);
    }

    // printf ("%f %f\n", t, h);
  }

  gsl_odeiv2_evolve_free(e);
  gsl_odeiv2_control_free(c);
  gsl_odeiv2_step_free(s);
}

/**
 * Piping method: wraps the non-static version of compute_dydt to be used in GSL
 */
int Cell_Hemato::compute_dydt(double t,
                           const double y[],
                           double dydt[],
                           void* cell) {
  return ((Cell_Hemato*) cell)->compute_dydt(t, y, dydt);
}

/**
 * Compute dy/dt given a time point and the state and parameters of the system
 *
 * \param t the initial time
 * \param y an estimate of the system state at time t
 * \param dydt computed dy/dt (output)
 * \return GSL_SUCCESS on success
 *
 * For a given small timestep (suppose we are at t0 and we want
 * to find a candidate new state for time t0+h), func will be called several times by gsl_odeiv2_evolve_apply,
 * with different time points, like, say,  t0, t0 + h/5, t0 + 3h/10 (actually 6 intermediate timepoints for the RK-Cash-Karp method,
 * for example). Then y(t+h) will be computed twice, with two different accuracies (4th and 5th order), with two different
 * linear combinations of the 6 values of f (a idea of Fehlberg's).
 * From the comparison between two orders, the local error can be estimated.
 * If this error is too large, then a smaller h must be used. If on the contrary, the error is smaller than the tolerance,
 * the candidate new state be will accepted, the system has really advanced to time t0+h.

 * In our case, y is internal_state_. func only uses it as an input to estimate the derivative (cf const keyword). func does
 * NOT update internal_state_. It is gsl_odeiv2_evolve_apply that updates internal_state_, with the right small timestep h,
 * by using the 5th order approximation.
 */
int Cell_Hemato::compute_dydt(double t, const double y[], double dydt[]) const {
  // NOTA: t is unused (for now). It is left here because it might be used in
  // the future and it helps understand what's going on.
  assert(t == t); // Suppress warning

  // this is the intracellular network

  // INPUT FROM other cells
  // double F = getInSignal(InterCellSignal::CLOCK);
  // double death_signal = getInSignal(InterCellSignal::DEATH);
  double in_signal = getInSignal(InterCellSignal::CYCLE_Z);
  double death_signal = getInSignal(InterCellSignal::DEATH);

  // CELL CYCLE VARIABLES (ref: Introducing a reduced cell cycle model, Battogtokh & Tyson 2006 PRE)
  double cy_x = y[CY_X];   // y is a fictive intermediate internal state where the stepper wants to know the derivative
  double cy_z = y[CY_Z];   // we copy it into local variables to make the equations shorter and clearer

  // DEATH AND SURVIVAL VARIABLES
  double cd_death = y[CD_DEATH];

  // CELL CYCLE AUXILIARY FUNCTION
  double e_temp = cy_p-cy_x+cy_p*cy_j+cy_x*cy_j;
  double W0 = 2*cy_x*cy_j/(e_temp+sqrt(e_temp*e_temp-4*cy_x*cy_j*(cy_p-cy_x)));
  double a_temp = cy_k6+cy_k7*cy_z;
  double b_temp = cy_k8*outer_volume()/ size_.volume_min() +cy_k9*cy_x;
  e_temp = b_temp-a_temp+b_temp*cy_j+a_temp*cy_j;
  double Y0 = 2*a_temp*cy_j/(e_temp+sqrt(e_temp*e_temp-4*a_temp*cy_j*(b_temp-a_temp)));

  // CELL CYCLE EQUATIONS
  // cycling signal variables cy
  // dX/dt = m (k1+k2*W0)-(k3+k4*Y0+k5*Z)*X
  // dZ/dt = k10(1+A(1+sin(f*t)))+k11*X-k12*Z
  // ORIGINAL EQ  f[0] = cell_volume*(cy_k1+cy_k2*W0) - (cy_k3+cy_k4*Y0+cy_k5*cy_z)*cy_x;
  dydt[CY_X] = (outer_volume() / size_.volume_min() * (cy_k1+cy_k2*W0) -
      (cy_k3+cy_k4*Y0+cy_k5*cy_z)*cy_x);
  dydt[CY_Z] = (cy_k10*(1+cy_A)+cy_k11*cy_x-cy_k12*cy_z);

  // death signal variable cd
  dydt[CD_DEATH] = death_signal - k_survival * cd_death;

  return GSL_SUCCESS;
}


// Register this class in Cell
bool Cell_Hemato::registered_ =
    Cell::RegisterClass(classId_, classKW_,
                        [](const CellType& type,
                           unique_ptr<MoveBehaviour>&& move_behaviour,
                           const Coordinates<double>& pos,
                           const double& initial_volume,
                           const double& volume_min,
                           const double& doubling_time) -> std::shared_ptr<Cell> {
                          return std::make_shared<Cell_Hemato>(
                              type,
                              std::move(move_behaviour),
                              pos,
                              initial_volume,
                              volume_min,
                              doubling_time);
                        },
                        [](BackupManager& backup) -> std::shared_ptr<Cell> {
                                   return std::make_shared<Cell_Hemato>(backup);
                        }
    );
