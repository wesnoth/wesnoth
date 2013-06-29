/*
   Copyright (C) 2013 by Pierre Talbot <ptalbot@mopong.net>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

#ifndef SERVER_GENERIC_FACTORY_HPP
#define SERVER_GENERIC_FACTORY_HPP

#include <string>
#include <memory>
#include <stdexcept>
#include <boost/lexical_cast.hpp>

class product_not_found : public std::logic_error
{
public:
   template <class Identifier>
   product_not_found(const Identifier& product_id)
   : logic_error(std::string("The product \"") 
      + boost::lexical_cast<std::string>(product_id) 
      + "\" is not registered in the factory.")
   {}
};

/* A generic factory which register the couple (identifier, product).
*  You can retrieve the product with the identifier later (after the registration step).
*  The product should be an abstract class.
*/
template <class Product, typename Identifier=std::string>
class generic_factory
{
public:
   typedef Product product_type;
   typedef Identifier identifier_type;
   typedef std::unique_ptr<product_type> product_ptr;
private:
   typedef std::map<identifier_type, product_ptr> id_to_product_map;
   id_to_product_map products;
public:
   void register_product(identifier_type&& id, product_ptr&& product)
   {
      products.emplace(std::move(id), std::move(product));
   }

   product_ptr make_product(const identifier_type& id)
   {
      typename id_to_product_map::const_iterator product = products.find(id);
      if(product == products.end())
         throw product_not_found(id);
      return std::unique_ptr<product_type>(new product_type(product->second));
   }
};

#endif // SERVER_GENERIC_FACTORY_HPP
