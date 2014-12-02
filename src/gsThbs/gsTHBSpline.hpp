#pragma once

#include <gsNurbs/gsBoehm.h>

namespace gismo{

//////////////////////////////////////////////////
// Public member functions
//////////////////////////////////////////////////


template<unsigned d, class T>
void gsTHBSpline<d, T>::convertToBSpline( gsTensorBSpline<d,T,gsCompactKnotVector<T> >& result )
{
    // Construct a box covering the whole parameter domain.
    const typename gsHDomain<d>::point & uCorner = this->basis().tree().upperCorner();
    std::vector<unsigned> wholeDomainAsBox(2*d+1,0);

    wholeDomainAsBox[0] = this->basis().tree().getMaxInsLevel();

    std::copy(uCorner.data(), uCorner.data()+d, wholeDomainAsBox.begin()+d+1);

    // Refine the whole domain to the finest level present there.
    refineElements( wholeDomainAsBox );

    // The & avoids copying of the basis.
    gsTensorBSplineBasis<d,T, gsCompactKnotVector<T> > &tpBasis = this->basis().tensorLevel(this->basis().tree().getMaxInsLevel());
    // makeGeometry returns an abstract class, so we need to cast to the particular.
    gsTensorBSpline<d,T,gsCompactKnotVector<T> > *newGeo = static_cast< gsTensorBSpline<d,T,gsCompactKnotVector<T> > *>(tpBasis.makeGeometry(this->coefs()));

    // Should work but crashes:
    //std::swap (*newGeo, result );
    // Instead, use
    result = *newGeo;

    // Don't forget:
    delete newGeo;
}


/// Return the list of B-spline patches to represent a THB-spline geometry
/// \returns b1 bottom left corners of the box (vector of indices with respect to the gsCompactKnotVector of the highest possible level)
/// \returns b2 top right corners of the box (vector of indices with respect to the gsCompactKnotVector of the highest possible level)
/// \returns level levels of the boxes (level[i]: level of the i-th box,)
/// \returns bpatches list of B-spline patches associated with the boxes
template<unsigned d, class T>
//void gsTHBSplineBasis<d,T>::getBsplinePatches(gsMatrix<T>& geom_coef, gsMatrix<T>& cp, gsMatrix<unsigned>& b1, gsMatrix<unsigned>& b2, gsVector<unsigned>& level, gsMatrix<unsigned>& nvertices) const
//void gsTHBSpline<d,T>::getBsplinePatches(gsMatrix<unsigned>& b1, gsMatrix<unsigned>& b2, gsVector<unsigned>& level, std::vector< gsTensorBSpline<2> >& bpatches) const
void gsTHBSpline<d, T>::getBsplinePatches(gsMatrix<unsigned>& b1, gsMatrix<unsigned>& b2, gsVector<unsigned>& level) const
{
    //------------------------------------------------------------------------------------------------------------------------------
    // define the 6 boxes and the corresponding levels for the MTU fillet example *** TO BE SUBSTITUTED WITH AUTOMATIC SPLITTING ***
    //------------------------------------------------------------------------------------------------------------------------------
    /*int nboxes = 6;
    level.resize(nboxes);
    b1.resize(nboxes,this->dim());
    b2.resize(nboxes,this->dim());

    level[0] = 0; b1(0,0) =  0; b1(0,1) =   0; b2(0,0) = 16; b2(0,1) = 80;
    level[1] = 1; b1(1,0) = 16; b1(1,1) =   0; b2(1,0) = 40; b2(1,1) = 80;
    level[2] = 2; b1(2,0) = 40; b1(2,1) =   0; b2(2,0) = 60; b2(2,1) = 80;
    level[3] = 2; b1(3,0) = 60; b1(3,1) =   0; b2(3,0) = 80; b2(3,1) = 16;
    level[4] = 2; b1(4,0) = 60; b1(4,1) =  66; b2(4,0) = 80; b2(4,1) = 80;
    level[5] = 3; b1(5,0) = 60; b1(5,1) =  16; b2(5,0) = 80; b2(5,1) = 66;*/
    //------------------------------------------------------------------------------------------------------------------------------
    // define the 8 boxes and the corresponding levels for the face example *** TO BE SUBSTITUTED WITH AUTOMATIC SPLITTING ***
    //------------------------------------------------------------------------------------------------------------------------------
    /*int nboxes = 8;
    level.resize(nboxes);
    b1.resize(nboxes,this->dim());
    b2.resize(nboxes,this->dim());

    level[0] = 1; b1(0,0) =  0; b1(0,1) =  20; b2(0,0) = 32; b2(0,1) = 24;
    level[1] = 2; b1(1,0) =  0; b1(1,1) =   0; b2(1,0) = 48; b2(1,1) = 20;
    level[2] = 2; b1(2,0) = 32; b1(2,1) =  20; b2(2,0) = 48; b2(2,1) = 24;
    level[3] = 2; b1(3,0) =  0; b1(3,1) =  24; b2(3,0) = 12; b2(3,1) = 96;
    level[4] = 2; b1(4,0) = 12; b1(4,1) =  24; b2(4,0) = 36; b2(4,1) = 48;
    level[5] = 2; b1(5,0) = 36; b1(5,1) =  24; b2(5,0) = 48; b2(5,1) = 96;
    level[6] = 2; b1(6,0) = 12; b1(6,1) =  82; b2(6,0) = 36; b2(6,1) = 96;
    level[7] = 3; b1(7,0) = 12; b1(7,1) =  48; b2(7,0) = 36; b2(7,1) = 82;*/
    //------------------------------------------------------------------------------------------------------------------------------
    // define the 5 boxes and the corresponding levels for file 15 *** TO BE SUBSTITUTED WITH AUTOMATIC SPLITTING ***
    //------------------------------------------------------------------------------------------------------------------------------
    /*int nboxes = 5;
    level.resize(nboxes);
    b1.resize(nboxes,this->dim());
    b2.resize(nboxes,this->dim());

    level[0] = 0; b1(0,0) =  0; b1(0,1) =   0; b2(0,0) = 16; b2(0,1) = 24;    //int l = 0;  b1[0] = 0;   b1[1] = 0;   b2[0] = 16;   b2[1] = 24;

    level[1] = 1; b1(1,0) =  16; b1(1,1) =   0; b2(1,0) = 28; b2(1,1) = 8;    //int l = 1;  b1[0] = 16;   b1[1] = 0;   b2[0] = 28;   b2[1] = 8;
    level[2] = 1; b1(2,0) =  16; b1(2,1) =  16; b2(2,0) = 28; b2(2,1) = 24;   //int l = 1;  b1[0] = 16;   b1[1] = 16;   b2[0] = 28;   b2[1] = 24;
    level[3] = 1; b1(3,0) =  24; b1(3,1) =   8; b2(3,0) = 28; b2(3,1) = 16;   //int l = 1;  b1[0] = 24;   b1[1] = 0;   b2[0] = 28;   b2[1] = 16;

    level[4] = 2; b1(4,0) =  16; b1(4,1) =  8; b2(4,0) = 24; b2(4,1) = 16;*/    //int l = 2;  b1[0] = 16;   b1[1] = 8;   b2[0] = 24;   b2[1] = 16;
    //------------------------------------------------------------------------------------------------------------------------------
    // AUTOMATIC SPLITTING
    //------------------------------------------------------------------------------------------------------------------------------
    //this->basis().component(1); //m_tree.getBoxes(b1,b2,level);
    //this->basis().component(i).degree();
    //int nboxes = level.size();
    //------------------------------------------------------------------------------------------------------------------------------
    // iteration on the boxes to call getBsplinePatchGlobal()
    //------------------------------------------------------------------------------------------------------------------------------
    /*gsVector<unsigned> p1, p2;
    p1.resize(this->dim());
    p2.resize(this->dim());
    gsMatrix<T> temp1, temp2;
    gsCompactKnotVector<T> cku, ckv;
    nvertices.resize(nboxes,this->dim());

    for (int i = 0; i < nboxes; i++){
        p1(0) = b1(i,0); p1(1) = b1(i,1); p2(0) = b2(i,0); p2(1) = b2(i,1);
        this->getBsplinePatchGlobal(p1, p2, level[i], geom_coef, temp1, cku, ckv);

        std::cout<<"cku: "<<cku<<std::endl<<"ckv: "<<ckv<<std::endl;

        if (i == 0){
            cp = temp1;
        }else{
            int cprows = cp.rows();
            temp2.resize(cp.rows()+temp1.rows(),cp.cols()); // is there a way to append the temp1 matrix to cp without temp2?

            for (int j = 0; j < cp.rows(); j++){
                for (int k = 0; k < cp.cols(); k++){
                    temp2(j,k) = cp(j,k);
                }
            }

            for (int j = 0; j < temp1.rows(); j++){
                for (int k = 0; k < temp1.cols(); k++){
                    //cout<<cprows+j<<endl;
                    temp2(cprows+j,k) = temp1(j,k);
                }
            }
            cp = temp2;
        }

        nvertices(i,0) = cku.size()-cku.degree()-1;
        nvertices(i,1) = ckv.size()-ckv.degree()-1;

   }*/
    //cout<<cp<<endl<<endl;
    //cout<<cp.rows()<<endl;//ok
    //cout<<"b1 "<<b1.rows()<<endl;
    //cout<<"b2 "<<b2.rows()<<endl;
    //cout<<"level "<<level.rows()<<endl;
    //cout<<"nv "<<nvertices.rows()<<endl;//ko
}
//////////////////////////////////////////////////
// Private member functions
//////////////////////////////////////////////////

///
/// Return the B-spline representation of a THB-spline subpatch
/// \param b1 bottom left corner of the box (vector of indices with respect to the gsCompactKnotVector of the highest possible level)
/// \param b2 top right corner of the box (vector of indices with respect to the gsCompactKnotVector of the highest possible level)
/// \param level level of the box
/// \param geom_coef control points of the THB-spline geometry
/// \returns cp control control points of the B-spline patch
/// \returns k1 knot vector of the B-spline patch (first dimension)
/// \returns k2 knot vector of the B-spline patch (second dimension)
/*template<unsigned d, class T>
void gsTHBSplineBasis<d,T>::getBsplinePatchGlobal(gsVector<unsigned> b1, gsVector<unsigned> b2, unsigned level, gsMatrix<T>& geom_coef, gsMatrix<T>& cp, gsCompactKnotVector<T>& k1, gsCompactKnotVector<T>& k2) const
{
    // check if the indices in b1, and b2 are correct with respect to the given level
    if(b1[0]%Qlocal2global(1,0, level) != 0 ){
        b1[0] -= b1[0]%Qlocal2global(1,0, level);
    }
    if(b1[1]%Qlocal2global(1,0, level) != 0 ){
        b1[1] -= (b1[1]%Qlocal2global(1,0, level));
    }
    if(b2[0]%Qlocal2global(1,0, level) != 0){
        b2[0] += Qlocal2global(1,0, level) -(b2[0]%Qlocal2global(1,0, level));
    }
    if(b2[1]%Qlocal2global(1,0, level) != 0){
        b2[1] += Qlocal2global(1,0, level) -(b2[1]%Qlocal2global(1,0, level));
    }
    // select the indices of all B-splines of the given level acting on the given box
    int i0 = Qglobal2local(b1[0],level,this->m_tree.m_index_level);
    i0 = this->m_bases[level]->component(0).knots().lastKnotIndex(i0) - this->m_deg[0];
    int i1 = Qglobal2local(b2[0],level,this->m_tree.m_index_level);
    i1 = this->m_bases[level]->component(0).knots().firstKnotIndex(i1) - 1;

    int j0 = Qglobal2local(b1[1],level,this->m_tree.m_index_level);
    j0 = this->m_bases[level]->component(1).knots().lastKnotIndex(j0) - this->m_deg[1];
    int j1 = Qglobal2local(b2[1],level,this->m_tree.m_index_level);
    j1 = this->m_bases[level]->component(1).knots().firstKnotIndex(j1) - 1;

    for(int i = 0; i < geom_coef.cols(); i++){
        initialize_cmatrix(geom_coef, i, level);
        gsMatrix<T> *temp = new gsMatrix<T>(1,1);
        globalRefinement(level, *temp);

        for(int k = i0; k <= i1; k++){
            for(int j = j0; j <= j1; j++){
                (*temp)(j-j0,k-i0) = (*temp)(j,k);
            }
        }
        temp->conservativeResize(j1-j0+1,i1-i0+1);

        if(i == 0){
            cp.resize(temp->cols()*temp->rows(), geom_coef.cols());
        }
        return_cp_1D(*temp, i, cp);
    }
    // compute the new vectors for the B-spline patch
    k1 = gsCompactKnotVector<T>(this->m_deg[0], this->m_bases[level]->component(0).knots().begin() + i0 , this->m_bases[level]->component(0).knots().begin() + i1 + this->m_deg[0] + 2);
    k2 = gsCompactKnotVector<T>(this->m_deg[1], this->m_bases[level]->component(1).knots().begin() + j0 , this->m_bases[level]->component(1).knots().begin() + j1 + this->m_deg[1] + 2);
}*/

///
/// Function called by getBsplinePatchGlobal
/// \param level
/*template<unsigned d, class T>
void gsTHBSplineBasis<d,T>::globalRefinement(int level, gsMatrix<T>& coeffs)const
{
    //coeffs.resize(this->m_bases[0]->component(1).knots().size()-this->m_deg[1]-1,this->m_bases[0]->component(0).knots().size()-this->m_deg[0]-1);
    coeffs.resize(this->m_bases[0]->size(1),this->m_bases[0]->size(0));
    for(int j = 0; j < coeffs.rows(); j++){
        for(int k = 0; k < coeffs.cols(); k++){
            unsigned s  = this->m_bases[0]->index(k,j);//this->fromPair(k,j, 0);
            if(this->m_xmatrix[0].count( s ) > 0){
                coeffs(j,k) = this->m_cmatrix[0].find( s )->second;
            }else{
                coeffs(j,k) = 0;
            }
        }
    }

    for(int l = 1; l <=level; l++){

        // global dyadic refinement with respect to previous level
        gsCompactKnotVector<T>k1;
        gsCompactKnotVector<T>k2;
        k1 = this->m_bases[l-1]->component(0).knots();
        k2 = this->m_bases[l-1]->component(1).knots();
        std::vector<T> knots_x;
        std::vector<T> knots_y;
        for(unsigned int i = 1; i < this->m_bases[l]->component(0).knots().unique().size(); i = i+2){
            knots_x.push_back(this->m_bases[l]->component(0).knots().unique()[i]);
        }
        for(unsigned int i = 1; i < this->m_bases[l]->component(1).knots().unique().size(); i = i+2){
            knots_y.push_back(this->m_bases[l]->component(1).knots().unique()[i]);
        }

        gsBoehmRefine(k2,coeffs,this->m_deg[1],knots_y);
        coeffs.transposeInPlace();

        gsBoehmRefine(k1,coeffs,this->m_deg[0],knots_x);
        coeffs.transposeInPlace();

        //overwrite the whole matrix
        for(int j = 0; j < coeffs.rows(); j++){
            for(int k = 0; k < coeffs.cols(); k++){
                unsigned s  = this->m_bases[l]->index(k,j);
                if(this->m_xmatrix[l].count( s ) > 0){
                    coeffs(j,k) = this->m_cmatrix[l].find( s )->second;
                }
            }
        }
    }
}*/

///
/// Initializes the m_cmatrix up to given level with the coeffs of the geometry
/// \param col dimension (0, 1, 2 = x, y, z)
/// \param c_level level
/*template<unsigned d, class T>
void gsTHBSplineBasis<d,T>::initialize_cmatrix(gsMatrix<T>&geom_coeff, int col, int c_level) const{
    int counter = 0;
    for(int i = 0; i <= c_level; i++){
        this->m_cmatrix.push_back( std::map <unsigned, T>());
        for(typename CMatrix::const_iterator it =
                this->m_xmatrix[i].begin(); it != this->m_xmatrix[i].end(); it++){
            this->m_cmatrix[i][ *it ] = geom_coeff(counter,col);
            counter++;
        }
    }
}*/

///
/// Converts the coefficient matrix to a column of the control points matrix with respect to the given direction
/// \param mat
/// \param direction dimension (0, 1, 2 = x, y, z)
/// \returns cp the column direction of cp is updated with the related coordinate of the control points
/*template<unsigned d, class T>
void gsTHBSplineBasis<d,T>::return_cp_1D(const gsMatrix<T> & mat, int direction, gsMatrix<T>& cp)const{
    GISMO_ASSERT((mat.cols()*mat.rows() == cp.rows()), "Wrong matrix dimension.");
    int counter = 0;
    for(int j = 0; j < mat.rows(); j++){
        for(int i = 0; i < mat.cols(); i++){
            cp(counter, direction) = mat(j,i);
            counter ++;
        }
    }
}*/

}
