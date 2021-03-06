// *****************************************************************************
/*!
  \file      src/PDE/CompFlow/RiemannSolver.h
  \copyright 2016-2018, Los Alamos National Security, LLC.
  \brief     Riemann solver interface class for various Riemann solvers
  \details   This file defines a generic Riemann solver interface class. The
    class uses runtime polymorphism without client-side inheritance: inheritance
    is confined to the internals of the class, invisible to client-code. The
    class exclusively deals with ownership enabling client-side value semantics.
    Credit for this idea goes to Sean Parent at Adobe:
    https://github.com/sean-parent/sean-parent.github.com/wiki/
    Papers-and-Presentations.
*/
// *****************************************************************************
#ifndef RiemannSolver_h
#define RiemannSolver_h

#include <array>
#include <vector>

#include "Types.h"
#include "Make_unique.h"
#include "Fields.h"

namespace inciter {

//! \brief Generic Riemann solver interface class for various Riemann solvers
//! \details This class uses runtime polymorphism without client-side
//!   inheritance: inheritance is confined to the internals of this class,
//!   invisble to client-code. The class exclusively deals with ownership
//!   enabling client-side value semantics. Credit for this idea goes to Sean
//!   Parent at Adobe:
//!   https://github.com/sean-parent/sean-parent.github.com/wiki/
//!   Papers-and-Presentations.
class RiemannSolver {

  public:
    //! \brief Constructor taking an object modeling Concept
    //! \details The object of class T comes pre-constructed.
    //! \param[in] x Instantiated object of type T given by the template
    //!   argument.
    template< typename T > explicit RiemannSolver( T x ) :
      self( tk::make_unique< Model<T> >( std::move(x) ) ) {}

    //! \brief Constructor taking a function pointer (std::function) to a
    //!   constructor of an object modeling Concept.
    //! \details Passing std::function allows late execution of the constructor,
    //!   i.e., as late as inside this class' constructor, and thus usage from
    //!   a factory. Note that there are at least two different ways of using
    //!   this constructor:
    //!   - Bind T's constructor arguments and place it in std::function<T()>
    //!   and passing no arguments as args.... This case then instantiates the
    //!   model via its constructor and stores it in here.
    //!   - Bind a single placeholder argument to T's constructor and pass it in
    //!   as host's args..., which then forwards it to model's constructor. This
    //!   allows late binding, i.e., binding the argument only here.
    //! \see See also the wrapper tk::recordModel() which does the former for
    //!   arbitrary number of model constructor arguments and
    //!   tk::recordModelLate() for a single model constructor argument which
    //!   does the latter, both defined in src/Base/Factory.h.
    //! \param[in] x Function pointer (std::function) to a constructor of an
    //!    object modeling Concept.
    //! \param[in] args Zero or more constructor arguments
    template< typename T, typename...Args >
    explicit RiemannSolver( std::function<T(Args...)> x, Args&&... args ) :
      self( tk::make_unique< Model<T> >(
              std::move( x( std::forward<Args>(args)... ) ) ) ) {}

    //! Public interface to computing the Riemann flux
    std::vector< tk::real >
    flux( std::size_t f,
          const tk::Fields& geoFace,
          const std::array< std::vector< tk::real >, 2 >& u ) const
    { return self->flux( f, geoFace, u ); }

    //! Copy assignment
    RiemannSolver& operator=( const RiemannSolver& x )
    { RiemannSolver tmp(x); *this = std::move(tmp); return *this; }
    //! Copy constructor
    RiemannSolver( const RiemannSolver& x ) : self( x.self->copy() ) {}
    //! Move assignment
    RiemannSolver& operator=( RiemannSolver&& ) noexcept = default;
    //! Move constructor
    RiemannSolver( RiemannSolver&& ) noexcept = default;

  private:
    //! \brief Concept is a pure virtual base class specifying the requirements
    //!   of polymorphic objects deriving from it, i.e., those functions that
    //!   can be overridden
    struct Concept {
      Concept() = default;
      Concept( const Concept& ) = default;
      virtual ~Concept() = default;
      virtual Concept* copy() const = 0;
      virtual std::vector< tk::real >
        flux( std::size_t,
              const tk::Fields&,
              const std::array< std::vector< tk::real >, 2 >& ) const = 0;
    };

    //! \brief Model models the Concept above by deriving from it and overriding
    //!   the virtual functions required by Concept
    template< typename T >
    struct Model : Concept {
      Model( T x ) : data( std::move(x) ) {}
      Concept* copy() const override { return new Model( *this ); }
      std::vector< tk::real >
        flux( std::size_t f,
              const tk::Fields& geoFace,
              const std::array< std::vector< tk::real >, 2 >& u ) const override
      { return data.flux( f, geoFace, u ); }
      T data;
    };

    std::unique_ptr< Concept > self;    //!< Base pointer used polymorphically
};

} // inciter::

#endif // RiemannSolver_h
