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
     Type, S_S, D_D 
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
  double get_GeneParams(void);
  double get_InitPos(void);
  bool isDividing() const override;
  

// Fast Gaussian Transform -----------------------------------------
  double gaussian_field_weight(InterCellSignal signal) override;
  const Coordinates<double>& gaussian_field_source(InterCellSignal signal) override; 
  std::vector<Coordinates<double>> gaussian_field_targets(InterCellSignal signal) override;
  // END Fast Gaussian Transform -------------------------------------


 protected :
  // ==========================================================================
  //                            Protected Methods
  // ==========================================================================
  Coordinates<double> MotileDisplacement(const double& dt) override;
  void Cell_update(const double& dt);  
  void UpdateType(double Mother_type); 
  void Get_Sigma( double * sigma, double * P, bool AcceptNegative, double S_S, const double x, double D_D, const double y);
  void Intracellular_ExactEvol(double DeltaT, double * P, double * M, double * S1);
  void ODE_update(const double& dt);
  void UpdatePhylogeny(std::vector<int> phy_id, std::vector<double> phy_t, int sz);
  void SetNewRNA_stem_symmetric(const double MotherRNA[],const  int sz)const ;
  void SetNewProtein_stem_symmetric(const double MotherProteins[],const  int sz)const ;
  double count_Neib_Stem();
  double count_Neib_Syp();
  void Update_count_division(double Mother_division, const int s )const;
  void Update_count_division_mother(double Mother_division);
     
    
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

  static constexpr uint32_t odesystemsize_ = 3;
  double* internal_state_;
  double* initialP_;
 
  double S2_; ///signal stem diffusive 
  double* mRNA_array_;
  double* Protein_array_;
  int PhantomJumpCounts_= 0;
  int* TrueJumpCounts_array_;

  double* GenesInteractionsMatrix_;
  double* KinParam_;
  double* Remember_division_ ;
  int Number_Of_Genes_;
  int Number_Of_Parameters_ = 15;
  double Time_NextJump_ = 0.;
  int ithGene_;
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
