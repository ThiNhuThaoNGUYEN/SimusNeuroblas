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
#include "Cell_Killer.h"

// ============================================================================
//                       Definition of static attributes
// ============================================================================
constexpr CellFormalism Cell_Killer::classId_;
constexpr char Cell_Killer::classKW_[];

// ============================================================================
//                                Constructors
// ============================================================================
/**
 * Create a new object with the provided constitutive elements and values
 */
Cell_Killer::Cell_Killer(CellType cellType,
                   const MoveBehaviour& move_behaviour,
                   const Coordinates<double>& pos,
                   double initial_volume,
                   double volume_min,
                   double doubling_time) :
    Cell(cellType, move_behaviour,
         pos, CellSize(initial_volume, volume_min),
         doubling_time) {

}

/** 
 * Create a new object from an existing one 
 */
Cell_Killer::Cell_Killer(const Cell_Killer &model):
  Cell(model) {
}

/**
 * Restore an object that was backed-up in backup_file
 */
Cell_Killer::Cell_Killer(gzFile backup_file) :
    Cell(backup_file) {
  Load(backup_file);
}

// ============================================================================
//                                 Destructor
// ============================================================================
Cell_Killer::~Cell_Killer() = default;

// ============================================================================
//                                   Methods
// ============================================================================
void Cell_Killer::InternalUpdate(const double& dt) {

  // basic speed sigma_
  nn_ = 0;
  Coordinates<double> where(0.0,0.0,0.0);
  if ( neighbours().size() > 0 ) {
    for ( auto cell : neighbours() ) {
      if ( cell->cell_type() != KILLER ) {
        where += cell->pos();
        nn_++;
      }
    }
  }

  UpdateCyclingStatus();

  if ( nn_ ) {
    sigma_ = 0.1;
    orientation_ = where/static_cast<double>(nn_);
  }
  else {
    sigma_ += dt*0.1;
    if ( Alea::random() > 0.9 ) // switch orientation
      orientation_ = -1.0*orientation(); 
  }

  // update orientation
  Coordinates<double> delta(dt*Alea::gaussian_random(),
                            dt*Alea::gaussian_random(),
                            dt*Alea::gaussian_random());
  orientation_ += delta;
  orientation_ =  orientation_/orientation().norm(); 

  if ( isCycling() ) Grow(dt); // Grow is a Cell method


}

Coordinates<double> Cell_Killer::MotileDisplacement(const double& dt) {
  // <TODO> Motile cell autonomous displacement behaviour
  Coordinates<double> displ = dt*sigma_*orientation();
  // Coordinates<double> displ { sqrt(dt)*sigma_t*Alea::gaussian_random(),
  //                             sqrt(dt)*sigma_t*Alea::gaussian_random(),
  //                             - dt*0.01 };
  return displ;

}

Cell* Cell_Killer::Divide(void) {
  // <TODO> Determine how the cell should divide </TODO>
  Cell_Killer* newCell = new Cell_Killer(*this);

  SeparateDividingCells(this, newCell);

  return newCell;
}

void Cell_Killer::Save(gzFile backup_file) const {
  // Write my classId
  gzwrite(backup_file, &classId_, sizeof(classId_));

  // Write the generic cell stuff
  Cell::Save(backup_file);

  // Write the specifics of this class
  // <TODO> Save your specific attributes </TODO>
}

void Cell_Killer::Load(gzFile backup_file) {
  // <TODO> Load your specific attributes </TODO>
}

double Cell_Killer::get_output(InterCellSignal signal) const {
  if ( signal == InterCellSignal::KILL )
    return toxic_signal_;
  else if ( signal == InterCellSignal::DEATH )
    return -1.0;
  else if ( signal == InterCellSignal::CYCLE )
    return static_cast<double>(isCycling_);
  return 0.0;
}

bool Cell_Killer::isCycling() const {
  // <TODO> Is the cell currently cycling ? </TODO>
  return isCycling_;
}

bool Cell_Killer::isDividing() const {
  // <TODO> Should the cell divide now ? </TODO>
  if ( isCycling() && outer_volume() > size_.volume_min() ) 
    return true;
  else
    return false;
}

bool Cell_Killer::isDying() const {
  // <TODO> Should the cell die now ? </TODO>
  // cell is immortal
  return false;
}

void Cell_Killer::UpdateCyclingStatus() {

  if ( nn_ ) {
    isCycling_ = true;
  }
  else {
    isCycling_ = false;
  }
 
}

bool Cell_Killer::StopCycling() {
  // <TODO> Should the cell stop cycling now ? </TODO>
  return false;
}

bool Cell_Killer::StartCycling() {
  // <TODO> Should the cell start cycling now ? </TODO>
  return false;
}


// Register this class in Cell
bool Cell_Killer::registered_ =
    Cell::RegisterClass(classId_, classKW_,
                        [](CellType type,
                           const MoveBehaviour& move_behaviour,
                           const Coordinates<double>& pos,
                           double initial_volume,
                           double volume_min,
                           double doubling_time){
                          return static_cast<Cell*>(
                              new Cell_Killer(type, move_behaviour,
                                           pos, initial_volume, volume_min,
                                           doubling_time));
                        },
                        [](gzFile backup_file){
                          return static_cast<Cell*>(new Cell_Killer(backup_file));
                        }
    );
