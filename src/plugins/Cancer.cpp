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
Cell(model)//,
//isMitotic_(model.isMitotic_)
 {
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
/*for (u_int32_t j = 0; j < odesystemsize_; j++){
      internal_state_[j] = 0.;}
*/
//cellType = CellType::DIFF_S;
//get_InitPos();
  
 
    
    get_GeneParams();
    mRNA_array_ = new double[Number_Of_Genes_];
    Protein_array_ = new double[Number_Of_Genes_+1];
    TrueJumpCounts_array_ = new int[Number_Of_Genes_];


for (int i = 0; i <  Number_Of_Genes_; i++) {
 mRNA_array_[i] = 0.;
 Protein_array_[i]= 0.;
 TrueJumpCounts_array_[i] = 0;
}

mRNA_array_[0] += Alea::exponential_random(1./KinParam_[5]);

cout << Simulation::usecontactarea() << endl;
Protein_array_[0]+= 2.*KinParam_[8];
//        Protein_array_[0] = 0.;//KinParam_[8];
  //      mRNA_array_[0] = 0.;
        
/*        for (u_int32_t j = 0; j < odesystemsize_; j++){
        mRNA_array_[j] =0.*Alea::random();
        Protein_array_[j] = 0.;
        TrueJumpCounts_array_[j] = 0;
            
        }
*/ 
   Protein_array_[Number_Of_Genes_] = 0.;

    PhantomJumpCounts_ = 0;


//    cout <<this->id()<<" "<< pos_.x<<endl;
/*
    pos_.x = initialP_[((this->id())-1)*6+1];
    pos_.y = initialP_[((this->id())-1)*6+2];
    pos_.z = initialP_[((this->id())-1)*6+3];
   cout <<this->id()<<" "<< pos_.x<<endl;
*/  
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
//    delete [] ThinningParam_array_;

    delete [] GenesInteractionsMatrix_;
    delete [] KinParam_;
}

// ============================================================================
//                                   Methods
// ============================================================================
void Cancer::InternalUpdate(const double& dt) {
    
//  if (Protein_array_[0] >= KinParam_[8]) {internal_state_[Type] = 1.;
//} else {internal_state_[Type] = 0.;}
//cout << "cellType" << CellType::DIFF_S << endl;

/*
if (this->internal_state_[Type] == 1.){cell_type_ = CellType::STEM;
   }
else {
 cell_type_ = CellType::DIFF_S;
}

cout << "cell_type()"<< CellType_Names.at(this->cell_type()).c_str()<< endl; 
  */
 if (cell_type_ != CellType::NICHE) {
   // if (Protein_array_[0] >0.9) {internal_state_[Type] = 1.;} //stem
    ODE_update(dt);

    }
   //   for (int i = 0; i<3*15;i++){
   //cout << initialP_[i] << endl;
   //UpdateCyclingStatus();
   // }
  if (isDying()) return;

Grow(dt);
}

Coordinates<double> Cancer::MotileDisplacement(const double& dt) {
 
  std::vector<Cell*> neighb = neighbours();
  auto cell_it = neighb.begin();
  bool contact_S = false;//contact S-S
  bool contact_D = false;//contact D-D
  bool contact_SD = false; //contact S-D
 switch (cell_type_) {
        case STEM:
        {
   while ( cell_it != neighb.end() ) {
     //if stem cells contact
    if ( (*cell_it)->cell_type() == STEM ) {
      contact_S = true;
      break;
    }
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
     //if stem cells contact
    if ( (*cell_it)->cell_type() == STEM ) {
      contact_SD = true;
      break;
    }
  if ( (*cell_it)->cell_type() == DIFF_S) {
      contact_D = true;
      break;
    }

    cell_it++;
   }
}
}
   double sigma_t;
if ( contact_S){
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


void Cancer::ODE_update(const double& dt){

double t = 0., t1 = dt;
 
/*if (internal_state_[Type]==1.){
        

std::vector<Cell*> neighb = neighbours();
  auto cell_it = neighb.begin();
  bool contact_S = false;
  double N_contact = 0.;
  while ( cell_it != neighb.end() ) {

  if ( (*cell_it)->cell_type() == NONE && (internal_state_[Type] = 1.)) {

 contact_S = true;
 N_contact += 1.;
//    break;
    }
    cell_it++;
   }
  // have TCC signaling because it contacted the effector cell
   if ( contact_S ) {
     internal_state_[S_S] = std::min(getInSignal(InterCellSignal::STEM_CONTACT), N_contact);
//cout <<"internal_state_[S_S]" << internal_state_[S_S]<< endl;
    }

neighb.clear();
     }
   */
if (this->internal_state_[Type] == 1.){cell_type_ = CellType::STEM;
   }else {
 cell_type_ = CellType::DIFF_S;}
//cout << cell_type_ << endl;
 
 switch (cell_type_) {
        case STEM:
        {

      internal_state_[S_S] = getInSignal(InterCellSignal::STEM_CONTACT);
//cout <<"internal_state_[S_S]" << internal_state_[S_S]<< endl;
break;
}
   case DIFF_S:
{
    internal_state_[S_S] = 0.;
break;}
}
//cout <<"internal_state_[S_S]" << internal_state_[S_S]<< endl;

       // Here: PDMP (intracellular signalling)*
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
           
           double *S1 = new double[Number_Of_Genes_]; //init
           for (u_int32_t i = 0; i < Number_Of_Genes_; i++) {
               S1[i] = 0.;
           }
           
           const double r = a2;
           double currentTime = 0.;
           double DeltaT = 0.;
           double *PPmax = new double[Number_Of_Genes_]; //init
           double *ProbaArray = new double[Number_Of_Genes_+1]; //init
           
           for (u_int32_t i = 0; i < Number_Of_Genes_; i++) {
               PPmax[i] = 0.;
           }
           for (u_int32_t i = 0; i < Number_Of_Genes_+1; i++) {
               ProbaArray[i] = 0.;
           }
           
           double *KKon = new double[Number_Of_Genes_]; //init
           for (u_int32_t i = 0; i < Number_Of_Genes_; i++) {
               KKon[i] = 0.;
           }
           
           for (u_int32_t i = 0; i < Number_Of_Genes_; i++) {
	     if (i== 0){
               S1[i] = d0 * D1_S * a2 / K1;}
	     else { S1[i] = d0 * D1_D * a2 / K1;}
           }
	   
           while (Time_NextJump_ < t1){
               
               double *Sigma = new double[Number_Of_Genes_]; //init
               
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
               
               
               Get_Sigma(Sigma, PPmax, false, internal_state_[S_S], KinParam_[9]);
               
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
               
               Get_Sigma(Sigma, Protein_array_, true, internal_state_[S_S], KinParam_[9]);
               
               for (u_int32_t i = 0; i < Number_Of_Genes_; i++) {
                   
                   KKon[i] = (1. - Sigma[i]) * K0 + Sigma[i]*K1;
                   ProbaArray[i] = KKon[i] / Tau;
               //cout << ProbaArray[i] << endl;    
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

//            break;

 }

            
/*
Coordinates<double> Cancer::MotileDisplacement(const double& dt) {
  // <TODO> Motile cell autonomous displacement behaviour
//  Coordinates<double> displ = dt*0.02*Alea::gaussian_random();
double sigma_t = 0.1;
 Coordinates<double> displ { sqrt(dt)*sigma_t*Alea::gaussian_random(),
                               sqrt(dt)*sigma_t*Alea::gaussian_random(),
                                sqrt(dt)*sigma_t*Alea::gaussian_random() };
  return displ;

}
*/
double num_division = 0.;
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
    
    SeparateDividingCells(this, newCell);
    

    std::cout << "Cell " << this->id() << " is DIVIDING!!" <<  " at t=" << Simulation::sim_time() << std::endl;    ///remember division
    
    Protein_array_[Number_Of_Genes_] += 1.;
    newCell->Update_count_division(Protein_array_[Number_Of_Genes_], Number_Of_Genes_);
    
    num_division += 1.;
    
    double m = 20.;
    
    double a = 10.;
    double K_p, K_m;
    double randomCell;
    
    K_p = 1.0 - (m / 100.)* Alea::gaussian_random_0_2();
    K_m = 1.0 - (a / 100.)* Alea::gaussian_random_0_2();
    
    
    if ( K_p >= 2. || K_m >= 2. || K_p < 0. || K_m < 0.) {
        cout << "negative,**************************************** "  << K_p<< K_m << endl;
        K_p = 1.0 - (m / 100.)* Alea::gaussian_random_0_2();
        K_m = 1.0 - (a / 100.)* Alea::gaussian_random_0_2();
        
    }
    
    //divide molecular content into two daughter cells
    
    randomCell = Alea::random();
    
   
	        newCell->SetNewProtein_stem_symmetric(K_p, Protein_array_, Number_Of_Genes_);
            newCell->SetNewRNA_stem_symmetric(K_m, mRNA_array_, Number_Of_Genes_);
    if (newCell->Protein_array_[0]> KinParam_[8]) { newCell->internal_state_[Type] =1.;}
    else{
        newCell->internal_state_[Type] =0.;}
    
            this->SetNewProtein_stem_symmetric(K_p, Protein_array_, Number_Of_Genes_);
            this->SetNewRNA_stem_symmetric(K_m, mRNA_array_, Number_Of_Genes_);
    if (this->Protein_array_[0]> KinParam_[8]) { this->internal_state_[Type] =1.;}
    else{
        this->internal_state_[Type] =0.;}
  
 // newCell->UpdateType(internal_state_[Type]);
  //  std::cout << "Type " << internal_state_[Type]<<std::endl;
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

void Cancer::SetNewProteinLevel1(const double K,const double MotherProteins[],const int sz)const {
    for (u_int32_t j = 0; j < sz; j++) {
      Protein_array_[j] =  K * MotherProteins[j]/2.;
    }
}
void Cancer::SetNewProteinLevel2(const  double K,const double MotherProteins[],const int sz)const{
    for (u_int32_t j = 0; j < sz; j++) {
      Protein_array_[j] = (2.0 - K) * MotherProteins[j]/2.;
    }
}

void Cancer::SetNewRNALevel1(const double K,const double MotherRNA[],const int sz)const {
    for (u_int32_t j = 0; j < sz; j++) {
     mRNA_array_[j] =  K * MotherRNA[j]/2.;
    }
}
void Cancer::SetNewRNALevel2(const  double K,const double MotherRNA[],const int sz)const {
    for (u_int32_t j = 0; j < sz; j++) {
      mRNA_array_[j] = (2.0 -  K) * MotherRNA[j]/2.;
    }
}

void Cancer::SetNewProtein_stem(const double K,const double MotherProteins[],const int sz)const {
    //90%->stem : CD133
    for (u_int32_t j = 0; j < sz; j++) {
      Protein_array_[j] =  1.8 * MotherProteins[j]/2.;
    }
}
void Cancer::SetNewProtein_differentiated(const  double K,const double MotherProteins[],const int sz)const{
  //differentiated : Synaptophysine
    for (u_int32_t j = 0; j < sz; j++) {
      Protein_array_[j] = 0.2 * MotherProteins[j]/2.;
    }
}

void Cancer::SetNewRNA_stem(const double K,const double MotherRNA[],const int sz)const {
    //90%->stem
    for (u_int32_t j = 0; j < sz; j++) {
     mRNA_array_[j] =  1.8 * MotherRNA[j]/2.;
    }
}
void Cancer::SetNewRNA_differentiated(const  double K,const double MotherRNA[],const int sz)const {
    //differentiated
    for (u_int32_t j = 0; j < sz; j++) {
      mRNA_array_[j] = 0.2 * MotherRNA[j]/2.;
    }
}

void Cancer::SetNewProtein_stem_symmetric(const double K,const double MotherProteins[],const int sz)const {                                                                                                                                         
    for (u_int32_t j = 0; j < sz; j++) {
      Protein_array_[j] =  MotherProteins[j]/2.;
    }
}

void Cancer::SetNewRNA_stem_symmetric(const double K,const double MotherRNA[],const int sz)const {
    for (u_int32_t j = 0; j < sz; j++) {
     mRNA_array_[j] =  MotherRNA[j]/2.;
    }
}



void Cancer::Update_count_division_mother( double Mother_division ) {
Mother_division += 1.;
}

void Cancer::Update_count_division( double Mother_division,const int s )const {

  Protein_array_[s]  = Mother_division;

  }


//############sigma#######
void Cancer::Get_Sigma( double *Sigma_i ,double * P, bool AcceptNegative, double S_S, const double x ) {

double Parr[Number_Of_Genes_];
 memcpy(Parr, P, Number_Of_Genes_ * sizeof (*P));

 double interIJ;
 double initval_i[Number_Of_Genes_];


  for (u_int32_t i = 0; i < Number_Of_Genes_; i++) {
 //if ( i == 0){ initval_i[i] = -3.0;}
// Basal activity for other genes                                                                                                                                                               
  //else {initval_i[i] = -5.0;}
if ( i == 0){
  initval_i[i] = -3.0;
//   if (Protein_array_[(Number_Of_Genes_)] < 1.) {
   if (S_S > 0.) {initval_i[i] += S_S*x;}
}
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


double Cancer::get_GeneParams(void) {
    KinParam_ = new double[Number_Of_Parameters_];

    ////////////////////////////////////                                                                                                                                                            
    ifstream indatakin; // indata is like cin                                                                                                                                                       

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
        
//cout <<a<< "\t"; 
}
    }

    indatakin.close();

    ifstream indataf; // indata is like cin                                                                                                                                                         

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

//cout << "GenesInteractionsMatrix_" << Number_Of_Genes_ << endl;

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
//cout<< a<< "\t";
            }
        }
    i = i + 1;

        // process pair (a,b)                                                                                                                                                                       
    }

     
     indataf.close();
 
    return 0;
}

double Cancer::get_InitPos() {
std::vector<double>  initialPos;

ifstream indataP; // indata

string filename = "initpop.txt";

 indataP.open(filename); // opens the file
  if (!indataP) { // file couldn't be opened
        std::cerr << "Error: file Pos_0.txt could not be opened" << std::endl;
  initialP_ = new double[6*15];
//cout << "Simulation::pop().Size()"<< Simulation::pop().Size()<<endl;
  /*for (u_int32_t i = 0; i < 6*15; i++){
    initialP_[i]= 0.;
  //  indataP.close();
    }
*/
  }
    std::string line;
    double val;
    int c = 0;
    int l = 0;

while (std::getline(indataP, line)) {
  // Number_Tcells += 1;
        std::istringstream Vss(line);
        l = 0;
        while ( l < 6) {
            if ((!(Vss >> val))) {
                std::cerr << "Error: Value missing in position/character" << std::endl;
                break;
            }// error
            else {
        initialPos.push_back(val);      
         l = l + 1;
            }
        }
        c = c + 1;
        }

    initialP_ =new double[6*15];
    indataP.close();
    
/*    for (u_int32_t i = 0; i < 6*15; i++){
    initialP_[i]=initialPos.at(i);
    cout <<initialP_[i]<< "\t";
      }
*/  
  return 0.0;
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
  case InterCellSignal::STEM_CONTACT:
            return internal_state_[S_S];
  case InterCellSignal::CANCER_S:
            return Protein_array_[0];
      case InterCellSignal::CANCER_D1:
            return  Protein_array_[1];
      case InterCellSignal::CANCER_D2:
                return Protein_array_[2];
      case InterCellSignal::CANCER_P:
              return Protein_array_[3];
  case InterCellSignal::REMEMBER_DIVISION:
            return  Protein_array_[4];
      case InterCellSignal::CANCER_mRNA_S:
            return mRNA_array_[0];
      case InterCellSignal::CANCER_mRNA_D1:
            return mRNA_array_[1];
      case InterCellSignal::CANCER_mRNA_D2:
                return  mRNA_array_[2];
      case InterCellSignal::CANCER_mRNA_P:
                return mRNA_array_[3];
    default:
          return 0.0;
}
}

/*bool Cancer::isCycling() const {
  // <TODO> Is the cell currently cycling ? </TODO>
  return isCycling_;
}
*/
bool Cancer::isDividing() const {
  // <TODO> Should the cell divide now ? </TODO>
 /*   if (internal_state_[Type] == 1. && Protein_array_[3] >= 0.4 && outer_volume() >= 2.)
        return true;
    else if (internal_state_[Type] == 0. && Protein_array_[3] >= 0.06 && outer_volume() >= 2.)
  //if  (outer_volume() >= 2.) // size_.volume_min())
    return true;
  else
    return false;*/

  //  return (Protein_array_[3] >= KinParam_[6] && size_.may_divide());
  
/*    if (internal_state_[Type] > 0.){
           return (Protein_array_[3] >= KinParam_[7] && size_.may_divide());}
      else {
*/  
     return (Protein_array_[3] >= KinParam_[6] && size_.may_divide());//}

  
}

bool Cancer::isDying() const {
  // <TODO> Should the cell die now ? </TODO>
  // cell is immortal
  return (Protein_array_[2] >= 0.1) ;

// return false;
}

/*void Cancer::UpdateCyclingStatus() {

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
*/

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
