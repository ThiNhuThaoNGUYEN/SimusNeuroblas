// ****************************************************************************
//
//              SiMuScale - Multi-scale simulation framework
//
// ****************************************************************************

// <TODO> Modify the include guard </TODO>
#ifndef SIMUSCALE_CANCER_H__
#define SIMUSCALE_CANCER_H__


// ============================================================================
//                                   Includes
// ============================================================================
#include "Cell.h"

/**
 * This is a cell formalism plugin boilerplate
 */
// <TODO> Modify the class name </TODO>
class Cancer : public Cell {

 public :
 typedef enum {
      Type 
    } StateVariable;


  // ==========================================================================
  //                               Constructors
  // ==========================================================================
  Cancer() = default; //< Default ctor
  Cancer(const Cancer &); //< Copy ctor
  Cancer(Cancer &&) = delete; //< Move ctor
  Cancer(CellType cellType,
                   const MoveBehaviour& move_behaviour,
                   const Coordinates<double>& pos,
                   double initial_volume,
                   double volume_min,
                   double doubling_time);
  Cancer(gzFile backup_file);

  // ==========================================================================
  //                                Destructor
  // ==========================================================================
  virtual ~Cancer();

  // ==========================================================================
  //                                Operators
  // ==========================================================================
  /// Copy assignment
  Cancer &operator=(const Cancer &other) = delete;

  /// Move assignment
  Cancer &operator=(Cancer &&other) = delete;

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
  void Cell_update(const double& dt);  
  void UpdateType(double Mother_type); 

  void UpdatePhylogeny(std::vector<int> phy_id, std::vector<double> phy_t, int sz);

  // ==========================================================================
  //                               Attributes
  // ==========================================================================
 public:
  // <TODO> Modify the class id and keyword </TODO>
  // Class ID, uniquely identifies this class
  static constexpr CellFormalism classId_ = 0xff277512; // CRC32 Hash
  // Keyword to be used in param files
  static constexpr char classKW_[] = "CANCER";

 protected:
  // <TODO> Add your formalism-specific attributes here </TODO>

  static constexpr uint32_t odesystemsize_ = 1;
  double* internal_state_;
  double toxic_signal_ = 10.0;
  double sigma_ = 1.5;
  int32_t nn_ = 0;

  bool isMitotic_ = false; // distinguishes cells about to divide

  std::vector<double>  phylogeny_t_;
  std::vector<int>  phylogeny_id_;
  const char phylogeny_T_filename[16] = "phylogeny_T.txt";
  const char phylogeny_ID_filename[32] = "phylogeny_ID.txt";
  FILE *phylogeny_T_file_, *phylogeny_ID_file_;
  
 private:
  /** dummy attribute - allows to register class in Simuscale statically */
  static bool registered_;
};

#endif // SIMUSCALE_CELL_KILLER_H__
