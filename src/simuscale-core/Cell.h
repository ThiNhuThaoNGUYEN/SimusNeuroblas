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

#ifndef SIMUSCALE_CELL_H__
#define SIMUSCALE_CELL_H__


// =================================================================
//                              Includes
// =================================================================
#include <cinttypes>
#include <cstdio>
#include <cstdlib>
#include <cfloat>

#include <list>

#include <zlib.h>

#include "CellType.h"
#include "Coordinates.h"
#include "Observable.h"
#include "Alea.h"
#include "InterCellSignal.h"
#include "InterCellSignals.h"
#include "CellSize.h"
#include "movement/MoveBehaviour.h"

// ============================================================================
//                         Class declarations, Using etc
// ============================================================================
class Mobile;

using real_type = InterCellSignals::real_type_;

/*!
  \brief An abstraction for any kind of cells.
*/
class Cell : public Observable
{
  friend class Mobile;
  friend class Motile;

 public :


  using CellFactory = Cell* (*)(CellType type,
                                const MoveBehaviour& move_behaviour,
                                const Coordinates<double>& pos,
                                double initial_volume,
                                double volume_min,
                                double doubling_time);
  using CellFactories = std::map<CellFormalism, CellFactory>;
  using CellLoader = Cell* (*)(gzFile backup_file);
  using CellLoaders = std::map<CellFormalism, CellLoader>;
  static bool RegisterClass(CellFormalism formalism,
                            const char param_file_keyword[],
                            CellFactory factory,
                            CellLoader loader);

  // =================================================================
  //                             Constructors
  // =================================================================
  Cell() = default;
  Cell(const Cell &model); //< Copy ctor
  Cell(CellType cell_type, 
       const MoveBehaviour& move_behaviour,
       Coordinates<double> pos,
       CellSize&& size,
       double doubling_time);
  Cell(gzFile backup_file);


  // =================================================================
  //                             Destructor
  // =================================================================
  virtual ~Cell() = default;


  // =================================================================
  //                            Public Methods
  // =================================================================
  /** \internal Update the cell. \endinternal */
  void Update(const double& dt);

  /** Distance between this cell and other cell.
   * @param dt the timestep
   * @return the distance 
   */
  double Distance(const Cell* other) const;
  /** \internal add a new neighbour to this cell. \endinternal */
  void AddNeighbour(Cell * neighbour);
  void ResetNeighbours();

  /**
   * Make the cell divide.
   *
   * The cell will continue to exist as one of the daughter cells
   * @return the second daughter cell
   */
  virtual Cell* Divide() = 0;

  void ComputeInteractions();
  void ComputeGaussianFields();
  void AddGaussianField(InterCellSignal signal, std::vector<real_type>& value);
  void ResetInteractions();
  void Move(const double& dt);

  void UpdateMinMaxSignal() const;
    
  /** Save the cell into file backup_file. 
   *
   * @param backup_file a write-opened gz file
   */
  virtual void Save(gzFile backup_file) const;

  /** Load a cell's generic data from file backup_file.
   *
   * @param backup_file a read-opened gz file
   * @see LoadCell(gzFile backup_file)
   */
  virtual void Load(gzFile backup_file);

  /** Load a cell of any known formalism from a backup (Polymorphic factory).
   *
   * @return a heap allocated Cell instance of the same derived type as the
   * original cell reading from a gzFile
   *
   * \throws std::out_of_range if the derived type is unknown
   */
  static Cell* MakeCell(CellFormalism formalism,  
                        const MoveBehaviour& move_behaviour,
                        CellType type,
                        Coordinates<double> pos,
                        double initial_volume,
                        double volume_min,
                        double doubling_time);

  static Cell* LoadCell(gzFile backup_file);
  static void SaveStatic(gzFile backup_file);
  static void LoadStatic(gzFile backup_file);

  /** Dump Cell content in JSON format. 
   */
  virtual void Dump();

  // =================================================================
  //                              Accessors
  // =================================================================
  /** unique ID of the cell. */  
  int32_t id() const {return id_;};                         
  /** 3D coordinates of cell center. */
  const Coordinates<double>& pos() const {return pos_;};    
  /** x-coordinate of cell center. */
  double pos_x() const {return pos_.x;};                    
  /** y-coordinate of cell center. */
  double pos_y() const {return pos_.y;};                    
  /** z-coordinate of cell center. */
  double pos_z() const {return pos_.z;};                    
  /** list of cell neighbours. */
  const std::vector<Cell*>& neighbours() const {return neighbours_;}; 

  /** 3D orientation normal vector. */
  const Coordinates<double>& orientation() const { return orientation_; }; 
  /** x-component of orientation(). */
  double orientation_x() const { return orientation_.x; };  
  /** y-component of orientation(). */
  double orientation_y() const { return orientation_.y; };  
  /** z-component of orientation(). */
  double orientation_z() const { return orientation_.z; };  
  
  /** Cell type. */
  const CellType& cell_type() const {return cell_type_;};
  /** Total cell volume. */
  double outer_volume() const {return size_.volume();};
  /** Internal radius. */
  double internal_radius() const {return size_.internal_radius();};
  /** External radius. */
  double external_radius() const {return size_.external_radius();};

  /** \deprecated
   * @return false */
  virtual bool isCycling() const { return false; }; // isCycling: deprecated, make false by default
  /** Is the cell to divide? */
  virtual bool isDividing() const = 0;
  /** Is the cell state dying?
   * Dying cells are marked D in the trajectory file. */
  virtual bool isDying() const { return is_dying_; };
  /** Is the cell dead? 
   * Dead cells are removed a the end of the time step 
   * and are not included in the trajectory file. */
  virtual bool isDead() const;

  /** \deprecated
   * Use local_signal instead. */
  virtual double get_output(InterCellSignal signal) const; // deprecated, use local_signal

  /** Value of signal emitted locally.
   * @param signal the name of the signal
   * @return expression level of signal 
   * @see get_local_signal
   */
  virtual double local_signal(InterCellSignal signal) const;

  static void set_volume_max_min_ratio(double volume_max_min_ratio) {
    CellSize::set_volume_max_min_ratio(volume_max_min_ratio);
  }
  static void set_radii_ratio(double radii_ratio) {
    CellSize::set_radii_ratio(radii_ratio);
  }

  static void set_max_force(double max_force) {
    max_force_ = max_force;
  }

  static void set_LJ_epsilon(double LJ_epsilon) {
    LJ_epsilon_ = LJ_epsilon;
  }

  /** Get content of hold_space_.
   * The hold space is a double-valued variable
   * that can be used to stored values in other cells.
   */
  double get_hold_space() const {return hold_space_;} /* get content of hold space */

  /** Set hold_space_ to stuff.
   * @param stuff value to set
   */
  void set_hold_space(double stuff) { hold_space_ = stuff; }
  
  /** Add stuff to hold_space_.
   * @param stuff value to add
   */
  void pushto_hold_space(double stuff) { hold_space_ += stuff; }

  // FGT Fast Gaussian Transform -----------------------------------------
  /** The strength of diffusive signal at the source location. 
   * @param signal the signal to express 
   * @return the strength of the diffusive signal */
  virtual double                            gaussian_field_weight(InterCellSignal signal);

  /** The source location of diffusive signal.
   * @param signal the signal to express 
   * @return the coordinates of the source of diffusive signal */
  virtual const Coordinates<double>&        gaussian_field_source(InterCellSignal signal); 

  /** The target locations of diffusive signal. 
   * @param signal the signal to express 
   * @return the vector of coordinates of the targets for the diffusive signal */
  virtual std::vector<Coordinates<double>>  gaussian_field_targets(InterCellSignal signal);
  // END Fast Gaussian Transform -------------------------------------

  /** The name of the cell formalism. */
  virtual const char* cell_formalism() const { return NULL;};

 protected :
  // =================================================================
  //                           Protected Methods
  // =================================================================
  /** Plugin-specific method to update the 
   * internal state of this cell.
   */
  virtual void InternalUpdate(const double& dt) = 0;

  /** \internal Separate physically newly divided cells.
   * \endinternal
   */
  static void SeparateDividingCells(Cell* c1, Cell* c2);

  /** Increase the volume off cell with a
   * mean double time as set in parameter file.
   */
  virtual void Grow(const double& dt); // Cell::Grow has a default behaviour


  void Move(Coordinates<double> dpos);

  /** \deprecated use motility instead. */
  virtual Coordinates<double> MotileDisplacement(const double& dt); // deprecated -> motility
                                                                    
  /** Set cell displacement due to motility behaviour. 
   * This displacement is added to the displacement induced
   * by mechanical forces.
   * @return the displacement vector */
  virtual Coordinates<double> motility(const double& dt);           // replaces MotileDisplacement

  /** \deprecated no replacement. */
  virtual void UpdateCyclingStatus() { /* do nothing */ };          // deprecated, no replacement
  /** \deprecated no replacement. */
  virtual bool StopCycling() { return false; };                     // deprecated, no replacement
  /** \deprecated no replacement. */
  virtual bool StartCycling() { return false; };                    // deprecated, no replacement 

  /**
   * Add the visco-elastic force applied to this cell by other cell.
   */
  virtual void AddMechForce(const Cell* other);

  /** \internal
   * Compute the biochemical action of other cell onto this
   * The signal is proportional to the surface of contact between the two cells
   * \endinternal
   */
  void AddChemCom(const Cell* other);
  void ResetMechForce();
  void ResetInSignals();

  /** Set the status of this cell to dying.
   * In principle, this status should not be unset. */
  inline void set_to_dying() { is_dying_ = true; };
  /** Set the status of this cell to death. 
   * This status cannot be unset. */
  inline void set_to_dead()  { is_dead_  = true; }; 

  /** \deprecated use get_local_signal instead. */
  double getInSignal(InterCellSignal inSignal) const;               // deprecated, -> get_local_signal

  /** Get the total expression of inSignal from neighbouring cells.
   * @param inSignal the name of the signal
   * @return the sum of expressions of inSignal by neighbouring cells
   * @see local_signal
   */
  double get_local_signal(InterCellSignal inSignal) const; 

  /** Get the total expression of diffisive signal inSignal from all cells.
   * @param inSignal the name of the signal
   * @return the sum of diffuse vales of inSignal by all cells at targets
   * @see local_signal
   */
  std::vector<real_type> get_diffusive_signal(InterCellSignal inSignal) const;


 private:
  // =================================================================
  //                           Private Methods
  // =================================================================

 protected:
  // =================================================================
  //                              Attributes
  // =================================================================
  /** Maximum ID that has been attributed so far */
  static uint32_t maxID_;

  /** Cell ID */
  int32_t id_;

  /** Coordinates in space */
  Coordinates<double> pos_;

  /** Orientation in space: normalized Coordinates */
  Coordinates<double> orientation_;

  /** Size */
  CellSize size_;

  /** Cell Type */
  CellType cell_type_;

  /** Time needed for the volume of the cell to be doubled */
  double doubling_time_;

  /**
   * Factor by which the volume is multiplied at each time step during growth.
   *
   * It is a simple transformation of the (exponential) growth rate when both
   * the time_step and doubling§ time are known.
   * It represents the common ratio of the geometric progression of the volume
   *
   * This attribute exists only for the sake of performance since it can be
   * computed once and for all.
   */
  double growth_factor_;

  /** Defines how the cell moves (strategy pattern).
   */
  const MoveBehaviour* move_behaviour_ = nullptr;

  /** Combination of all the mechanical forces exerted on the cell.
   */
  Coordinates<double> mechanical_force_;

  /** Max mechanical force.
   */
  static double max_force_;

  /** Lennard Jones scale constant.
   */
  static double LJ_epsilon_;

  /** Whether the cell is cycling or not.
   */
  bool isCycling_ = false;  // deprecated, do not use anymore

  /**
   * Whether the cell is alive or not.
   *
   * set is_dying_ to flag the cell as dying. This will
   * mark the cell as 'D' in the output trajectory file
   * but keep the cell in the cell list.
   *
   * getter: isDying() 
   * setter: set_to_dying() 
   */
  bool is_dying_ = false;
  
  /** 
   * set is_dead_ to flag the cell as dead. Cell will be removed
   * from the cell population by the core simulator, 
   * and will not be listed in the trajectory file.
   *
   * getters: isDead()
   * setters: set_to_dead().
   */
  bool is_dead_ = false;

  /**
   * Cells that are in physical contact with the object
   */
  std::vector<Cell*> neighbours_;

  /**
   * Chemical inputs perceived by the cell
   *
   * This is basically the sum of the outputs emitted by neighbouring cells
   * @see get_local_signal 
   */
  InterCellSignals intrinsic_inputs_;

  /**
   * hold_space_ Placeholder for mass/material transfer
   * to the cell from external sources.
   * @see set_hold_space, get_hold_space, pushto_hold_space
   */
  double hold_space_ = 0.0;

  /**
   * Container for derived class factories.
   *
   * This is a method for technical reasons only, it returns a reference to
   * the actual "attribute" that can be used normaly, including as an lvalue
   */
  static CellFactories& factories();

  /**
   * Container for derived class loader factories
   *
   * This is a method for technical reasons only, it returns a reference to
   * the actual "attribute" that can be used normaly, including as an lvalue
   */
  static CellLoaders& loaders();
};

#endif // SIMUSCALE_CELL_H__
