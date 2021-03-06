/** @file gsField.h

    @brief Provides declaration of the Field class.

    This file is part of the G+Smo library. 

    This Source Code Form is subject to the terms of the Mozilla Public
    License, v. 2.0. If a copy of the MPL was not distributed with this
    file, You can obtain one at http://mozilla.org/MPL/2.0/.
    
    Author(s): A. Mantzaflaris
*/

#pragma once

#include <gsCore/gsFunctionSet.h>
#include <gsCore/gsGeometry.h>
#include <gsCore/gsMultiPatch.h>
#include <gsCore/gsMultiBasis.h>
#include <gsUtils/gsNorms.h>

namespace gismo
{

/**
 * \brief A scalar of vector field defined on a parametrized geometry.
 *
 * A gsField is, generally speaking, some mathematical function that is defined on a domain of interest
 * (the name "field" is motivated by, e.g., "scalar field" or "vector field").
 *
 * The gsField combines the following:\n
 * - <b>Geometric information</b> on the domain:\n
 *
 * The domain can be represented as one single patch or as a
 * collection of multiple patches (a.k.a. subdomains).\n This
 * information is stored in a member of the gsMultiPatch class.\n
 *
 * - The <b>function</b> defined on the domain:\n
 *
 * For each patch (a.k.a. subdomain), the gsField contains a member
 * of class gsFunction (which represents the "local field", so to
 * say).  On this, the operations of gsFunction can be carried out
 * (e.g., function evaluation or computation of derivatives).\n
 * Remark: The collection of patch-wise gsFunction is stored in the
 * private member gsField::m_fields.
 *
 * Note that the geometry representation of a single patch can be
 * extracted by calling the member function gsField::patch.
 *
 * The "local field" on a single patch can be extracted by calling gsField::function.
 *
 * \ingroup Core
 */
template<class T>
class gsField
{
public:
    typedef typename memory::shared< gsField >::ptr  Ptr;// todo: remove
    typedef typename memory::unique< gsField >::ptr  uPtr;// todo: remove

    gsField(): m_patches(NULL) { }

    gsField( const gsFunctionSet<T> & mp, 
             typename gsFunctionSet<T>::Ptr fs, 
             const bool isparam)
    : m_patches(&mp), m_fields(fs), parametrized(isparam)
    { }

    gsField( const gsGeometry<T> & sp, const gsFunctionSet<T> & pf, const bool isparam = false) 
    : m_patches(&sp), m_fields(memory::make_shared_not_owned(&pf)), parametrized(isparam)
    { }

    gsField( const gsGeometry<T> & sp, const gsGeometry<T> & pf) 
    : m_patches(&sp), m_fields(memory::make_shared_not_owned(&pf)), parametrized(true)
    { }

    gsField( const gsMultiPatch<T> & mp, const gsFunctionSet<T> & f, const bool isparam = false) 
    : m_patches(&mp), m_fields(memory::make_shared_not_owned(&f)), parametrized(isparam)
    { }

    gsField( const gsMultiPatch<T> & mp, const gsMultiPatch<T> & f) 
    : m_patches(&mp), m_fields(memory::make_shared_not_owned(&f)), parametrized(true)
    { }

public:
    
// TO DO:

// EVAL_physical_position
// Need to solve x = m_geometry(u) for u

    // Return a point in  the physical domain at parameter value u
    /**
     * @brief Maps points \a u from the parameter domain to the physical domain.
     *
     * @param[in] u Evaluation points as gsMatrix of size <em>d</em> x <em>n</em>.\n
     * \a d denotes the dimension of the parameter domain (i.e., d = parDim()).\n
     * \a n denotes the number of evaluation points.\n
     * Each column of \a u corresponds to one evaluation point.
     * @param[in] i Index of the considered patch/subdomain.
     * @returns uPtr The <em>j</em>-th column of \a uPtr corresponds
     * to the image of the point \a u_j (which is defined by the \a j-th column of the input parameter \a u).
     */
    typename gsMatrix<T>::uPtr point(const gsMatrix<T>& u, int i = 0) const
    {
        return m_patches->piece(i).eval(u);
    }

    // Return the value of the Field at parameter value u
    // TO DO: rename to evalParam()
    /**
     * @brief Evaluation of the field at points \a u.
     *
     * @param[in] u Evaluation points as gsMatrix of size <em>d</em> x <em>n</em>.\n
     * \a d denotes the dimension of the parameter domain (i.e., d = parDim()).\n
     * \a n denotes the number of evaluation points.\n
     * Each column of \a u corresponds to one evaluation point.
     * @param[in] i Index of the considered patch/subdomain.
     * @returns uPtr The <em>j</em>-th column of \a uPtr corresponds
     * to the value of the field at the point \a u_j (which is defined by the \a j-th column of the input parameter \a u).
     */
    typename gsMatrix<T>::uPtr value(const gsMatrix<T>& u, int i = 0)  const
    {
        return parametrized
            ? m_fields->piece(i).eval(u)
            : m_fields->piece(i).eval( *point(u, i) );
    }

    // Return the value of the Field at physical value u 
    // TO DO: rename to evalPhys()
    typename gsMatrix<T>::uPtr pvalue(const gsMatrix<T>& u, int i)  const
    {
        GISMO_ASSERT(!parametrized, "Cannot compute physical value");
        return ( m_fields->piece(i).eval(u) ); 
    }

    /// Computes the L2-distance between the two fields, on the physical domain
    T distanceL2(gsField<T> const & field, int numEvals= 1000) const 
    {
        return computeL2Distance(*this, field, numEvals);
    }

    /// Computes the L2-distance between the field and a function \a func on the physical domain
    T distanceL2(gsFunction<T> const & func, 
                 bool isFunc_param = false,
                 int numEvals=1000) const 
    {
        if ( parametrized ) // isogeometric field
            return igaFieldL2Distance(*this, func, isFunc_param);
        else
            return computeL2Distance(*this, func, isFunc_param,  numEvals);
    }

    /// Computes the L2-distance between the field and a function \a
    /// func on the physical domain, using mesh from B
    T distanceL2(gsFunction<T> const & func,
                 gsMultiBasis<T> const & B,
                 bool isFunc_param = false,
                 int numEvals=1000) const
    {
        if ( parametrized ) // isogeometric field
            return igaFieldL2Distance(*this, func, B,isFunc_param);
        else
            return computeL2Distance(*this, func, isFunc_param,  numEvals);
    }

    /// Computes the H1-distance between the field and a function \a
    /// func on the physical domain
    T distanceH1(gsFunction<T> const & func, 
                 bool isFunc_param = false,
                 int numEvals=1000) const 
    {
        if ( parametrized ) // isogeometric field
            return igaFieldH1Distance(*this, func, isFunc_param);
        else
        {
            GISMO_UNUSED(numEvals);
            gsWarn <<"H1 seminorm not implemented.\n";
            return -1;
        }
    }

    /// Computes the H1-distance between the field and a function \a
    /// func on the physical domain, using mesh from B
    T distanceH1(gsFunction<T> const & func,
                 gsMultiBasis<T> const & B,
                 bool isFunc_param = false,
                 int numEvals=1000) const
    {
        if ( parametrized ) // isogeometric field
            return igaFieldH1Distance(*this, func, B,isFunc_param);
        else
        {
            GISMO_UNUSED(numEvals);
            gsWarn <<"H1 seminorm not implemented.\n";
            return -1;
        }
    }

    /// Computes the DG-distance between the field and a function \a
    /// func on the physical domain
    T distanceDG(gsFunction<T> const & func, 
                 bool isFunc_param = false,
                 int numEvals=1000) const 
    {
        if ( parametrized ) // isogeometric field
            return igaFieldDGDistance(*this, func, isFunc_param);
        else
        {
            GISMO_UNUSED(numEvals);
            gsWarn <<"DG norm not implemented.\n";
            return -1;
        }
    }
    
    /// Prints the object as a string.
    std::ostream &print(std::ostream &os) const
    { 
        os << ( parametrized ? "Parameterized f" : "F") 
           << "unction field.\n Defined on " << m_patches;
        return os; 
    }
    
    /// \brief Returns the dimension of the parameter domain
    /// (e.g., if the domain is a surface in three-dimensional space, it returns 2).
    int parDim() const { return m_patches->domainDim(); }

    /// \brief Returns the dimension of the physical domain
    /// (e.g., if the domain is a surface in three-dimensional space, it returns 3).
    int geoDim() const { return m_patches->targetDim(); }

    /// \brief Returns the dimension of the physical domain
    /// (e.g., if the domain is a surface in three-dimensional space, it returns 3).
    int dim() const { return m_fields->targetDim(); }

    /// Returns the number of patches.
    int nPatches()  const { return m_patches->size(); }

    const gsGeometry<T> & geometry() const 
    {
        GISMO_ASSERT(dynamic_cast<const gsGeometry<T>*>(m_patches),
                     "No geometry in field. The domain is"<< *m_patches);
        return *static_cast<const gsGeometry<T>*>(m_patches);
    }

    /// Returns gsMultiPatch containing the geometric information on the domain.
    const gsMultiPatch<T> & patches() const    
    { 
        GISMO_ASSERT(dynamic_cast<const gsMultiPatch<T>*>(m_patches),
                     "No patches in field. The field domain is "<< *m_patches);
        return *static_cast<const gsMultiPatch<T>*>(m_patches);
    }

    /// Returns the gsGeometry of patch \a i.
    const gsGeometry<T> & patch(int i=0) const 
    { 
        GISMO_ASSERT( i<m_patches->size(),
                      "gsField: Invalid patch index.");
        GISMO_ASSERT(dynamic_cast<const gsGeometry<T>*>(&m_patches->piece(i)),
                     "No geometry in field. The domain is"<< m_patches->piece(i));
        return static_cast<const gsGeometry<T>&>(m_patches->piece(i));
    }

    /// Returns the gsFunction of patch \a i.
    const gsFunction<T> & function(int i=0) const  
    { 
        GISMO_ASSERT(dynamic_cast<const gsFunction<T>*>(&m_patches->piece(i)),
                     "No function in field. The domain is"<< m_patches->piece(i));
        return static_cast<const gsFunction<T>&>(m_fields->piece(i)); 
    }

    /// Attempts to return an Isogeometric function for patch i
    const gsGeometry<T> & igaFunction(int i=0) const
    { 
        GISMO_ASSERT(parametrized,
                     "Cannot get an IGA function from non-parametric field.");
        GISMO_ASSERT(i<m_fields->size(),
                      "gsField: Invalid patch index.");
        return static_cast<const gsGeometry<T> &>(m_fields->piece(i));
    }

    bool isParametrized() const { return parametrized; }

    /** \brief Returns the coefficient vector (if it exists)
        corresponding to the function field for patch \a i.
    
    Returns the coefficients of the field corresponding to the \a i-th
    patch. This is only possible in the case when the field is defined
    in terms of basis functions (ie. it derives from gsGeometry).
    
    */
    const gsMatrix<T> & coefficientVector(int i=0) const
    {
        return igaFunction(i).coefs();
    }

// Data members
private:

    /// The isogeometric field is defined on this multipatch domain
    const gsFunctionSet<T> * m_patches;

    // If there are many patches, one field per patch

    /// \brief Vector containing "local fields" for each patch/subdomain.
    ///
    /// For each patch/subdomain, the "local field" is represented by
    /// a gsFunction. This local field can be accessed with
    /// gsField::function.
    typename gsFunctionSet<T>::Ptr m_fields;

    /**
     * @brief \a True iff this is an isogeometric field.
     *
     * If \a parametrized is \a true, the evaluation points for calling gsField::value have to be placed in the
     * \a parameter domain.
     *
     * If \a parametrized is \a false, then the evaluation points are in the \a physical domain.
     * This applies to, e.g., given exact solutions which are defined on the physical domain.
     */
    bool parametrized;// True iff this is an Isogeometric field, living on parameter domain

}; // class gsField


/// Print (as string) operator to be used by all derived classes
template<class T>
std::ostream &operator<<(std::ostream &os, const gsField<T>& b)
{return b.print(os); }


} // namespace gismo
