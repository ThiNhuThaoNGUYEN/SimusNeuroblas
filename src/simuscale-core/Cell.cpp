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
#include "Cell.h"

#include <cmath>
#include <cassert>

#include <cstdlib>
#include <iostream>
#include <params/SimulationParams.h>

#include "Alea.h"
#include "WorldSize.h"
#include "movement/MoveBehaviour.h"
#include "movement/Immobile.h"
#include "movement/Mobile.h"
#include "Simulation.h"

using std::string;
using std::map;
using std::cout;
using std::endl;

// =================================================================
//                    Definition of static attributes
// =================================================================
uint32_t Cell::maxID_ = 1;
double Cell::max_force_ = 0.5;
double Cell::LJ_epsilon_ = 0.002;

// =================================================================
//                             Constructors
// =================================================================
Cell::Cell(const Cell &model) :
    Observable(),
    id_(maxID_++),
    pos_(model.pos_),
    orientation_(model.orientation_),
    size_(model.size_),
    cell_type_(model.cell_type_),
    doubling_time_(model.doubling_time_),
    move_behaviour_(model.move_behaviour_) {
}

Cell::Cell(CellType cell_type,
           const MoveBehaviour& move_behaviour,
           Coordinates<double> pos,
           CellSize&& size,
           double doubling_time) :
    id_(maxID_++),
    pos_(pos),
    orientation_(0.0, 0.0, 0.0),
    size_(std::move(size)),
    cell_type_(cell_type),
    doubling_time_(doubling_time),
    move_behaviour_(&move_behaviour),
    mechanical_force_(0, 0, 0) {
}

Cell::Cell(gzFile backup_file) : Cell() {
  Load(backup_file);
}


// =================================================================
//                             Destructor
// =================================================================

// =================================================================
//                            Public Methods
// =================================================================
void Cell::Update(const double& dt) {
  //std::cout << "CellUpdate"<< std::endl;
  Move(dt);
  InternalUpdate(dt);
}

double Cell::Distance(const Cell* other) const {
  return (pos_ - other->pos_).norm();
}

void Cell::Move(const double& dt) {
  move_behaviour_->Move(this, dt);
}

bool Cell::isDead() const {
  return isDying(); /* default behaviour: cell is dead as soon as it is dying */
}

/* 
 * get_output: deprecated, see local_signal
 */
double Cell::get_output(InterCellSignal signal) const {
  return local_signal(signal);
}

/* 
 * local_signal: same as get_output. preferred over 
 * get_output, which is deprecated.
 */
double Cell::local_signal(InterCellSignal signal) const {
  return get_output(signal);
}

Coordinates<double> Cell::MotileDisplacement(const double& ) {
  Coordinates<double> displ { 0.0, 0.0, 0.0 }; // default MOTILE Movement Behaviour
  return displ;
}

/*
 * motility: same as MotileDisplacement
 */
Coordinates<double> Cell::motility(const double& dt) {
  return MotileDisplacement(dt); 
}


void Cell::Save(gzFile backup_file) const {
  gzwrite(backup_file, &id_, sizeof(id_));
  move_behaviour_->Save(backup_file);
  gzwrite(backup_file, &doubling_time_, sizeof(doubling_time_));
  gzwrite(backup_file, &is_dying_, sizeof(is_dying_));
  gzwrite(backup_file, &is_dead_, sizeof(is_dead_));
  gzwrite(backup_file, &hold_space_, sizeof(hold_space_));
  int8_t cell_type = static_cast<int8_t>(cell_type_);
  gzwrite(backup_file, &cell_type, sizeof(cell_type));
  gzwrite(backup_file, &max_force_, sizeof(max_force_));
  gzwrite(backup_file, &LJ_epsilon_, sizeof(LJ_epsilon_));
  mechanical_force_.Save(backup_file);
  intrinsic_inputs_.Save(backup_file);
  pos_.Save(backup_file);
  orientation_.Save(backup_file);
  size_.Save(backup_file);

//  int8_t isCycling = isCycling_ ? 1 : 0;
//  gzwrite(backup_file, &isCycling, sizeof(isCycling));

  // TODO <david.parsons@inria.fr> Interactions should be recomputed ?
}

void Cell::Load(gzFile backup_file) {
  gzread(backup_file, &id_, sizeof(id_));
  move_behaviour_ = MoveBehaviour::Load(backup_file);
  gzread(backup_file, &doubling_time_, sizeof(doubling_time_));
  gzread(backup_file, &is_dying_, sizeof(is_dying_));
  gzread(backup_file, &is_dead_, sizeof(is_dead_));
  gzread(backup_file, &hold_space_, sizeof(hold_space_));
  int8_t cell_type;
  gzread(backup_file, &cell_type, sizeof(cell_type));
  cell_type_ = static_cast<CellType>(cell_type);
  gzread(backup_file, &max_force_, sizeof(max_force_));
  gzread(backup_file, &LJ_epsilon_, sizeof(LJ_epsilon_));
  mechanical_force_.Load(backup_file);
  intrinsic_inputs_.Load(backup_file);
  pos_.Load(backup_file);
  orientation_.Load(backup_file);
  size_ = size_.Load(backup_file);

//  move_behaviour_ = (cell_type_ == NICHE) ?
//                    static_cast<MoveBehaviour*>(&Immobile::instance()) :
//                    static_cast<MoveBehaviour*>(&Mobile::instance());

//  int8_t tmp;
//  gzread(backup_file, &tmp, sizeof(tmp));
//  isCycling_ = tmp;

  // TODO <david.parsons@inria.fr> Interactions should be recomputed ?
}

void Cell::Dump() {

  std::cout << "  {\n";
  std::cout << "    \"id\": " << id_ << ",\n"
            << "    \"cell formalism\": \"" << cell_formalism() << "\",\n" 
            << "    \"cell type\": \"" << CellType_Names.at(cell_type_) << "\",\n"
            << "    \"move behaviour\": " << static_cast<int>(move_behaviour_->type()) << ",\n"
            << "    \"doubling time\": " << doubling_time_ << ",\n"
            << "    \"is dying\": " << is_dying_ << ",\n"
            << "    \"is dead\": "  << is_dead_  << ",\n"
            << "    \"hold space\": " << hold_space_ << ",\n"
            << "    \"position\": [" << pos_.x << ", " << pos_.y << ", " << pos_.z << "],\n"
            << "    \"orientation\": [" << orientation_.x << ", " << orientation_.y << ", " << orientation_.z << "],\n"
            << "    \"mechanical force\": [" << mechanical_force_.x << ", " << mechanical_force_.y << ", " << mechanical_force_.z << "],\n"
            << "    \"max force\": " << max_force_ << ",\n"
            << "    \"LJ epsilon\": " << LJ_epsilon_ << ",\n"
            << "    \"radii\": [" << size_.internal_radius() << ", " << size_.external_radius() << "],\n";

  /* dump local signal input */
  std::cout << "    \"signal input\": [\n";
  for ( auto signal : Simulation::using_signals() ) {  
    std::cout << "      {\n";
    std::cout << "        \"signal\": \"" << InterCellSignal_Names.at(signal) << "\",\n"
              << "        \"type\": \"local\",\n"
              << "        \"value\": " << getInSignal(signal) << "\n"
              << "      }" << (signal == Simulation::using_signals().back() && Simulation::fgt()->using_diffusive_signals().size() == 0 ? "" : ",") << "\n";
  }

  /* dump diffusive signal input */
  for ( auto signal : Simulation::fgt()->using_diffusive_signals() ) {
    std::cout << "      {\n";
    std::cout << "        \"signal\": \"" << InterCellSignal_Names.at(signal) << "\",\n"
              << "        \"type\": \"diffusive\",\n" 
              << "        \"targets\": [\n"; 
    auto n = get_diffusive_signal(signal).size(); 
    auto ts = gaussian_field_targets(signal); // define ts to avoid dangling references
    auto&& t = ts.begin();
    for ( auto val : get_diffusive_signal(signal) ) {
      std::cout << "          {\n";
      std::cout << "            \"position\": [" << t->x << ", " << t->y << ", " << t->z << "],\n";
      std::cout << "            \"value\": " << val << "\n"
                << "          }" << ( --n ? "," : "") << "\n";
      t++;
    }
    std::cout << "        ]\n"
              << "      }" << (signal ==  Simulation::fgt()->using_diffusive_signals().back() ? "" : ",") << "\n";
  }
  std::cout << "    ],\n";

  /* dump local signal output */
  std::cout << "    \"signal output\": [\n";
  for ( auto signal : Simulation::using_signals() ) {   
    std::cout << "      {\n";
    std::cout << "        \"signal\": \"" << InterCellSignal_Names.at(signal) << "\",\n"
              << "        \"type\": \"local\",\n"
              << "        \"value\": " << local_signal(signal) << "\n"
              << "      }" << (signal == Simulation::using_signals().back() && Simulation::fgt()->using_diffusive_signals().size() == 0 ? "" : ",") << "\n";
  }

  /* dump diffusive signal output */
  for ( auto signal : Simulation::fgt()->using_diffusive_signals() ) {
    auto weight = gaussian_field_weight(signal);
    auto source = gaussian_field_source(signal);
    std::cout << "      {\n";
    std::cout << "        \"signal\": \"" << InterCellSignal_Names.at(signal) << "\",\n"
              << "        \"type\": \"diffusive\",\n";
    std::cout << "        \"position\": [" << source.x << ", " << source.y << ", " << source.z << "],\n";
    std::cout << "        \"value\": " << weight << "\n"
              << "      }" << (signal ==  Simulation::fgt()->using_diffusive_signals().back() ? "" : ",") << "\n";
  }
  std::cout << "    ]";

}

Cell* Cell::MakeCell(CellFormalism formalism,
                     const MoveBehaviour& move_behaviour,
                     CellType type,
                     Coordinates<double> pos,
                     double initial_volume,
                     double volume_min,
                     double doubling_time) {
  return factories().at(formalism)(type, move_behaviour,
                                   pos, initial_volume, volume_min,
                                   doubling_time);
}

Cell* Cell::LoadCell(gzFile backup_file) noexcept(false) {
  CellFormalism formalism;
  gzread(backup_file, &formalism, sizeof(formalism));
  return loaders().at(formalism)(backup_file);
}

/*
 * Container for derived class factories
 *
 * NOTE: Use the "Construct On First Use Idiom" to avoid the
 * "static initialization order fiasco"
 */
Cell::CellFactories& Cell::factories() {
  static CellFactories* factories = new CellFactories();
  return *factories;
}

/*
 * Container for derived class loader factories
 *
 * NOTE: Use the "Construct On First Use Idiom" to avoid the
 * "static initialization order fiasco"
 */
Cell::CellLoaders& Cell::loaders() {
  static CellLoaders* loaders = new CellLoaders();
  return *loaders;
}

void Cell::SaveStatic(gzFile backup_file) {
  gzwrite(backup_file, &maxID_, sizeof(maxID_));

  CellSize::SaveStatic(backup_file);
}

void Cell::LoadStatic(gzFile backup_file) {
  gzread(backup_file, &maxID_, sizeof(maxID_));

  CellSize::LoadStatic(backup_file);
}

// =================================================================
//                           Protected Methods
// =================================================================
void Cell::ResetNeighbours() {
  neighbours_.clear();
}

void Cell::AddNeighbour(Cell* neighbour) {
  neighbours_.push_back(neighbour);
}

void Cell::ComputeInteractions() {
  ResetInteractions();
  for (auto neighbour : neighbours_) {
    AddMechForce(neighbour);
    AddChemCom(neighbour);
  }
}

/* \deprecated ComputeGaussianFields computes Gaussian interaction, direct method.
 * Use dedicated methods gaussian_field_weight, gaussian_field_source, and gaussian_field_targets
 * instead.
 */
void Cell::ComputeGaussianFields() {

  std::cerr << "Cell::ComputeGaussianFields. This methods is deprecated and should not be used." << std::endl;
   
  for ( unsigned long i = 0; i < nbr_signals; ++i) { 
    intrinsic_inputs_.gaussian_field(static_cast<InterCellSignal>(i)).assign(gaussian_field_targets(static_cast<InterCellSignal>(i)).size(),0.0);
    for (auto other : Simulation::pop().cell_list() ) {
      unsigned int j = 0;
      for (auto target : gaussian_field_targets(static_cast<InterCellSignal>(i)) ) { 
        // do something
        // target is a Coordinate
        const Coordinates<double>& source = other->gaussian_field_source(static_cast<InterCellSignal>(i));
        double  q = other->gaussian_field_weight(static_cast<InterCellSignal>(i));
        intrinsic_inputs_.gaussian_field(static_cast<InterCellSignal>(i)).at(j) += 
          q*exp((source.x - target.x)*(source.x - target.x))*\
            exp((source.y - target.y)*(source.y - target.y))*\
            exp((source.z - target.z)*(source.z - target.z));
        j++;
      }
    }
  }
}

void Cell::ResetInteractions() {
  ResetMechForce();
  ResetInSignals();
}

void Cell::SeparateDividingCells(Cell* oldCell, Cell* newCell) {
  double old_external_radius = oldCell->external_radius();

  // Update cell sizes
  oldCell->size_.divide();
  newCell->size_.set_volume(oldCell->size_.volume());

  // Update positions : divide along the axis with the lowest mechanical
  // constraints (already computed in mech_force_)
  // The resulting daughter cells will be highly interpenetrating and will
  // continue to gradually separate in the following time steps. The idea is
  // that the outer boundaries of the daughter cells will be bounded by those of
  // their parent (except for the noise that can push them a little further) so
  // that they won't penetrate other cells
  double distance = old_external_radius -
                    (oldCell->external_radius() + newCell->external_radius()) /
                        2.0;
  assert(distance > 0);

  // Cells divide along an axis parallel to the direction of less constraints
  double norm_direction = oldCell->mechanical_force_.norm();

  // Generate a random point on a sphere
  // cf. e.g. http://mathworld.wolfram.com/SpherePointPicking.html
  double theta = 2. * M_PI * Alea::random();
  double cos_phi = 2. * Alea::random() - 1;
  double sin_phi = sqrt(1 - cos_phi*cos_phi);

  Coordinates<double> deltapos;
  if (norm_direction > 0) {
    // Add a bit of noise to the cell division axis: we add a point on a small
    // sphere to the direction of division
    // convert v-theta to cartesian coords.
    const double noise_proportion = 0.1;
    Coordinates<double> noise{
        noise_proportion * distance * sin_phi * cos(theta),
        noise_proportion * distance * sin_phi * sin(theta),
        noise_proportion * distance * cos_phi };

    // Calculate the new position relative to the center of the mother cell
    deltapos.x = distance * oldCell->mechanical_force_.x / norm_direction + noise.x;
    deltapos.y = distance * oldCell->mechanical_force_.y / norm_direction + noise.y;
    deltapos.z = distance * oldCell->mechanical_force_.z / norm_direction + noise.z;
  }
  else { // random division axis
    // convert v-theta to cartesian coords.
    deltapos.x = distance * sin_phi * cos(theta);
    deltapos.y = distance * sin_phi * sin(theta);
    deltapos.z = distance * cos_phi;
  }

  // TODO: code not robust: it is possible for cell to have the same new position
  // is that still true ?

  // Update daughter cell positions
  oldCell->Move(-deltapos);
  newCell->Move(deltapos);
}

void Cell::Grow(const double& dt) {
  // Make cell grow
  growth_factor_ = pow(2, dt / doubling_time_);
  size_.set_volume(size_.volume() * growth_factor_);

  // NB: cell division is controlled by both cell size and
  // the intracellular state (e.g. concentration of a certain kind of molecules)
}

/*
 * Add the visco-elastic force applied to 'this' by 'other'
 */
void Cell::AddMechForce(const Cell* other) {
  /// don't move NICHE or other IMMOBILE cells
  if ( move_behaviour_->type() == MoveBehaviour::Type::IMMOBILE ) 
  {
    return;
  }
  
  // max_force should not be too large (~0.25-0.5 for Lennard-Jones force)
  // double max_force = 0.5;
  static constexpr double LJFACTOR = 1.122462048309373;

  auto dpos = other->pos_ - pos_;
  auto distance = Distance(other);

  // Capped Lennard-Jones Force
  // double epsilon = 0.002; // default 0.002, not much larger
  double sigma = (other->internal_radius() + internal_radius()) / LJFACTOR;
  double force = -24.0 * LJ_epsilon_ *
                 (2.0 * pow(sigma, 12) / pow(distance, 13) -
                  pow(sigma, 6) / pow(distance, 7));

  if (fabs(force) > max_force_) {
    force = (force > 0) ? max_force_ : -max_force_;
  }

  auto mech_force = force * dpos / distance;
  mech_force.assert_no_nan();
  mechanical_force_ += mech_force;
}

void Cell::ResetMechForce() {
  mechanical_force_.reset();
}

/*
 * Compute the biochemical action of 'other' onto 'this'
 * The signal is proportional to the surface of contact between the two cells
 */
void Cell::AddChemCom(const Cell* other) {
  // Compute surface of interaction, if distance_ < ext_rad1 + ext_rad2
  // No communication if cells are not touching
    

  double mean_radius = (other->external_radius() + external_radius()) / 2.;
  double h = mean_radius - Distance(other) / 2.;

  if (h > 0) { // if cells are touching one another

    double contact_area = Simulation::usecontactarea() ? M_PI * (mean_radius * h - h*h / 4) : 1.0;

    for ( unsigned long i = 0; i < nbr_signals; ++i) {
      intrinsic_inputs_.Add(static_cast<InterCellSignal>(i),
                            contact_area * other->local_signal(static_cast<InterCellSignal>(i)));
    }

    if (other->cell_type() == NICHE) {
//std::cout<<"isNICHE"<<std::endl;
      // This signal is either on or off
      intrinsic_inputs_.Add(InterCellSignal::NICHE, 1.);
    }
  }
}

void Cell::ResetInSignals() {
  intrinsic_inputs_.Reset();
}

/*
 * getInSignal: deprecated, use get_local_signal
 */
double Cell::getInSignal(InterCellSignal inSignal) const {
  return intrinsic_inputs_.getInSignal(inSignal);
}

/*
 * get_local_signal: 
 * input: InterCellSignal Signal
 * output: sum of signal Signal from neighbouring cells
 * to this cell.
 */
double Cell::get_local_signal(InterCellSignal inSignal) const {
  return intrinsic_inputs_.getInSignal(inSignal);
}


void Cell::Move(Coordinates<double> dpos) {
  Coordinates<double> old_pos = pos_;

  // If the movement in any of the (x, y or z) components would cause the
  // sphere defined by the internal radius of the cell to extend beyond
  // the world boundaries, cancel the movement in this component
  WorldSize::ValidateMovement(dpos, pos_, internal_radius());

  // Apply the movement
  pos_ += dpos;

  // Inform observers
  NotifyObservers(CELL_MOVED, &old_pos);
}

// FGT 
void Cell::AddGaussianField(InterCellSignal signal, std::vector<real_type>& value) {

  intrinsic_inputs_.AddGaussianField(signal, value);

}


double Cell::gaussian_field_weight(InterCellSignal signal) {

  (void)signal;
  return 0.0;

}

const Coordinates<double>& Cell::gaussian_field_source(InterCellSignal signal) {

  (void)signal;
  return pos();
}

std::vector<Coordinates<double>> Cell::gaussian_field_targets(InterCellSignal signal) {

  (void)signal;
  std::vector<Coordinates<double>> targets;
  targets.push_back(pos());
  return targets;
}


bool Cell::RegisterClass(CellFormalism formalism,
                         const char param_file_keyword[],
                         CellFactory factory,
                         CellLoader loader) {
  SimulationParams::CellFormalisms()[param_file_keyword] = formalism;
  Cell::factories()[formalism] = factory;
  Cell::loaders()[formalism] = loader;
  return true;
}


std::vector<real_type> Cell::get_diffusive_signal(InterCellSignal inSignal) const {
  return intrinsic_inputs_.gaussian_field(inSignal);
}

// ============================================================================
//                                  Accessors
// ============================================================================
