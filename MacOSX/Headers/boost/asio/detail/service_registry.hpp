//
// service_registry.hpp
// ~~~~~~~~~~~~~~~~~~~~
//
// Copyright (c) 2003-2008 Christopher M. Kohlhoff (chris at kohlhoff dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef BOOST_ASIO_DETAIL_SERVICE_REGISTRY_HPP
#define BOOST_ASIO_DETAIL_SERVICE_REGISTRY_HPP

#if defined(_MSC_VER) && (_MSC_VER >= 1200)
# pragma once
#endif // defined(_MSC_VER) && (_MSC_VER >= 1200)

#include <boost/asio/detail/push_options.hpp>

#include <boost/asio/detail/push_options.hpp>
#include <memory>
#include <typeinfo>
#include <boost/asio/detail/pop_options.hpp>

#include <boost/asio/io_service.hpp>
#include <boost/asio/detail/mutex.hpp>
#include <boost/asio/detail/noncopyable.hpp>
#include <boost/asio/detail/service_id.hpp>

#if defined(BOOST_NO_TYPEID)
# if !defined(BOOST_ASIO_NO_TYPEID)
#  define BOOST_ASIO_NO_TYPEID
# endif // !defined(BOOST_ASIO_NO_TYPEID)
#endif // defined(BOOST_NO_TYPEID)

namespace boost {
namespace asio {
namespace detail {

#if defined(__GNUC__)
# if (__GNUC__ == 4 && __GNUC_MINOR__ >= 1) || (__GNUC__ > 4)
#  pragma GCC visibility push (default)
# endif // (__GNUC__ == 4 && __GNUC_MINOR__ >= 1) || (__GNUC__ > 4)
#endif // defined(__GNUC__)

template <typename T>
class typeid_wrapper {};

#if defined(__GNUC__)
# if (__GNUC__ == 4 && __GNUC_MINOR__ >= 1) || (__GNUC__ > 4)
#  pragma GCC visibility pop
# endif // (__GNUC__ == 4 && __GNUC_MINOR__ >= 1) || (__GNUC__ > 4)
#endif // defined(__GNUC__)

class service_registry
  : private noncopyable
{
public:
  // Constructor.
  service_registry(boost::asio::io_service& o)
    : owner_(o),
      first_service_(0)
  {
  }

  // Destructor.
  ~service_registry()
  {
    // Shutdown all services. This must be done in a separate loop before the
    // services are destroyed since the destructors of user-defined handler
    // objects may try to access other service objects.
    boost::asio::io_service::service* service = first_service_;
    while (service)
    {
      service->shutdown_service();
      service = service->next_;
    }

    // Destroy all services.
    while (first_service_)
    {
      boost::asio::io_service::service* next_service = first_service_->next_;
      delete first_service_;
      first_service_ = next_service;
    }
  }

  // Get the service object corresponding to the specified service type. Will
  // create a new service object automatically if no such object already
  // exists. Ownership of the service object is not transferred to the caller.
  template <typename Service>
  Service& use_service()
  {
    boost::asio::detail::mutex::scoped_lock lock(mutex_);

    // First see if there is an existing service object for the given type.
    boost::asio::io_service::service* service = first_service_;
    while (service)
    {
      if (service_id_matches(*service, Service::id))
        return *static_cast<Service*>(service);
      service = service->next_;
    }

    // Create a new service object. The service registry's mutex is not locked
    // at this time to allow for nested calls into this function from the new
    // service's constructor.
    lock.unlock();
    std::auto_ptr<Service> new_service(new Service(owner_));
    init_service_id(*new_service, Service::id);
    Service& new_service_ref = *new_service;
    lock.lock();

    // Check that nobody else created another service object of the same type
    // while the lock was released.
    service = first_service_;
    while (service)
    {
      if (service_id_matches(*service, Service::id))
        return *static_cast<Service*>(service);
      service = service->next_;
    }

    // Service was successfully initialised, pass ownership to registry.
    new_service->next_ = first_service_;
    first_service_ = new_service.release();

    return new_service_ref;
  }

  // Add a service object. Returns false on error, in which case ownership of
  // the object is retained by the caller.
  template <typename Service>
  bool add_service(Service* new_service)
  {
    boost::asio::detail::mutex::scoped_lock lock(mutex_);

    // Check if there is an existing service object for the given type.
    boost::asio::io_service::service* service = first_service_;
    while (service)
    {
      if (service_id_matches(*service, Service::id))
        return false;
      service = service->next_;
    }

    // Take ownership of the service object.
    init_service_id(*new_service, Service::id);
    new_service->next_ = first_service_;
    first_service_ = new_service;

    return true;
  }

  // Check whether a service object of the specified type already exists.
  template <typename Service>
  bool has_service() const
  {
    boost::asio::detail::mutex::scoped_lock lock(mutex_);

    boost::asio::io_service::service* service = first_service_;
    while (service)
    {
      if (service_id_matches(*service, Service::id))
        return true;
      service = service->next_;
    }

    return false;
  }

private:
  // Set a service's id.
  void init_service_id(boost::asio::io_service::service& service,
      const boost::asio::io_service::id& id)
  {
    service.type_info_ = 0;
    service.id_ = &id;
  }

#if !defined(BOOST_ASIO_NO_TYPEID)
  // Set a service's id.
  template <typename Service>
  void init_service_id(boost::asio::io_service::service& service,
      const boost::asio::detail::service_id<Service>& /*id*/)
  {
    service.type_info_ = &typeid(typeid_wrapper<Service>);
    service.id_ = 0;
  }
#endif // !defined(BOOST_ASIO_NO_TYPEID)

  // Check if a service matches the given id.
  static bool service_id_matches(
      const boost::asio::io_service::service& service,
      const boost::asio::io_service::id& id)
  {
    return service.id_ == &id;
  }

#if !defined(BOOST_ASIO_NO_TYPEID)
  // Check if a service matches the given id.
  template <typename Service>
  static bool service_id_matches(
      const boost::asio::io_service::service& service,
      const boost::asio::detail::service_id<Service>& /*id*/)
  {
    return service.type_info_ != 0
      && *service.type_info_ == typeid(typeid_wrapper<Service>);
  }
#endif // !defined(BOOST_ASIO_NO_TYPEID)

  // Mutex to protect access to internal data.
  mutable boost::asio::detail::mutex mutex_;

  // The owner of this service registry and the services it contains.
  boost::asio::io_service& owner_;

  // The first service in the list of contained services.
  boost::asio::io_service::service* first_service_;
};

} // namespace detail
} // namespace asio
} // namespace boost

#include <boost/asio/detail/pop_options.hpp>

#endif // BOOST_ASIO_DETAIL_SERVICE_REGISTRY_HPP
