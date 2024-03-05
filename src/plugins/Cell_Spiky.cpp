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
#include "Cell_Spiky.h"
#include "OutputManager.h"
#include <iostream>

// ============================================================================
//                       Definition of static attributes
// ============================================================================
constexpr CellFormalism Cell_Spiky::classId_;
constexpr char Cell_Spiky::classKW_[];

// ============================================================================
//                                Constructors
// ============================================================================
/**
 * Create a new object with the provided constitutive elements and values
 */
Cell_Spiky::Cell_Spiky(CellType cellType,
                   const MoveBehaviour& move_behaviour,
                   const Coordinates<double>& pos,
                   double initial_volume,
                   double volume_min,
                   double doubling_time) :
    Cell(cellType, move_behaviour,
         pos, CellSize(initial_volume, volume_min),
         doubling_time) {
    x1_ = 0.0;
    x2_ = 0.0;
    x3_ = 0.0;
    
    // override the random initial positions
    // std::cout << "override initial position of cell id " << id() << "\n";
    // pos_.x = 10.0 + 10.0 * (Alea::random());
    // pos_.y = 10.0 + 10.0 * (Alea::random());
    // pos_.z = 3.0; 

}

/** 
 * Create a new object from an existing one 
 */
Cell_Spiky::Cell_Spiky(const Cell_Spiky &model):
  Cell(model) {
  x1_ = 0.0;
  x2_ = model.x2_/2.0;
  x3_ = model.x3_/2.0;
}

/**
 * Restore an object that was backed-up in backup_file
 */
Cell_Spiky::Cell_Spiky(gzFile backup_file) :
    Cell(backup_file) {
  Load(backup_file);
}

// ============================================================================
//                                 Destructor
// ============================================================================
Cell_Spiky::~Cell_Spiky() = default;

// ============================================================================
//                                   Methods
// ============================================================================
void Cell_Spiky::InternalUpdate(const double& dt) {
  // Example: 1. update the intracellular state.
  // Here, is a a very simple, discrete model. 
  x1_ += dt; // x1_ just count the time since birth
  x3_ = getInSignal(InterCellSignal::SPIKY_1);  // Try to get the signal called SPIKY_1

  // x2_ is a protein with constant production and linear degradation
  // and a unimodal nonlinear production rate that depends on x3_
  x2_ += dt*( production_ - degradation_*x2_) + dt*0.3*x3_/(0.09 + x3_*x3_); 

  // Example: 2. Check whether it is dying
  if ( isDying() ) return; // if dying, return immediately, don't go through cycling 

  // Example: 3. Update cycling status
  // here we don't update anything

  // Example: 4. if cycling, then grow
  if ( isCycling() ) Grow(dt); // Grow is a Cell method

  // Example: 5. Division status is managed at simulation level
  // nothing to do here, but complete Cell_Spiky::isDividing()

}

Cell* Cell_Spiky::Divide(void) {
  // Create a copy of the cell with no mechanical forces nor chemical stimuli
  // applied to it
  Cell_Spiky* newCell = new Cell_Spiky(*this);

  // Move cells so they do not have the same center
  SeparateDividingCells(this, newCell);

  x1_ = 0;  // reset the age of the cell to 0
  
  return newCell;
}




void Cell_Spiky::Save(gzFile backup_file) const {
  // Write my classId
  gzwrite(backup_file, &classId_, sizeof(classId_));

  // Write the generic cell stuff
  Cell::Save(backup_file);

  // Write the specifics of this class
  // <TODO> Save your specific attributes </TODO> -- DONE
  gzwrite(backup_file,&x1_,sizeof(x1_));
  gzwrite(backup_file,&x2_,sizeof(x2_));
  gzwrite(backup_file,&x3_,sizeof(x3_));
}

void Cell_Spiky::Load(gzFile) {
  // <TODO> Load your specific attributes </TODO>
}

double Cell_Spiky::get_output(InterCellSignal signal) const {
  if ( signal == InterCellSignal::SPIKY_1 )
    return x2_;
  else
    return 0.0;
}

bool Cell_Spiky::isCycling() const {
  return true; // cell is always cycling
}

bool Cell_Spiky::isDividing() const {
  // Should the cell divide now?
  if ( isCycling()       // is the cell in cell cycle?
      && ( x1_ > 36 )    // x1_: age of cell. Is it more than 24h since last division?
      && ( x2_ > 0.50 )  // x2_: protein concentration large enough?
      ) 
  {
    return true;         // then divide
  }
  else
    return false;        // keep growing
}

bool Cell_Spiky::isDying() const {
  // Should the cell die now?
  if ( x1_ > max_age_ ) {
    return true;
  }
  else if ( getInSignal(InterCellSignal::KILL) > 0.1 ) 
    return true;
  else
    return false;
}

void Cell_Spiky::UpdateCyclingStatus() {
}

bool Cell_Spiky::StopCycling() {
  // <TODO> Should the cell stop cycling now ? </TODO>
  return false;
}

bool Cell_Spiky::StartCycling() {
  // <TODO> Should the cell start cycling now ? </TODO>
  return false;
}

Coordinates<double> Cell_Spiky::MotileDisplacement(const double& dt) {
  // <TODO> Motile cell autonomous displacement behaviour
  size_t nbr_neighbours = neighbours().size();
  double sigma_t = ( nbr_neighbours == 0 )*sigma_ + ( nbr_neighbours > 0 )*sigma_/10.0;
  Coordinates<double> displ { sqrt(dt)*sigma_t*Alea::gaussian_random(),
                              sqrt(dt)*sigma_t*Alea::gaussian_random(),
                              sqrt(dt)*sigma_t*Alea::gaussian_random() - dt*0.0 };
  return displ;

}

// Register this class in Cell
bool Cell_Spiky::registered_ =
    Cell::RegisterClass(classId_, classKW_,
                        [](CellType type,
                           const MoveBehaviour& move_behaviour,
                           const Coordinates<double>& pos,
                           double initial_volume,
                           double volume_min,
                           double doubling_time){
                          return static_cast<Cell*>(
                              new Cell_Spiky(type, move_behaviour,
                                           pos, initial_volume, volume_min,
                                           doubling_time));
                        },
                        [](gzFile backup_file){
                          return static_cast<Cell*>(new Cell_Spiky(backup_file));
                        }
    );
