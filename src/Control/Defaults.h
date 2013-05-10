//******************************************************************************
/*!
  \file      src/Control/Defaults.h
  \author    J. Bakosi
  \date      Thu May  9 19:12:43 2013
  \copyright Copyright 2005-2012, Jozsef Bakosi, All rights reserved.
  \brief     Defaults for control
  \details   Defaults for control
*/
//******************************************************************************
#ifndef Defaults_h
#define Defaults_h

#include <limits>

#include <ControlTypes.h>

using namespace std;

namespace Quinoa {

namespace control {

//! Default bundle for parsed data
const Bundle DEFAULTS(
  "",                         //!< Title
  NO_PHYSICS,                 //!< Physics
  NO_POSITION,                //!< Position
  NO_HYDRO,                   //!< Hydrodynamics model
  NO_MIX,                     //!< Material mix model
  numeric_limits<int>::max(), //!< Number of time steps to take
  1.0,                        //!< Time to terminate time stepping
  0.5,                        //!< Size of time step
  3,                          //!< Number of position components
  3,                          //!< Number of velocity components
  1,                          //!< Number of scalar components
  1,                          //!< Total number of particles
  1,                          //!< TTY output interval
  0,                          //!< Dump output interval
  0,                          //!< Plot output interval
  1,                          //!< PDF output interval
  1,                          //!< Glob output interval
  "jpdf",                     //!< Default jpdf base filename
  "glob",                     //!< Default glob filename
  "plot",                     //!< Default plot base filename
  vector<real>(),             //!< Parameters 'b'
  vector<real>(),             //!< Paramaters 'S'
  vector<real>(),             //!< Parameters 'kappa'
  vector<real>(),             //!< Parameters 'c_ij'
  2.1,                        //!< Parameter C0
  vector<Product>()           //!< Statistics
);

} // namespace control

} // namespace Quinoa

#endif // Defaults_h
