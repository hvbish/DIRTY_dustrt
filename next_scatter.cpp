// ======================================================================
//   Procedure to scatter a photon into a new direction.  This procedure
// returns a 0 if the photon scatters inside the dust and a 1 if it 
// escapes.
//
// 2004 Dec/KDG - written
// 03 Sep 2015/KDG - added sampling from an exp(tau/100) distribution
//                   1/2 of the time to better sample high optical depth scattering
// ======================================================================
#include "next_scatter.h"
#define DEBUG_NS

int next_scatter (geometry_struct& geometry,
		  photon_data& photon,
		  random_dirty& random_obj)

{
  // determine the optical depth to the next scattering
  double target_tau = 0.0;
  double odd_photon_tau_factor = 10.0;

  if ((photon.number % 2) == 0) { // even photons are classical scattering
    target_tau = -log(random_obj.random_num());
  } else { // odd photons are sampled from exp(-tau/100) to better sample high depths
    //target_tau = -1.*odd_photon_tau_factor*log(random_obj.random_num()/odd_photon_tau_factor);
    target_tau = -1.*odd_photon_tau_factor*log(random_obj.random_num());

    // cout << target_tau << " ";
    // cout << (odd_photon_tau_factor*exp(-target_tau))/exp(-target_tau/odd_photon_tau_factor) << endl;
  }

  photon.target_tau = target_tau;

  // check to see if we will start in a subgrid
  if (photon.current_grid_num > 0) {
#ifdef DEBUG_NS
    if (photon.number == OUTNUM) {
      cout << "starting in a subgrid" << endl;
    }
#endif
    photon.current_grid_num = 0;
  }
  
  // determine the site of the next scattering
  int escape = 0;
  double distance_traveled = 0.0;
  double tau_traveled = 0.0;
  photon.path_cur_cells = 0;  // set to 0 to save cells tranversed

  distance_traveled = calc_photon_trajectory(photon, geometry, target_tau, escape, tau_traveled);
#ifdef DEBUG_NS
  if (photon.number == OUTNUM) {
    cout << "ns cpt done; ";
    cout << "distance_traveled = " << distance_traveled << endl;
    cout << "target_tau = " << target_tau << endl;
    cout << "photon.scat_weight = " << photon.scat_weight << endl;
  }
#endif

//   int j = 0;
//   for (j = 0; j < 3; j++) 
//     cout << photon.position[j] << " ";
//   cout << endl;

  escape = 0;
  // check if the photon has left the dust
  if ((target_tau - tau_traveled) > ROUNDOFF_ERR_TRIG)
    escape = 1;

  // check if the photon has scattered enough and there is just no significant weight left
  if (photon.num_scat > geometry.max_num_scat)
    escape = 1;

  // update the scattered weight for the odd photons
  if ((photon.number % 2) == 1)
    photon.scat_weight *= (odd_photon_tau_factor*exp(-target_tau))/exp(-target_tau/odd_photon_tau_factor);
  //photon.scat_weight *= exp(-target_tau)/exp(-target_tau/odd_photon_tau_factor);

#ifdef DEBUG_NS
  if (photon.number == OUTNUM) {
    cout << "ns escape = " << escape << endl;
  }
#endif
#ifdef DEBUG_NS
  if (photon.number == OUTNUM) {
    if ((target_tau - tau_traveled) < -ROUNDOFF_ERR_TRIG) {
      cout << "*****error*****next_scatter*****" << endl;
      cout << "target_tau = " << target_tau << endl;
      cout << "tau_traveled = " << tau_traveled << endl;
      cout << "diff = " << target_tau - tau_traveled << endl;
    }
  }
#endif
  
  // return escape (1 = yes, 0 = no)
  return(escape);
}
