// *****************************************************************************
/*!
  \file      src/PDE/CompFlow/Riemann/HLLC.h
  \copyright 2016-2018, Los Alamos National Security, LLC.
  \brief     Harten-Lax-van Leer-Contact (HLLC) Riemann flux function
  \details   This file implements the Harten-Lax-van Leer-Contact (HLLC) Riemann
             solver.
*/
// *****************************************************************************
#ifndef HLLC_h
#define HLLC_h

#include <vector>

#include "Types.h"
#include "Fields.h"
#include "Inciter/Options/Flux.h"

namespace inciter {

//! HLLC approximate Riemann solver
//! \details This class is used polymorphically with inciter::RiemannSolver
struct HLLC {
  //! HLLC approximate Riemann solver flux function
  //! \param[in] f Face ID
  //! \param[in] geoFace Face geometry array
  //! \param[in] u Left and right unknown/state vector
  //! \return Riemann solution using a central difference method
  std::vector< tk::real >
  flux( std::size_t f,
        const tk::Fields& geoFace,
        const std::array< std::vector< tk::real >, 2 >& u ) const
  {
    std::vector< tk::real > flx( u[0].size(), 0 );

    std::array< tk::real, 3 > fn {{ geoFace(f,1,0),
                                    geoFace(f,2,0),
                                    geoFace(f,3,0) }};

    // ratio of specific heats
    auto g = g_inputdeck.get< tag::param, tag::compflow, tag::gamma >()[0];

    // Primitive variables
    auto rhol = u[0][0];
    auto rhor = u[1][0];

    auto pl = (g-1.0)*(u[0][4] - (u[0][1]*u[0][1] +
                                  u[0][2]*u[0][2] +
                                  u[0][3]*u[0][3]) / (2.0*rhol));

    auto pr = (g-1.0)*(u[1][4] - (u[1][1]*u[1][1] +
                                  u[1][2]*u[1][2] +
                                  u[1][3]*u[1][3]) / (2.0*rhor));

    auto al = sqrt(g * pl / rhol);
    auto ar = sqrt(g * pr / rhor);

    // Face-normal velocities
    auto ul = u[0][1]/rhol;
    auto vl = u[0][2]/rhol;
    auto wl = u[0][3]/rhol;

    tk::real vnl = ul*fn[0] + vl*fn[1] + wl*fn[2];

    auto ur = u[1][1]/rhor;
    auto vr = u[1][2]/rhor;
    auto wr = u[1][3]/rhor;

    tk::real vnr = ur*fn[0] + vr*fn[1] + wr*fn[2];

    // Roe-averaged variables
    auto rlr = sqrt(rhor/rhol);
    auto rlr1 = 1.0 + rlr;

    auto vnroe = (vnr*rlr + vnl)/rlr1 ;
    auto aroe = (ar*rlr + al)/rlr1 ;

    // Signal velocities
    auto Sl = fmin(vnl-al, vnroe-aroe);
    auto Sr = fmax(vnr+ar, vnroe+aroe);
    auto Sm = ( rhor*vnr*(Sr-vnr) - rhol*vnl*(Sl-vnl) + pl-pr )
             /( rhor*(Sr-vnr) - rhol*(Sl-vnl) );

    // Middle-zone (star) variables
    auto pStar = rhol*(vnl-Sl)*(vnl-Sm) + pl;
    auto uStar = u;

    uStar[0][0] = (Sl-vnl) * rhol/ (Sl-Sm);
    uStar[0][1] = ((Sl-vnl) * u[0][1] + (pStar-pl)*fn[0]) / (Sl-Sm);
    uStar[0][2] = ((Sl-vnl) * u[0][2] + (pStar-pl)*fn[1]) / (Sl-Sm);
    uStar[0][3] = ((Sl-vnl) * u[0][3] + (pStar-pl)*fn[2]) / (Sl-Sm);
    uStar[0][4] = ((Sl-vnl) * u[0][4] - pl*vnl + pStar*Sm) / (Sl-Sm);

    uStar[1][0] = (Sr-vnr) * rhor/ (Sr-Sm);
    uStar[1][1] = ((Sr-vnr) * u[1][1] + (pStar-pr)*fn[0]) / (Sr-Sm);
    uStar[1][2] = ((Sr-vnr) * u[1][2] + (pStar-pr)*fn[1]) / (Sr-Sm);
    uStar[1][3] = ((Sr-vnr) * u[1][3] + (pStar-pr)*fn[2]) / (Sr-Sm);
    uStar[1][4] = ((Sr-vnr) * u[1][4] - pr*vnr + pStar*Sm) / (Sr-Sm);

    // Numerical fluxes
    if (Sl > 0.0) {
      flx[0] = u[0][0] * vnl;
      flx[1] = u[0][1] * vnl + pl*fn[0];
      flx[2] = u[0][2] * vnl + pl*fn[1];
      flx[3] = u[0][3] * vnl + pl*fn[2];
      flx[4] = ( u[0][4] + pl ) * vnl;
    }
    else if (Sl <= 0.0 && Sm > 0.0) {
      flx[0] = uStar[0][0] * Sm;
      flx[1] = uStar[0][1] * Sm + pStar*fn[0];
      flx[2] = uStar[0][2] * Sm + pStar*fn[1];
      flx[3] = uStar[0][3] * Sm + pStar*fn[2];
      flx[4] = ( uStar[0][4] + pStar ) * Sm;
    }
    else if (Sm <= 0.0 && Sr >= 0.0) {
      flx[0] = uStar[1][0] * Sm;
      flx[1] = uStar[1][1] * Sm + pStar*fn[0];
      flx[2] = uStar[1][2] * Sm + pStar*fn[1];
      flx[3] = uStar[1][3] * Sm + pStar*fn[2];
      flx[4] = ( uStar[1][4] + pStar ) * Sm;
    }
    else {
      flx[0] = u[1][0] * vnr;
      flx[1] = u[1][1] * vnr + pr*fn[0];
      flx[2] = u[1][2] * vnr + pr*fn[1];
      flx[3] = u[1][3] * vnr + pr*fn[2];
      flx[4] = ( u[1][4] + pr ) * vnr;
    }

    return flx;
  }

  //! Flux type accessor
  //! \return Flux type
  static ctr::FluxType type() noexcept { return ctr::FluxType::HLLC; }
};

} // inciter::

#endif // HLLC_h
