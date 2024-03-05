// ****************************************************************************
//
//              SiMuScale - Multi-scale simulation framework
//
// ****************************************************************************

// ============================================================================
//                                   Includes
// ============================================================================
#include "Cancer.h"
#include <vector>
using std::vector;
#include <iostream>
#include <random>
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
#include "Cell.h"
using std::unique_ptr;
using std::shared_ptr;
#include <sstream>
#include <string>
#include <fstream>
using std::ifstream;
using namespace std;
// ============================================================================
//                       Definition of static attributes
// ============================================================================
constexpr CellFormalism Cancer::classId_;
constexpr char Cancer::classKW_[];
constexpr uint32_t Cancer::odesystemsize_;

// ============================================================================
//                                Constructors
// ============================================================================
Cancer::Cancer(const Cancer &model) :
Cell(model),
isMitotic_(model.isMitotic_) {
    internal_state_ = new double[odesystemsize_];
    memcpy(internal_state_, model.internal_state_, odesystemsize_ * sizeof (*internal_state_));
    phylogeny_id_.push_back(this->id());
    phylogeny_t_.push_back(Simulation::sim_time());

}
/**
 * Create a new object with the provided constitutive elements and values
 */
Cancer::Cancer(CellType cellType,
                   const MoveBehaviour& move_behaviour,
                   const Coordinates<double>& pos,
                   double initial_volume,
                   double volume_min,
                   double doubling_time) :
    Cell(cellType, move_behaviour,
         pos, CellSize(initial_volume, volume_min),
         doubling_time) {
    internal_state_ = new double[odesystemsize_];
    internal_state_[Type] = 0.;
    phylogeny_id_.push_back(this->id());
    phylogeny_t_.push_back(Simulation::sim_time());
}

/** 
 * Create a new object from an existing one 
 */
/*Cancer::Cancer(const Cancer &model):
  Cell(model) {
}
*/
/**
 * Restore an object that was backed-up in backup_file
 */
Cancer::Cancer(gzFile backup_file) :
    Cell(backup_file) {
    internal_state_ = new double[odesystemsize_];
  Load(backup_file);
}

// ============================================================================
//                                 Destructor
// ============================================================================
Cancer::~Cancer() = default;

// ============================================================================
//                                   Methods
// ============================================================================
void Cancer::InternalUpdate(const double& dt) {

if (cell_type_ != CellType::NICHE) {
    Cell_update(dt);

    } 
  // basic speed sigma_
/* nn_ = 0;
  Coordinates<double> where(0.0,0.0,0.0);
  if ( neighbours().size() > 0 ) {
    for ( auto cell : neighbours() ) {
      if ( cell->cell_type() != CANCER) {
        where += cell->pos();
        nn_++;
     }
*/   
  UpdateCyclingStatus();

 
/*  if ( nn_ ) {
    sigma_ = 0.1;
    orientation_ = where/static_cast<double>(nn_);
  }
  else {
    sigma_ += dt*0.1;
    if ( Alea::random() > 0.9 ) // switch orientation
      orientation_ = -1.0*orientation(); 
  }*/

  // update orientation
/*  Coordinates<double> delta(dt*Alea::gaussian_random(),
                            dt*Alea::gaussian_random(),
                            dt*Alea::gaussian_random());
  orientation_ += delta;
  orientation_ =  orientation_/orientation().norm(); 
*/
//  if ( isCycling() ) Grow(dt); // Grow is a Cell method
if ( internal_state_[Type] == 0.){ 

Grow(dt);} 
 else {
Grow(dt*4.);}
}
void Cancer::Cell_update(const double& dt){
  int C;
   if (this->pos_x()==20. && this->pos_y()==20. && this->pos_z()==20.)
	{

cout << this->id()<<endl;}
if ( Simulation::pop().Size()>4000 && this->id()==5){ internal_state_[Type] = 1.;}
}

Coordinates<double> Cancer::MotileDisplacement(const double& dt) {
  // <TODO> Motile cell autonomous displacement behaviour
//  Coordinates<double> displ = dt*0.02*Alea::gaussian_random();
double sigma_t = 0.1;  
 Coordinates<double> displ { sqrt(dt)*sigma_t*Alea::gaussian_random(),
                               sqrt(dt)*sigma_t*Alea::gaussian_random(),
                                sqrt(dt)*sigma_t*Alea::gaussian_random() };
  return displ;

}

void Cancer::UpdatePhylogeny(vector<int> phy_id, vector<double> phy_t, int sz) {
    phylogeny_id_.insert(phylogeny_id_.begin(), phy_id.begin(), phy_id.end());
    phylogeny_t_.insert(phylogeny_t_.begin(), phy_t.begin(), phy_t.end());
}
void Cancer::UpdateType(double Mother_type) {
      internal_state_[Type] =  Mother_type;
}
Cell* Cancer::Divide(void) {
  // <TODO> Determine how the cell should divide </TODO>
  Cancer* newCell = new Cancer(*this);

  //std::cout << "Cell " << this->id() << " is DIVIDING!!" <<  " at t=" << Simulation::sim_time() << std::endl;    ///remember dividin for the cell

  SeparateDividingCells(this, newCell);
 // std::cout << "Cell " << this->id() << " is DIVIDING!!" <<  " at t=" << Simulation::sim_time()<< ", Pop= "<<  Simulation::pop().Size() << std::endl;    ///remember dividin for the cell

  newCell->UpdateType(internal_state_[Type]);
  //  std::cout << "Type " << internal_state_[Type]<<std::endl;
  newCell->UpdatePhylogeny(phylogeny_id_, phylogeny_t_, phylogeny_id_.size());
  phylogeny_id_.push_back(this->id());

  phylogeny_t_.push_back(Simulation::sim_time());

  if ( ( phylogeny_ID_file_ = fopen(phylogeny_ID_filename,"a") ) != NULL ) {
    fprintf(phylogeny_ID_file_, "%d -> %d , t= %f \n", this->id(),  newCell-> id(),  Simulation::sim_time());
      fclose(phylogeny_ID_file_);
    }
    if ( ( phylogeny_T_file_ = fopen(phylogeny_T_filename,"a") ) != NULL ) {
      fprintf(phylogeny_T_file_, "%f \n ", Simulation::sim_time());
      fclose(phylogeny_T_file_);
    }

  return newCell;
}

void Cancer::Save(gzFile backup_file) const {
  // Write my classId
  gzwrite(backup_file, &classId_, sizeof(classId_));

  // Write the generic cell stuff
  Cell::Save(backup_file);

  for (u_int32_t i = 0; i < odesystemsize_; i++){
        gzwrite(backup_file, &internal_state_[i], sizeof (internal_state_[i]));

    }

  // Write the specifics of this class
  // <TODO> Save your specific attributes </TODO>
}

void Cancer::Load(gzFile backup_file) {
  // <TODO> Load your specific attributes </TODO>
 for (u_int32_t i = 0; i < odesystemsize_; i++)
        gzread(backup_file, &internal_state_[i], sizeof (internal_state_[i]));
}

double Cancer::get_output(InterCellSignal signal) const {
  /*if ( signal == InterCellSignal::KILL )
    return toxic_signal_;
  else if ( signal == InterCellSignal::DEATH )
    return -1.0;
  else if ( signal == InterCellSignal::CYCLE )
    return static_cast<double>(isCycling_);
*/
switch (signal) {
 case InterCellSignal::CANCER_TYPE:
      return  internal_state_[Type]; 
 case InterCellSignal::VOLUME:
            return outer_volume();


 return 0.0;
}
}

bool Cancer::isCycling() const {
  // <TODO> Is the cell currently cycling ? </TODO>
  return isCycling_;
}

bool Cancer::isDividing() const {
  // <TODO> Should the cell divide now ? </TODO>
  if  (outer_volume() >= 2.)// size_.volume_min()) 
    return true;
  else
    return false;
}

bool Cancer::isDying() const {
  // <TODO> Should the cell die now ? </TODO>
  // cell is immortal
  return false;
}

void Cancer::UpdateCyclingStatus() {

  if ( nn_ ) {
    isCycling_ = true;
  }
  else {
    isCycling_ = false;
  }
 
}

bool Cancer::StopCycling() {
  // <TODO> Should the cell stop cycling now ? </TODO>
  return false;
}

bool Cancer::StartCycling() {
  // <TODO> Should the cell start cycling now ? </TODO>
  return false;
}


// Register this class in Cell
bool Cancer::registered_ =
    Cell::RegisterClass(classId_, classKW_,
                        [](CellType type,
                           const MoveBehaviour& move_behaviour,
                           const Coordinates<double>& pos,
                           double initial_volume,
                           double volume_min,
                           double doubling_time){
                          return static_cast<Cell*>(
                              new Cancer(type, move_behaviour,
                                           pos, initial_volume, volume_min,
                                           doubling_time));
                        },
                        [](gzFile backup_file){
                          return static_cast<Cell*>(new Cancer(backup_file));
                        }
    );
