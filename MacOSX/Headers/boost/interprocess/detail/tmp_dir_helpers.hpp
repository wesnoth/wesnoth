//////////////////////////////////////////////////////////////////////////////
//
// (C) Copyright Ion Gaztanaga 2007-2008. Distributed under the Boost
// Software License, Version 1.0. (See accompanying file
// LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// See http://www.boost.org/libs/interprocess for documentation.
//
//////////////////////////////////////////////////////////////////////////////

#ifndef BOOST_INTERPROCESS_DETAIL_TMP_DIR_HELPERS_HPP
#define BOOST_INTERPROCESS_DETAIL_TMP_DIR_HELPERS_HPP

#include <boost/interprocess/detail/config_begin.hpp>
#include <boost/interprocess/detail/workaround.hpp>
#include <boost/interprocess/detail/os_file_functions.hpp>
#include <boost/interprocess/errors.hpp>
#include <boost/interprocess/exceptions.hpp>
#include <string>

namespace boost {
namespace interprocess {
namespace detail {

inline void tmp_filename(const char *filename, std::string &tmp_name)
{
   const char *tmp_dir = get_temporary_path();
   if(!tmp_dir){
      error_info err = system_error_code();
      throw interprocess_exception(err);
   }
   tmp_name = tmp_dir;

   //Remove final null.
   tmp_name += "/boost_interprocess/";
   tmp_name += filename;
}

inline void create_tmp_dir_and_get_filename(const char *filename, std::string &tmp_name)
{
   const char *tmp_path = get_temporary_path(); 
   if(!tmp_path){
      error_info err = system_error_code();
      throw interprocess_exception(err);
   }

   tmp_name = tmp_path;
   tmp_name += "/boost_interprocess";

   //Create the temporary directory.
   //If fails, check that it's because already exists
   if(!create_directory(tmp_name.c_str())){
      error_info info(system_error_code());
      if(info.get_error_code() != already_exists_error){
         throw interprocess_exception(info);
      }
   }

   //Add filename
   tmp_name += '/';
   tmp_name += filename;
}

inline void add_leading_slash(const char *name, std::string &new_name)
{
   if(name[0] != '/'){
      new_name = '/';
   }
   new_name += name;
}

}  //namespace boost {
}  //namespace interprocess {
}  //namespace detail {

#include <boost/interprocess/detail/config_end.hpp>

#endif   //ifndef BOOST_INTERPROCESS_DETAIL_TMP_DIR_HELPERS_HPP
