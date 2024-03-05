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

// =================================================================
//                              Includes
// =================================================================
#include "Cell_SyncClock.h"

#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <cassert>

#include <gsl/gsl_errno.h>
#include <gsl/gsl_matrix.h>
#include <gsl/gsl_odeiv2.h>

#include "Simulation.h"
#include "Alea.h"
#include "Coordinates.h"


// =================================================================
//                    Definition of static attributes
// =================================================================
constexpr CellFormalism Cell_SyncClock::classId_;
constexpr char Cell_SyncClock::classKW_[];

constexpr uint32_t Cell_SyncClock::odesystemsize_;

// =================================================================
//                             Constructors
// =================================================================
Cell_SyncClock::Cell_SyncClock(const Cell_SyncClock &model):
    Cell(model),
    isMitotic_(model.isMitotic_) {
  internal_state_ = new double[odesystemsize_];
  memcpy(internal_state_, model.internal_state_, odesystemsize_ * sizeof(*internal_state_));
  this->per_scal_ = model.per_scal_;
}

/**
 * \brief Initialization ctor
 *
 * Create a new Cell_ODE object with randomly initialized internal
 * state and per_scal_ parameter
 */
Cell_SyncClock::Cell_SyncClock(CellType cellType,
                   const MoveBehaviour& move_behaviour,
                   const Coordinates<double>& pos,
                   double initial_volume,
                   double volume_min,
                   double doubling_time) :
    Cell(cellType, move_behaviour,
         pos, CellSize(initial_volume, volume_min),
         doubling_time) {
  internal_state_ = new double[odesystemsize_];
  for (u_int32_t j = 0 ; j < odesystemsize_; j++)
    internal_state_[j] = 0.05 * Alea::random();
  per_scal_ = 1 / (1 + 0.05 * Alea::gaussian_random());
}

Cell_SyncClock::Cell_SyncClock(gzFile backup_file) :
    Cell(backup_file) {
  internal_state_ = new double[odesystemsize_];
  Load(backup_file);
}


// =================================================================
//                             Destructor
// =================================================================
Cell_SyncClock::~Cell_SyncClock(void) noexcept {
  delete [] internal_state_;
}


// =================================================================
//                             Accessors
// =================================================================
double Cell_SyncClock::get_output(InterCellSignal signal) const {
  switch (signal) {
    case InterCellSignal::CYCLE:
      return internal_state_[CY_X];
    case InterCellSignal::CLOCK:
      return internal_state_[V];
    case InterCellSignal::DEATH:
      return internal_state_[CD_DEATH];
    default:
      return 0.0;
      // printf("%s:%d: error: case not implemented.\n", __FILE__, __LINE__);
      //exit(EXIT_FAILURE);
  }
}


// =================================================================
//                            Public Methods
// =================================================================
// WARNING : intrinsic inputs MUST have been computed before !
void Cell_SyncClock::InternalUpdate(const double& dt) {
  // Update the intracellular state of the cell
  ODE_update(dt);

  // If the cell is dying, don't take any further actions
  if (isDying()) return;

  UpdateCyclingStatus();

  // Make the cell grow if it's cycling (and not dying)
  if (isCycling_) Grow(dt);

  UpdateMitoticStatus();
}

Coordinates<double> Cell_SyncClock::MotileDisplacement(const double& dt) {
  // <TODO> Motile cell autonomous displacement behaviour
  size_t nbr_neighbours = neighbours().size();
  double sigma_t = ( nbr_neighbours == 0 )*sigma_ + ( nbr_neighbours > 0 )*sigma_/10.0;
  Coordinates<double> displ { sqrt(dt)*sigma_t*Alea::gaussian_random(),
                              sqrt(dt)*sigma_t*Alea::gaussian_random(),
                              sqrt(dt)*sigma_t*Alea::gaussian_random() - dt*0.0 };
  return displ;

}

/**
 *
 */
Cell* Cell_SyncClock::Divide(void) {
  // Reset mitotic status
  isMitotic_ = false;

  // Create a copy of the cell with no mechanical forces nor chemical stimuli
  // applied to it
  Cell_SyncClock* newCell = new Cell_SyncClock(*this);

  SeparateDividingCells(this, newCell);

  return newCell;
}


void Cell_SyncClock::Save(gzFile backup_file) const {
  // Write my classId
  gzwrite(backup_file, &classId_, sizeof(classId_));

  // Write the generic cell stuff
  Cell::Save(backup_file);

  // Write the specifics of this class
//  int8_t isMitotic = isMitotic_ ? 1 : 0;
//  gzwrite(backup_file, &isMitotic, sizeof(isMitotic));
  for (u_int32_t i = 0 ; i < odesystemsize_; i++)
    gzwrite(backup_file, &internal_state_[i], sizeof(internal_state_[i]));
  gzwrite(backup_file, &per_scal_, sizeof(per_scal_));
}

void Cell_SyncClock::Load(gzFile backup_file) {
//  int8_t isMitotic;
//  gzread(backup_file, &isMitotic, sizeof(isMitotic));
//  isMitotic_ = isMitotic;

  for (u_int32_t i = 0 ; i < odesystemsize_; i++)
    gzread(backup_file, &internal_state_[i], sizeof(internal_state_[i]));
  gzread(backup_file, &per_scal_, sizeof(per_scal_));
}

bool Cell_SyncClock::isCycling() const {
  return isCycling_;
}

bool Cell_SyncClock::isDividing() const {
  return (cell_type_ != NICHE) &&
      isMitotic_ &&
      internal_state_[CY_X] < dividing_threshold_ &&
      size_.may_divide();
}

bool Cell_SyncClock::isDying() const {
  return (cell_type_ != NICHE) &&
         (internal_state_[CD_DEATH] > death_threshold_);
}

// =================================================================
//                           Protected Methods
// =================================================================
void Cell_SyncClock::UpdateMitoticStatus() {
  // Once a cell enters the mitotic state, it remains mitotic until it
  // dies or divides
  if (not isMitotic_)
    isMitotic_ = isCycling_ && (internal_state_[CY_X] > mitotic_threshold_);
}

void Cell_SyncClock::UpdateCyclingStatus() {
  if (isCycling_) {
    if (StopCycling()) isCycling_ = false;
  }
  else if (StartCycling()) {
    isCycling_ = true;
  }
}

bool Cell_SyncClock::StartCycling() {
  return cell_type_ != NICHE &&
      internal_state_[CY_X] > cycling_threshold_ &&
      ((cell_type_ == STEM && getInSignal(InterCellSignal::NICHE) > 0) ||
          cell_type_ == CANCER);
}

bool Cell_SyncClock::StopCycling() {
  return (cell_type_ == NICHE or internal_state_[CY_X] <= cycling_threshold_);
}

// =================================================================
//                           Private Methods
// =================================================================
void Cell_SyncClock::ODE_update(const double& dt) {
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
int Cell_SyncClock::compute_dydt(double t,
                           const double y[],
                           double dydt[],
                           void* cell) {
  return ((Cell_SyncClock*) cell)->compute_dydt(t, y, dydt);
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
int Cell_SyncClock::compute_dydt(double t, const double y[], double dydt[]) const {
  // NOTA: t is unused (for now). It is left here because it might be used in
  // the future and it helps understand what's going on.
  assert(t == t); // Suppress warning

  // this is the intracellular network


  // Circadian clock parameters
  int16_t p = 4;
  int16_t h = 4;
  int16_t q = 2;
  int16_t r = 3;

  // INPUT FROM other cells
  double F = getInSignal(InterCellSignal::CLOCK);
  double death_signal = 0.0*getInSignal(InterCellSignal::CYCLE) + getInSignal(InterCellSignal::KILL);
//  double cell_volume = outer_volume();


  // CELL CYCLE VARIABLES (ref: Introducing a reduced cell cycle model, Battogtokh & Tyson 2006 PRE)
  double cy_x = y[CY_X];   // y is a fictive intermediate internal state where the stepper wants to know the derivative
  double cy_z = y[CY_Z];   // we copy it into local variables to make the equations shorter and clearer

  // DEATH AND SURVIVAL VARIABLES
  double cd_death = y[CD_DEATH];

  // CIRCADIAN CLOCK VARIABLES (re: Bernard et al. 2007 PLoS Comp Biol.)
  double y1 = y[Y1];
  double y2 = y[Y2];
  double y3 = y[Y3];
  double y4 = y[Y4];
  double y5 = y[Y5];
  double y6 = y[Y6];
  double y7 = y[Y7];
  double v  = y[V];
  double x1 = y[X1];
  double x2 = y[X2];

  // CELL CYCLE AUXILIARY FUNCTION
  double e_temp = cy_p-cy_x+cy_p*cy_j+cy_x*cy_j;
  double W0 = 2*cy_x*cy_j/(e_temp+sqrt(e_temp*e_temp-4*cy_x*cy_j*(cy_p-cy_x)));
  double a_temp = cy_k6+cy_k7*cy_z;
  double b_temp = cy_k8*outer_volume()/ size_.volume_min() +cy_k9*cy_x;
  e_temp = b_temp-a_temp+b_temp*cy_j+a_temp*cy_j;
  double Y0 = 2*a_temp*cy_j/(e_temp+sqrt(e_temp*e_temp-4*a_temp*cy_j*(b_temp-a_temp)));

  // CIRCADIAN CLOCK NON-LINEARITIES
  // These are non linear functions of the clock parameters and intracellular state
  // Their values will change at each gsl_odeiv2_step (small time step h < DT)
  double Q;        // coupling term
  double fpercry;  // Hill function
  double fbmal;    // Hill function
  double L;        // Light input (or neuropeptide input, the same pathway is activated)
  //double nb_neighbors = this->_nb_neighbors;
  //if (nb_neighbors > 0) Q = K*F/nb_neighbors; else Q = 0.0; // Normalize with nb_neighbors, if > 0
  // The coupling is not exactly as in Bernard et al 2007. The factor Q is not normalized
  // by the number of neighbors. As a result, the coupling strength must be adapted, with
  // K ~ 0.2 instead of K ~ 1.0, if there are around 5 neighbors. To normalize with the
  // number of neighbors, the code just above can be used.
  Q = K*F;
  fpercry = v1b*(y7+pow(x2,h))/(k1b*(1+pow(y3/k1i,p))+(y7+pow(x2,h)));
  fbmal = v4b*pow(y3,r)/(pow(k4b,r)+pow(y3,r));
  L = L0;

  // CELL CYCLE EQUATIONS
  // cycling signal variables cy
  // dX/dt = m (k1+k2*W0)-(k3+k4*Y0+k5*Z)*X
  // dZ/dt = k10(1+A(1+sin(f*t)))+k11*X-k12*Z
  // ORIGINAL EQ  f[0] = cell_volume*(cy_k1+cy_k2*W0) - (cy_k3+cy_k4*Y0+cy_k5*cy_z)*cy_x;
  dydt[CY_X] = (outer_volume() / size_.volume_min() * (cy_k1+cy_k2*W0) -
      (cy_k3+cy_k4*Y0+cy_k5*cy_z)*cy_x) + 0.1*y7;
  dydt[CY_Z] = (cy_k10*(1+cy_A)+cy_k11*cy_x-cy_k12*cy_z);

  // death signal variable cd
  dydt[CD_DEATH] = death_signal - k_survival * cd_death;

  // CIRCADIAN CLOCK EQUATIONS
  dydt[Y1]  = per_scal_ * (fpercry - k1d*y1) + L;                    // y1'
  dydt[Y2]  = per_scal_ * (k2b*pow(y1,q) - (k2d+k2t)*y2 + k3t*y3);   // y2'
  dydt[Y3]  = per_scal_ * (k2t*y2 - k3t*y3 - k3d*y3);                // y3'
  dydt[Y4]  = per_scal_ * (fbmal - k4d*y4);                          // y4'
  dydt[Y5]  = per_scal_ * (k5b*y4 - (k5d+k5t)*y5 + k6t*y6);          // y5'
  dydt[Y6]  = per_scal_ * (k5t*y5 - (k6t+k6d)*y6 + k7a*y7 - k6a*y6); // y6'
  dydt[Y7]  = per_scal_ * (k6a*y6 - (k7a+k7d)*y7);                   // y7'
  dydt[V]  = k8*y2 - k8d*v;                           // v'
  dydt[X1] = kx1*Q*(x1T - x1) - kdx1*x1;              // x1'
  dydt[X2] = kx2*x1*(x2T - x2) - kdx2*x2;             // x2'

  return GSL_SUCCESS;
}

bool Cell_SyncClock::registered_ =
    Cell::RegisterClass(classId_, classKW_,
                        [](CellType type,
                           const MoveBehaviour& move_behaviour,
                           const Coordinates<double>& pos,
                           double initial_volume,
                           double volume_min,
                           double doubling_time){
                          return static_cast<Cell*>(
                              new Cell_SyncClock(type, move_behaviour,
                                           pos, initial_volume, volume_min,
                                           doubling_time));
                        },
                        [](gzFile backup_file){
                          return static_cast<Cell*>(new Cell_SyncClock(backup_file));
                        }
    );
