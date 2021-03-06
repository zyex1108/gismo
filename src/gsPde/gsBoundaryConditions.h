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

/// @brief Specifies the type of boundary condition
///
/// \ingroup Pde
struct condition_type
{
    /// Specifies the type of boundary condition
    enum type
    {
        dirichlet = 0, ///< Dirichlet type
        neumann   = 1, ///< Neumann type
        robin     = 2  ///< Robin type
        //mixed BD means: there are both dirichlet and neumann sides
        //robin: a linear combination of value and derivative
        //cauchy: there are 2 conditions (value+deriv) defined on the same side
    };

};

// Print (as string) a boundary type
inline std::ostream &operator<<(std::ostream &os, const condition_type::type& o)
{
    switch (o)
    {
    case condition_type::dirichlet:
    {
        os<< "Dirichlet";
        break;
    }
    case condition_type::neumann:
    {
        os<< "Neumann";
        break;
    }
    case condition_type::robin:
    {
        os<< "Mixed";
        break;
    }
    default:
        gsInfo<<"condition type not known.\n";
    };
    return os;
}

/** 
    @brief Class that defines a boundary condition for a side of a
    patch for some unknown variable of a PDE.
    
    \todo rename to boundaryCondition
    
    \ingroup Pde
*/
template<class T>
struct boundary_condition
{
    typedef typename gsFunction<T>::Ptr function_ptr;

    boundary_condition( int p, boxSide s, const function_ptr & f_shptr,
                        condition_type::type t, int unknown = 0,
                        bool parametric = false)
        : ps(p, s),
          m_function(f_shptr),
          m_type(t),
          m_unknown(unknown),
          m_parametric(parametric)
    { }

    boundary_condition( int p, boxSide s, gsFunction<T> * f_ptr,
                        condition_type::type t, int unknown = 0,
                        bool parametric = false)
        : ps(p, s),
          m_type(t),
          m_unknown(unknown),
          m_parametric(parametric)
    {
        m_function = function_ptr(f_ptr, memory::null_deleter<gsFunction<T> >);
    }

    boundary_condition( int p, boxSide s, const gsFunction<T> & func,
                        condition_type::type t, int unknown = 0,
                        bool parametric = false)
        : ps(p, s),
          m_function(func.clone()),
          m_type(t),
          m_unknown(unknown),
          m_parametric(parametric)
    {
        //m_function = function_ptr(func.clone());
    }

    boundary_condition( int p, boxSide s, condition_type::type t,
                        int unknown = 0, bool parametric = false)
        : ps(p, s), m_function(NULL), m_type(t), m_unknown(unknown),m_parametric(parametric)
    { }
    
    /// Returns true if there is no function data (homogeneous condition)
    bool isHomogeneous() const { return m_function.get() == NULL; }
    
    /// Returns the function data pointer of the boundary condition
    function_ptr function() const { return m_function; }

    // Returns a reference to the function data
    //const gsFunction<T> & function() const { return *m_function; }

    /// Returns the type of the boundary condition
    condition_type::type  type() const { return m_type; }
    
    /// Returns the patch to which this boundary condition refers to
    int     patch()    const { return ps.patch; }

    /// Returns the side to which this boundary condition refers to
    boxSide side()     const { return ps.side(); }

    /// Returns the unknown to which this boundary condition refers to
    int     unknown()  const { return m_unknown; }

    /// Returns true if the function data for this boundary condition
    /// is defined in parametric coordinates
    bool    parametric()  const { return m_parametric; }


    patchSide ps;                ///< Side of a patch for this boundary condition

    function_ptr m_function;     ///< Function data for this boundary condition

    // TO DO : robin coefficients?

    condition_type::type m_type; ///< Type of the boundary condition

    int m_unknown;               ///< Unknown to which this boundary condition refers to

    bool m_parametric;
};

/** 
    @brief Class prescribing a value related to a corner of a patch
*/
template<class T>
struct corner_value
{
    corner_value(int p, boxCorner c, T v, int unk = 0)
        : patch(p), corner(c), value(v), unknown(unk) { }

    index_t patch;    ///< The index of the patch.
    boxCorner corner; ///< The corner
    T value;          ///< The value
    int   unknown;    ///< Unknown to which this boundary condition refers to
};

/** @brief
    Class containing a set of  boundary conditions.
    
    The boundary conditions are stored in the form of a list of boundary_condition
    instances.
    
    \ingroup Pde
*/
template<class T>
class gsBoundaryConditions 
{
    
public:

    typedef typename std::vector<boundary_condition<T> > bcContainer;
    typedef typename std::vector<corner_value<T> >       cornerContainer;

    typedef typename bcContainer::iterator iterator;
    typedef typename bcContainer::const_iterator const_iterator;

    typedef typename cornerContainer::iterator citerator;
    typedef typename cornerContainer::const_iterator const_citerator;

    typedef typename boundary_condition<T>::function_ptr function_ptr;

    typedef memory::shared_ptr< gsBoundaryConditions > Ptr;

    typedef typename memory::unique< gsBoundaryConditions >::ptr uPtr;

public:

    /// Default empty constructor
    gsBoundaryConditions()
    { }

    ~gsBoundaryConditions() // Destructor
    { }


    gsBoundaryConditions & operator= (uPtr other)
    {
        if ( other.get() != NULL )
        {
            drchlt_sides.swap( other->drchlt_sides);
            nmnn_sides  .swap( other->nmnn_sides  );
            robin_sides .swap( other->robin_sides );
        }

        return *this;
    }

    
public:

    void clear()
    {
        drchlt_sides .clear();
        nmnn_sides   .clear();
        robin_sides  .clear();
        corner_values.clear();
    }

    size_t size() const
    {
        return drchlt_sides.size() + nmnn_sides.size() +
                robin_sides.size() + corner_values.size();
    }

    /// Return a reference to the Dirichlet sides
    const bcContainer & dirichletSides() const {return drchlt_sides; }

    /// Return a reference to the Neumann sides
    const bcContainer & neumannSides()   const {return nmnn_sides;   }

    /// Return a reference to the Dirichlet sides
    const bcContainer & robinSides()     const {return robin_sides;  }

    const cornerContainer & cornerValues() const  {return corner_values;  }

    /// Extracts the BC, comming from a certain component.
    bcContainer reducedContainer(const bcContainer & container, index_t unknown) const
    {
        bcContainer red;
        red.reserve(container.size());
        for(typename bcContainer::const_iterator iter=container.begin(); iter!=container.end();++iter)
        {
            if(iter->unknown()==unknown)
                red.push_back(*iter);
        }
        return red;
    }

    bcContainer allConditions() const
    {
        bcContainer all;
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

    /// Get an iterator to the beginning of the corner values
    /// \return an iterator to the beginning of the corner values
    const_citerator cornerBegin() const
    { return corner_values.begin(); }
    
    /// Get an iterator to the end of corner values
    /// \return an iterator to the end of the corner values
    const_citerator cornerEnd() const
    { return corner_values.end(); }
    
    /// Get an iterator to the beginning of the Robin sides
    /// \return an iterator to the beginning of the Robin sides
    iterator robinBegin()
    { return robin_sides.begin(); }
    
    /// Get an iterator to the end of the Robin sides
    /// \return an iterator to the end of the Robin sides
    iterator robinEnd()
    { return robin_sides.end(); }

    /// Get an iterator to the beginning of the corner values
    /// \return an iterator to the beginning of the corner values
    citerator cornerBegin()
    { return corner_values.begin(); }
    
    /// Get an iterator to the end of corner values
    /// \return an iterator to the end of the corner values
    citerator cornerEnd()
    { return corner_values.end(); }




    /** \brief Adds another boundary condition
     *
     * Creates an object of type boundary_condition and adds is to
     * the list of corresponding boundary conditions.
     *
     * \param p Index of the patch
     * \param s Side of the patch
     * \param t Type of boundary condition (see condition_type::type)
     * \param f Function defining the boundary condition
     * \param unknown Specifies which unknown variable the boundary condition
     * refers to (to be used if more than one variable is unknown, e.g.,
     * velocity and pressure)
     * \param parametric True if the function data for this boundary condition
     * is defined in parametric coordinates.
     */
    void addCondition(int p, boxSide s, condition_type::type t,
                      gsFunction<T> * f, int unknown = 0, bool parametric = false)
    {
        switch (t) {
        case condition_type::dirichlet :
            drchlt_sides.push_back( boundary_condition<T>(p,s,f,t,unknown,parametric) );
            break;
        case condition_type::neumann :
            nmnn_sides.push_back( boundary_condition<T>(p,s,f,t,unknown,parametric) );
            break;
        case condition_type::robin :
            robin_sides.push_back( boundary_condition<T>(p,s,f,t,unknown,parametric) );
            break;
        default:
            std::cout<<"gsBoundaryConditions: Unknown boundary condition.\n";
        }
    }

    void addCondition(int p, boxSide s, condition_type::type t,
                      const function_ptr & f_shptr, int unknown = 0,
                      bool parametric = false)
    {
        switch (t) {
        case condition_type::dirichlet :
            drchlt_sides.push_back( boundary_condition<T>(p,s,f_shptr,t,unknown,parametric) );
            break;
        case condition_type::neumann :
            nmnn_sides.push_back( boundary_condition<T>(p,s,f_shptr,t,unknown,parametric) );
            break;
        case condition_type::robin :
            robin_sides.push_back( boundary_condition<T>(p,s,f_shptr,t,unknown,parametric) );
            break;
        default:
            std::cout<<"gsBoundaryConditions: Unknown boundary condition.\n";
        }
    }

    void addCondition(int p, boxSide s, condition_type::type t,
                      const gsFunction<T> & func, int unknown = 0,
                      bool parametric = false)
    {
        switch (t) {
        case condition_type::dirichlet :
            drchlt_sides.push_back( boundary_condition<T>(p,s,func,t,unknown,parametric) );
            break;
        case condition_type::neumann :
            nmnn_sides.push_back( boundary_condition<T>(p,s,func,t,unknown,parametric) );
            break;
        case condition_type::robin :
            robin_sides.push_back( boundary_condition<T>(p,s,func,t,unknown,parametric) );
            break;
        default:
            std::cout<<"gsBoundaryConditions: Unknown boundary condition.\n";
        }
    }

    void addCondition( boxSide s, condition_type::type t,
                       gsFunction<T> * f, int unknown = 0, bool parametric = false)
    {
        // for single-patch only
        addCondition(0,s,t,f,unknown,parametric);
    }

    void addCondition(const patchSide& ps, condition_type::type t,
                      gsFunction<T> * f, int unknown = 0, bool parametric = false)
    {
        addCondition(ps.patch, ps.side(), t, f, unknown,parametric);
    }

    void addCondition(const patchSide& ps, condition_type::type t,
                      const function_ptr & f_shptr, int unknown = 0, bool parametric = false)
    {
        addCondition(ps.patch, ps.side(), t, f_shptr, unknown,parametric);
    }

    void addCondition(const patchSide& ps, condition_type::type t,
                      const gsFunction<T> & func, int unknown = 0, bool parametric = false)
    {
        addCondition(ps.patch, ps.side(), t, func, unknown,parametric);
    }

    void addCornerValue(boxCorner c, T value, int p = 0, int unknown = 0)
    {
        corner_values.push_back( corner_value<T>(p,c,value,unknown) );
    }

    /// Prints the object as a string.
    std::ostream & print(std::ostream &os) const
    {
        os << "gsBoundaryConditions :\n";
        os << "* Dirichlet boundaries: "<< drchlt_sides.size() <<"\n";
        os << "* Neumann boundaries  : "<< nmnn_sides.size() <<"\n";
        os << "* Corner values       : "<< corner_values.size() <<"\n";
        return os;
    }

    /**
     * @brief   getSideCondition
     * @param   ps the patch side
     * @return  the boundary condition associated to ps or NULL if no condition is associated to ps
     */
    const boundary_condition<T>* getConditionFromSide (patchSide ps) const
    {
        typename std::vector<boundary_condition<T> >::const_iterator beg, end, cur;
        patchSideComparison psRef(ps);
        beg=drchlt_sides.begin();
        end=drchlt_sides.end();
        cur=std::find_if(beg,end,psRef);
        if (cur != end)
            return &(*cur);
        beg=nmnn_sides.begin();
        end=nmnn_sides.end();
        cur=std::find_if(beg,end,psRef);
        if (cur != end)
            return &(*cur);
        beg=robin_sides.begin();
        end=robin_sides.end();
        cur=std::find_if(beg,end,psRef);
        if (cur != end)
            return &(*cur);

        return NULL;
    }

    /**
     * @brief   getConditionFromSide returns the boundary conditions associated to the given patch side
     * @param[in] ps patch side
     * @param[out] result bcContainer containing the boundary conditions associated to ps
     */

    void getConditionFromSide (patchSide ps, bcContainer& result) const
    {
        result.clear();
        const_iterator begin, end, cur;

        begin=drchlt_sides.begin();
        end=drchlt_sides.end();
        for(cur=begin; cur!=end; cur++)
            if(cur->ps == ps)
                result.push_back(*cur);

        begin=nmnn_sides.begin();
        end=nmnn_sides.end();
        for(cur=begin; cur!=end; cur++)
            if(cur->ps == ps)
                result.push_back(*cur);

        begin=robin_sides.begin();
        end=robin_sides.end();
        for(cur=begin; cur!=end; cur++)
            if(cur->ps == ps)
                result.push_back(*cur);
    }


    /**
     * @brief returns the set of all boundary conditions which refer to patch \a np
     * @param[in] np the patch index
     * @param[out] result the new set of boundary conditions
     */
    void getConditionsForPatch(const int np, gsBoundaryConditions& result) const
    {
        result.clear();
        std::vector<boundary_condition<T> > bc_all = allConditions(); //inefficient, but fewer code
        for(typename std::vector<boundary_condition<T> >::const_iterator it = bc_all.begin(); it!= bc_all.end();it++)
        {
            if((*it).patch()==np)
                result.addCondition(0,(*it).side(),(*it).type(),(*it).function(),(*it).unknown());
        }

        for(const_citerator it = cornerBegin(); it!= cornerEnd();it++)
        {
            if((*it).patch==np)
                result.addCornerValue( (*it).corner, (*it).value,  0, (*it).unknown);
        }
    }

    // Data members
private:
    struct patchSideComparison
    {
        const patchSide m_ps;
        patchSideComparison(patchSide ps)
            : m_ps(ps)
        {}

        bool operator() (const boundary_condition<T> &bc) const
        {
            return bc.ps==m_ps;
        }
    };

    bcContainer     drchlt_sides;  ///< List of Dirichlet sides
    bcContainer     nmnn_sides;    ///< List of Neumann sides
    bcContainer     robin_sides;   ///< List of Robin sides
    cornerContainer corner_values; ///< List of corners with fixed value

    // Pointer to associated multipatch domain
    //gsMultiPatch<T> * m_patches;
    
}; // class gsBoundaryConditions


//////////////////////////////////////////////////
//////////////////////////////////////////////////

/// Print (as string)
template<class T>
std::ostream &operator<<(std::ostream &os, const gsBoundaryConditions<T>& bvp)
{return bvp.print(os); }

} // namespace gismo
