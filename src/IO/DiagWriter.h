// *****************************************************************************
/*!
  \file      src/IO/DiagWriter.h
  \copyright 2012-2015, J. Bakosi, 2016-2018, Los Alamos National Security, LLC.
  \brief     Text diagnostics writer declaration
  \details   This file declares the ASCII diagnostics writer class that
     facilitates outputing diagnostics to text files.
*/
// *****************************************************************************
#ifndef DiagWriter_h
#define DiagWriter_h

#include <string>
#include <vector>
#include <fstream>

#include "Types.h"
#include "Writer.h"
#include "Keywords.h"
#include "Options/TxtFloatFormat.h"

namespace tk {

//! \brief DiagWriter : tk::Writer
//! \details ASCII diagnostics writer class that facilitates outputing
//!   diagnostics to text files.
class DiagWriter : public tk::Writer {

  public:
    //! Constructor
    explicit DiagWriter(
      const std::string& filename,
      tk::ctr::TxtFloatFormatType format = tk::ctr::TxtFloatFormatType::DEFAULT,
      kw::precision::info::expect::type precision = std::cout.precision(),
      std::ios_base::openmode mode = std::ios_base::out );

    //! Write out diagnostics file header
    void header( const std::vector< std::string >& name ) const;

    //! Write diagnostics file
    std::size_t diag( uint64_t it,
                      tk::real t,
                      tk::real dt,
                      const std::vector< tk::real >& diagnostics );

  private:
    int m_precision;    //!< Floating-point precision in digits
    int m_width;        //!< Floating-point number width
};

} // tk::

#endif // DiagWriter_h
