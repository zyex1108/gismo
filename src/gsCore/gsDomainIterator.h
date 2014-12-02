/** @file gsDomainIterator.h

    @brief Provides declaration of DomainIterator abstract interface.

    This file is part of the G+Smo library.
    
    This Source Code Form is subject to the terms of the Mozilla Public
    License, v. 2.0. If a copy of the MPL was not distributed with this
    file, You can obtain one at http://mozilla.org/MPL/2.0/.

    Author(s): C. Hofreither
*/

#pragma once

#include <gsCore/gsBasis.h>
#include <gsCore/gsDomain.h>
#include <gsCore/gsDofMapper.h>

namespace gismo
{

/** The gsDomainIterator \n
 * Class which enables iteration over all elements of a parameter domain.
 * It also includes some additional functionality which is typically used
 * when assembling the system matrix for numerically solving a PDE.
 *
 * - <b>Iteration through the elements:</b>\n
 * The function next() jumps to the "next" element and sets up the quadrature
 * nodes and weights on that element.
 * The specific implementation
 * of how to select the next element depends on the structure of the underlying mesh.\n
 * The function good() indicates whether there still is a "next" element to be found.
 *
 * - <b>Quadrature nodes and weights:</b>\n
 * The quadrature nodes/points and weights on the current element are initialized up by calling
 * computeQuadratureRule() or computeQuadratureRuleDefault(). During the iteration, they are updated by
 * the function next().
 *
 * - <b>Evaluation of basis functions:</b>\n
 * Once the quadrature points have been set up, the basis functions
 * can be evaluated at the quadrature points by calling
 * evaluateBasis(). The function values and/or derivatives can be accessed by
 * basisValues() and basisDerivs().
 *
 * Note that the features of the gsDomainIterator strongly depend on the underlying basis.
 * Hence the gsBasis is given as an input argument to the constructor.
 *
 * An example of the typical use of gsDomainIterator (remark: replace the constructor by the constructor of the actually used derived class):
 *
 * \verbatim
     gsDomainIterator domIter( basis );         // constructor
     domIter.computeQuadratureRule( numNodes ); // compute/initialize quad. points and weights

     for (; domIter.good(); domIter.next() )    // loop over all elements
     {
         domIt.evaluateBasis();                 // evaluate basis functions
                                                // at the quadrature nodes of the
                                                // current element.

         // Your source code using
         // the domain iterator's functions.
         // Access function values with domIt.basisValues() and
         // the k-th derivatives with domIt.basisDerivs( k ).

     }
     \endverbatim
 *
 *
 *
 */



template <class T>
class gsDomainIterator
{
public:
    typedef std::auto_ptr< gsDomainIterator > uPtr;

public:
    /// Constructor
    gsDomainIterator( const gsBasis<T>& basis )
        : center( gsVector<T>::Zero(basis.dim()) ), m_basis( basis ), m_isGood( true )
    { }

    virtual ~gsDomainIterator() { }

public:

    /** @brief Proceeds to the next element.
     *
     * The function returns true if there are still elements remaining that have not been treated.\n
     * This function must also <em>update the quadrature nodes and weights for the next element</em>,
     * and call <em>computeActiveFunctions()</em>, such that, when evaluateBasis() is called,
     * it will be evaluated
     * at the correct nodes.\n
     * For the typical usage of this function, see the example in the
     * documentation of gsDomainIterator.
     */
    virtual bool next() = 0;

    /// Resets the iterator so that it points to the first element
    virtual void reset()
    {
        GISMO_NO_IMPLEMENTATION
    }

    /// \brief Computes a default quadrature rule for the degree of the given basis functions.
    ///
    /// The number of quadrature nodes in the <em>i</em>-th coordinate direction is
    /// set to <em>p_i + 1</em>, where <em>p_i</em> denotes the degree of the basis functions
    /// in the <em>i</em>-th coordinate.
    ///
    /// See also computeQuadratureRule().
    ///
    void computeQuadratureRuleDefault()
    {

        // uses same formula as gsGaussAssembler::getNumIntNodesFor( gsBasis )
        gsVector<int> numIntNodes( m_basis.dim() );
        for (int i = 0; i < m_basis.dim(); ++i)
            numIntNodes[i] = m_basis.degree(i) + 1;

        computeQuadratureRule( numIntNodes );
    }

    /// \brief Computes quadrature rule with \a numIntNodes quadrature points.
    ///
    /// The function computes quadrature nodes and weights, where the number
    /// of nodes in the <em>i</em>-th coordinate direction is specified by \a numIntNodes[i].\n
    /// See also computeQuadratureRuleDefault().
    ///
    /// \param[in] numIntNodes gsVector of length \a d,
    /// where \a d is the dimension of the parameter space.
    ///
    ///
    virtual void computeQuadratureRule(const gsVector<int>& numIntNodes) = 0;

    /// \brief Compute the (patch-local) active basis function indices.
    ///
    /// The global indices of the basis functions which are not identical zero on the
    /// current element are computed and stored in gsDomainIterator::activeFuncs.
    virtual const gsMatrix<unsigned>& computeActiveFunctions() = 0;

public:
    /// Is the iterator still pointing to a valid element?
    bool good() const   { return m_isGood; }

    /// Return dimension of the elements
    int dim() const   { return center.size(); }

    /// Returns the number of quadrature points that are used.
    index_t numQuNodes() const  { return quNodes.cols(); }

    // \brief Compute the active DOF on the current element. @@@
    //
    // The global indices of the basis functions which are not identical zero on the
    // current element are computed and stored in gsDomainIterator::activeDofs.
    const gsVector<int>& computeActiveDofs(const gsDofMapper& dofMapper, int patchIndex)
    {
        computeActiveFunctions();
        const index_t numActive = activeFuncs.rows();

        activeDofs.resize(numActive);
        for (index_t i = 0; i < numActive; ++i)
            activeDofs[i] = dofMapper.index(activeFuncs(i,0), patchIndex);
        return activeDofs;
    }

    /// \brief Returns the number of active basis functions on this element
    ///
    index_t numActive() const
    {
        // Assumes that activeFuncs is already computed during the element update
        return activeFuncs.rows();
    }

    /// \brief Evaluates the underlying basis functions (and derivatives).
    ///
    /// The result is stored in gsDomainIterator::basisEvals. It can be accessed via basisValues() and basisDerivs().
    ///
    /// \param[in] numDerivs Specifies up to which order the derivatives have to be computed.
    ///
    /// \warning This depends on computeActiveFunctions() having been called before. The call
    /// of computeActiveFunctions() thus has to included somehow in the call of next()!
    void evaluateBasis(int numDerivs = 0)
    {
        GISMO_ASSERT(numDerivs <= 2, "evaluateBasis() not implemented for derivatives > 1");

        // WARN: this depends on computeActiveFunctions() having been called before.
        // computeActiveFunctions() thus has to be called when next() is called!
        const index_t numActive = activeFuncs.rows();

        m_basis.evalAllDers_into(quNodes, numDerivs, allValues);

        basisEvals.clear();
        //gsDebug << "numActive " << numActive <<" m_basis.dim(): " << m_basis.dim() << " allValues.rows(): " << allValues.rows() << "\n";
        index_t curRow = 0;
        basisEvals.push_back( allValues.topRows(numActive) );
        curRow += numActive;

        if (numDerivs > 0)
        {
            // curently only 1 implemented
            const int numDerivs = numActive * m_basis.dim();
            basisEvals.push_back( allValues.middleRows( curRow, numDerivs ) );
            curRow += numDerivs;
        }
        if (numDerivs > 1)
        {
            const int num2Der = numActive * (m_basis.dim() +(m_basis.dim()*(m_basis.dim()-1))/2);
            basisEvals.push_back( allValues.middleRows( curRow, num2Der ) );
            curRow += num2Der;
        }
        GISMO_ASSERT(curRow == allValues.rows(), "Unexpected number of rows in evaluateBasis()");
    }

    /// \brief Returns basis function values at quadrature nodes.
    ///
    /// \param[out] basisValues gsMatrix of size <em>k</em> x <em>n</em>, where \n
    /// <em>k</em> is the number of active basis functions on the current element.\n
    /// <em>n</em> is the number of quadrature nodes (which can be accessed via numQuNodes()).\n
    /// The entry <em>(i,j)</em> of the matrix corresponds to the value of the <em>i</em>-th function evaluated
    /// at the <em>j</em>-th quadrature point.
    typename gsMatrix<T>::Block basisValues()           { return basisEvals[0]; }

    /// \brief Returns derivatives of the basis functions at quadrature nodes.
    ///
    /// \param[out] basisDerivs gsMatrix of size <em>(k*d)</em> x <em>n</em>, where \n
    /// <em>k</em> is the number of active basis functions on the current element.\n
    /// <em>d</em> is the dimension of the parameter domain.
    /// <em>n</em> is the number of quadrature nodes (which can be accessed via numQuNodes()).\n
    /// See documentation of gsBasis::deriv_into (the one \em without input parameter "coefs") for details on the format of the data.
    // The entry <em>(i,j)</em> of the matrix corresponds to the value of the <em>i</em>-th function evaluated
    // at the <em>j</em>-th quadrature point.
    typename gsMatrix<T>::Block basisDerivs(int der)    { return basisEvals[der]; }

    /// \brief Returns the value of a basis function at a point.
    ///
    /// \param[in] basisNum <b>Local</b> index of the basis function to be evaluated.
    /// \param[in] pt Index of the point.
    T basisValue(index_t basisNum, index_t pt) const    { return basisEvals[0](basisNum, pt); }

    /// Returns the <em>der</em>-th derivatives of the basis function with index \a basisNum evaluated at \a pt.
    /// \todo WHAT IS THIS REALLY SUPPOSED TO BE RETURNING?
    /// \todo SEEMS LIKE IT IS NOT USED ANYWHERE
    T basisDeriv(int der, index_t basisNum, index_t pt) const    { return basisEvals[der](basisNum, pt); }

    /// Updates \a other with and adjacent element
    /// \todo upgrade to return adjacent range instead
    virtual void adjacent( const gsVector<bool> & orient, 
                           gsDomainIterator & other )
    {
        GISMO_NO_IMPLEMENTATION
    }

    /// \brief Returns the center of the current element.
    ///
    /// The current element is a <em>d</em>-dimensional hypercube.
    /// The coordinates of its upper corner is returned as a gsVector of length \a d.\n
    /// \n
    /// E.g., if the current two-dimensional element is defined by <em>[a,b]x[c,d]</em>, then <em>[b,d]</em> is returned (see also lowerCorner()).
    const gsVector<T>& centerPoint () const
    { return center; }

    /// \brief Returns the lower corner of the current element.
    ///
    /// The current element is a <em>d</em>-dimensional hypercube.
    /// The coordinates of its lower corner is returned as a gsVector of length \a d.\n
    /// \n
    /// E.g., if the current two-dimensional element is defined by <em>[a,b]x[c,d]</em>, then <em>[a,c]</em> is returned (see also upperCorner()).
    virtual const gsVector<T>& lowerCorner() const
    {
        GISMO_NO_IMPLEMENTATION
    }

    /// \brief Returns the upper corner of the current element.
    ///
    /// The current element is a <em>d</em>-dimensional hypercube.
    /// The coordinates of its upper corner is returned as a gsVector of length \a d.\n
    /// \n
    /// E.g., if the current two-dimensional element is defined by <em>[a,b]x[c,d]</em>, then <em>[b,d]</em> is returned (see also lowerCorner()).
    virtual const gsVector<T>& upperCorner() const
    {
        GISMO_NO_IMPLEMENTATION
    }

    /// \brief Returns the perdicular cell size of boundary iterator.
    ///
    /// Only works for boundary iterators. Returns the lenght from
    /// the boundary side to the parallell side not on the boudary.
    virtual const T getPerpendicularCellSize() const
    {
        GISMO_NO_IMPLEMENTATION
    }

    /// Return the diagonal of the element
    T getCellSize() const
    {
        return (upperCorner() - lowerCorner()).norm();
    }

    /// Return the volume of the element
    T volume() const
    { return (upperCorner() - lowerCorner()).prod(); }

    /// Returns the number of elements.
    virtual index_t numElements() const
    {
        //\todo Remove this implementation, as it gives wrong results
        //for boundary iterators. Probably one using "reset" and
        //"next" would do this correctly.

        // Buggy, and probably a terrible implementation,
        // but needed and therefore can be useful
        // sometimes.
        typename gsBasis<T>::domainIter domIter = m_basis.makeDomainIterator();

        index_t numEl = 0;
        for (; domIter->good(); domIter->next(), numEl++){}

        return numEl;
    }

public:
    // quadrature nodes and weights

    /// Stores the quadrature nodes.
    gsMatrix<T> quNodes;

    /// Stores the quadrature weights.
    gsVector<T> quWeights;

    // patch-local basis function numbers which are active in the current element
    /// \brief Stores the indices of the active functions.
    ///
    /// This is a gsMatrix<unsigned> of size \em N x 1, where \em N is the
    /// number of active functions.\n
    /// <b>Note</b> that it is assumed that the
    /// active functions are the same everywhere within one cell, and that
    /// the test whether a function
    /// is active or not is performed
    /// <em>at the center point</em> of the element/cell.
    ///
    /// \warning This must be filled by the function next()
    /// in any derived class!
    gsMatrix<unsigned> activeFuncs;

    // global dof numbers corresponding to the local element dofs
    // @@@
    /// \brief Global indices of the local element DOFs. REDUNDANT?
    ///
    /// NOTE: Has something to do with the multipatch-geometries
    ///
    gsVector<int> activeDofs;

    /// Matrix in which all values (including derivatives) of the active basis
    /// functions at the quadrature nodes of the current element are stored.
    gsMatrix<T> allValues;

    // Matrix nlocks pointing to allValues sub-matrices
    /// \brief Stores all computed function values and derivatives.
    ///
    /// The values are stored in a vector. <em>basisEvals[0]</em> is a gsMatrix containing
    /// functions values, <em>basisEvals[1]</em> is a gsMatrix containing the first derivatives.\n
    /// For details on the format of these matrices, see documentation of gsBasis::eval_into and gsBasis::deriv_into.
    /// The accessors are basisValues() and basisDerivs().
    std::vector< typename gsMatrix<T>::Block > basisEvals;

    /// Coordinates of a central point in the element (in the parameter domain).
    gsVector<T> center;

protected:
    /// The basis on which the domain iterator is defined.
    const gsBasis<T> & m_basis;

    /// Flag indicating whether the domain iterator is "good". If it is "good", the iterator can continue to the next element.
    bool m_isGood;

private:
    // disable copying
    gsDomainIterator( const gsDomainIterator& );
    gsDomainIterator& operator= ( const gsDomainIterator& );
}; // class gsDomainIterator


/// Print (as string) operator to be used by all derived classes
//template<class T>
//std::ostream &operator<<(std::ostream &os, const gsDomainIterator<T>& b)
//{return b.print(os); }


}; // namespace gismo