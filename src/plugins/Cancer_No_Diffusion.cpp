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
#include "WorldSize.cpp"
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
Cell(model){
  phylogeny_id_.push_back(this->id());
    phylogeny_t_.push_back(Simulation::sim_time());
    internal_state_ = new double[odesystemsize_];
    memcpy(internal_state_, model.internal_state_, odesystemsize_ * sizeof (*internal_state_));

    this->Number_Of_Genes_ = model.Number_Of_Genes_;


    mRNA_array_ = new double[Number_Of_Genes_];
    Protein_array_ = new double[Number_Of_Genes_+1];
    TrueJumpCounts_array_ = new int[Number_Of_Genes_];
   
     memcpy(mRNA_array_, model.mRNA_array_, Number_Of_Genes_ * sizeof (*mRNA_array_));
     memcpy(Protein_array_, model.Protein_array_, (Number_Of_Genes_+1) * sizeof (*Protein_array_));
     memcpy(TrueJumpCounts_array_, model.TrueJumpCounts_array_, Number_Of_Genes_ * sizeof (*TrueJumpCounts_array_));
     

    KinParam_ = new double[Number_Of_Parameters_];
    GenesInteractionsMatrix_ = new double[Number_Of_Genes_ * Number_Of_Genes_];

    memcpy(KinParam_, model.KinParam_, Number_Of_Parameters_ * sizeof (*KinParam_));
    memcpy(GenesInteractionsMatrix_, model.GenesInteractionsMatrix_, Number_Of_Genes_ * Number_Of_Genes_ * sizeof (*GenesInteractionsMatrix_));


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
       	 pos,
	 CellSize(initial_volume, volume_min),
         doubling_time) {

        pos_.x = WorldSize::size().x/2. + 2.*(Alea::gaussian_random_0_2());
        pos_.y = WorldSize::size().y/2. + 3.*(Alea::gaussian_random_0_2());
        pos_.z = WorldSize::size().z/2. + 4.*(Alea::gaussian_random_0_2());
  	
	internal_state_ = new double[odesystemsize_];
    	internal_state_[Type] = 1.;
   	internal_state_[S_S] = 0.; 
    
     	get_GeneParams();
    	mRNA_array_ = new double[Number_Of_Genes_];
    	Protein_array_ = new double[Number_Of_Genes_+1];
    	TrueJumpCounts_array_ = new int[Number_Of_Genes_];


	for (int i = 0; i <  Number_Of_Genes_; i++) {
 	mRNA_array_[i] = 0.;
 	Protein_array_[i]= 0.;
 	TrueJumpCounts_array_[i] = 0;
	}
      // initial protein and mRNA values of stem cells  
	mRNA_array_[0] += Alea::exponential_random(1./KinParam_[5]);

	Protein_array_[0]+= 2.*KinParam_[8];

   	Protein_array_[Number_Of_Genes_] = 0.;

    	PhantomJumpCounts_ = 0;
}


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
Cancer::~Cancer() noexcept {
    delete [] internal_state_;
    delete [] mRNA_array_;
    delete [] Protein_array_;
    delete [] TrueJumpCounts_array_;
    delete [] GenesInteractionsMatrix_;
    delete [] KinParam_;
}

// ============================================================================
//                                   Methods
// ============================================================================
void Cancer::InternalUpdate(const double& dt) {
 // update internal states   
 if (cell_type_ != CellType::NICHE) {
     ODE_update(dt);
    }
Grow(dt);//cell growth
}

//movement 'motile'
Coordinates<double> Cancer::MotileDisplacement(const double& dt) {
 
  std::vector<Cell*> neighb = neighbours();
  auto cell_it = neighb.begin();
  bool contact_S = false;//contact S-S
  bool contact_D = false;//contact D-D
  bool contact_SD = false;//contact S-D
 switch (cell_type_) {
        case STEM:
        {
   while ( cell_it != neighb.end() ) {
     //if stem cells contact
    if ( (*cell_it)->cell_type() == STEM ) {
      contact_S = true;
      break;
    }
    //if stem cells come into contact with differentiated cells
    if ( (*cell_it)->cell_type() == DIFF_S) {
      contact_SD = true;
      break;
    }
    cell_it++;
   }
}

case DIFF_S:
{
while ( cell_it != neighb.end() ) {
    //if differentiated cells come into contact with stem cells 
    if ( (*cell_it)->cell_type() == STEM ) {
      contact_SD = true;
      break;
    }
    //if differentiated cells contact
    if ( (*cell_it)->cell_type() == DIFF_S) {
      contact_D = true;
      break;
    }
    cell_it++;
   }
}
}
   double sigma_t;

   if (contact_S){
   	sigma_t = KinParam_[10];}
   else if ( contact_S && contact_SD){
   	sigma_t = KinParam_[10];}
   else if ( contact_S && contact_D){
   	sigma_t = KinParam_[10];}
   else if( contact_SD ){
      	sigma_t = KinParam_[11];}
   else if( contact_D ){
      	sigma_t = KinParam_[12];}
   else {sigma_t = KinParam_[13];}
  	Coordinates<double> displ { sqrt(dt)*sigma_t*Alea::gaussian_random(),
                              sqrt(dt)*sigma_t*Alea::gaussian_random(),
                              sqrt(dt)*sigma_t*Alea::gaussian_random() };
  return displ;
}

//compute the stem cell neighbours
double Cancer::count_Neib_Stem(){
std::vector<Cell*> neighb = neighbours();
  auto cell_it = neighb.begin();
  double count_S = 0.;//contact S-S
  while ( cell_it != neighb.end() ) {
   //if stem cells contact
  double mean_radis = ((*cell_it)->external_radius() + external_radius()) / 2.;
  double h= mean_radis - Distance(*cell_it)/2.;
  if (h>0. && (*cell_it)->cell_type() == STEM ) {
      count_S += 1.;
}
 cell_it++;
}
return count_S;
}

//compute the differentiated cell neighbours
double Cancer::count_Neib_Syp(){
std::vector<Cell*> neighb = neighbours();
  auto cell_it = neighb.begin();
  double count_D = 0.;//contact D-D
  while ( cell_it != neighb.end() ) {
  //if differentiated cells contact
  double mean_radis = ((*cell_it)->external_radius() + external_radius()) / 2.;
  double h= mean_radis - Distance(*cell_it)/2.;
  if (h>0. && (*cell_it)->cell_type() == DIFF_S ) {
      count_D += 1.;
}
 cell_it++;
}
return count_D;
}


//update the molecular contents
void Cancer::ODE_update(const double& dt){

double t = 0., t1 = dt;
//define the cell type
if (this->internal_state_[Type] == 1.){cell_type_ = CellType::STEM;
   }else {
 cell_type_ = CellType::DIFF_S;}

// the number of cells in contact S-S or D-D
 switch (cell_type_) {
        case STEM:
        {
	internal_state_[S_S] = count_Neib_Stem();
	internal_state_[D_D] = 0.;
	break;
	}
   	case DIFF_S:
	{
    	internal_state_[S_S] = 0.;
	internal_state_[D_D] = count_Neib_Syp();
	break;}
}

      
////////// Here: PDMP (intracellular signalling)*
           // from harissa/simulation/pdmp.py
           // Default bursting parameters
           double a0 = KinParam_[3]; //0;
           double a1 = KinParam_[4];/// change to normalize protein
           double a2 = KinParam_[5];
           // Default degradation rates
           double d0 = KinParam_[0];  // mRNA degradation rates
           double D1_D= KinParam_[2];  //protein degradation rates of differentiated cell
	   double D1_S = KinParam_[1];  //protein degradation rates of stem cell
	   
           const double K0 = a0*d0;
           const double K1 = a1*d0;
           
           bool jump = false;
           
           double *S1 = new double[Number_Of_Genes_];  //S1
           for (u_int32_t i = 0; i < Number_Of_Genes_; i++) {
               S1[i] = 0.;
           }
           
           const double r = a2;
           double currentTime = 0.;
           double DeltaT = 0.;
           double *PPmax = new double[Number_Of_Genes_]; //Pmax
           double *ProbaArray = new double[Number_Of_Genes_+1]; //init
           
           for (u_int32_t i = 0; i < Number_Of_Genes_; i++) {
               PPmax[i] = 0.;
           }
           for (u_int32_t i = 0; i < Number_Of_Genes_+1; i++) {
               ProbaArray[i] = 0.;
           }
           
           double *KKon = new double[Number_Of_Genes_]; //Kon
           for (u_int32_t i = 0; i < Number_Of_Genes_; i++) {
               KKon[i] = 0.;
           }
           
           for (u_int32_t i = 0; i < Number_Of_Genes_; i++) {
	     if (i== 0){
               S1[i] = d0 * D1_S * a2 / K1;}
	     else { S1[i] = d0 * D1_D * a2 / K1;}
           }
	   
           while (Time_NextJump_ < t1){
               
               double *Sigma = new double[Number_Of_Genes_];//Sigma
               
               for (u_int32_t i = 0; i < Number_Of_Genes_; i++) {
                   Sigma[i] = 0.;
               }
               
               double Tau = 0.;
               
               // -------------------------- ------------------------------- ------------------
               //-- Advance until next jump, then solve ODE with new init values ---
               // -------------------------- ------------------------------- ------------------
               
               Intracellular_ExactEvol(std::max(Time_NextJump_ - currentTime, 0.), Protein_array_, mRNA_array_, S1);
               
               if ((ithGene_ < Number_Of_Genes_)&&(ithGene_ >= 0)) {// i represents a real gene, it's a true jump
                   jump = true;
                   TrueJumpCounts_array_[ithGene_] += 1;
                 mRNA_array_[ithGene_] += Alea::exponential_random(1./r);
               } else {
                   PhantomJumpCounts_ += 1;
               }
               
               
               
               // ----------------- Calculate Kon & Tau ----------
               double thetime_D = log(d0 / D1_D) / (d0 - D1_D);
	       double thetime_S = log(d0 / D1_S) / (d0 - D1_S);
               
               for (u_int32_t i = 0; i < Number_Of_Genes_; i++) {
		 if (i == 0) { 
                   PPmax[i] = Protein_array_[i] + (S1[i] / (d0 - D1_S)) * mRNA_array_[i]*(exp(-thetime_S * D1_S) - exp(-thetime_S * d0));}
		 else {
		   PPmax[i] = Protein_array_[i] + (S1[i] / (d0 - D1_D)) * mRNA_array_[i]*(exp(-thetime_D * D1_D) - exp(-thetime_D * d0));}

               }
               
              //for contact cell-cell
              
	       Get_Sigma(Sigma, PPmax, false, internal_state_[S_S], KinParam_[9], internal_state_[D_D], KinParam_[14]);
            
               for (u_int32_t i = 0; i < Number_Of_Genes_; i++) {
                   
                   KKon[i] = (1. - Sigma[i]) * K0 + Sigma[i] * K1 + exp(-10. * log(10.)); // Fix precision errors
                   
                   Tau += KKon[i];
                   
               }
               
               // ----------------- Calculate Delta = Next Jump Time ----------
               
               // Draw the waiting time before the next jump
               DeltaT = Alea::exponential_random(1./Tau);
               
               // ---------------------------------------- Select NEXT burst ------------------
               // ----------------- Construct probability array ----------
               double Proba_no_jump = 1.;
               
            //   Get_Sigma(Sigma, Protein_array_, true, internal_state_[S_S], KinParam_[9]);
              
	     //for contact cell-cell
              Get_Sigma(Sigma, Protein_array_, true, internal_state_[S_S], KinParam_[9],internal_state_[D_D], KinParam_[14]);

 
               for (u_int32_t i = 0; i < Number_Of_Genes_; i++) {
                   
                   KKon[i] = (1. - Sigma[i]) * K0 + Sigma[i]*K1;
                   ProbaArray[i] = KKon[i] / Tau;
                   Proba_no_jump = Proba_no_jump - ProbaArray[i];
               }
               
               
               ProbaArray[Number_Of_Genes_] = Proba_no_jump;
               
               // ----------------- Find gene # from probability array --------
               
               ithGene_ =  Alea::discrete_distribution(Number_Of_Genes_+1, ProbaArray);
               
               delete [] Sigma;
               
               currentTime = Time_NextJump_;
               
               Time_NextJump_ += DeltaT;
               
           }
    
           
           // -------------------------- ------------------------------- ------------------
           // -------------------------- If next jump is too far away, solve ODE ----------
           // -------------------------- ------------------------------- ------------------
           Time_NextJump_ -= dt;
           
           Intracellular_ExactEvol(t1-currentTime, Protein_array_, mRNA_array_, S1);
           
           delete [] PPmax;
           delete [] ProbaArray;
           delete [] KKon;
           delete [] S1;
 }

  
void Cancer::UpdatePhylogeny(vector<int> phy_id, vector<double> phy_t, int sz) {
    phylogeny_id_.insert(phylogeny_id_.begin(), phy_id.begin(), phy_id.end());
    phylogeny_t_.insert(phylogeny_t_.begin(), phy_t.begin(), phy_t.end());
}
void Cancer::UpdateType(double Mother_type) {
      internal_state_[Type] =  Mother_type;
}


double num_division = 0.;
Cell* Cancer::Divide(void) {
    // <TODO> Determine how the cell should divide </TODO>
    Cancer* newCell = new Cancer(*this);
    
    SeparateDividingCells(this, newCell);
    
    std::cout << "Cell " << this->id() << " is DIVIDING!!" <<  " at t=" << Simulation::sim_time() << std::endl;    ///remember division
    
    Protein_array_[Number_Of_Genes_] += 1.;
    newCell->Update_count_division(Protein_array_[Number_Of_Genes_], Number_Of_Genes_);
    
    num_division += 1.;

    //divide molecular content into two daughter cells
    
    newCell->SetNewProtein_stem_symmetric(Protein_array_, Number_Of_Genes_);
    newCell->SetNewRNA_stem_symmetric(mRNA_array_, Number_Of_Genes_);
    
    //cell type for daughter cell 
    if (newCell->Protein_array_[0]> KinParam_[8]) { newCell->internal_state_[Type] =1.;}
    else{
        newCell->internal_state_[Type] =0.;}
    
    this->SetNewProtein_stem_symmetric(Protein_array_, Number_Of_Genes_);
    this->SetNewRNA_stem_symmetric(mRNA_array_, Number_Of_Genes_);
     //cell type for daughter cell 
    if (this->Protein_array_[0]> KinParam_[8]) { this->internal_state_[Type] =1.;}
    else{
        this->internal_state_[Type] =0.;}
  

  newCell->UpdatePhylogeny(phylogeny_id_, phylogeny_t_, phylogeny_id_.size());
  phylogeny_id_.push_back(this->id());

  phylogeny_t_.push_back(Simulation::sim_time());

  if ( ( phylogeny_ID_file_ = fopen(phylogeny_ID_filename,"a") ) != NULL ) {
    fprintf(phylogeny_ID_file_, "%d\t%d\t%f\n", this->id(),  newCell-> id(),  Simulation::sim_time());
      fclose(phylogeny_ID_file_);
    }
    if ( ( phylogeny_T_file_ = fopen(phylogeny_T_filename,"a") ) != NULL ) {
      fprintf(phylogeny_T_file_, "%f \n ", Simulation::sim_time());
      fclose(phylogeny_T_file_);
    }

  return newCell;
}

//update protein and mRNA

 void Cancer::Intracellular_ExactEvol(double thetime, double * P, double * M, double * S1) {
        //degradation rates
    double d0 = KinParam_[0];  // mRNA degradation rates                                                                                                                                
    double D1_D= KinParam_[2];  //protein degradation rates of differentiated cell                                                                                                                                                                                  
    double D1_S = KinParam_[1];  //protein degradation rates of stem cell                                                                                                                                                                                           
    double Marr[Number_Of_Genes_];
    double Parr[Number_Of_Genes_];

    memcpy(Marr, M, Number_Of_Genes_ * sizeof (*M));
    memcpy(Parr, P, Number_Of_Genes_ * sizeof (*P));
    // New values:                                                                                                                                                                              
    for (u_int32_t i = 0; i < Number_Of_Genes_; i++) {
      if (i == 0) {
        M[i] = Marr[i] * exp(-thetime * d0);
        P[i] = ((S1[i] / (d0 - D1_S)) * Marr[i]*(exp(-thetime * D1_S) - exp(-thetime * d0)) + Parr[i] * exp(-thetime * D1_S));
      }
      else {
	M[i] = Marr[i] * exp(-thetime * d0);
        P[i] = ((S1[i] / (d0 - D1_D)) * Marr[i]*(exp(-thetime * D1_D) - exp(-thetime * d0)) + Parr[i] * exp(-thetime * D1_D));
      }
      
 }
 }

void Cancer::SetNewProtein_stem_symmetric(const double MotherProteins[],const int sz)const {                                                                                                                                         
    for (u_int32_t j = 0; j < sz; j++) {
      Protein_array_[j] =  MotherProteins[j]/2.;
    }
}

void Cancer::SetNewRNA_stem_symmetric(const double MotherRNA[],const int sz)const {
    for (u_int32_t j = 0; j < sz; j++) {
     mRNA_array_[j] =  MotherRNA[j]/2.;
    }
}


void Cancer::Update_count_division( double Mother_division,const int s )const {

  Protein_array_[s]  = Mother_division;

  }


//Sigma function
void Cancer::Get_Sigma(double *Sigma_i, double * P, bool AcceptNegative, double S_S, const double x, double D_D, const double y) {

double Parr[Number_Of_Genes_];
 memcpy(Parr, P, Number_Of_Genes_ * sizeof (*P));

 double interIJ;
 double initval_i[Number_Of_Genes_];


  for (u_int32_t i = 0; i < Number_Of_Genes_; i++) {
// Basal activity for other genes                                                                                                                                                               
  if ( i == 0){
  initval_i[i] = -3.0;
// signaling between the stem cells or diffusion
  if (S_S > 0.) {initval_i[i] += S_S*x;}
}

  else if ( i == 1){
  initval_i[i] = -5.0;
// signaling between differentiated cells
//  if (D_D > 0.) {initval_i[i] += D_D * y;}
}

//other genes
else {initval_i[i] = -5.0;}


 for (u_int32_t j = 0; j < Number_Of_Genes_; j++) {
   interIJ = GenesInteractionsMatrix_[i + Number_Of_Genes_ * j]; // J acts on I                                                                                                                 
    if (((interIJ > 0.)||AcceptNegative)) {
      initval_i[i] += Parr[j] * interIJ;
      }
 }
}

 for (u_int32_t i = 0; i < Number_Of_Genes_; i++) {
Sigma_i[i] = (1. / (1. + exp(-initval_i[i])));
}
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

// the gene parameters
double Cancer::get_GeneParams(void) {
    KinParam_ = new double[Number_Of_Parameters_];
                                                                                                                                                            
    ifstream indatakin; // indata for kineticsparam.txt                                                                                                                                                      

    string filename ="kineticsparam.txt";

    indatakin.open(filename); // opens the file                                                                                                                                                     
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

    ifstream indataf; // indata for GeneInteractionsMatrix.txt                                                                                                                                                        
    string filename2 ="GeneInteractionsMatrix.txt";

    indataf.open(filename2); // opens the file                                                                                                                                                      

    if (!indataf) { // file couldn't be opened                                                                                                                                                      
        std::cerr << "Error: file GeneInteractionsMatrix.txt could not be opened" << std::endl;
        exit(1);
    }
    Number_Of_Genes_ = 0;
    std::getline(indataf, line);
    std::stringstream ss(line);
    std::string word;
    int i = 0;
 int j = 0;

    while (ss >> word) {
        Number_Of_Genes_ += 1;
    }

     Number_Of_Genes_ -= 1; // to include title column                                                                                                                                              

    GenesInteractionsMatrix_ = new double[Number_Of_Genes_ * Number_Of_Genes_];

     while (std::getline(indataf, line)) {
        std::istringstream iss(line);
        iss >> word;
        j = 0;
        while (j < Number_Of_Genes_) {
            if ((!(iss >> a))) {
                std::cerr << "Error: possible value missing" << std::endl;
                break;
            }// error                                                                                                                                                                               
            else {
                GenesInteractionsMatrix_[(j + Number_Of_Genes_ * i)] = a;
                j = j + 1;
            }
        }
    i = i + 1;}

    indataf.close();
 
    return 0;
}


//print the output signal
 
double Cancer::get_output(InterCellSignal signal) const {
 switch (signal) {
 	case InterCellSignal::CANCER_TYPE:
      	 return  internal_state_[Type]; 
 	case InterCellSignal::VOLUME:
      	 return outer_volume();
  	case InterCellSignal::STEM_CONTACT:
         return internal_state_[S_S];
  	//case InterCellSignal::SYP_CONTACT:
      	//return internal_state_[D_D];
	case InterCellSignal::CANCER_S:
         return Protein_array_[0];
      	case InterCellSignal::CANCER_D1:
         return  Protein_array_[1];
   	case InterCellSignal::CANCER_P:
         return Protein_array_[2];
  	case InterCellSignal::REMEMBER_DIVISION:
         return  Protein_array_[3];
      	case InterCellSignal::CANCER_mRNA_S:
         return mRNA_array_[0];
      	case InterCellSignal::CANCER_mRNA_D1:
         return mRNA_array_[1];
   	case InterCellSignal::CANCER_mRNA_P:
         return mRNA_array_[2];
    default:
          return 0.0;
}
}

bool Cancer::isDividing() const {
  // <TODO> Should the cell divide now ? </TODO>  
     return (Protein_array_[2] >= KinParam_[6] && size_.may_divide());
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
