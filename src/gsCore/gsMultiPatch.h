/** @file gsMultiPatch.h

    @brief Provides declaration of the MultiPatch class.

    This file is part of the G+Smo library. 

    This Source Code Form is subject to the terms of the Mozilla Public
    License, v. 2.0. If a copy of the MPL was not distributed with this
    file, You can obtain one at http://mozilla.org/MPL/2.0/.
    
    Author(s): A. Mantzaflaris
*/

#pragma once

#include <gsCore/gsBoxTopology.h>

namespace gismo
{

/** @brief
    Holds a set of geometry patches and their
    interface/outer boundary information.

    \tparam T coefficient type
*/
template<class T>
class gsMultiPatch : public gsBoxTopology
{

public:
    /// Shared pointer for gsMultiPatch
    typedef memory::shared_ptr<gsMultiPatch> Ptr;
    typedef std::vector<gsGeometry<T> *> Base;
    //typedef memory::unique_ptr< gsGeometry > LocalPtr;
    //typedef struct interface interface;
    //typedef gsGraph< gsGeometry<T> *, interface > PatchContainer;
    typedef std::vector<gsGeometry<T> *> PatchContainer;

public:

    /// Type definitions
    typedef typename std::vector<gsGeometry<T> *>::size_type size_t;
    typedef typename std::vector<gsGeometry<T> *>::iterator iterator;
    typedef typename std::vector<gsGeometry<T> *>::const_iterator const_iterator;

    /// Get a const-iterator to the patches
    /// \return an iterator to the beginning of the patches
    const_iterator begin() const {
        return m_patches.begin();
    }

    /// Get a const iterator to the end of the patches
    /// \return an iterator to the end of the patches
    const_iterator end() const {
        return m_patches.end();
    }

    /// Get an iterator to the beginning of the  patches
    /// \return an iterator to the beginning of the  patches
    iterator begin() {
        return m_patches.begin();
    }

    /// Get an iterator to the end of the  patches
    /// \return an iterator to the end of the  patches
    iterator end() {
        return m_patches.end();
    }

    /// Default empty constructor
    gsMultiPatch() : gsBoxTopology() { }

    /// Copy constructor (makes deep copy)
    gsMultiPatch( const gsMultiPatch& other );

    /// Assignment operator (uses copy-and-swap idiom)
    gsMultiPatch& operator= ( gsMultiPatch other )
    {
        this->swap( other );
        return *this;
    }

    /// Create from a vector of patches
    explicit gsMultiPatch( const std::vector<gsGeometry<T> *>& patches );

    /// Create a single-patch instance
    explicit gsMultiPatch( const gsGeometry<T> & geo );

    /// Create from patches and boundary/interface information
    gsMultiPatch( const PatchContainer& patches,
            const std::vector<patch_side>& boundary,
                  const std::vector<boundaryInterface>& interfaces );

    /// Destructor
    ~gsMultiPatch();

    /// Clone function. Used to make a copy of the object
    gsMultiPatch* clone() const {
        return new gsMultiPatch( *this );
    }

public:

    /// Swap with another gsMultiPatch.
    void swap(gsMultiPatch& other)
    {
        gsBoxTopology::swap( other );
        m_patches.swap( other.m_patches );
    }

    /// Prints the object as a string.
    std::ostream& print( std::ostream& os ) const
    {
        if ( this->size() > 0 ) {
            os << "gsMultiPatch (" << this->size() << "): ";
            os << "#Boundaries= " << nBoundary() << ", ";
            os << "#Interfaces= " << nInterfaces() << ".\n";
            //gsBoxTopology::print( os );
        } else {
            os << "gsMultiPatch ( empty! ).\n";
        }
        return os;
    }

    /// Dimension of the parameter domain (must match for all patches).
    int parDim() const 
    {
        //GISMO_ASSERT( m_patches.size() > 0 , "Empty multipatch object.");
        return m_dim;
    }

    /// Dimension of the geometry (must match for all patches).
    int geoDim() const;

    /// Co-dimension of the geometry (must match for all patches).
    int coDim() const;

    /// Returns true if the multipatch object is a closed manifold
    /// (ie. it has no boundaries)
    bool isClosed() { return this->nBoundary() == 0; }

    /// Returns the range of parameter
    gsMatrix<T> parameterRange(int i = 0) const;

    /// Number of patches
    std::size_t nPatches() const          { return m_patches.size(); }

    /// Returns a vector of paches // to do : replace by copies
    PatchContainer const& patches() const { return m_patches; }

    /// Return the \a i-th patch.
    const gsGeometry<T> & operator []( size_t i ) const { return *m_patches[i]; }

    /// Makes a deep copy of all bases and puts them in a vector
    std::vector<gsBasis<T> *> basesCopy() const;

    /// Return the \a i-th patch.
    gsGeometry<T>& patch( std::size_t i ) const
    {
        GISMO_ASSERT( i < m_patches.size(), "Invalid patch index requested from gsMultiPatch" );
        return *m_patches[i];
    }

    /// Return the basis of the \a i-th patch.
    gsBasis<T> & basis( std::size_t i ) const;

    /// Add a patch.
    void addPatch( gsGeometry<T>* g );

    // TO DO Add a patch by copying the argument.
    //void addPatch( cosnt gsGeometry<T> & geo );

    /// Search for the given geometry and return its patch index.
    int findPatchIndex( gsGeometry<T>* g ) const;

    /// @brief Add an interface joint betweeen side \a s1 of geometry
    /// \a g1 side \a s2 of geometry \a g2.
    ///
    /// \todo add orientation information
    void addInterface( gsGeometry<T>* g1, boundary::side s1,
            gsGeometry<T>* g2, boundary::side s2 );

    /// Add side s of patch g to the outer boundary of the domain
    void addPatchBoundary( gsGeometry<T>* g, boundary::side s ) {
        int p = findPatchIndex( g );
        gsBoxTopology::addBoundary( patch_side( p, s ) );
    }

    void uniformRefine(int numKnots = 1);

    /// Attempt to compute interfaces and boundaries automatically.
    bool computeTopology( T tol = 1e-4 );

    // Data members
private:

    PatchContainer m_patches;

}; // class gsMultiPatch


//////////////////////////////////////////////////
//////////////////////////////////////////////////

/// Print (as string) a multipatch structure
template<class T>
std::ostream& operator<<( std::ostream& os, const gsMultiPatch<T>& b )
{
    return b.print( os );
}


} // namespace gismo


#ifndef GISMO_HEADERS_ONLY
#include GISMO_HPP_HEADER(gsMultiPatch.hpp)
#endif