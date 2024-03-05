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
#include "Lymphocyte.h"

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

// ============================================================================
//                       Definition of static attributes
// ============================================================================
constexpr CellFormalism Lymphocyte::classId_;
constexpr char Lymphocyte::classKW_[];

constexpr uint32_t Lymphocyte::odesystemsize_;

// ============================================================================
//                                Constructors
// ============================================================================

Lymphocyte::Lymphocyte(const Lymphocyte &model) :
Cell(model),
isMitotic_(model.isMitotic_) {
    phylogeny_id_.push_back(this->id());
    phylogeny_t_.push_back(Simulation::sim_time());
    internal_state_ = new double[odesystemsize_];
    memcpy(internal_state_, model.internal_state_, odesystemsize_ * sizeof (*internal_state_));

    this->Number_Of_Genes_ = model.Number_Of_Genes_;

    mRNA_array_ = new double[Number_Of_Genes_];
    Protein_array_ = new double[Number_Of_Genes_];
    TrueJumpCounts_array_ = new int[Number_Of_Genes_];
    ThinningParam_array_ = new double[Number_Of_Genes_];

    memcpy(mRNA_array_, model.mRNA_array_, Number_Of_Genes_ * sizeof (*mRNA_array_));
    memcpy(Protein_array_, model.Protein_array_, Number_Of_Genes_ * sizeof (*Protein_array_));
    memcpy(TrueJumpCounts_array_, model.TrueJumpCounts_array_, Number_Of_Genes_ * sizeof (*TrueJumpCounts_array_));
    memcpy(ThinningParam_array_, model.ThinningParam_array_, Number_Of_Genes_ * sizeof (*ThinningParam_array_));

    KinParam_ = new double[Number_Of_Parameters_];
    GenesInteractionsMatrix_ = new double[Number_Of_Genes_ * Number_Of_Genes_];

    memcpy(KinParam_, model.KinParam_, Number_Of_Parameters_ * sizeof (*KinParam_));
    memcpy(GenesInteractionsMatrix_, model.GenesInteractionsMatrix_, Number_Of_Genes_ * Number_Of_Genes_ * sizeof (*GenesInteractionsMatrix_));

    get_CyclingParams();
}

/**
 * Create a new object with the provided constitutive elements and values
 */
Lymphocyte::Lymphocyte(CellType cellType,
        const MoveBehaviour& move_behaviour,
        const Coordinates<double>& pos,
        double initial_volume,
        double volume_min,
        double doubling_time) :
Cell(cellType, std::move(move_behaviour),
pos, CellSize(initial_volume, volume_min),
doubling_time) {
    internal_state_ = new double[odesystemsize_];
    phylogeny_id_.push_back(this->id());
    phylogeny_t_.push_back(Simulation::sim_time());

    get_CyclingParams();
    get_GeneParams();

    mRNA_array_ = new double[Number_Of_Genes_];
    Protein_array_ = new double[Number_Of_Genes_];
    TrueJumpCounts_array_ = new int[Number_Of_Genes_];
    ThinningParam_array_ = new double[Number_Of_Genes_];

    for (u_int32_t j = 0; j < odesystemsize_; j++)
        internal_state_[j] = 0.05 * Alea::random();

    double thenorm = 0.05;

    if (cellType == NICHE) {
        thenorm = 0;
    }

    for (decltype(Number_Of_Genes_) j = 0; j < Number_Of_Genes_; j++) {
        mRNA_array_[j] = 0 * Alea::random();
        Protein_array_[j] = 0 * Alea::random();
        TrueJumpCounts_array_[j] = 0;
        ThinningParam_array_[j] = thenorm * Alea::random();
    }

    PhantomJumpCounts_ = 0;

    internal_state_[APC_Encounters] = 0.0;
    if (cell_type_ != CellType::NICHE) {
        if (cell_type_==LYMPHOCYTE)
            internal_state_[DifferentiationState] = 0;
        else
            internal_state_[DifferentiationState] = -10;
    }
    else{
        internal_state_[DifferentiationState] = -20;
    }
    internal_state_[CY_X] = mitotic_threshold_*Alea::random();
    internal_state_[CD_DEATH] = death_threshold_*Alea::random();
}

/**
 * Restore an object that was backed-up in backup_file
 */
Lymphocyte::Lymphocyte(gzFile& backup) :
Cell(backup) {

    internal_state_ = new double[odesystemsize_];
    Load(backup);
}

// ============================================================================
//                                 Destructor
// ============================================================================

Lymphocyte::~Lymphocyte() noexcept {
    delete [] internal_state_;

    delete [] mRNA_array_;
    delete [] Protein_array_;
    delete [] TrueJumpCounts_array_;
    delete [] ThinningParam_array_;

    delete [] GenesInteractionsMatrix_;
    delete [] KinParam_;
}

// ============================================================================
//                                   Methods
// ============================================================================

void Lymphocyte::InternalUpdate(const double& dt) {
    // Describe what happens inside the cell when taking a time step
    //std::cout << "Internal update" << std::endl;
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
}

void Lymphocyte::UpdatePhylogeny(vector<int> phy_id, vector<double> phy_t, int) {
    phylogeny_id_.insert(phylogeny_id_.begin(), phy_id.begin(), phy_id.end());
    phylogeny_t_.insert(phylogeny_t_.begin(), phy_t.begin(), phy_t.end());
}
void Lymphocyte::SetNewProteinLevels(double Ki[], double MotherProteins[], int sz) {
    for (decltype(sz) j = 0; j < sz; j++) {
        Protein_array_[j] = Ki[j] * MotherProteins[j];
    }
}
void Lymphocyte::ResetProteinLevels(double Ki[], double MotherProteins[], int sz) {
    for (decltype(sz) j = 0; j < sz; j++) {
        Protein_array_[j] = (2 - Ki[j]) * MotherProteins[j];
    }
}

Cell* Lymphocyte::Divide() {
    // Reset mitotic status
    isMitotic_ = false;

    // Create a copy of the cell with no mechanical forces nor chemical stimuli
    // applied to it
    Lymphocyte* newCell = new Lymphocyte(*this);

    SeparateDividingCells(this, newCell);
    std::cout << "Cell " << this->id() << "is DIVIDING!!" << std::endl;

    int m = 10;
    double Ki_arr[Number_Of_Genes_];

    for (decltype(Number_Of_Genes_) j = 0; j < Number_Of_Genes_; j++) {
        Ki_arr[j] = 1 - (m / 100) * Alea::random();
    }

    newCell->SetNewProteinLevels(Ki_arr, Protein_array_, Number_Of_Genes_);
    newCell->UpdatePhylogeny(phylogeny_id_, phylogeny_t_, phylogeny_id_.size());

    this->ResetProteinLevels(Ki_arr, Protein_array_, Number_Of_Genes_);
    phylogeny_id_.push_back(this->id());
    phylogeny_t_.push_back(Simulation::sim_time());

    if ( ( phylogeny_id_file_ = fopen(phylogeny_id_filename,"a") ) != NULL ) {
      fprintf(phylogeny_id_file_, "%d ", this->id());
      fclose(phylogeny_id_file_);
    }
    if ( ( phylogeny_t_file_ = fopen(phylogeny_t_filename,"a") ) != NULL ) {
      fprintf(phylogeny_t_file_, "%f ", Simulation::sim_time());
      fclose(phylogeny_t_file_);
    }


    /*for (int i = 0; i < phylogeny_id_.size(); i++) {
        std::cout << phylogeny_id_[i];
    }
    std::cout << " " << std::endl;*/

    return newCell;
}


Coordinates<double> Lymphocyte::MotileDisplacement(const double& dt) {
  
  std::vector<Cell*> neighb = neighbours();
  auto cell_it = neighb.begin();
  bool contact = false; 
  while ( cell_it != neighb.end() ) {
    if ( (*cell_it)->cell_type() == APC ) {
      contact = true;
      break;
    }
    cell_it++;
  }
  double sigma_t;
  if ( contact ) {
    sigma_t = 0.1;
  }
  else
  {
    sigma_t = 0.5;
  }
  Coordinates<double> displ { sqrt(dt)*sigma_t*Alea::gaussian_random(),
                              sqrt(dt)*sigma_t*Alea::gaussian_random(),
                              sqrt(dt)*sigma_t*Alea::gaussian_random() };
  return displ;
}

void Lymphocyte::Save(gzFile backup) const {
    // Write my classId
    gzwrite(backup, &classId_, sizeof (classId_));

    // Write the generic cell stuff
    Cell::Save(backup);

    // Write the specifics of this class
    //  int8_t isMitotic = isMitotic_ ? 1 : 0;
    //  gzwrite(backup_file, &isMitotic, sizeof(isMitotic));
        // <TODO> write your specific attributes </TODO>

    for (u_int32_t i = 0; i < odesystemsize_; i++){
        gzwrite(backup, &internal_state_[i], sizeof (internal_state_[i]));
        
    }

}

void Lymphocyte::Load(gzFile& backup) {
    // <TODO> Load your specific attributes </TODO>
    
    for (u_int32_t i = 0; i < odesystemsize_; i++)
        gzread(backup, &internal_state_[i], sizeof (internal_state_[i]));
}

int Lymphocyte::get_GeneParams(void) {
    KinParam_ = new double[Number_Of_Parameters_];

    ////////////////////////////////////
    ifstream indatakin; // indata is like cin
    indatakin.open("kineticsparam.txt"); // opens the file
    if (!indatakin) { // file couldn't be opened
        std::cerr << "Error: file kineticsparam.txt could not be opened" << std::endl;
        exit(1);
    }
    int iter = 0;
    std::string line;
    std::getline(indatakin, line);
    std::getline(indatakin, line);
    std::istringstream iss(line);
    double a;

    while (iter < Number_Of_Parameters_) {
        if (!(iss >> a)) {
            std::cerr << "Error: file kineticsparam.txt not readable - possible value missing" << std::endl;
            break;
        }// error
        else {
            KinParam_[iter] = a;
            iter = iter + 1;
        }
    }

    indatakin.close();
    ////////////////////////////////////
    ifstream indataf; // indata is like cin
    indataf.open("GeneInteractionsMatrix.txt"); // opens the file
    if (!indatakin) { // file couldn't be opened
        std::cerr << "Error: file GeneInteractionsMatrix.txt could not be opened" << std::endl;
        exit(1);
    }
    Number_Of_Genes_ = 0;
    std::getline(indataf, line);
    std::stringstream ss(line);
    std::string word;
    int i = 0;
    int j = 0;
    Number_Of_Genes_ = -1; // to include title column

    while (ss >> word) {
        Number_Of_Genes_ += 1;
    }

    GenesInteractionsMatrix_ = new double[Number_Of_Genes_ * Number_Of_Genes_];

    while (std::getline(indataf, line)) {
        std::istringstream iss(line);
        iss >> word;
        j = 0;
        while (j < Number_Of_Genes_) {
            if ((!(iss >> a))) {
                std::cerr << "Error: file not readable - possible value missing" << std::endl;
                break;
            }// error
            else {
                GenesInteractionsMatrix_[static_cast<int> (j + Number_Of_Genes_ * i)] = a;
                j = j + 1;
            }
        }
        i = i + 1;

        // process pair (a,b)
    }
    indataf.close();

    return 0;
}

int Lymphocyte::get_CyclingParams(void) {
    vector<double> cyclingparam;

    ////////////////////////////////////
    ifstream indatakin; // indata is like cin
    indatakin.open("cyclingparam.txt"); // opens the file
    if (!indatakin) { // file couldn't be opened
        std::cerr << "Error: file cyclingparam.txt could not be opened" << std::endl;
        exit(1);
    }
    int iter = 0;
    std::string line;
    std::getline(indatakin, line);
    std::getline(indatakin, line);
    std::istringstream iss(line);
    double a;

    while (iter < 6) {
        if (!(iss >> a)) {
            std::cerr << "Error: file cyclingparam.txt not readable - possible value missing" << std::endl;
            break;
        }// error
        else {
            cyclingparam.push_back(a);
            iter = iter + 1;
        }
    }

    indatakin.close();
    mitotic_threshold_ = cyclingparam[0];
    cycling_threshold_ = cyclingparam[1];
    dividing_threshold_ = cyclingparam[2];
    death_threshold_ = cyclingparam[3];
    mitotic_threshold_P0_= cyclingparam[4];
    k_survival = cyclingparam[5];


    return 0;
}

double Lymphocyte::get_P(int i) const {
    if (i > Number_Of_Genes_ - 1) {
        i = Number_Of_Genes_ - 1;
    }
    return (Protein_array_[i]);
}
double Lymphocyte::getPhylogeny_t(int i) const {
    double ans;
    if (i <0) {
        ans = phylogeny_t_.size();
    }
    else {
        ans = phylogeny_t_[i];
    }
    return ans;
}
int Lymphocyte::getPhylogeny_id(int i) const {
    int ans;
    if (i <0) {
        ans = phylogeny_id_.size();
    }
    else {
        ans = phylogeny_id_[i];
    }
    return ans;
}

double Lymphocyte::get_mRNA(int i) const {
    if (i > Number_Of_Genes_ - 1) {
        i = Number_Of_Genes_ - 1;
    }
    return (mRNA_array_[i]);
}
double Lymphocyte::get_output(InterCellSignal signal) const {
    // <TODO> Determine the amount of <signal> the cell is secreting </TODO>
    switch (signal) {
        case InterCellSignal::LYMPHOCYTE_TYPE:
            return 1.0 * (cell_type_ == CellType::LYMPHOCYTE)
                    + 2.0 * (cell_type_ == CellType::APC);
        case InterCellSignal::CYCLE_X:
            return internal_state_[CY_X];
        case InterCellSignal::CYCLE_Z:
            return internal_state_[CY_Z];
        case InterCellSignal::DEATH:
            return internal_state_[CD_DEATH];
        case InterCellSignal::DIFFERENTIATION_STATE:
            return internal_state_[DifferentiationState];
	    case InterCellSignal::VOLUME:
            return outer_volume();
         case InterCellSignal::LYMPHOCYTE_P1:
            return Protein_array_[0];
        case InterCellSignal::LYMPHOCYTE_P2:
            return  Protein_array_[1];
        case InterCellSignal::LYMPHOCYTE_P3:
            return  Protein_array_[2];
	  case InterCellSignal::LYMPHOCYTE_mRNA1:
            return mRNA_array_[0];
        case InterCellSignal::LYMPHOCYTE_mRNA2:
            return mRNA_array_[1];   
        case InterCellSignal::LYMPHOCYTE_mRNA3:
            return  mRNA_array_[2];
        case InterCellSignal::APC_CONTACT:
            return internal_state_[APC_Encounters]; 
        default:
            return 0.0;  // if signal is unknown, return 0.0, not a failure
            // printf("%s:%d: error: case %d not implemented.\n", __FILE__, __LINE__,
            //         ((int) getInSignal(signal)));
            // exit(EXIT_FAILURE);
    }
}

bool Lymphocyte::isCycling() const {
    // <TODO> Is the cell currently cycling ? </TODO>
    return isCycling_;
}

bool Lymphocyte::isDividing() const {
        return ((cell_type_ != NICHE) &&
            internal_state_[APC_Encounters] > 0 &&
	    Protein_array_[0] > 10. &&
	    isMitotic_ &&
            size_.may_divide());

}

bool Lymphocyte::isDying() const {
    // <TODO> Should the cell die now ? </TODO>
    return (cell_type_ != CellType::NICHE) &&
      //( Protein_array_[2] >= 18.0 );
      ( Protein_array_[2] >= 1000.0 );
      //  (internal_state_[CD_DEATH] >  death_threshold_ );
}

int Lymphocyte::GetDifferentiationState() {
    int state = internal_state_[DifferentiationState] ;

//    double IL2R = Protein_array_[1];
//    double Tbet = 0; //Protein_array_[3];
//    double Eomes = 0; //Protein_array_[4];
//
//    double IL2R_th = 0.001;
//    double Tbet_th = 0.001;
//    double Eomes_th = 0.001;

    bool A = (internal_state_[APC_Encounters] > 0) || (internal_state_[DifferentiationState] > 0);
    bool B = false; //(IL2R > IL2R_th);
    bool C = false; //(Tbet > Tbet_th);
    bool D = false; // (Eomes > Eomes_th);

    if (A && (state < 4)) { // if memory TCD8 (state = 4), no update
        if (B) {
            if (C) {
                if (D) {
                    state = 4;
                } else {
                    state = 3;
                }
            } else {
                if (D) {
                    state = 4;
                } else {
                    state = 2;
                }
            }
        } else {
            if (C) {
                if (D) {
                    state = 3.5;
                } else {
                    state = 3;
                }
            } else {
                if (D) {
                    state = 1;
                } else {
                    state = 1;
                }
            }
        }
    }
    return state;
}

double Lymphocyte::GetSigma_i(int i, double * P, bool AcceptNegative) {
    double initval = GenesInteractionsMatrix_[i + Number_Of_Genes_ * i];
    double interIJ;
    for (decltype(Number_Of_Genes_) j = 0; j < Number_Of_Genes_; j++) {
        interIJ = GenesInteractionsMatrix_[i + Number_Of_Genes_ * j]; // J acts on I
        if (((interIJ > 0)||AcceptNegative)&&(i!=j)) {
            initval += P[j] * interIJ;
        }
    }
    return (1 / (1 + exp(-initval)));
}

void Lymphocyte::Intracellular_ExactEvol(double thetime, double * P, double * M, const double S1) {
    double d0 = KinParam_[0]; //1; // mRNA degradation rates
    double d1 = KinParam_[1]; //0.2; //protein degradation rates
    double Marr[Number_Of_Genes_];
    double Parr[Number_Of_Genes_];

    memcpy(Marr, M, Number_Of_Genes_ * sizeof (*M));
    memcpy(Parr, P, Number_Of_Genes_ * sizeof (*P));

    // New values:
    for (decltype(Number_Of_Genes_) i = 0; i < Number_Of_Genes_; i++) {
        M[i] = Marr[i] * exp(-thetime * d0);
        P[i] = ((S1 / (d0 - d1)) * Marr[i]*(exp(-thetime * d1) - exp(-thetime * d0))
                + Parr[i] * exp(-thetime * d1));
    }
}

void Lymphocyte::UpdateMitoticStatus() {
    // Once a cell enters the mitotic state, it remains mitotic until it
    // dies or divides
    if (not isMitotic_)
    {
      isMitotic_ = (isCycling_ && (Protein_array_[0] > 10. ));
	// && (cell_type_==APC || (get_P(0)>random_mitotic_threshold()));
        //if APC, condition for division is only on cycle variable CY_X
        //otherwise, type = Lymphocyte -> division depends on protein content P0
    }
}

void Lymphocyte::UpdateCyclingStatus() {
    if (isCycling_) {
        if (StopCycling()) isCycling_ = false;
    } else if (StartCycling()) {
        isCycling_ = true;
    }
}

bool Lymphocyte::StopCycling() {
    // <TODO> Should the cell stop cycling now ? </TODO>
    return (cell_type_ == CellType::NICHE or get_P(0) < 1.);
}

bool Lymphocyte::StartCycling() {
    // <TODO> Should the cell start cycling now ? </TODO>
    return cell_type_ != NICHE && // internal_state_[CY_X] > cycling_threshold_ &&
             (Protein_array_[0]>= 1.) &&
            ((cell_type_ == STEM && getInSignal(InterCellSignal::NICHE) > 0) ||
            cell_type_ == CANCER ||
            cell_type_ == LYMPHOCYTE ||
            cell_type_ == APC);

}

void Lymphocyte::ODE_update(const double& dt){
    //std::cout << "SyncClock RK45";
    //std::cout << "Sys size:" << odesystemsize_ << std::endl;
    //std::cout << "!- Cell type=" << static_cast<int>(cell_type()) <<  std::endl;

    // RK-Cash-Karp seems faster than rkf45 (runge-kutta-fehlberg)
    gsl_odeiv2_step* s = gsl_odeiv2_step_alloc(gsl_odeiv2_step_rkck,
            odesystemsize_);
    gsl_odeiv2_control* c = gsl_odeiv2_control_y_new(1e-6, 0.0); // first parameter is the tolerance for ODE system
    gsl_odeiv2_evolve* e = gsl_odeiv2_evolve_alloc(odesystemsize_);
    gsl_odeiv2_system sys = {compute_dydt, NULL, odesystemsize_, this};

    double t = 0.0, t1 = dt;
    double h = 1e-3;
    int count_last = 0;
    int count_update = 0;

    switch (cell_type_) {
        case LYMPHOCYTE:
        {
            //Construct random device
            std::random_device rd;
            std::mt19937 gen(rd());
	  
	     internal_state_[APC_Encounters] = getInSignal(InterCellSignal::APC_CONTACT);
	     count_update  =   internal_state_[APC_Encounters];
	   
            /// adding the APC-encounter condition
	     /*  if ( (count_last == 1&& internal_state_[APC_Encounters] == 0)||( count_last == 1 && internal_state_[APC_Encounters] == 1)){
		 internal_state_[APC_Encounters] = 1 ;}*/

	     if ( count_update <= count_last){ //if touching more APC
	       internal_state_[APC_Encounters] = count_last;
	     }
	     else {
	       internal_state_[APC_Encounters]= count_update; 
	     }


            internal_state_[DifferentiationState] = GetDifferentiationState();
            // Here: PDMP (intracellular signalling)*
            // Default bursting parameters
            double a0 = KinParam_[2]; //0;
            double a1 = KinParam_[3]; //2;
            double a2 = KinParam_[4]; //0.02;
            // Default degradation rates
            double d0 = KinParam_[0]; //1; // mRNA degradation rates
            double d1 = KinParam_[1]; //0.2; //protein degradation rates


            const double K0 = a0*d0;
            const double K1 = a1*d0;

            bool jump = false;
            const double S1 = d0 * d1 * a2 / K1;
            const double r = a2;
            
            std::exponential_distribution<double> ExpDistribR(r);
            double currentTime = t;
            double DeltaT = 0.;
            double *PPmax = new double[Number_Of_Genes_]; //init

            while (Time_NextJump_ < t1)
            {
// -------------------------- ------------------------------- ------------------
//-- Advance until next jump, then solve ODE with new init values ---
// -------------------------- ------------------------------- ------------------

                double KKon[Number_Of_Genes_]; // init
                std::vector<double> ProbaArray; // init
                double Sigma;
                double Tau = 0;

                Intracellular_ExactEvol(std::max(Time_NextJump_ - currentTime, 0.), Protein_array_, mRNA_array_, S1);
                if ((ithGene_ < Number_Of_Genes_)&&(ithGene_ >= 0)) {// i represents a real gene, it's a true jump
                    jump = true;
                    TrueJumpCounts_array_[ithGene_] += 1;
                    mRNA_array_[ithGene_] += ExpDistribR(gen);
                } else {
                    PhantomJumpCounts_ += 1;
                }

                // ----------------- Calculate Kon & Tau ----------
                double thetime = log(d0 / d1) / (d0 - d1);

                for (decltype(Number_Of_Genes_) i = 0; i < Number_Of_Genes_; i++) {
                    PPmax[i] = Protein_array_[i] + (S1 / (d0 - d1)) * mRNA_array_[i]*(exp(-thetime * d1) - exp(-thetime * d0));
                    Sigma = GetSigma_i(i, PPmax, false);
                    KKon[i] = (1 - Sigma) * K0 + Sigma * K1 + exp(-10 * log(10)); // Fix precision errors
                    Tau += KKon[i];
                }

                // ----------------- Calculate Delta = Next Jump Time ----------
                std::exponential_distribution<double> ExpDistribTTau(Tau);
                // Draw the waiting time before the next jump
                DeltaT = ExpDistribTTau(gen);

// ---------------------------------------- Select NEXT burst ------------------
                // ----------------- Construct probability array ----------
                double Proba_no_jump = 1;
                

                for (decltype(Number_Of_Genes_) i = 0; i < Number_Of_Genes_; i++) {
                    Sigma = GetSigma_i(i, Protein_array_, true);
                    KKon[i] = (1 - Sigma) * K0 + Sigma*K1; // Fix precision errors
                    ProbaArray.push_back(KKon[i] / Tau);
                    Proba_no_jump = Proba_no_jump - ProbaArray[i];
                }
                ProbaArray.push_back(Proba_no_jump);
                std::discrete_distribution<int> DiscreteJumpDistrib(ProbaArray.begin(), ProbaArray.end());
                
                // ----------------- Find gene # from probability array --------
                ithGene_ = DiscreteJumpDistrib(gen);
                ProbaArray.clear();

                currentTime = Time_NextJump_;

                Time_NextJump_ = Time_NextJump_ + DeltaT;

            }
            
// -------------------------- ------------------------------- ------------------
// -------------------------- If next jump is too far away, solve ODE ----------
// -------------------------- ------------------------------- ------------------
            Time_NextJump_ -= dt;
            Intracellular_ExactEvol(t1-currentTime, Protein_array_, mRNA_array_, S1);
            
            delete [] PPmax;
            
            break;
        }
        case APC:
        {
            internal_state_[DifferentiationState] = -10;
	    internal_state_[APC_Encounters] = -5; // to distinguish a APC and lymphocyte
            break;}
        default:
        {
            internal_state_[DifferentiationState] = -20;
            break;}

    }
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

    while (t < t1) {
        int status = gsl_odeiv2_evolve_apply(e, c, s,
                &sys,
                &t, t1, /* t will be updated several times until t1=t+DT is reached */
                &h, /* the small time step can be updated several times too (adaptative time step) */
                internal_state_); /* internal_state_ will be updated several times */


        if (status != GSL_SUCCESS) {

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
int Lymphocyte::compute_dydt(double t,
        const double y[],
        double dydt[],
        void* cell) {
    return ((Lymphocyte*) cell)->compute_dydt(t, y, dydt);
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
int Lymphocyte::compute_dydt(double t, const double y[], double dydt[]) const {
    // NOTA: t is unused (for now). It is left here because it might be used in
    // the future and it helps understand what's going on.
    assert(t == t); // Suppress warning

    // this is the intracellular network

    // INPUT FROM other cells
    // double F = getInSignal(InterCellSignal::CLOCK);
    // double death_signal = getInSignal(InterCellSignal::DEATH);
    // double in_signal = getInSignal(InterCellSignal::CYCLE_Z);
    double death_signal = getInSignal(InterCellSignal::DEATH);

    // CELL CYCLE VARIABLES (ref: Introducing a reduced cell cycle model, Battogtokh & Tyson 2006 PRE)
    double cy_x = y[CY_X]; // y is a fictive intermediate internal state where the stepper wants to know the derivative
    double cy_z = y[CY_Z]; // we copy it into local variables to make the equations shorter and clearer

    // DEATH AND SURVIVAL VARIABLES
    double cd_death = y[CD_DEATH];

    // CELL CYCLE AUXILIARY FUNCTION
    double e_temp = cy_p - cy_x + cy_p * cy_j + cy_x*cy_j;
    double W0 = 2 * cy_x * cy_j / (e_temp + sqrt(e_temp * e_temp - 4 * cy_x * cy_j * (cy_p - cy_x)));
    double a_temp = cy_k6 + cy_k7*cy_z;
    double b_temp = cy_k8 *  (outer_volume() / size_.volume_min()) + cy_k9*cy_x;
    e_temp = b_temp - a_temp + b_temp * cy_j + a_temp*cy_j;
    double Y0 = 2 * a_temp * cy_j / (e_temp + sqrt(e_temp * e_temp - 4 * a_temp * cy_j * (b_temp - a_temp)));

    // CELL CYCLE EQUATIONS
    // cycling signal variables cy
    // dX/dt = m (k1+k2*W0)-(k3+k4*Y0+k5*Z)*X
    // dZ/dt = k10(1+A(1+sin(f*t)))+k11*X-k12*Z
    // ORIGINAL EQ  f[0] = cell_volume*(cy_k1+cy_k2*W0) - (cy_k3+cy_k4*Y0+cy_k5*cy_z)*cy_x;  (outer_volume() / size_.volume_min()
     dydt[CY_X] = ( (outer_volume() / size_.volume_min())* (cy_k1 + cy_k2 * W0) -
            (cy_k3 + cy_k4 * Y0 + cy_k5 * cy_z) * cy_x);
     dydt[CY_Z] = (cy_k10 * (1 + cy_A) + cy_k11 * cy_x - cy_k12 * cy_z);
    //dydt[CY_Z] = (cy_k10 * (1 + cy_A)  - cy_k12 * cy_z);
 
    // death signal variable cd
    dydt[CD_DEATH] = death_signal - k_survival * cd_death;

     dydt[APC_Encounters] = 0.0 * dydt[CY_Z];
     dydt[DifferentiationState] = 0.0 * dydt[CY_Z];
    return GSL_SUCCESS;
}


// Register this class in Cell
bool Lymphocyte::registered_ =
        Cell::RegisterClass(classId_, classKW_,
        [](const CellType type,
        const MoveBehaviour& move_behaviour,
        const Coordinates<double>& pos,
        double initial_volume,
        double volume_min,
        double doubling_time) {
            return static_cast<Cell*> (
                    new Lymphocyte(type,
                    move_behaviour,
                    pos,
                    initial_volume,
                    volume_min,
                    doubling_time));
        },
[](gzFile backup_file) {
    return static_cast<Cell*> (new Lymphocyte(backup_file));
}
);

