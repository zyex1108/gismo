 
#pragma once

#include <gsIO/gsWriteParaview.h>

#include <gsIO/gsParaviewCollection.h>

#include <gsIO/gsIOUtils.h>

#include <gsCore/gsGeometry.h>
#include <gsModeling/gsTrimSurface.h>

#include <gsCore/gsField.h>
#include <gsCore/gsDebug.h>
#include <gsModeling/gsSolid.h>
#include <gsUtils/gsMesh/gsHeMesh.h>

#define PLOT_PRECISION 5

namespace gismo
{
  
/// Export a parametric mesh
template<class T>
void writeSingleBasisMesh(const gsBasis<T> & basis,
                         std::string const & fn)
{
    gsMesh<T> msh;
    makeMesh<T>(basis, msh);
    gsWriteParaview(msh, fn, false);
}

/// Export a computational mesh
template<class T>
void writeSingleCompMesh(const gsBasis<T> & basis, const gsGeometry<T> & Geo, 
                         std::string const & fn, unsigned resolution = 8)
{
    gsMesh<T> msh;
    makeMesh<T>(basis, msh, resolution);
    Geo.evaluateMesh(msh);
    gsWriteParaview(msh, fn, false);
}

/// Export a control net
template<class T>
void writeSingleControlNet(const gsGeometry<T> & Geo, 
                           std::string const & fn)
{
    gsMesh<T> msh;
    Geo.controlNet(msh);
    const unsigned n = Geo.geoDim();
    if ( n == 1 )
    {
        gsMatrix<T> anch = Geo.basis().anchors();
        // Lift vertices at anchor positions
        for ( int i = 0; i!= msh.numVertices; ++i)
        {
            msh.vertex[i]->y() = anch(0,i);
            std::swap( msh.vertex[i]->x(),  msh.vertex[i]->y() );
        }
    }


    gsWriteParaview(msh, fn, false);
}


/// Write a file containing a solution field over a single geometry
template<class T>
void writeSinglePatchField(const gsField<T> & field, int patchNr, 
                           std::string const & fn, unsigned npts)
{
    unsigned n = field.geoDim();
    unsigned d = field.parDim();

    gsMatrix<T> ab = field.patches().parameterRange(patchNr);
    gsVector<T> a = ab.col(0);
    gsVector<T> b = ab.col(1);

    gsVector<unsigned> np = uniformSampleCount(a, b, npts);
    gsMatrix<T> pts = gsPointGrid(a, b, np);

    gsMatrix<T> eval_geo = field.point ( pts, patchNr );//pts

    if ( 3 - d > 0 )
    {
        np.conservativeResize(3);
        np.bottomRows(3-d).setOnes();
    }

    if ( 3 - n > 0 )
    {
        eval_geo.conservativeResize(3,eval_geo.cols() );
        eval_geo.bottomRows(3-n).setZero();
    }

    gsMatrix<T>  eval_field = field.value ( pts, patchNr );//values
    GISMO_ASSERT( eval_field.rows() == field.dim(), "Error in field dimension");
    if ( eval_field.rows() > 1 )
    {
        eval_field.conservativeResize(3,eval_geo.cols() );
        eval_field.bottomRows( 3-field.dim() ).setZero();
    }
    
    std::string mfn(fn);
    mfn.append(".vts");
    std::ofstream file(mfn.c_str());
    file << std::fixed; // no exponents
    file << std::setprecision (PLOT_PRECISION);

    file <<"<?xml version=\"1.0\"?>\n";
    file <<"<VTKFile type=\"StructuredGrid\" version=\"0.1\">\n";
    file <<"<StructuredGrid WholeExtent=\"0 "<< np(0)-1<<" 0 "<<np(1)-1<<" 0 "<<np(2)-1<<"\">\n";
    file <<"<Piece Extent=\"0 "<< np(0)-1<<" 0 "<<np(1)-1<<" 0 "<<np(2)-1<<"\">\n";
    file <<"<PointData "<< ( field.dim()==1 ?"Scalars":"Vectors")<<"=\"SolutionField\">\n";
    file <<"<DataArray type=\"Float32\" Name=\"SolutionField\" format=\"ascii\" NumberOfComponents=\""<< eval_field.rows() <<"\">\n";
    for ( index_t j=0; j<eval_field.cols(); ++j)
        for ( index_t i=0; i<eval_field.rows(); ++i)
            file<< eval_field(i,j) <<" ";
    file <<"</DataArray>\n";
    file <<"</PointData>\n";
    file <<"<Points>\n";
    file <<"<DataArray type=\"Float32\" NumberOfComponents=\""<<eval_geo.rows()<<"\">\n";
    for ( index_t j=0; j<eval_geo.cols(); ++j)
        for ( index_t i=0; i<eval_geo.rows(); ++i)
            file<< eval_geo.at(i,j) <<" ";
    file <<"</DataArray>\n";
    file <<"</Points>\n";
    file <<"</Piece>\n";
    file <<"</StructuredGrid>\n";
    file <<"</VTKFile>\n";

    file.close();

}

template<class T>
void writeSingleGeometry(const gsGeometry<T> & Geo, std::string const & fn, unsigned npts)
{
//    gsMesh<T> msh;
//    Geo.toMesh(msh, npts);
//    gsWriteParaview(msh, fn);
//    return;

    const unsigned n = Geo.geoDim();
    const unsigned d = Geo.parDim();

    gsMatrix<T> ab = Geo.parameterRange() ;
    gsVector<T> a = ab.col(0);
    gsVector<T> b = ab.col(1);
    gsVector<unsigned> np = uniformSampleCount(a,b, npts );
    gsMatrix<T> pts = gsPointGrid(a,b,np) ;

    gsMatrix<T>  eval_geo = Geo.eval  ( pts ) ;//pts

    if ( 3 - d > 0 )
    {
        np.conservativeResize(3);
        np.bottomRows(3-d).setOnes();
    }

    if ( 3 - n > 0 )
    {
        eval_geo.conservativeResize(3,eval_geo.cols() );
        eval_geo.bottomRows(3-n).setZero();

        if ( n == 1 )
        {
            eval_geo.row(1) = eval_geo.row(0);
            eval_geo.row(0) = pts;
        }            
    }

    std::string mfn(fn);
    mfn.append(".vts");
    std::ofstream file(mfn.c_str());
    if ( ! file.is_open() )
        std::cout<<"Problem opening "<<fn<<std::endl;
    file << std::fixed; // no exponents
    file << std::setprecision (PLOT_PRECISION);
    file <<"<?xml version=\"1.0\"?>\n";
    file <<"<VTKFile type=\"StructuredGrid\" version=\"0.1\">\n";
    file <<"<StructuredGrid WholeExtent=\"0 "<<np(0)-1<<" 0 "<<np(1)-1<<" 0 "<<np(2)-1<<"\">\n";
    file <<"<Piece Extent=\"0 "<< np(0)-1<<" 0 "<<np(1)-1<<" 0 "<<np(2)-1<<"\">\n";
    // Add norm of the point as data 
    // file <<"<PointData Scalars =\"PointNorm\">\n";
    // file <<"<DataArray type=\"Float32\" Name=\"PointNorm\" format=\"ascii\" NumberOfComponents=\""<< 1 <<"\">\n";
    // for ( index_t j=0; j<eval_geo.cols(); ++j)
    //     file<< eval_geo.col(j).norm() <<" ";
    // file <<"</DataArray>\n";
    // file <<"</PointData>\n";
    // end norm
    file <<"<Points>\n";
    file <<"<DataArray type=\"Float32\" NumberOfComponents=\""<<eval_geo.rows()<<"\">\n";
    for ( index_t j=0; j<eval_geo.cols(); ++j)
        for ( index_t i=0; i<eval_geo.rows(); ++i)
            file<< eval_geo.at(i,j) <<" ";
    file <<"</DataArray>\n";
    file <<"</Points>\n";
    file <<"</Piece>\n";
    file <<"</StructuredGrid>\n";
    file <<"</VTKFile>\n";
    file.close();

}

template<class T>
void writeSingleTrimSurface(const gsTrimSurface<T> & surf, 
                            std::string const & fn, 
                            unsigned npts)
{
    gsMesh<T> * msh = surf.toMesh(npts);
    gsWriteParaview( *msh, fn);
    delete msh;
}

/// Write a file containing a solution field over a geometry
template<class T>
void gsWriteParaview(const gsField<T> & field, 
                     std::string const & fn, 
                     unsigned npts, bool mesh)
{
    if (mesh && (!field.isParametrized()) )
    {
        gsWarn<< "Cannot plot mesh from non-parametric field.";
        mesh = false;
    }

    const unsigned n = field.nPatches();
    gsParaviewCollection collection(fn);

    for ( unsigned i=0; i < n; ++i )
    {
        std::string fileName = fn + internal::toString<unsigned>(i);
        writeSinglePatchField( field, i, fileName, npts );
        collection.addPart(fileName, ".vts");
        if ( mesh ) 
        {
            fileName+= "_mesh";
            writeSingleCompMesh(field.igaFunction(i).basis(), 
                                field.patch(i), fileName);

            collection.addPart(fileName, ".vtp");
        }

    }
    collection.save();
}


/// Export a Geometry without scalar information
template<class T>
void gsWriteParaview(const gsGeometry<T> & Geo, std::string const & fn, 
                     unsigned npts, bool mesh, bool ctrlNet)
{
    gsParaviewCollection collection(fn);
    
    writeSingleGeometry(Geo, fn, npts);
    collection.addPart(fn, ".vts");

    if ( mesh ) // Output the underlying mesh
    {
        const std::string fileName = fn + "_mesh";
        writeSingleCompMesh(Geo.basis(), Geo, fileName, npts);
        collection.addPart(fileName, ".vtp");
    }

    if ( ctrlNet ) // Output the control net
    {
        const std::string fileName = fn + "_cnet";
        writeSingleControlNet(Geo, fileName);
        collection.addPart(fileName, ".vtp");
    }

    // Write out the collection file
    collection.save();
}

/// Export a multipatch Geometry without scalar information
template<class T>
void gsWriteParaview( std::vector<gsGeometry<T> *> const & Geo, std::string const & fn, 
                      unsigned npts, bool mesh, bool ctrlNet)
{
    const size_t n = Geo.size();
    gsParaviewCollection collection(fn);

    for ( size_t i=0; i<n ; i++)
    {
        std::string fnBase = fn + internal::toString<index_t>(i);
        writeSingleGeometry( *Geo[i], fnBase, npts ) ;
        collection.addPart(fnBase, ".vts");

        if ( mesh ) 
        {
            const std::string fileName = fnBase + "_mesh";
            writeSingleCompMesh(Geo[i]->basis(), *Geo[i], fileName);
            collection.addPart(fileName, ".vtp");
        }
        
        if ( ctrlNet ) // Output the control net
        {
            const std::string fileName = fnBase + "_cnet";
            writeSingleControlNet(*Geo[i], fileName);
            collection.addPart(fileName, ".vtp");
        }
    }
    collection.save();
}

/// Export i-th Basis function
template<class T>
void gsWriteParaview_basisFnct(int i, gsBasis<T> const& basis, std::string const & fn, unsigned npts)
{
    // basis.support(i) --> returns a (tight) bounding box for the
    // supp. of i-th basis func.
    int d= basis.dim();
    int n= d+1;

    gsMatrix<T> ab = basis.support(i) ;
    gsVector<T> a = ab.col(0);
    gsVector<T> b = ab.col(1);
    gsVector<unsigned> np = uniformSampleCount(a,b, npts );
    gsMatrix<T> pts = gsPointGrid(a,b,np) ;

    gsMatrix<T>  eval_geo = basis.evalSingle ( i, pts ) ;

    if ( 3 - d > 0 )
    {
        np.conservativeResize(3);
        np.bottomRows(3-d).setOnes();
    }

    if ( 2 - d > 0 )
    {
        pts.conservativeResize(2,eval_geo.cols());
        pts.bottomRows(2-d).setZero();
    }

    if ( d > 2 )
    {
        gsWarn<<"Info: The dimension is to big, projecting into first 2 coordinatess..\n";
    d=2;
        pts.conservativeResize(2,eval_geo.cols());
    }

    if ( 3 - n > 0 )
    {
        eval_geo.conservativeResize(3,eval_geo.cols() );
        eval_geo.bottomRows(3-n).setZero();
    }

    std::string mfn(fn);
    mfn.append(".vts");
    std::ofstream file(mfn.c_str());
    if ( ! file.is_open() )
        std::cout<<"Problem opening "<<fn<<std::endl;
    file << std::fixed; // no exponents
    file << std::setprecision (PLOT_PRECISION);
    file <<"<?xml version=\"1.0\"?>\n";
    file <<"<VTKFile type=\"StructuredGrid\" version=\"0.1\">\n";
    file <<"<StructuredGrid WholeExtent=\"0 "<<np(0)-1<<" 0 "<<np(1)-1<<" 0 "<<np(2)-1<<"\">\n";
    file <<"<Piece Extent=\"0 "<< np(0)-1<<" 0 "<<np(1)-1<<" 0 "<<np(2)-1<<"\">\n";
    // Scalar information
    file <<"<PointData "<< "Scalars"<<"=\"SolutionField\">\n";
    file <<"<DataArray type=\"Float32\" Name=\"SolutionField\" format=\"ascii\" NumberOfComponents=\""<<1<<"\">\n";
    for ( index_t j=0; j<eval_geo.cols(); ++j)
            file<< eval_geo.at(0,j) <<" ";
    file <<"</DataArray>\n";
    file <<"</PointData>\n";
    //
    file <<"<Points>\n";
    file <<"<DataArray type=\"Float32\" NumberOfComponents=\""<<3<<"\">\n";
    for ( index_t j=0; j<eval_geo.cols(); ++j)
    {
        for ( int i=0; i< d; ++i)
            file<< pts(i,j) <<" ";
        file<< eval_geo.at(0,j) <<" ";
         for ( index_t i=d; i< pts.rows(); ++i)
             file<< pts(i,j) <<" ";
    }
    file <<"</DataArray>\n";
    file <<"</Points>\n";
    file <<"</Piece>\n";
    file <<"</StructuredGrid>\n";
    file <<"</VTKFile>\n";
    file.close();
}


/// Export a function
template<class T>
void gsWriteParaview(gsFunction<T> const& func, gsMatrix<T> const& supp, std::string const & fn, unsigned npts)
{
    int d= 2;
    //int n= d+1;

    gsVector<T> a = supp.col(0);
    gsVector<T> b = supp.col(1);
    gsVector<unsigned> np = uniformSampleCount(a,b, npts );
    gsMatrix<T> pts = gsPointGrid(a,b,np);

    gsMatrix<T> ev;
    func.eval_into(pts, ev);

    if ( 3 - d > 0 )
    {
        np.conservativeResize(3);
        np.bottomRows(3-d).setOnes();
    }

    std::string mfn(fn);
    mfn.append(".vts");
    std::ofstream file(mfn.c_str());
    if ( ! file.is_open() )
        std::cout<<"Problem opening "<<fn<<std::endl;
    file << std::fixed; // no exponents
    file << std::setprecision (PLOT_PRECISION);
    file <<"<?xml version=\"1.0\"?>\n";
    file <<"<VTKFile type=\"StructuredGrid\" version=\"0.1\">\n";
    file <<"<StructuredGrid WholeExtent=\"0 "<<np(0)-1<<" 0 "<<np(1)-1<<" 0 "<<np(2)-1<<"\">\n";
    file <<"<Piece Extent=\"0 "<< np(0)-1<<" 0 "<<np(1)-1<<" 0 "<<np(2)-1<<"\">\n";
    // Scalar information
    file <<"<PointData "<< "Scalars"<<"=\"SolutionField\">\n";
    file <<"<DataArray type=\"Float32\" Name=\"SolutionField\" format=\"ascii\" NumberOfComponents=\""<<1<<"\">\n";
    for ( index_t j=0; j<ev.cols(); ++j)
            file<< ev(0,j) <<" ";
    file <<"</DataArray>\n";
    file <<"</PointData>\n";
    //
    file <<"<Points>\n";
    file <<"<DataArray type=\"Float32\" NumberOfComponents=\""<<3<<"\">\n";
    for ( index_t j=0; j<ev.cols(); ++j)
    {
        for ( int i=0; i< d; ++i)
            file<< pts(i,j) <<" ";
        file<< ev(0,j) <<" ";
//         for ( index_t i=d; i< pts.rows(); ++i)
//             file<< pts(i,j) <<" ";
    }
    file <<"</DataArray>\n";
    file <<"</Points>\n";
    file <<"</Piece>\n";
    file <<"</StructuredGrid>\n";
    file <<"</VTKFile>\n";
    file.close();
}


/// Export Basis functions
template<class T>
void gsWriteParaview(gsBasis<T> const& basis, std::string const & fn, 
                     unsigned npts, bool mesh)
{
    const index_t n = basis.size();
    gsParaviewCollection collection(fn);

    for ( index_t i=0; i< n; i++)
    {
        std::string fileName = fn + internal::toString<index_t>(i);
        gsWriteParaview_basisFnct<T>(i, basis, fileName, npts ) ;
        collection.addPart(fileName, ".vts");
    }

    if ( mesh )
    {
        std::string fileName = fn + "_mesh";
        writeSingleBasisMesh(basis, fileName);
        collection.addPart(fileName, ".vtp");
    }

    collection.save();
}


/// Export Point set to Paraview
template<class T>
void gsWriteParaviewPoints(gsMatrix<T> const& X, gsMatrix<T> const& Y, std::string const & fn)
{
    assert( X.cols() == Y.cols() );
    assert( X.rows() == 1 && Y.rows() == 1 );
    index_t np = X.cols();

    std::string mfn(fn);
    mfn.append(".vtp");
    std::ofstream file(mfn.c_str());
    if ( ! file.is_open() )
        std::cout<<"Problem opening "<<fn<<std::endl;
    file << std::fixed; // no exponents
    file << std::setprecision (PLOT_PRECISION);
    file <<"<?xml version=\"1.0\"?>\n";
    file <<"<VTKFile type=\"PolyData\" version=\"0.1\" byte_order=\"LittleEndian\">\n";
    file <<"<PolyData>\n";
    file <<"<Piece NumberOfPoints=\""<<np<<"\" NumberOfVerts=\"1\" NumberOfLines=\"0\" NumberOfStrips=\"0\" NumberOfPolys=\"0\">\n";
    file <<"<PointData>\n";
    file <<"</PointData>\n";
    file <<"<CellData>\n";
    file <<"</CellData>\n";
    file <<"<Points>\n";
    file <<"<DataArray type=\"Float32\" Name=\"Points\" NumberOfComponents=\"3\" format=\"ascii\" RangeMin=\""<<X.minCoeff()<<"\" RangeMax=\""<<X.maxCoeff()<<"\">\n";
    for (index_t i=0; i< np; ++i )
        file << X(0,i) <<" "<<Y(0,i)<<" "<< 0.0 <<"\n";
    file <<"\n</DataArray>\n";
    file <<"</Points>\n";
    file <<"<Verts>\n";
    file <<"<DataArray type=\"Int64\" Name=\"connectivity\" format=\"ascii\" RangeMin=\""<<0<<"\" RangeMax=\""<<np-1<<"\">\n";
    for (index_t i=0; i< np; ++i )
        file << i<<" ";
    file <<"\n</DataArray>\n";
    file <<"<DataArray type=\"Int64\" Name=\"offsets\" format=\"ascii\" RangeMin=\""<<np<<"\" RangeMax=\""<<np<<"\">\n"<<np<<"\n";
    file <<"</DataArray>\n";
    file <<"</Verts>\n";
    file <<"<Lines>\n";
    file <<"<DataArray type=\"Int64\" Name=\"connectivity\" format=\"ascii\" RangeMin=\"0\" RangeMax=\""<<np-1<<"\">\n";
    file <<"</DataArray>\n";
    file <<"<DataArray type=\"Int64\" Name=\"offsets\" format=\"ascii\" RangeMin=\""<<np<<"\" RangeMax=\""<<np<<"\">\n";
    file <<"</DataArray>\n";
    file <<"</Lines>\n";
    file <<"<Strips>\n";
    file <<"<DataArray type=\"Int64\" Name=\"connectivity\" format=\"ascii\" RangeMin=\"0\" RangeMax=\""<<np-1<<"\">\n";
    file <<"</DataArray>\n";
    file <<"<DataArray type=\"Int64\" Name=\"offsets\" format=\"ascii\" RangeMin=\""<<np<<"\" RangeMax=\""<<np<<"\">\n";
    file <<"</DataArray>\n";
    file <<"</Strips>\n";
    file <<"<Polys>\n";
    file <<"<DataArray type=\"Int64\" Name=\"connectivity\" format=\"ascii\" RangeMin=\"0\" RangeMax=\""<<np-1<<"\">\n";
    file <<"</DataArray>\n";
    file <<"<DataArray type=\"Int64\" Name=\"offsets\" format=\"ascii\" RangeMin=\""<<np<<"\" RangeMax=\""<<np<<"\">\n";
    file <<"</DataArray>\n";
    file <<"</Polys>\n";
    file <<"</Piece>\n";
    file <<"</PolyData>\n";
    file <<"</VTKFile>\n";
    file.close();
}

template<class T>
void gsWriteParaviewPoints(gsMatrix<T> const& X,
                           gsMatrix<T> const& Y,
                           gsMatrix<T> const& Z,
                           std::string const & fn)
{
    GISMO_ASSERT(X.cols() == Y.cols() && X.cols() == Z.cols(),
                 "X, Y and Z must have the same size of columns!");
    GISMO_ASSERT(X.rows() == 1 && Y.rows() == 1 && Z.cols(),
                 "X, Y and Z must be row matrices!");
    index_t np = X.cols();

    std::string mfn(fn);
    mfn.append(".vtp");
    std::ofstream file(mfn.c_str());

    if (!file.is_open())
    {
        gsWarn << "Problem opening " << fn << " Aborting..." << std::endl;
        return;
    }

    file << std::fixed; // no exponents
    file << std::setprecision (PLOT_PRECISION);

    file <<"<?xml version=\"1.0\"?>\n";
    file <<"<VTKFile type=\"PolyData\" version=\"0.1\" byte_order=\"LittleEndian\">\n";
    file <<"<PolyData>\n";
    file <<"<Piece NumberOfPoints=\""<<np<<"\" NumberOfVerts=\"1\" NumberOfLines=\"0\" NumberOfStrips=\"0\" NumberOfPolys=\"0\">\n";
    file <<"<PointData>\n";
    file <<"</PointData>\n";
    file <<"<CellData>\n";
    file <<"</CellData>\n";
    file <<"<Points>\n";
    file <<"<DataArray type=\"Float32\" Name=\"Points\" NumberOfComponents=\"3\" format=\"ascii\" RangeMin=\""<<X.minCoeff()<<"\" RangeMax=\""<<X.maxCoeff()<<"\">\n";

    for (index_t i = 0; i < np; ++i)
    {
        file << X(0, i) << " " << Y(0, i) << " " << Z(0, i) << "\n";
    }

    file <<"\n</DataArray>\n";
    file <<"</Points>\n";
    file <<"<Verts>\n";
    file <<"<DataArray type=\"Int64\" Name=\"connectivity\" format=\"ascii\" RangeMin=\""<<0<<"\" RangeMax=\""<<np-1<<"\">\n";

    for (index_t i=0; i< np; ++i )
    {
        file << i << " ";
    }

    file <<"\n</DataArray>\n";
    file <<"<DataArray type=\"Int64\" Name=\"offsets\" format=\"ascii\" RangeMin=\""<<np<<"\" RangeMax=\""<<np<<"\">\n"<<np<<"\n";
    file <<"</DataArray>\n";
    file <<"</Verts>\n";
    file <<"<Lines>\n";
    file <<"<DataArray type=\"Int64\" Name=\"connectivity\" format=\"ascii\" RangeMin=\"0\" RangeMax=\""<<np-1<<"\">\n";
    file <<"</DataArray>\n";
    file <<"<DataArray type=\"Int64\" Name=\"offsets\" format=\"ascii\" RangeMin=\""<<np<<"\" RangeMax=\""<<np<<"\">\n";
    file <<"</DataArray>\n";
    file <<"</Lines>\n";
    file <<"<Strips>\n";
    file <<"<DataArray type=\"Int64\" Name=\"connectivity\" format=\"ascii\" RangeMin=\"0\" RangeMax=\""<<np-1<<"\">\n";
    file <<"</DataArray>\n";
    file <<"<DataArray type=\"Int64\" Name=\"offsets\" format=\"ascii\" RangeMin=\""<<np<<"\" RangeMax=\""<<np<<"\">\n";
    file <<"</DataArray>\n";
    file <<"</Strips>\n";
    file <<"<Polys>\n";
    file <<"<DataArray type=\"Int64\" Name=\"connectivity\" format=\"ascii\" RangeMin=\"0\" RangeMax=\""<<np-1<<"\">\n";
    file <<"</DataArray>\n";
    file <<"<DataArray type=\"Int64\" Name=\"offsets\" format=\"ascii\" RangeMin=\""<<np<<"\" RangeMax=\""<<np<<"\">\n";
    file <<"</DataArray>\n";
    file <<"</Polys>\n";
    file <<"</Piece>\n";
    file <<"</PolyData>\n";
    file <<"</VTKFile>\n";

    file.close();
}

template<class T>
void gsWriteParaviewPoints(gsMatrix<T> const& points, std::string const & fn)
{
    index_t rows = points.rows();
    index_t cols = points.cols();

    GISMO_ASSERT(rows == 2 || rows == 3,
                 "This function is implemented just for 2D and 3D!");

    gsMatrix<T> X(1, cols);
    gsMatrix<T> Y(1, cols);
    gsMatrix<T> Z(1, cols);

    for (index_t col = 0; col < cols; col++)
    {
        X(0, col) = points(0, col);
        Y(0, col) = points(1, col);
        if (rows == 3)
        {
            Z(0, col) = points(2, col);
        }
    }

    if (rows == 2)
    {
        gsWriteParaviewPoints(X, Y, fn);
    }
    else if (rows == 3)
    {
        gsWriteParaviewPoints(X, Y, Z, fn);
    }
}

/// Depicting edge graph of each volume of one gsSolid with a segmenting loop
/// INPUTS:
/// \param eloop: a vector of ID numbers of vertices, often for representing a segmenting loop
template <class T>
void gsWriteParaview(gsSolid<T> const& sl, std::string const & fn, unsigned numPoints_for_eachCurve, int vol_Num,
                     T edgeThick, gsVector3d<T> const & translate, int color_convex,
                     int color_nonconvex, int color_eloop, std::vector<unsigned> const & eloop)
{
    // options
    int color=color_convex;

    gsSolidHalfFace<T>* face;
    int numOfCurves;
    int numOfPoints = numPoints_for_eachCurve;

    T faceThick = edgeThick;
//    T camera1 = 1;
//    T camera2 = 1;
//    T camera3 = 1;

    std::string mfn(fn);
    mfn.append(".vtp");
    std::ofstream file(mfn.c_str());
    if ( ! file.is_open() )
        std::cout<<"Problem opening "<<fn<<std::endl;
    file << std::fixed; // no exponents
    file << std::setprecision (PLOT_PRECISION);
    file <<"<?xml version=\"1.0\"?>\n";
    file <<"<VTKFile type=\"PolyData\" version=\"0.1\" byte_order=\"LittleEndian\">\n";
    file <<"<PolyData>\n";


    // collect HEs representing the edge loop
    numOfCurves = eloop.size();
    gsSolidHalfEdge<T>* he;
    std::vector< typename gsSolid<T>::gsSolidHalfEdgeHandle > heSet;
    typename gsSolid<T>::gsSolidHeVertexHandle source,target;
    if (eloop.size()>0){
    for (int iedge=0; iedge!= numOfCurves; iedge++)
    {
        source = sl.vertex[eloop[iedge]];
        target = sl.vertex[eloop[(iedge+1)%numOfCurves]];
        he = source->getHalfEdge(target);
        heSet.push_back(he);
        face = he->face;
    }}


    //std::cout<<"\n ------------------------------------- number of hafl faces: "<< sl.nHalfFaces();
    for (int iface=0;iface!= sl.nHalfFaces();iface++)
    {
        face = sl.getHalfFaceFromID(iface);
        //std::cout<<"\n ------------------------------------- vol of face:"<< face->vol->getId()<< " :for face: "<< iface <<"\n";
        //std::cout << std::flush;
        if (face->vol->getId()==vol_Num)
        {
            numOfCurves=face->nCurvesOfOneLoop(0);
            //std::cout<<"\n -----------INSIDE-------------------- vol of face:"<< face->vol->getId()<< " :for face: "<< iface <<"\n";

            for (int iedge=0; iedge!= numOfCurves; iedge++)
            {
                he = face->getHalfEdgeFromBoundaryOrder(iedge);
                // search if he is in heSet
                bool isMember(false);
                for (std::size_t iheSet=0;iheSet<heSet.size();iheSet++)
                {
                    if ( he->isEquiv(heSet.at(iheSet))==true || he->mate->isEquiv(heSet.at(iheSet))==true)
                    {isMember=true;
                        break;}
                }
                gsMatrix<T> curvePoints = face->surf->sampleBoundaryCurve(iedge, numPoints_for_eachCurve);
                if (iedge==0) assert( numOfPoints == curvePoints.cols());
                color=color_convex;
                if (isMember==true) color=color_eloop;
                if (face->getHalfEdgeFromBoundaryOrder(iedge)->is_convex==false){color = color_nonconvex;}                
                /// Number of vertices and number of faces
                file <<"<Piece NumberOfPoints=\""<< 2*numOfPoints <<"\" NumberOfVerts=\"0\" NumberOfLines=\""<< 0
                    <<"\" NumberOfStrips=\"0\" NumberOfPolys=\""<< numOfPoints-1 << "\">\n";

                /// Coordinates of vertices
                file <<"<Points>\n";
                file <<"<DataArray type=\"Float32\" NumberOfComponents=\"3\" format=\"ascii\">\n";
                // translate the volume towards the *translate* vector
                for (index_t iCol = 0;iCol!=curvePoints.cols();iCol++)
                {
                    file << curvePoints(0,iCol) + translate(0) << " " << curvePoints(1,iCol) + translate(1) << " " << curvePoints(2,iCol) + translate(2) << " \n";
                    // translate the vertex about along the vector (faceThick,0,0)
                    file << curvePoints(0,iCol) + faceThick + translate(0) << " " << curvePoints(1,iCol) + faceThick + translate(1)
                         << " " << curvePoints(2,iCol) +faceThick + translate(2) << " \n";
                };
                file << "\n";
                file <<"</DataArray>\n";
                file <<"</Points>\n";

                /// Scalar field attached to each degenerate face on the "edge"
                file << "<CellData Scalars=\"cell_scalars\">\n";
                file << "<DataArray type=\"Int32\" Name=\"cell_scalars\" format=\"ascii\">\n";
                /// limit: for now, assign all scalars to 0
                for (index_t iCol = 0;iCol!=curvePoints.cols()-1;iCol++)
                {
                    file << color << " ";
                }
                file << "\n";
                file << "</DataArray>\n";
                file << "</CellData>\n";

                /// Which vertices belong to which faces
                file << "<Polys>\n";
                file << "<DataArray type=\"Int32\" Name=\"connectivity\" format=\"ascii\">\n";
                for (index_t iCol = 0;iCol<=curvePoints.cols()-2;iCol++)
                {
                    //file << iCol << " " << iCol+1 << " "<< iCol+1 << " " << iCol << " ";
                    file << 2*iCol << " " << 2*iCol+1 << " "<< 2*iCol+3 << " " << 2*iCol+2 << " ";
                }
                //file << curvePoints.cols()-1 << " " << 0 << " "<< 0 << " "<< curvePoints.cols()-1 << " ";
                file << "\n";
                file << "</DataArray>\n";
                file << "<DataArray type=\"Int32\" Name=\"offsets\" format=\"ascii\">\n";
                unsigned offsets(0);
                for (index_t iCol = 0;iCol!=curvePoints.cols()-1;iCol++)
                {
                    offsets +=4;
                    file << offsets << " ";
                }
                file << "\n";
                file << "</DataArray>\n";
                file << "</Polys>\n";

                file << "</Piece>\n";

                /// Space between edges
                file << "\n";
                file << "\n";
            }
        }
    }

    ///////////////////////////////
    file <<"</PolyData>\n";
    file <<"</VTKFile>\n";
    file.close();

    makeCollection(fn, ".vtp"); // make also a pvd file
}

template <class T>
void gsWriteParaviewSolid(gsSolid<T> const& sl, 
                          std::string const & fn, 
                          unsigned numSamples)
{
    const size_t n = sl.numHalfFaces;
    gsParaviewCollection collection(fn);

    // for( typename gsSolid<T>::const_face_iterator it = sl.begin();
    //      it != sl.end(); ++it)

    for ( size_t i=0; i<n ; i++)
    {
        std::string fnBase = fn + internal::toString<index_t>(i);
        writeSingleTrimSurface(*sl.face[i]->surf, fnBase, numSamples);
        collection.addPart(fnBase, ".vtp");
    }

    // Write out the collection file
    collection.save();
}


/// Visualizing a mesh
template <class T>
void gsWriteParaview(gsMesh<T> const& sl, std::string const & fn, bool pvd)
{
    std::string mfn(fn);
    mfn.append(".vtp");
    std::ofstream file(mfn.c_str());
    if ( ! file.is_open() )
        std::cout<<"Problem opening "<<fn<<std::endl;
    file << std::fixed; // no exponents
    file << std::setprecision (PLOT_PRECISION);
    
    file <<"<?xml version=\"1.0\"?>\n";
    file <<"<VTKFile type=\"PolyData\" version=\"0.1\" byte_order=\"LittleEndian\">\n";
    file <<"<PolyData>\n";
    
    /// Number of vertices and number of faces
    file <<"<Piece NumberOfPoints=\""<< sl.numVertices <<"\" NumberOfVerts=\"0\" NumberOfLines=\""
         << sl.numEdges<<"\" NumberOfStrips=\"0\" NumberOfPolys=\""<< sl.numFaces << "\">\n";
    
    /// Coordinates of vertices
    file <<"<Points>\n";
    file <<"<DataArray type=\"Float32\" NumberOfComponents=\"3\" format=\"ascii\">\n";
    for (typename std::vector< gsVertex<T>* >::const_iterator it=sl.vertex.begin(); it!=sl.vertex.end(); ++it)
    {
        file << ((*it)->coords)[0] << " " << ((*it)->coords)[1] << " " << ((*it)->coords)[2] << " \n";
    }
    
    file << "\n";
    file <<"</DataArray>\n";
    file <<"</Points>\n";

    // Scalar field attached to each face
    // file << "<PointData Scalars=\"point_scalars\">\n";
    // file << "<DataArray type=\"Int32\" Name=\"point_scalars\" format=\"ascii\">\n";
    // for (typename std::vector< gsVertex<T>* >::const_iterator it=sl.vertex.begin(); 
    //      it!=sl.vertex.end(); ++it)
    // {
    //     file << 0 << " ";
    // }
    // file << "\n";
    // file << "</DataArray>\n";
    // file << "</PointData>\n";
    
    // Write out edges
    file << "<Lines>\n";
    file << "<DataArray type=\"Int32\" Name=\"connectivity\" format=\"ascii\">\n";
    for (typename std::vector< gsEdge<T> >::const_iterator it=sl.edge.begin();
         it!=sl.edge.end(); ++it)
    {
            file << it->source->getId() << " " << it->target->getId() << "\n";
    }
    file << "</DataArray>\n";
    file << "<DataArray type=\"Int32\" Name=\"offsets\" format=\"ascii\">\n";
    int count=0;
    for (typename std::vector< gsEdge<T> >::const_iterator it=sl.edge.begin();
         it!=sl.edge.end(); ++it)
    {
        count+=2;
        file << count << " ";
    }
    file << "\n";
    file << "</DataArray>\n";
    file << "</Lines>\n";
    
    // Scalar field attached to each face (* if edges exists, this has a problem)
    // file << "<CellData Scalars=\"cell_scalars\">\n";
    // file << "<DataArray type=\"Int32\" Name=\"cell_scalars\" format=\"ascii\">\n";
    // for (typename std::vector< gsFace<T>* >::const_iterator it=sl.face.begin();
    //      it!=sl.face.end(); ++it)
    // {
    //     file << 1 << " ";
    // }
    // file << "\n";
    // file << "</DataArray>\n";
    // file << "</CellData>\n";
    
    /// Which vertices belong to which faces
    file << "<Polys>\n";
    file << "<DataArray type=\"Int32\" Name=\"connectivity\" format=\"ascii\">\n";
    for (typename std::vector< gsFace<T>* >::const_iterator it=sl.face.begin();
         it!=sl.face.end(); ++it)
    {
        for (typename std::vector< gsVertex<T>* >::const_iterator vit= (*it)->vertices.begin();
             vit!=(*it)->vertices.end(); ++vit)
        {
            file << (*vit)->getId() << " ";
        }
        file << "\n";
    }
    file << "</DataArray>\n";
    file << "<DataArray type=\"Int32\" Name=\"offsets\" format=\"ascii\">\n";
    count=0;
    for (typename std::vector< gsFace<T>* >::const_iterator it=sl.face.begin();
         it!=sl.face.end(); ++it)
    {
        count += (*it)->vertices.size();
        file << count << " ";
    }
    file << "\n";
    file << "</DataArray>\n";
    file << "</Polys>\n";

    file << "</Piece>\n";
    file <<"</PolyData>\n";
    file <<"</VTKFile>\n";
    file.close();
    
    if( pvd ) // make also a pvd file
        makeCollection(fn, ".vtp");
}

template <typename T>
void gsWriteParaview(const std::vector<gsMesh<T> >& meshes,
                     const std::string& fn)
{
    for (unsigned index = 0; index < meshes.size(); index++)
    {
        std::string file = fn + "Level" + internal::toString<unsigned>(index);
        gsWriteParaview(meshes[index], file, false);
    }
}


/// Visualizing an edge graph of a 3D solid structured as gsHeMesh
template <class T>
void gsWriteParaview(gsHeMesh<T> const& sl, std::string const & fn)
{
    std::string mfn(fn);
    mfn.append(".vtp");
    std::ofstream file(mfn.c_str());
    if ( ! file.is_open() )
        std::cout<<"Problem opening "<<fn<<std::endl;
    file << std::fixed; // no exponents
    file << std::setprecision (PLOT_PRECISION);

    file <<"<?xml version=\"1.0\"?>\n";
    file <<"<VTKFile type=\"PolyData\" version=\"0.1\" byte_order=\"LittleEndian\">\n";
    file <<"<PolyData>\n";

    /// Number of vertices and number of faces
    file <<"<Piece NumberOfPoints=\""<< sl.numVertices <<"\" NumberOfVerts=\"0\" NumberOfLines=\"0\" NumberOfStrips=\"0\" NumberOfPolys=\""<< sl.numHalfFaces << "\">\n";

    /// Coordinates of vertices
    file <<"<Points>\n";
      file <<"<DataArray type=\"Float32\" NumberOfComponents=\"3\" format=\"ascii\">\n";
    for (typename std::vector< gsHeVertex<T>* >::const_iterator it=sl.vertex.begin(); it!=sl.vertex.end(); ++it)
    {
      file << ((*it)->coords)[0] << " " << ((*it)->coords)[1] << " " << ((*it)->coords)[2] << " \n";
    }

    file << "\n";
    file <<"</DataArray>\n";
    file <<"</Points>\n";

    /// Scalar field attached to each vertex
    file << "<PointData Scalars=\"my_scalars\">\n";
      file << "<DataArray type=\"Float32\" Name=\"my_scalars\" format=\"ascii\">\n";
    /// limit: for now, assign all scalars to 0
    for (int i=1; i<=sl.numVertices; ++i)
    {
      file << 0 << " ";
    }
    file << "\n";
      file << "</DataArray>\n";
    file << "</PointData>\n";

    /// Scalar field attached to each face
    file << "<CellData Scalars=\"cell_scalars\">\n";
    file << "<DataArray type=\"Int32\" Name=\"cell_scalars\" format=\"ascii\">\n";
    /// limit: for now, assign all scalars to 0
    for (int i=1; i<=sl.numHalfFaces; ++i)
    {
      file << 0 << " ";
    }
    file << "\n";
    file << "</DataArray>\n";
    file << "</CellData>\n";

    /// Which vertices belong to which faces
    file << "<Polys>\n";
      file << "<DataArray type=\"Int32\" Name=\"connectivity\" format=\"ascii\">\n";
    for (typename std::vector< gsHalfFace<T>* >::const_iterator it=sl.face.begin(); it!=sl.face.end(); ++it)
    {
      /// limit: for now only valid for faces with no holes
      gsHalfEdge<T> * edge1 = (*it)->boundary;
      gsHalfEdge<T> * current_edge=edge1->next;
      file << edge1->source->getId() << " ";
      while ( current_edge->getId() != edge1->getId() )
      {
        file << current_edge->source->getId() << " ";
        current_edge=current_edge->next;
      }
    }
    file << "\n";
      file << "</DataArray>\n";
      file << "<DataArray type=\"Int32\" Name=\"offsets\" format=\"ascii\">\n";
    int count=0;
    for (typename std::vector< gsHalfFace<T>* >::const_iterator it=sl.face.begin(); it!=sl.face.end(); ++it)
    {
      /// limit: for now only valid for faces with no holes
      gsHalfEdge<T> * edge1 = (*it)->boundary;
      count++;
      gsHalfEdge<T> * current_edge=edge1->next;
      while ( current_edge->getId() != edge1->getId() )
      {
        count++;
        current_edge=current_edge->next;
      }
      file << count << " ";
    }
    file << "\n";
      file << "</DataArray>\n";
    file << "</Polys>\n";

    file << "</Piece>\n";
    file <<"</PolyData>\n";
    file <<"</VTKFile>\n";
    file.close();

    makeCollection(fn, ".vtp"); // make also a pvd file
}

template<class T>
void gsWriteParaview(gsPlanarDomain<T> const & pdomain, std::string const & fn, unsigned npts)
{
    std::vector<gsGeometry<T> *> all_curves;
    for(index_t i =0; i<pdomain.numLoops();i++)
        for(index_t j =0; j< pdomain.loop(i).numCurves() ; j++)
            all_curves.push_back( const_cast<gsCurve<T> *>(&pdomain.loop(i).curve(j)) );

    gsWriteParaview( all_curves, fn, npts);       
}

template<class T>
void gsWriteParaview(const gsTrimSurface<T> & surf, std::string const & fn, 
                     unsigned npts, bool trimCurves)
{
    gsParaviewCollection collection(fn);

    writeSingleTrimSurface(surf, fn, npts);
    collection.addPart(fn, ".vtp");

    if ( trimCurves )
    {
        gsWarn<<"trimCurves: To do.\n";
    }

    // Write out the collection file
    collection.save();
}

template<typename T>
void gsWriteParaview(const gsVolumeBlock<T>& volBlock,
                     std::string const & fn,
                     unsigned npts)
{
    using internal::toString;

    gsParaviewCollection collection(fn);

    // for each face
    for (unsigned idFace = 0; idFace != volBlock.face.size(); idFace++)
    {
        typename gsVolumeBlock<T>::HalfFace* face = volBlock.face[idFace];
        gsPlanarDomain<T>& domain = face->surf->domain();

        // for each curve loop (boundary + holes)
        unsigned numLoops = static_cast<unsigned>(domain.numLoops());
        for (unsigned idLoop = 0; idLoop < numLoops; idLoop++)
        {
            gsCurveLoop<T>& curveLoop = domain.loop(idLoop);

            unsigned clSize = static_cast<unsigned>(curveLoop.size());

            // for each curve in curve loop
            for (unsigned idCurve = 0; idCurve < clSize; idCurve++)
            {
                // file name is fn_curve_Fface_Lloop_Ccurve
                std::string fileName = fn + "_curve_F";
                fileName += toString<unsigned>(idFace) + "_L" +
                            toString<unsigned>(idLoop) + "_C" +
                            toString<unsigned>(idCurve);

                gsWriteParaviewTrimmedCurve(*(face->surf), idLoop, idCurve,
                                            fileName, npts);

                collection.addPart(fileName,".vts");

            } // for each curve
        } // for each curve loop
    } // for each face

    collection.save();
}

template<typename T>
void gsWriteParaviewTrimmedCurve(const gsTrimSurface<T>& surf,
                                 const unsigned idLoop,
                                 const unsigned idCurve,
                                 const std::string fn,
                                 unsigned npts)
{
    // computing parameters and points

    int idL = static_cast<int>(idLoop);
    int idC = static_cast<int>(idCurve);

    gsCurve<T>& curve = surf.getCurve(idL, idC);

    gsMatrix<T> ab = curve.parameterRange() ;
    gsVector<T> a = ab.col(0);
    gsVector<T> b = ab.col(1);

    gsVector<unsigned> np = uniformSampleCount(a, b, npts);
    gsMatrix<T> param = gsPointGrid(a, b, np);

    gsMatrix<T> points;
    surf.evalCurve_into(idLoop, idCurve, param, points);

    np.conservativeResize(3);
    np.bottomRows(3 - 1).setOnes();


    // writing to the file

    std::string myFile(fn);
    myFile.append(".vts");

    std::ofstream file(myFile.c_str());
    if (!file.is_open())
    {
        gsWarn << "Problem opening " << fn << " Aborting..." << std::endl;
        return;
    }

    file << std::fixed; // no exponents
    file << std::setprecision (PLOT_PRECISION);

    file << "<?xml version=\"1.0\"?>\n";
    file << "<VTKFile type=\"StructuredGrid\" version=\"0.1\">\n";
    file << "<StructuredGrid WholeExtent=\"0 "<< np(0) - 1 <<
            " 0 " << np(1) - 1 << " 0 " << np(2) - 1 << "\">\n";

    file << "<Piece Extent=\"0 " << np(0) - 1 << " 0 " << np(1) - 1 << " 0 "
         << np(2) - 1 << "\">\n";

    file << "<Points>\n";
    file << "<DataArray type=\"Float32\" NumberOfComponents=\"" << points.rows()
         << "\">\n";

    for (index_t j = 0; j < points.cols(); ++j)
    {
        for (index_t i = 0; i < points.rows(); ++i)
        {
            file << points.at(i, j) << " ";
        }
        file << "\n";
    }

    file << "</DataArray>\n";
    file << "</Points>\n";
    file << "</Piece>\n";
    file << "</StructuredGrid>\n";
    file << "</VTKFile>\n";
    file.close();

}

} // namespace gismo


#undef PLOT_PRECISION
