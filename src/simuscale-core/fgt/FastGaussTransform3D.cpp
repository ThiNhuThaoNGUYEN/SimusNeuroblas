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


#include "FastGaussTransform3D.h"

using namespace std;

using real_type = FastGaussTransform3D::real_type_;
using uint_type = FastGaussTransform3D::uint_type_;
using point_type = FastGaussTransform3D::point_type_;

/* static member factorial_ needs to be declared */
constexpr unsigned long long FastGaussTransform3D::factorial_[20];

/**
 * 
 */
FastGaussTransform3D::FastGaussTransform3D( vector<real_type> q, 
                                            vector<point_type> s, 
                                            vector<point_type> t, 
                                            real_type delta, 
                                            real_type epsilon) : q_(q), 
                                                                 s_(s), 
                                                                 t_(t), 
                                                                 delta_(delta), 
                                                                 epsilon_(epsilon)
{ }

void FastGaussTransform3D::Setup(const SimulationParams& simParams) {

  // Intercellular signals that diffuse
  using_diffusive_signals_ = simParams.using_diffusive_signals();
  diffusive_delta_ = simParams.diffusive_delta();
  diffusive_epsilon_ = simParams.diffusive_epsilon();


}

void FastGaussTransform3D::init_transform(InterCellSignal signal) {

  point_type p;
  cell_list_ = &Simulation::pop().cell_list();
  signal_ = signal;

  unsigned int sig_ind = 0;
  for ( auto sig : using_diffusive_signals_ ) {
    if ( sig == signal_ ) {
      break;
    }
    sig_ind++;
  }
  delta_ = diffusive_delta_[sig_ind];
  epsilon_ = diffusive_epsilon_[sig_ind];

  for (Cell* cell : *cell_list_) {
    real_type q = cell->gaussian_field_weight(signal_);
    if ( q != 0.0f ) {
      q_.push_back(cell->gaussian_field_weight(signal_));
      p[0] = cell->gaussian_field_source(signal_).x;
      p[1] = cell->gaussian_field_source(signal_).y;
      p[2] = cell->gaussian_field_source(signal_).z;
      s_.push_back(p);
    }
    for ( auto pc : cell->gaussian_field_targets(signal_) ) {
      p[0] = pc.x;
      p[1] = pc.y;
      p[2] = pc.z;
      t_.push_back(p);
    }

  }

  N_ = s_.size();
  M_ = t_.size();

  G_.assign(M_,0.0);
  Gexact_.assign(M_,0.0);

  // printf( "  N = %u  M = %u, delta = %g\n", N_, M_, delta_);

  scale_ = 1.0;
  renormalize();
  delta_ /= scale_*scale_;

  N_side_ = sqrt(2.0/delta_) + 1;    /* nbr box in each dimensions */
  r_      = 1.0/N_side_/sqrt(2.0*delta_);  /* r largest value < 1/2 such that N_side_ is a integer */

  evals_ = {0,0,0,0};

}

void FastGaussTransform3D::renormalize() {
  
  real_type r_min = INFINITY, r_max = -INFINITY;  /* normalisation parameters */
  for ( uint_type_ i = 0; i < N_; i++)
  {
    if ( s_.at(i).at(0) < r_min ) r_min = s_.at(i).at(0);
    if ( s_.at(i).at(1) < r_min ) r_min = s_.at(i).at(1);
    if ( s_.at(i).at(2) < r_min ) r_min = s_.at(i).at(2);
    if ( s_.at(i).at(0) < r_min ) r_min = s_.at(i).at(0);
    if ( s_.at(i).at(1) < r_min ) r_min = s_.at(i).at(1);
    if ( s_.at(i).at(2) < r_min ) r_min = s_.at(i).at(2);
    if ( s_.at(i).at(0) > r_max ) r_max = s_.at(i).at(0);
    if ( s_.at(i).at(1) > r_max ) r_max = s_.at(i).at(1);
    if ( s_.at(i).at(2) > r_max ) r_max = s_.at(i).at(2);
    if ( s_.at(i).at(0) > r_max ) r_max = s_.at(i).at(0);
    if ( s_.at(i).at(1) > r_max ) r_max = s_.at(i).at(1);
    if ( s_.at(i).at(2) > r_max ) r_max = s_.at(i).at(2);
  }
  for ( uint_type_ i = 0; i < M_; i++)
  {
    if ( t_.at(i).at(0) < r_min ) r_min = t_.at(i).at(0);
    if ( t_.at(i).at(1) < r_min ) r_min = t_.at(i).at(1);
    if ( t_.at(i).at(2) < r_min ) r_min = t_.at(i).at(2);
    if ( t_.at(i).at(0) < r_min ) r_min = t_.at(i).at(0);
    if ( t_.at(i).at(1) < r_min ) r_min = t_.at(i).at(1);
    if ( t_.at(i).at(2) < r_min ) r_min = t_.at(i).at(2);
    if ( t_.at(i).at(0) > r_max ) r_max = t_.at(i).at(0);
    if ( t_.at(i).at(1) > r_max ) r_max = t_.at(i).at(1);
    if ( t_.at(i).at(2) > r_max ) r_max = t_.at(i).at(2);
    if ( t_.at(i).at(0) > r_max ) r_max = t_.at(i).at(0);
    if ( t_.at(i).at(1) > r_max ) r_max = t_.at(i).at(1);
    if ( t_.at(i).at(2) > r_max ) r_max = t_.at(i).at(2);
  }

  scale_ = (r_max - r_min)*1.000001;  /* scale */

  for ( uint_type_ i = 0; i < N_; i++)
  {
    s_.at(i).at(0) = (s_.at(i).at(0) - r_min)/ scale_;
    s_.at(i).at(1) = (s_.at(i).at(1) - r_min)/ scale_;
    s_.at(i).at(2) = (s_.at(i).at(2) - r_min)/ scale_;
  }
  for ( uint_type_ i = 0; i < M_; i++)
  {
    t_.at(i).at(0) = (t_.at(i).at(0) - r_min)/ scale_;
    t_.at(i).at(1) = (t_.at(i).at(1) - r_min)/ scale_;
    t_.at(i).at(2) = (t_.at(i).at(2) - r_min)/ scale_;
  }

}

void FastGaussTransform3D::fast_transform() {

  /* array of list of source and target points in each box */
  Bs_.assign(N_side_*N_side_*N_side_,Box3(0,{0,0,0}));
  Bt_.assign(N_side_*N_side_*N_side_,Box3(0,{0,0,0}));  


  /* check for maximal value of epsilon */
  if ( epsilon_ > 0.1 ) 
  { 
    fprintf(stderr,"  Warning: epsilon = %g is too large, using epsilon = 0.1 instead\n",epsilon_);
    epsilon_ = 0.1;
  }

  /* number of neighbours in each dimension in which to compute the gaussians */
  n_ = (int)sqrt(-0.5/r_/r_*log(epsilon_)); /* nbr box span in each direction */

  // printf("  N_side = %d, r = %g, n = %d, scale = %g (new delta = %g)\n",N_side_, r_, n_, scale_, delta_);

  /* Bound on error 
   *
   * E_T ~ E_H <= Q*(1/p!)^(1/2)*(1 - (1 - r^p)^d)/(1 - r)^d 
   *
   * Choose Hermite/Taylor expansion order p_ such 
   * that E_T and E_H < Q*epsilon
   * 
   */
  p_ = 1;
  while ( - 0.5*lgamma(p_ + 1) + log(1 - ipow(1 - ipow(r_,p_),d_)) - d_*log(1-r_) > log(epsilon_) )  
  {
    p_++;
    if ( p_ > 11 )
    {
      fprintf(stderr,"  Error: Hermite expansion order too large.\n");
      fprintf(stderr,"  Setting p_ = 11. Error may be above tolerance epsilon = %g\n", epsilon_);
      p_ = 11;
      break;
    }
  }

  // printf("  p = %d, RelE_tot = %g, eps = %g\n", p_, \
  //  sqrt(1.0/factorial_[p_])*(1 - ipow(1 - ipow(r_,p_),d_))/ipow(1 - r_,d_),epsilon_);

  /* N_F, M_L_: bounds on numbers of sources/targets to use 
   * Hermite or Taylor expansions
   * Greengard proposes O(p_^(d-1)), but 
   * TODO: get a more optimal threshold
   */
  N_F_ = (int)ipow(p_,d_-1);
  M_L_ = (int)ipow(p_,d_-1);
  // printf("  N_F = M_L_ = %d\n\n",N_F_);

  /* list all non empty boxes, non empty source boxes and non empty target boxes */ 
  for ( auto point : s_ ) {
    non_empty_source_list_.insert(p2b(point));
    non_empty_box_list_.insert(p2b(point));
  }
  for ( auto point : t_ ) {
    non_empty_target_list_.insert(p2b(point));
    non_empty_box_list_.insert(p2b(point));
  }

  /* Initialize boxes 
   * range over all non_empty boxes only. 
   * unitilized boxes are never used, so they require no action.
   */
  for ( auto i : non_empty_source_list_ )   {
    Bs_.at(i).set_center({ (i % N_side_ + 0.5f)/N_side_,
                          ( (i / N_side_) % N_side_ + 0.5f)/N_side_,
                          (i / (N_side_*N_side_) + 0.5f)/N_side_});
    Bs_.at(i).set_index(i);
    Bs_.at(i).B().assign(p_*p_*p_,0);
  }
  for ( auto i : non_empty_target_list_ )   {
    Bt_.at(i).set_center({ (i % N_side_ + 0.5f)/N_side_,
                          ( (i / N_side_) % N_side_ + 0.5f)/N_side_,
                          (i / (N_side_*N_side_) + 0.5f)/N_side_});
    Bt_.at(i).set_index(i);

    /* Taylor coefficient for each target box that requires it */
    Bt_.at(i).B().assign(p_*p_*p_,0); 
  }

  /* Distribute source and target points in each box */
  for ( uint_type i = 0; i < N_; i++ ) {
    Bs_.at(p2b(s_.at(i))).p().push_back(i);
  }
  for ( uint_type i = 0; i < M_; i++) {
    Bt_.at(p2b(t_.at(i))).p().push_back(i);
  }
  
  /* A_alpha: Hermite coefficient of far field (source) expansion */
  A_alpha_.assign(p_*p_*p_, 0.0);

  /* Main loop */
  /* range over all non-empty source boxes */
  for ( auto i : non_empty_source_list_ )   
  {
      size_t N_B = Bs_.at(i).n(); /* nbr sources in Box i */

      form_interaction_list(i); 
      if ( N_B < N_F_ )                          /* Source Box sends out N_B Gaussians */
      {
        for ( auto k : ilist_ )  /* range over target boxes in range */
        {
          size_t Mc = Bt_.at(k).n();         /* nbr of targets in k-th box */
          if ( Mc == 0 ) continue;
          if ( Mc <= M_L_ )                      /* few targets. C evaluates all fields immediately */
          {
            direct_direct(Bs_.at(i),Bt_.at(k));
          }
          else                                  /* Mc > M_L_: many targets. C transforms all fields to Taylor series */
          {
            /* Bt[ilist.l[k]].B points to the Taylor coefficients
             * of target Box with index ilist.l[k] 
             * direct_taylor accumulates the Taylor coefficients for box ilist.l[k]
             */
            direct_taylor(Bs_.at(i), Bt_.at(k));
            taylor_box_list_.insert(k);
          }

        }
      }
      else                                      /* B sends out a Hermite expansion */
      {
        /* compute A_alpha: the _p^3 Hermite expansion coefficients */
        hermite_coeffs(Bs_.at(i));   
        for ( auto k : ilist_ )  /* range over target boxes in range */
        {
          size_t Mc = Bt_.at(k).n();         /* nbr of targets in k-th box */
          if ( Mc == 0 ) continue;
          if ( Mc <= M_L_ )                      /* few targets. C evaluates all fields immediately */
          {
            /* direct evaluation of the _p-th order Hermite expansion */ 
            hermite_direct(Bs_.at(i).center(), Bt_.at(k));
          }
          else                                  /* Mc > M_L_: many targets. C transforms all fields to Taylor series */
          {
            /* accumulate Taylor coefficient from Hermite expansion */  
            hermite_taylor(Bt_.at(k), Bs_.at(i).center());
            taylor_box_list_.insert(k);
          }
        }
      }
  }

  /* evaluate Taylor expansion at target points */ 
  /* range over all non empty target boxes */
  for ( auto i : taylor_box_list_ )   
  {
    size_t Mc = Bt_.at(i).n(); /* nbr of targets in i-th target box */
    if ( Mc > M_L_ )
    {
      for ( auto k : Bt_.at(i).p() )   /* range over all targets in box i */
      {
        for ( uint_type beta1 = 0; beta1 < p_; beta1++)  
        {
          for ( uint_type beta2 = 0; beta2 < p_; beta2++)
          {
            for ( uint_type beta3 = 0; beta3 < p_; beta3++)
            {
              G_.at(k) += Bt_.at(i).B().at(beta1*p_*p_ + beta2*p_ + beta3)* \
                               ipow((t_.at(k).at(0) - Bt_.at(i).center().at(0))/sqrt(delta_),beta1)* \
                               ipow((t_.at(k).at(1) - Bt_.at(i).center().at(1))/sqrt(delta_),beta2)* \
                               ipow((t_.at(k).at(2) - Bt_.at(i).center().at(2))/sqrt(delta_),beta3);
            }
          }
        }
      }
    }
  }

} /* endof fast_gaussian_transform_3d */

void FastGaussTransform3D::finish_transform() {

  unsigned int i = 0;
  for (Cell* cell : *cell_list_) {
    unsigned int nbr_targets = cell->gaussian_field_targets(signal_).size();
    std::vector<real_type> value = std::vector<real_type>(G().begin() + i, G().begin() + i + nbr_targets);
    cell->AddGaussianField(signal_, value); 
    i += nbr_targets;
  }


  q_.clear();
  s_.clear();
  t_.clear();
  G_.clear();
  Gexact_.clear();
  Bs_.clear();
  Bt_.clear();
  A_alpha_.clear();
  ilist_.clear();
  taylor_box_list_.clear();
  non_empty_source_list_.clear(); 
  non_empty_target_list_.clear(); 
  non_empty_box_list_.clear(); 



}

void FastGaussTransform3D::direct_transform() {

  if ( N_ == 0 ) { return; }

  for ( uint_type_ i = 0; i < M_; i++)
  {
    for ( uint_type_ j = 0; j < N_; j++)
    {
      G()[i] += q_[j]*exp(-dist2(s_[j],t_[i])/delta_);
    }
  }

}

/** form_interaction_list: list the indices of target boxes 
  * around box i in a neighbourhood of size n around it.
  * The list is stored in ls->l.
  */
void FastGaussTransform3D::form_interaction_list(uint_type i)
{
  uint_type ix = i % N_side_; /* center box in (i,j,k) coordinates */
  uint_type iy = (i / N_side_) % N_side_; /* center box in (i,j,k) coordinates */
  uint_type iz = i / (N_side_*N_side_);
  uint_type x0 = n_ > ix ? 0 : ix - n_;
  uint_type x1 = min(ix + n_, N_side_ - 1) ;
  uint_type y0 = n_ > iy ? 0 : iy - n_;
  uint_type y1 = min(iy + n_, N_side_ - 1);
  uint_type z0 = n_ > iz ? 0 : iz - n_;
  uint_type z1 = min(iz + n_, N_side_ - 1);
  // uint_type nn = (x1 - x0 + 1)*(y1 - y0 + 1)*(z1 - z0 + 1); /* nbr of n-neighbors */

  ilist_.erase(ilist_.begin(),ilist_.end());

  for ( uint_type kz = z0; kz <= z1; kz++)
  {
    for ( uint_type ky = y0; ky <= y1; ky++)
    {
      for ( uint_type kx = x0; kx <= x1; kx++)
      {
        ilist_.insert(kx + N_side_*ky + N_side_*N_side_*kz);
      }
    }
  }

}

/** direct_direct: direct evaluation of gaussian field from 
 *  box _source_ to box _target_. given sources _s_, targets _t_,
 *  and weights _q_.
 */
void FastGaussTransform3D::direct_direct(Box3 source, Box3 target)
{
  for ( auto i : target.p() )
  {
    for (auto j : source.p() )
    {
      G_.at(i) += q_.at(j)*exp(-dist2(s_.at(j),t_.at(i))/delta_);
    }
  }

  evals_.at(0)++;
}


/** direct_taylor: accumulates Taylor coefficients in
 *  the array _B_, given the subset of sources _s_ with
 *  weights _q_, listed in box _source_, 
 *  about target center _tC_.
 *  N_B Gaussians into Taylor series to order p_ - 1
 *  G(t) = q*exp(-|t-s|^2/delta_)
 *  G(t) = sum_{beta>0} B_beta ((t-tc)/sqrt delta_)^beta
 *  B_beta = qj (-1)^|beta|/beta! h_beta((sj-tc)/sqrt delta_)
 */
void FastGaussTransform3D::direct_taylor(Box3 source, Box3& target)
{
  real_type taylor_coeff;

  /* compute Taylor coefficients B */
  for ( uint_type beta1 = 0; beta1 < p_; beta1++)  
  {
    for ( uint_type beta2 = 0; beta2 < p_; beta2++)
    {
      for ( uint_type beta3 = 0; beta3 < p_; beta3++)
      {
        taylor_coeff = 0.0;
        for ( auto j : source.p() )
        {
          taylor_coeff += q_.at(j)*Hermite::Function((s_.at(j).at(0) - target.center().at(0))/sqrt(delta_),beta1)* \
                                   Hermite::Function((s_.at(j).at(1) - target.center().at(1))/sqrt(delta_),beta2)* \
                                   Hermite::Function((s_.at(j).at(2) - target.center().at(2))/sqrt(delta_),beta3);
        }
        taylor_coeff /= factorial_[beta1]*factorial_[beta2]*factorial_[beta3];
        target.B().at(beta1*p_*p_ + beta2*p_ + beta3) += taylor_coeff; /* accumulate Taylor coefficient in B for later evaluation */
      }
    }
  }

  evals_.at(1)++;
}


/** hermite_coeffs
 * Compute Hermite expansion up to order p_ - 1 at source _box_,
 * given sources _s_, weights _q_, box center _sB_.
 * Coefficients are stored in A.
 */
void FastGaussTransform3D::hermite_coeffs(Box3 source)
{
  real_type a;

  /* form Hermite coefficients O(p^d*N) */
  for ( uint_type alpha1 = 0; alpha1 < p_; alpha1++)  
  {
    for ( uint_type alpha2 = 0; alpha2 < p_; alpha2++)
    {
      for ( uint_type alpha3 = 0; alpha3 < p_; alpha3++)
      {
        a = 0.0;
        for ( auto j : source.p() ) /* range over source points in source box */
        {
          a += q_.at(j)*ipow((s_.at(j).at(0) - source.center().at(0))/sqrt(delta_),alpha1)* \
                        ipow((s_.at(j).at(1) - source.center().at(1))/sqrt(delta_),alpha2)* \
                        ipow((s_.at(j).at(2) - source.center().at(2))/sqrt(delta_),alpha3);
        }
        a /= (factorial_[alpha1]*factorial_[alpha2]*factorial_[alpha3]);
        A_alpha_.at(alpha1*p_*p_ + alpha2*p_ + alpha3) = a;
      }
    }
  }
}

/** hermite_direct: Evaluate the Hermite expansion at each target point
 * in box _target_, with targets _t_, from the Hermite expansion of 
 * source box with center _sB_, and Hermite coefficients _A_alpha_. Accumulates
 * the Gaussian field in _G_.
 * G(t) = Sum_sourceBoxe Sum_sourcePoint A_alpha Hermite::Function(t - s_B) 
 * Few targets, many sources.
 */
void FastGaussTransform3D::hermite_direct(point_type sB, Box3 target )
{
  real_type c1, c2, c3;

  /* evaluate the Hermite series */
  for ( auto i : target.p() )   /* range over all targets in target box */
  {
    for ( uint_type alpha1 = 0; alpha1 < p_; alpha1++)  
    {
      c1 = Hermite::Function((t_.at(i).at(0) - sB.at(0))/sqrt(delta_),alpha1);
      for ( uint_type alpha2 = 0; alpha2 < p_; alpha2++)
      {
        c2 = Hermite::Function((t_.at(i).at(1) - sB.at(1))/sqrt(delta_),alpha2);
        for ( uint_type alpha3 = 0; alpha3 < p_; alpha3++)
        {
          c3 = Hermite::Function((t_.at(i).at(2) - sB.at(2))/sqrt(delta_),alpha3);
          G_.at(i) += A_alpha_.at(alpha1*p_*p_ + alpha2*p_ + alpha3)*c1*c2*c3;
        }
      }
    }
  }

  evals_.at(2)++;
}

/** hermite_taylor: accumulate taylor coefficients into array _B_ 
 * from precomputed Hermite expansion _A_ 
 * of box centered at _sB_, and target box centered at _tC_ 
 * Many sources, many targets
 */
void FastGaussTransform3D::hermite_taylor(Box3& target, point_type sB)
{
  real_type taylor_coeff;
  vector<real_type> h1(2*p_ - 1, 0.0);
  vector<real_type> h2(2*p_ - 1, 0.0);
  vector<real_type> h3(2*p_ - 1, 0.0);

  /* precompute Hermite functions h_{alpha + beta} */
  for ( uint_type k = 0; k < 2*p_ - 1; k++ )
  {
    /* in the paper, the argument of Hermite::Function is inverted: sB - tC instead of tC - sB */
    h1.at(k) = Hermite::Function((target.center().at(0) - sB.at(0))/sqrt(delta_),k);
    h2.at(k) = Hermite::Function((target.center().at(1) - sB.at(1))/sqrt(delta_),k);
    h3.at(k) = Hermite::Function((target.center().at(2) - sB.at(2))/sqrt(delta_),k);
  }

  /* compute Taylor coefficients B O(d*p^(d+1))?? */
  for ( uint_type beta1 = 0; beta1 < p_; beta1++)  
  {
    for ( uint_type beta2 = 0; beta2 < p_; beta2++)
    {
      for ( uint_type beta3 = 0; beta3 < p_; beta3++)
      {
        taylor_coeff = 0.0;
        for ( uint_type alpha1 = 0; alpha1 < p_; alpha1++)  
        {
          for ( uint_type alpha2 = 0; alpha2 < p_; alpha2++)
          {
            for ( uint_type alpha3 = 0; alpha3 < p_; alpha3++)
            {
              taylor_coeff += A_alpha_.at(alpha1*p_*p_ + alpha2*p_ + alpha3)* \
                              h1[alpha1 + beta1]* \
                              h2[alpha2 + beta2]* \
                              h3[alpha3 + beta3];
            }
          }
        }

        if ( (beta1 + beta2 + beta3) % 2 )
          taylor_coeff *= -1;

        taylor_coeff /= (factorial_[beta1]*factorial_[beta2]*factorial_[beta3]);

        target.B().at(beta1*p_*p_ + beta2*p_ + beta3) += taylor_coeff; /* accumulate Taylor coefficient in B for later evaluation */
      }
    }
  }

  evals_.at(3)++;
}


real_type FastGaussTransform3D::ipow(const real_type x, uint_type n) {

  real_type z = 1.0;
  while ( n-- )
  {
    z *= x;
  }
  return z;

}

real_type FastGaussTransform3D::dist2(point_type p1, point_type p2)
{
  return  (p1.at(0) - p2.at(0))*(p1.at(0) - p2.at(0)) + \
          (p1.at(1) - p2.at(1))*(p1.at(1) - p2.at(1)) + \
          (p1.at(2) - p2.at(2))*(p1.at(2) - p2.at(2));
}


/** p2b: returns the box linear index containing point p */
uint_type FastGaussTransform3D::p2b(point_type p)
{
  size_t ix = (size_t)(N_side_*p.at(0));
  size_t iy = (size_t)(N_side_*p.at(1));
  size_t iz = (size_t)(N_side_*p.at(2));
  size_t b = ix + N_side_*iy + N_side_*N_side_*iz;
  if ( b < N_side_*N_side_*N_side_ )
  {
    return b;
  }
  else
  {
    fprintf(stderr,"  Error in p2b: index exceeds box number\n");
    fprintf(stderr,"  index: %zu, point (%g,%g,%g)\n", b, p.at(0), p.at(1), p.at(2));
    exit(1);
  }
}

void FastGaussTransform3D::Save(gzFile backup_file) const {

  uint16_t size = using_diffusive_signals_.size();
  uint16_t i = 0;
  gzwrite(backup_file,&size,sizeof(size));
  for ( auto signal : using_diffusive_signals_ ) {
    uint16_t int_sig = static_cast<uint16_t>(signal);
    gzwrite(backup_file,&int_sig,sizeof(int_sig)); 
    gzwrite(backup_file, &diffusive_delta_[i], sizeof(diffusive_delta_[i]));
    gzwrite(backup_file, &diffusive_epsilon_[i], sizeof(diffusive_epsilon_[i]));
    i++;
  }
}

void FastGaussTransform3D::Load(gzFile backup_file) {
  uint16_t size;
  gzread(backup_file,&size,sizeof(size));
  for ( uint16_t i = 0; i < size; ++i ) {
    uint16_t int_sig;
    real_type val;
    gzread(backup_file,&int_sig,sizeof(int_sig)); 
    using_diffusive_signals_.push_back(static_cast<InterCellSignal>(int_sig));
    gzread(backup_file, &val, sizeof(val));
    diffusive_delta_.push_back(val);
    gzread(backup_file, &val, sizeof(val));
    diffusive_epsilon_.push_back(val);
  }

}

