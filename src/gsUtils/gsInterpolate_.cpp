#include <gsCore/gsTemplateTools.h>

#include <gsUtils/gsInterpolate.h>
#include <gsUtils/gsInterpolate.hpp>

#define T real_t
#define uZ unsigned
#define Z int

namespace gismo
{

TEMPLATE_INST
gsGeometry<T> * gsInterpolate( gsBasis<T> const& g, gsMatrix<T> 
			       const& pts, gsMatrix<T> const& vals );

TEMPLATE_INST
gsGeometry<T> * gsInterpolate( const gsBasis<T>& g, const gsFunction<T>& f );

// TEMPLATE_INST
// gsBSpline<T> gsInterpolate(gsKnotVector<T> & kv,const gsMatrix<T> & preImage,
// 			   const gsMatrix<T> & image,
//                            const gsMatrix<T> & preNormal,const gsMatrix<T> & normal,
//                            const gsMatrix<T> & preImageApp,const gsMatrix<T> & imageApp,
// 			   T const & w_reg,T const & w_app,
//                            gsMatrix<T> &outPointResiduals, gsMatrix<T> &outNormalResiduals);


TEMPLATE_INST
void gsL2ProjectOnBoundary( const gsBasis<T> & basis,
                           const gsFunction<T> & f,
                           const gsGeometry<T> & geo,
                           const gsVector<int> & Sides,
                           gsVector<unsigned> & vecIdx,
                           gsMatrix<T> & vecCoeff,
                           bool getGlobalData );

} // namespace gismo

#undef T
#undef uZ
#undef Z