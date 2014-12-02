/**
 * @file   gsFileData.h
 * @author A. Mantzaflaris <Angelos.Mantzaflaris@oeaw.ac.at>
 * @date   February 2013
 * 
 * @brief  Reads a file
 * 
 *  Part of GISMO library, developed at RICAM-Linz, Austria
 */

#pragma once

#include <iostream>
#include <string>

#include <gsCore/gsLinearAlgebra.h>

#include <gsIO/gsXmlUtils.h>

namespace gismo {


template<class T = real_t >
class gsFileData
{
public:
    typedef internal::gsXmlTree         FileData;
    typedef internal::gsXmlNode         gsXmlNode;
    typedef internal::gsXmlAttribute    gsXmlAttribute;
    typedef std::string                 String;

    typedef gsVector3d<T>               Point_3;

public:

    gsFileData() ;
    
    /** 
     * Initializes a a gsFileData object with the containts of a file
     * 
     * @param fn filename string
     */
    explicit gsFileData(String const & fn);
    
    /** 
     * Loads the contents of a file into a gsFileData object
     * 
     * @param fn filename string
     */
    void read(String const & fn) ;
    
    ~gsFileData();
    
    // Clear all data
    void clear();
    
    // Dump file containts to an xml file
    void dump(std::string const & fname = "dump") const;
    
private:
    /// File data as an xml tree
    FileData * data;
    
    // Used to hold parsed data of native gismo XML files
    std::vector<char> m_buffer;
    
    /// Max id
    int max_id;
    
protected:
    
////////////////////////////////////////////////////////
// File readers
////////////////////////////////////////////////////////

    /// Reads a file with xml extension
    bool readXmlFile( String const & fn );

    /// Reads Gismo's native XML file
    void readGismoFile(std::vector<char> & bf, FileData * xmlTree);

    /// Reads Axel file
    bool readAxelFile(String const & fn);
    bool readAxelSurface( gsXmlNode * node );
    bool readAxelCurve  ( gsXmlNode * node );

    /// Reads GeoPDEs txt file
    bool readGeompFile( String const & fn );

    /// Reads GoTools file
    bool readGoToolsFile(String const & fn);

    /// Reads Off mesh file
    bool readOffFile(String const & fn);

    /// Reads STL mesh file
    bool readStlFile(String const & fn);

    /// Reads Wavefront OBJ file
    bool readObjFile(String const & fn);

    /// Reads Iges file
    bool readIgesFile(String const & fn);

    /// Reads X3D file
    bool readX3dFile(String const & fn);

#ifdef GISMO_WITH_ONURBS
    /// Reads 3DM file
    bool read3dmFile(String const & fn);
#endif

#ifdef GISMO_WITH_PSOLID
    /// Reads parasolid files
    bool readParasolidFile(String const & fn);
#endif

    // Show the line number where something went wrong
    void ioError(int lineNumber,const std::string& str);

public:

////////////////////////////////////////////////////////
// Generic functions to fetch Gismo objects
////////////////////////////////////////////////////////

    // Generic functions to fetch Gismo object
    // template<class Object>
    // inline Object * get( gsXmlNode * node)
    // {
    //     return internal::gsXml<Object>::get(node);// Using gsXmlUtils
    // }
    
    /// Searches and fetches the Gismo object with a given id
    template<class Object>
    inline Object * getId( const int & id)  const
    {
        return internal::gsXml<Object>::getId( getXmlRoot(), id );
    }
    
    /// Prints the XML tag of a Gismo object
    template<class Object>
    inline String tag() const
    { return internal::gsXml<Object>::tag(); }
    
    /// Prints the XML tag type of a Gismo object
    template<class Object>
    inline String type() const
    { return internal::gsXml<Object>::type(); }
    
    /// Returns true if an Object exists in the filedata
    template<class Object> 
    inline bool has() const
    {
        return getFirstNode( internal::gsXml<Object>::tag(), 
                             internal::gsXml<Object>::type() ) != 0 ;
    }

    /// Returns true if an Object exists in the filedata, even nested
    /// inside other objects
    template<class Object> 
    inline bool hasAny() const
    {
        return getAnyFirstNode( internal::gsXml<Object>::tag(), 
                                internal::gsXml<Object>::type() ) != 0 ;
    }
    
    /// Counts the number of Objects in the filedata
    template<class Object> 
    inline int count() const
    {
        int i(0);
        for (gsXmlNode * child = getFirstNode( internal::gsXml<Object>::tag(), 
                                               internal::gsXml<Object>::type() ) ; 
             child; child = getNextSibling(child, internal::gsXml<Object>::tag(), 
                                           internal::gsXml<Object>::type() ))
            ++i;
        return i;
    }

    
    /// Inserts an object to the XML tree
    template<class Object>    
    void operator<<(const Object & obj)
    {
        this->add<Object>(obj);
    }
    
    /// Add the object to the Xml tree, same as <<
    template<class Object>
    void add (const Object & obj)
    {
        gsXmlNode* node = 
            internal::gsXml<Object>::put(obj, *data);
        if ( ! node )
        {
            gsInfo<<"gsFileData: Trouble inserting "<<internal::gsXml<Object>::tag()
                         <<" to the XML tree. is \"put\" implemented ??\n";
        }
        else
        {                
            appendId(node);
            appendToRoot(node);
        }            
    }
    
    /// Prints the XML data as a string
    std::ostream &print(std::ostream &os) const;

    /// Constructs the first Object found in the XML tree and assigns
    /// it to the pointer obj and then deletes it from the data tree.
    /// Returns true if there was something assigned, false if object
    /// did not exist.
    /// WARNING: Use getFirst<Object>() instead. This is buggy due to
    /// template resolution.
    template<class Object>    
    bool operator>>(Object * obj)
    {        
        gsWarn<< "getting "<< typeid(Object).name() <<"\n";
        gsXmlNode* node = getFirstNode(internal::gsXml<Object>::tag(), 
                                       internal::gsXml<Object>::type() );
        if ( !node )
	    {
            gsWarn<<"gsFileData: false!\n";
            return false;
	    }
        else
	    {
            obj = internal::gsXml<Object>::get(node);
            this->deleteXmlSubtree( node );
            return true;
	    }
    }
    
    /// Returns the first Object found in the XML data
    template<class Object> 
    inline Object * getFirst() const
    {
        gsXmlNode* node = getFirstNode(internal::gsXml<Object>::tag(), 
                                       internal::gsXml<Object>::type() );
        if ( !node )
        {
            gsWarn<<"gsFileData: getFirst: Didn't find any "<<
                internal::gsXml<Object>::type()<<" "<< 
                internal::gsXml<Object>::tag() <<". Error.\n";
            return NULL;
        }           
        return internal::gsXml<Object>::get(node);// Using gsXmlUtils
    }
    
    /// Returns a vector with all Objects found in the XML data
    template<class Object>
    inline std::vector<Object *> getAll()  const
    {
        std::vector<Object *> result;
        
        for (gsXmlNode * child = getFirstNode( internal::gsXml<Object>::tag(), 
                                               internal::gsXml<Object>::type() ) ; 
             child; child = getNextSibling(child, internal::gsXml<Object>::tag(), 
                                           internal::gsXml<Object>::type() ))
        {
            result.push_back( internal::gsXml<Object>::get(child) );
        }
        return result;
    }
    
    /// Returns the first Object found in the XML data
    template<class Object> 
    inline Object * getAnyFirst() const
    {
        gsXmlNode* node = getAnyFirstNode(internal::gsXml<Object>::tag(), 
                                          internal::gsXml<Object>::type() );
        if ( !node )
        {
            gsWarn <<"gsFileData: getAnyFirst: Didn't find any "<<
                internal::gsXml<Object>::type()<<" "<< 
                internal::gsXml<Object>::tag() <<". Error.\n";
            return NULL;
      }           
        return internal::gsXml<Object>::get(node);// Using gsXmlUtils
    }

    /// Lists the contents of the filedata
    String contents () const;         
    /// Counts the number of Objects in the filedata
    int size () const;

    /// Returns the extension of the filename \a fn
    static String getExtension(String const & fn)
    {
        if(fn.find_last_of(".") != std::string::npos)
        {
            String ext = fn.substr(fn.rfind(".")+1);
            std::transform(ext.begin(), ext.end(), ext.begin(), ::tolower); 
            return ext;
        }
        return "";
    }

    /// Returns the base name without extension of the filename \a fn
    static String getBasename(String const & fn)
    {
        if(fn.find_last_of(".") != std::string::npos)
        {
            std::size_t pos1 = fn.find_last_of("/\\");
            std::size_t pos2 = fn.rfind(".");
            String      name = fn.substr(pos1+1, pos2-pos1-1);
            return name;
        }
        return fn;
    }

    /// Returns the filename without the path of \a fn
    static String getFilename(String const & fn)
    {
        std::size_t pos1 = fn.find_last_of("/\\");
        if(pos1 != std::string::npos)
        {
            String      name = fn.substr(pos1+1);
            return name;
        }
        return fn;
    }
    
private:

    gsXmlNode * getXmlRoot() const;
    void        appendToRoot  (gsXmlNode* node);
    void        appendId      (gsXmlNode* node);
    static void deleteXmlSubtree (gsXmlNode* node);

    // getFirst ? (tag and or type)
    gsXmlNode * getFirstNode  ( const std::string & name = "",
                                const std::string & type = "" ) const;

    // getAny
    gsXmlNode * getAnyFirstNode( const std::string & name = "",
                                 const std::string & type = "" ) const;

    // getNext
    static gsXmlNode * getNextSibling( gsXmlNode* const & node, 
                                       const std::string & name = "", 
                                       const std::string & type = "" );
    
    // Helpers for X3D files
    void addX3dShape(gsXmlNode * shape);
    void addX3dTransform(gsXmlNode * shape);
    
}; // class gsFileData

// Print out operator    
template<class T>
std::ostream &operator<<(std::ostream &os, const gsFileData<T> & fd)
{return fd.print(os); }

} // namespace gismo


#ifndef GISMO_HEADERS_ONLY
#include GISMO_HPP_HEADER(gsFileData.hpp)
#endif