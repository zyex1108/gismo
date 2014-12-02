
# pragma once

// Assumes that Eigen library has been already included

namespace gismo
{

/// Creates a mapped object to an eigen matrix without copying data.
template<class T, int _Rows, int _Cols>
class gsAsMatrix : public Eigen::Map< Eigen::Matrix<T,_Rows,_Cols> >
{
public:
    typedef Eigen::Map< Eigen::Matrix<T,_Rows,_Cols> > Base;

public:
    gsAsMatrix( std::vector<T> & v, index_t n, index_t m)
    : Base( &v[0], n, m)
    { 
        GISMO_ASSERT( v.size() != 0, "Tried to map an empty vector." ); 
        GISMO_ASSERT( m*n <= index_t(v.size()), "Not enough coefficients in vector to map." ); 
    }

    gsAsMatrix( std::vector<T> & v)
    : Base( &v[0], 1, v.size() ) 
    {  
        GISMO_ASSERT( v.size() != 0, "Tried to map an empty vector." ); 
    }

    gsAsMatrix( T * pt, unsigned n, unsigned m)
    : Base( pt, n, m) {  }

private:
    gsAsMatrix() { }
};

/// Creates a mapped object to an eigen matrix without copying data.
template<class T, int _Rows, int _Cols>
class gsAsConstMatrix : public Eigen::Map< const Eigen::Matrix<T,_Rows,_Cols> >
{
public:
    typedef Eigen::Map<const Eigen::Matrix<T,_Rows,_Cols> > Base;

public:

    gsAsConstMatrix( const std::vector<T> & v, index_t n, index_t m)
    : Base( &v[0], n, m)
    { 
        GISMO_ASSERT( v.size() != 0, "Tried to map an empty vector." ); 
        GISMO_ASSERT( m*n <= index_t(v.size()), "Not enough coefficients in vector to map." ); 
    }

    gsAsConstMatrix( const std::vector<T> & v)
    : Base( &v[0], 1, v.size() ) 
    {  
        GISMO_ASSERT( v.size() != 0, "Tried to map an empty vector." ); 
    }

    gsAsConstMatrix( const T * pt, unsigned n, unsigned m)
    : Base( pt, n, m) {  }

private:
    gsAsConstMatrix() { }
};


/// Creates a mapped object to an eigen matrix without copying data.
template<class T, int _Rows=Dynamic>
class gsAsVector : public Eigen::Map< Eigen::Matrix<T,_Rows,1> >
{
public:
    typedef Eigen::Map< Eigen::Matrix<T,_Rows,1> > Base;

public:
    gsAsVector( std::vector<T> & v)
    : Base( &v[0], v.size(), 1 ) 
    {  
        GISMO_ASSERT( v.size() != 0, "Tried to map an empty vector." ); 
    }

    gsAsVector( T * pt, unsigned n)
    : Base( pt, n) {  }

private:
    gsAsVector() { }
};

/// Creates a mapped object to an eigen matrix without copying data.
template<class T, int _Rows=Dynamic>
class gsAsConstVector : public Eigen::Map< const Eigen::Matrix<T,_Rows,1> >
{
public:
    typedef Eigen::Map<const Eigen::Matrix<T,_Rows,1> > Base;

public:

    gsAsConstVector( const std::vector<T> & v)
    : Base( &v[0], v.size(), 1) 
    {  
        GISMO_ASSERT( v.size() != 0, "Tried to map an empty vector." ); 
    }

    gsAsConstVector( const T * pt, unsigned n)
    : Base( pt, n, 1) {  }

private:
    gsAsConstVector() { }
};


/// Utility to make a matrix out of an iterator tp values
template<class T, class iterator>
typename gsMatrix<T>::uPtr makeMatrix(iterator it, index_t n, index_t m)
{
    typename gsMatrix<T>::uPtr result ( new gsMatrix<T>(n,m) );
    for ( index_t i = 0; i!=n; ++i)
        for ( index_t j = 0; j!=m; ++j)
            (*result)(i,j)= *(it++);
    return result;
}


}; // namespace gismo