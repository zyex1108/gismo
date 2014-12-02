/** @file gsBoundaryConditions.h

    @brief Provides gsBoundaryConditions class.

    This file is part of the G+Smo library.

    This Source Code Form is subject to the terms of the Mozilla Public
    License, v. 2.0. If a copy of the MPL was not distributed with this
    file, You can obtain one at http://mozilla.org/MPL/2.0/.

    Author(s): A. Mantzaflaris
*/

#pragma once

#include <gsCore/gsBoundary.h>


namespace gismo
{

    template <class T> class gsFunction;

/** 
    Class that defines a Boundary single condition for a side of a
    patch for some unknown Pde variable.
*/
template<class T>
struct boundary_condition
{
    boundary_condition( int p, boundary::side s, gsFunction<T> * f, boundary::type t, int unknown = 0)
	: ps(p, s), m_function(f), m_type(t), m_unknown(unknown) { }
      
    boundary_condition( int p, boundary::side s, boundary::type t, int unknown = 0)
	: ps(p, s), m_function(NULL), m_type(t), m_unknown(unknown)  { }
      
    /// homogeneous boundary condition ?
    bool isHomogeneous()       const { return m_function == 0; }

    /// Returns the function data of the boundary condition
    gsFunction<T> * function() const { return m_function; }

    /// Returns the type of the boundary condition
    boundary::type  type()     const { return m_type; }

    /// Returns the patch to which this boundary condition refers to
    int             patch()    const { return ps.patch; }

    /// Returns the side to which this boundary condition refers to
    boundary::side  side()     const { return ps.side; }

    /// Returns the unknown to which this boundary condition refers to
    int             unknown()  const { return m_unknown; }


    patch_side ps;
    gsFunction<T> * m_function;
    boundary::type m_type;// TO DO : robin coefficients?
    int m_unknown;
};
    
/** @brief
    Class containing a set of  boundary conditions.
    
    The boundary conditions are stored in the form of a list of boundary_condition
    instances.
*/

template<class T>
class gsBoundaryConditions 
{
    
public:

    typedef typename std::vector<boundary_condition<T> >::iterator iterator;
    typedef typename std::vector<boundary_condition<T> >::const_iterator const_iterator;

public:

    /// Default empty constructor
    gsBoundaryConditions() 
    { }

    ~gsBoundaryConditions() // Destructor
    { }
    
public:

    /// Return a reference to the Dirichlet sides 
    const std::vector<boundary_condition<T> > & dirichletSides() const {return drchlt_sides; }

    /// Return a reference to the Neumann sides 
    const std::vector<boundary_condition<T> > & neumannSides()   const {return nmnn_sides;   }

    /// Return a reference to the Dirichlet sides 
    const std::vector<boundary_condition<T> > & robinSides()     const {return robin_sides;  }

    std::vector<boundary_condition<T> > allConditions() const
    {
        std::vector<boundary_condition<T> > all;
        all.reserve( drchlt_sides.size()+nmnn_sides.size()+robin_sides.size());
        all.insert( all.end(), drchlt_sides.begin(), drchlt_sides.end() );
        all.insert( all.end(), nmnn_sides.begin()  , nmnn_sides.end()   );
        all.insert( all.end(), robin_sides.begin() , robin_sides.end()  );
        return all;
    }
    
    /// Get a const-iterator to the beginning of the Dirichlet sides
    /// \return an iterator to the beginning of the Dirichlet sides
    const_iterator dirichletBegin() const
	{ return drchlt_sides.begin(); }
    
    /// Get a const-iterator to the end of the Dirichlet sides
    /// \return an iterator to the end of the Dirichlet sides
    const_iterator dirichletEnd() const
	{ return drchlt_sides.end(); }
    
    /// Get an iterator to the beginning of the Dirichlet sides
    /// \return an iterator to the beginning of the Dirichlet sides
    iterator dirichletBegin()
	{ return drchlt_sides.begin(); }
    
    /// Get an iterator to the end of the Dirichlet sides
    /// \return an iterator to the end of the Dirichlet sides
    iterator dirichletEnd()
	{ return drchlt_sides.end(); }

    /// Get a const-iterator to the beginning of the Neumann sides
    /// \return an iterator to the beginning of the Neumann sides
    const_iterator neumannBegin() const
	{ return nmnn_sides.begin(); }
    
    /// Get a const-iterator to the end of the Neumann sides
    /// \return an iterator to the end of the Neumann sides
    const_iterator neumannEnd() const
	{ return nmnn_sides.end(); }
    
    /// Get an iterator to the beginning of the Neumann sides
    /// \return an iterator to the beginning of the Neumann sides
    iterator neumannBegin()
	{ return nmnn_sides.begin(); }
    
    /// Get an iterator to the end of the Neumann sides
    /// \return an iterator to the end of the Neumann sides
    iterator neumannEnd()
	{ return nmnn_sides.end(); }

    /// Get a const-iterator to the beginning of the Robin sides
    /// \return an iterator to the beginning of the Robin sides
    const_iterator robinBegin() const
	{ return robin_sides.begin(); }
    
    /// Get a const-iterator to the end of the Robin sides
    /// \return an iterator to the end of the Robin sides
    const_iterator robinEnd() const
	{ return robin_sides.end(); }
    
    /// Get an iterator to the beginning of the Robin sides
    /// \return an iterator to the beginning of the Robin sides
    iterator robinBegin()
	{ return robin_sides.begin(); }
    
    /// Get an iterator to the end of the Robin sides
    /// \return an iterator to the end of the Robin sides
    iterator robinEnd()
	{ return robin_sides.end(); }
    
    void addCondition(int p, boundary::side s, boundary::type t, gsFunction<T> * f, int unknown = 0)
    {
        switch (t) {
        case boundary::dirichlet :
            drchlt_sides.push_back( boundary_condition<T>(p,s,f,t,unknown) );
            break;
        case boundary::neumann :
            nmnn_sides.push_back( boundary_condition<T>(p,s,f,t,unknown) );
            break;
        case boundary::robin :
            robin_sides.push_back( boundary_condition<T>(p,s,f,t,unknown) );
            break;
        default:
            std::cout<<"gsBoundaryConditions: Unknown boundary condtion.\n";
        }
    }

    void addCondition( boundary::side s, boundary::type t, gsFunction<T> * f, int unknown = 0)
        {
            // for single-patch only
            addCondition(0,s,t,f,unknown);
        }

    void addCondition(const patch_side& ps, boundary::type t, gsFunction<T> * f, int unknown = 0)
        {
            addCondition(ps.patch, ps.side, t, f, unknown);
        }

  /// Prints the object as a string.
  std::ostream &print(std::ostream &os) const
    { 
        os << "gsBoundaryConditions :\n";
        os << "* Dirichlet boundaries: "<< drchlt_sides.size() <<"\n";
        os << "* Neumann boundaries  : "<< nmnn_sides.size() <<"\n";
        return os; 
    };

// Data members
private:

    std::vector<boundary_condition<T> > drchlt_sides;
    std::vector<boundary_condition<T> > nmnn_sides;
    std::vector<boundary_condition<T> > robin_sides;

}; // class gsBoundaryConditions


//////////////////////////////////////////////////
//////////////////////////////////////////////////

/// Print (as string)
template<class T>
std::ostream &operator<<(std::ostream &os, const gsBoundaryConditions<T>& bvp)
{return bvp.print(os); };
    
}; // namespace gismo