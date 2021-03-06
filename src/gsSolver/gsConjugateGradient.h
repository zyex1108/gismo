/** @file gsConjugateGradient.h

    @brief Conjugate gradient solver

    This file is part of the G+Smo library.

    This Source Code Form is subject to the terms of the Mozilla Public
    License, v. 2.0. If a copy of the MPL was not distributed with this
    file, You can obtain one at http://mozilla.org/MPL/2.0/.

    Author(s): C. Hofreither
*/

#pragma once

#include <gsSolver/gsIterativeSolver.h>

namespace gismo
{

/** The conjugate gradient implementation from Eigen, adapted to allow for more
 *  general preconditioners and better iteration control. Also capable of using
 *  a gsLinearOperator as matrix.
 */

class GISMO_EXPORT gsConjugateGradient : public gsIterativeSolver<real_t>
{
public:
    typedef gsIterativeSolver<real_t> Base;
    
    typedef gsMatrix<real_t>  VectorType;
    
    typedef Base::LinOpPtr LinOpPtr;
    
    /// Constructor using a matrix (operator) and optionally a preconditionner
    template< typename OperatorType >
    explicit gsConjugateGradient( const OperatorType& mat, const LinOpPtr & precond = LinOpPtr() )
    : Base(mat, precond) {}

    /// @brief Returns a list of default options
    static gsOptionList defaultOptions()
    {
        gsOptionList opt = Base::defaultOptions();
        opt.addSwitch("CalcEigenvalues", "Additionally to solving the system,"
                      " CG computes the eigenvalues of the Lanczos matrix", false );
        return opt;
    }
    
    void setOptions(const gsOptionList & opt)
    {
        Base::setOptions(opt);
        m_calcEigenvals = opt.askSwitch("CalcEigenvalues", m_calcEigenvals);
    }

    bool initIteration( const VectorType& rhs, VectorType& x );
    bool step( VectorType& x );

    /// @brief specify if you want to store data for eigenvalue estimation
    /// @param flag true stores the coefficients of the lancos matrix, false not.
    void setCalcEigenvalues( bool flag )     { m_calcEigenvals = flag ;}

    /// @brief returns the condition number of the (preconditioned) system matrix
    real_t getConditionNumber();

    /// @brief returns the eigenvalues of the Lanczos matrix
    void getEigenvalues( gsMatrix<real_t>& eigs );

private:
    using Base::m_mat;
    using Base::m_precond;
    using Base::m_max_iters;
    using Base::m_tol;
    using Base::m_num_iter;
    using Base::m_rhs_norm;
    using Base::m_error;


    VectorType m_res;
    VectorType m_update;
    VectorType m_tmp;
    real_t m_abs_new;

    bool m_calcEigenvals;
    bool m_eigsAreCalculated;

    std::vector<real_t> delta, gamma;
};

} // namespace gismo

