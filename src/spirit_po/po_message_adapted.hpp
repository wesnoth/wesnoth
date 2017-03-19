//  (C) Copyright 2015 - 2017 Christopher Beck

//  Distributed under the Boost Software License, Version 1.0. (See accompanying
//  file LICENSE or copy at http://www.boost.org/LICENSE_1_0.txt)

#ifndef SPIRIT_PO_PO_MESSAGE_ADAPTED_HPP_INCLUDED
#define SPIRIT_PO_PO_MESSAGE_ADAPTED_HPP_INCLUDED

#include <boost/fusion/adapted/struct/adapt_struct.hpp>
#include <boost/fusion/include/adapt_struct.hpp>
#include <boost/fusion/include/std_pair.hpp>

#include <boost/optional/optional.hpp>
#include <string>
#include <utility>
#include <vector>

#include <spirit_po/po_message.hpp>

BOOST_FUSION_ADAPT_STRUCT(
  spirit_po::po_message,
  (boost::optional<std::string>, context)
  (std::string, id)
  (spirit_po::plural_and_strings_type, plural_and_strings))

#endif // SPIRIT_PO_PO_MESSAGE_ADAPTED_HPP_INCLUDED
