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

// <TODO> Modify the include guard </TODO>
#ifndef SIMUSCALE_LYMPHOCYTE_H__
#define SIMUSCALE_LYMPHOCYTE_H__


// ============================================================================
//                                   Includes
// ============================================================================
#include <memory>


#include <inttypes.h>
#include <stdio.h>
#include <stdlib.h>
#include <vector>

#include "Cell.h"
#include "Alea.h"
#include "CellType.h"

/**
 * This is a cell formalism plugin boilerplate
 */
// <TODO> Modify the class name </TODO>

class Lymphocyte : public Cell {
public:

    typedef enum {
        CY_X, CY_Z,
        CD_DEATH, APC_Encounters, DifferentiationState
    } StateVariable;

    // =================================================================
    //                         Factory functions
    // =================================================================

    template <typename... Args>
    static std::shared_ptr<Cell> MakeCell(Args&&... args) {
        return std::shared_ptr<Lymphocyte>(new Lymphocyte(std::forward<Args>(args)...));
    }

    // ==========================================================================
    //                               Constructors
    // ==========================================================================
    Lymphocyte() = delete; //< no default Default ctor
    Lymphocyte(const Lymphocyte &model); //< Copy ctor
    Lymphocyte(Lymphocyte &&) = delete; //< Move ctor
    Lymphocyte(CellType cellType,
            const MoveBehaviour& move_behaviour,
            const Coordinates<double>& pos,
            double initial_volume,
            double volume_min,
            double doubling_time);
    Lymphocyte(gzFile& backup);
    
    

    // ==========================================================================
    //                                Destructor
    // ==========================================================================
    virtual ~Lymphocyte() noexcept;

    // ==========================================================================
    //                                Operators
    // ==========================================================================
    /// Copy assignment
    Lymphocyte &operator=(const Lymphocyte &other) = delete;

    /// Move assignment
    Lymphocyte &operator=(Lymphocyte &&other) = delete;

    // ==========================================================================
    //                              Public Methods
    // ==========================================================================
    void InternalUpdate(const double& dt) override;
    Cell* Divide() override;

    void Save(gzFile backup) const override;
    void Load(gzFile& backup);

    int get_GeneParams(void);
    int get_CyclingParams(void);
    double get_P(int i) const ;
    double get_mRNA(int i) const ;
    double getPhylogeny_t(int i) const;
    int getPhylogeny_id(int i) const;
    // ==========================================================================
    //                                Accessors
    // ==========================================================================
    double get_output(InterCellSignal signal) const override;

    bool isCycling() const override;
    bool isDividing() const override;
    bool isDying() const override;

    Coordinates<double> MotileDisplacement(const double& dt) override;

protected:
    // ==========================================================================
    //                            Protected Methods
    // ==========================================================================
    int GetDifferentiationState();
    double GetSigma_i(int i, double * P, bool AcceptNegative);
    
    void Intracellular_ExactEvol(double DeltaT, double * P, double * M, const double S1);
    double random_mitotic_threshold(void){return Alea::exponential_random(mitotic_threshold_P0_);};

    void ODE_update(const double& dt);
    static int compute_dydt(double t, const double y[], double dydt[],
            void* cell);
    int compute_dydt(double t, const double y[], double dydt[]) const;

    void UpdateMitoticStatus();
    void UpdateCyclingStatus() override;
    bool StopCycling() override;
    bool StartCycling() override;

    void SetNewProteinLevels(double Ki[], double MotherProteins[], int sz);

    void ResetProteinLevels(double Ki[], double MotherProteins[], int sz);
    void UpdatePhylogeny(std::vector<int> phy_id, std::vector<double> phy_t, int sz);
    // ==========================================================================
    //                               Attributes
    // ==========================================================================
public:
    // <TODO> Modify the class id and keyword </TODO>
    // Class ID, uniquely identifies this class
    static constexpr CellFormalism classId_ = 0xdaa16ede; // CRC32 Hash
    // Keyword to be used in param files
    static constexpr char classKW_[] = "LYMPHOCYTE";

protected:
    // <TODO> Add your formalism-specific attributes here </TODO>
    static constexpr uint32_t odesystemsize_ = 5;

    // The internal state of the cell
    // Because we use the GSL, the internal state of the system must be an array
    // of double values.
  double* internal_state_;
  double* mRNA_array_;
  double* Protein_array_;
  int PhantomJumpCounts_;
  int* TrueJumpCounts_array_;
  double* ThinningParam_array_;

  double* GenesInteractionsMatrix_;
  double* KinParam_;
  
  int Number_Of_Genes_ = 0;
  int Number_Of_Parameters_ = 5;

  int ithGene_ = -1;
  double Time_NextJump_ = 0;

  std::vector<double>  phylogeny_t_;
  std::vector<int>  phylogeny_id_;

  const char phylogeny_t_filename[16] = "phylogeny_t.txt";
  const char phylogeny_id_filename[32] = "phylogeny_id.txt";
  FILE *phylogeny_t_file_, *phylogeny_id_file_;
  
    bool isMitotic_ = false; // distinguishes cells about to divide

    double mitotic_threshold_ = 0.0;
    double cycling_threshold_ = 0.0;
    double dividing_threshold_ = 0;
    double death_threshold_ = 0;
    
    double mitotic_threshold_P0_=0;

    // Death/Survival parameters
    double k_survival = 10.0; // higher values = higher survival.

    // Cell cycle parameters
    // The values are taken from (Battogtokh & Tyson 2006 PRE)
    static constexpr double cy_j = 0.05;
    static constexpr double cy_p = 0.15;
    static constexpr double cy_k1 = 0.002;
    static constexpr double cy_k2 = 0.0795;
    static constexpr double cy_k3 = 0.01;
    static constexpr double cy_k4 = 2.0;
    static constexpr double cy_k5 = 0.05;
    static constexpr double cy_k6 = 0.04;
    static constexpr double cy_k7 = 1.5;
    static constexpr double cy_k8 = 0.19;
    static constexpr double cy_k9 = 0.64;
    static constexpr double cy_k10 = 0.0025;
    static constexpr double cy_k11 = 0.07;
    static constexpr double cy_k12 = 0.08;
    static constexpr double cy_A = 0.52;

private:
    /** dummy attribute - allows to register class in Simuscale statically */
    static bool registered_;
};

#endif // SIMUSCALE_Lymphocyte_H__
