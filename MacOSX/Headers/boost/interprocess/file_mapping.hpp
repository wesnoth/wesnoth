//////////////////////////////////////////////////////////////////////////////
//
// (C) Copyright Ion Gaztanaga 2005-2008. Distributed under the Boost
// Software License, Version 1.0. (See accompanying file
// LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// See http://www.boost.org/libs/interprocess for documentation.
//
//////////////////////////////////////////////////////////////////////////////

#ifndef BOOST_INTERPROCESS_MAPPED_FILE_HPP
#define BOOST_INTERPROCESS_MAPPED_FILE_HPP

#include <boost/interprocess/detail/config_begin.hpp>
#include <boost/interprocess/detail/workaround.hpp>

#include <boost/interprocess/interprocess_fwd.hpp>
#include <boost/interprocess/exceptions.hpp>
#include <boost/interprocess/detail/utilities.hpp>
#include <boost/interprocess/creation_tags.hpp>
#include <boost/interprocess/detail/os_file_functions.hpp>
#include <boost/interprocess/detail/move.hpp>
#include <string>    //std::string
#include <cstdio>    //std::remove
#include <string>

//!\file
//!Describes file_mapping and mapped region classes

namespace boost {

namespace interprocess {

//!A class that wraps a file-mapping that can be used to
//!create mapped regions from the mapped files
class file_mapping
{
   /// @cond
   //Non-copyable and non-assignable
   file_mapping(const file_mapping &);
   file_mapping &operator=(const file_mapping &);
   /// @endcond

   public:
   //!Constructs an empty file mapping.
   //!Does not throw
   file_mapping();

   //!Opens a file mapping of file "filename", starting in offset 
   //!"file_offset", and the mapping's size will be "size". The mapping 
   //!can be opened for read-only "read_only" or read-write "read_write"
   //!modes. Throws interprocess_exception on error.
   file_mapping(const char *filename, mode_t mode);

   //!Moves the ownership of "moved"'s shared memory object to *this. 
   //!After the call, "moved" does not represent any shared memory object. 
   //!Does not throw
   #ifndef BOOST_INTERPROCESS_RVALUE_REFERENCE
   file_mapping(detail::moved_object<file_mapping> moved)
      :  m_handle(file_handle_t(detail::invalid_file()))
   {  this->swap(moved.get());   }
   #else
   file_mapping(file_mapping &&moved)
      :  m_handle(file_handle_t(detail::invalid_file()))
   {  this->swap(moved);   }
   #endif

   //!Moves the ownership of "moved"'s shared memory to *this.
   //!After the call, "moved" does not represent any shared memory. 
   //!Does not throw
   #ifndef BOOST_INTERPROCESS_RVALUE_REFERENCE
   file_mapping &operator=
      (detail::moved_object<file_mapping> moved)
   {  
      file_mapping tmp(moved);
      this->swap(tmp);
      return *this;  
   }
   #else
   file_mapping &operator=(file_mapping &&moved)
   {  
      file_mapping tmp(detail::move_impl(moved));
      this->swap(tmp);
      return *this;  
   }
   #endif

   //!Swaps to file_mappings.
   //!Does not throw.
   void swap(file_mapping &other);

   //!Returns access mode
   //!used in the constructor
   mode_t get_mode() const;

   //!Obtains the mapping handle
   //!to be used with mapped_region
   mapping_handle_t get_mapping_handle() const;

   //!Destroys the file mapping. All mapped regions created from this are still
   //!valid. Does not throw
   ~file_mapping();

   //!Returns the name of the file
   //!used in the constructor.
   const char *get_name() const;

   /// @cond
   private:
   //!Closes a previously opened file mapping. Never throws.
   void priv_close();
   file_handle_t  m_handle;
   mode_t    m_mode;
   std::string       m_filename;
   /// @endcond
};

inline file_mapping::file_mapping() 
   :  m_handle(file_handle_t(detail::invalid_file()))
{}

inline file_mapping::~file_mapping() 
{  this->priv_close(); }

inline const char *file_mapping::get_name() const
{  return m_filename.c_str(); }

inline void file_mapping::swap(file_mapping &other)
{  
   std::swap(m_handle, other.m_handle);
   std::swap(m_mode, other.m_mode);
   m_filename.swap(other.m_filename);   
}

inline mapping_handle_t file_mapping::get_mapping_handle() const
{  return detail::mapping_handle_from_file_handle(m_handle);  }

inline mode_t file_mapping::get_mode() const
{  return m_mode; }

inline file_mapping::file_mapping
   (const char *filename, mode_t mode)
   :  m_filename(filename)
{
   //Check accesses
   if (mode != read_write && mode != read_only){
      error_info err = other_error;
      throw interprocess_exception(err);
   }

   //Open file
   m_handle = detail::open_existing_file(filename, mode);

   //Check for error
   if(m_handle == detail::invalid_file()){
      error_info err = system_error_code();
      this->priv_close();
      throw interprocess_exception(err);
   }
   m_mode = mode;
}

///@cond

inline void file_mapping::priv_close()
{
   if(m_handle != detail::invalid_file()){
      detail::close_file(m_handle);
      m_handle = detail::invalid_file();
   }
}


//!Trait class to detect if a type is
//!movable
template<>
struct is_movable<file_mapping>
{
   enum {  value = true };
};

///@endcond

//!A class that stores the name of a file
//!and call std::remove(name) in its destructor
//!Useful to remove temporary files in the presence
//!of exceptions
class remove_file_on_destroy
{
   const char * m_name;
   public:
   remove_file_on_destroy(const char *name)
      :  m_name(name)
   {}

   ~remove_file_on_destroy()
   {  std::remove(m_name);  }
};

}  //namespace interprocess {
}  //namespace boost {

#include <boost/interprocess/detail/config_end.hpp>

#endif   //BOOST_INTERPROCESS_MAPPED_FILE_HPP
