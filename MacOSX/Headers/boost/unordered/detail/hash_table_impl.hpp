
// Copyright (C) 2003-2004 Jeremy B. Maitin-Shepard.
// Copyright (C) 2005-2008 Daniel James
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#if BOOST_UNORDERED_EQUIVALENT_KEYS
#define BOOST_UNORDERED_TABLE hash_table_equivalent_keys
#define BOOST_UNORDERED_TABLE_DATA hash_table_data_equivalent_keys
#define BOOST_UNORDERED_ITERATOR hash_iterator_equivalent_keys
#define BOOST_UNORDERED_CONST_ITERATOR hash_const_iterator_equivalent_keys
#define BOOST_UNORDERED_LOCAL_ITERATOR hash_local_iterator_equivalent_keys
#define BOOST_UNORDERED_CONST_LOCAL_ITERATOR hash_const_local_iterator_equivalent_keys
#else
#define BOOST_UNORDERED_TABLE hash_table_unique_keys
#define BOOST_UNORDERED_TABLE_DATA hash_table_data_unique_keys
#define BOOST_UNORDERED_ITERATOR hash_iterator_unique_keys
#define BOOST_UNORDERED_CONST_ITERATOR hash_const_iterator_unique_keys
#define BOOST_UNORDERED_LOCAL_ITERATOR hash_local_iterator_unique_keys
#define BOOST_UNORDERED_CONST_LOCAL_ITERATOR hash_const_local_iterator_unique_keys
#endif

namespace boost {
    namespace unordered_detail {

        //
        // Hash Table Data
        //
        // Responsible for managing the hash buckets.

        template <typename Alloc>
        class BOOST_UNORDERED_TABLE_DATA
        {
        public:
            typedef BOOST_UNORDERED_TABLE_DATA data;

            struct node_base;
            struct node;
            struct bucket;
            typedef std::size_t size_type;
            typedef std::ptrdiff_t difference_type;

            typedef Alloc value_allocator;

            typedef BOOST_DEDUCED_TYPENAME
                boost::unordered_detail::rebind_wrap<Alloc, node>::type
                node_allocator;
            typedef BOOST_DEDUCED_TYPENAME
                boost::unordered_detail::rebind_wrap<Alloc, node_base>::type
                node_base_allocator;
            typedef BOOST_DEDUCED_TYPENAME
                boost::unordered_detail::rebind_wrap<Alloc, bucket>::type
                bucket_allocator;

            typedef BOOST_DEDUCED_TYPENAME allocator_value_type<Alloc>::type value_type;
            typedef BOOST_DEDUCED_TYPENAME allocator_pointer<node_allocator>::type node_ptr;
            typedef BOOST_DEDUCED_TYPENAME allocator_pointer<bucket_allocator>::type bucket_ptr;
            typedef BOOST_DEDUCED_TYPENAME allocator_reference<value_allocator>::type reference;
            typedef BOOST_DEDUCED_TYPENAME allocator_reference<bucket_allocator>::type bucket_reference;

            typedef bucket_ptr link_ptr;

            // Hash Bucket
            //
            // all no throw

            struct bucket
            {
            private:
                bucket& operator=(bucket const&);
            public:
                link_ptr next_;

                bucket() : next_()
                {
                    BOOST_UNORDERED_MSVC_RESET_PTR(next_);
                }

                bucket(bucket const& x) : next_(x.next_)
                {
                    // Only copy construct when allocating.
                    BOOST_ASSERT(!x.next_);
                }

                bool empty() const
                {
                    return !this->next_;
                }
            };

            // Hash Node
            //
            // all no throw

            struct node_base : bucket
            {
#if BOOST_UNORDERED_EQUIVALENT_KEYS
            public:
                node_base() : group_prev_()
                {
                    BOOST_UNORDERED_MSVC_RESET_PTR(group_prev_);
                }

                link_ptr group_prev_;
#endif
            };

            struct node : node_base
            {
            public:
#if defined(BOOST_HAS_RVALUE_REFS) && defined(BOOST_HAS_VARIADIC_TMPL)
                template <typename... Args>
                node(Args&&... args)
                    : node_base(), value_(std::forward<Args>(args)...) {}
#else
                node(value_type const& v) : node_base(), value_(v) {}
#endif

                value_type value_;
            };

#if defined(BOOST_HAS_RVALUE_REFS) && defined(BOOST_HAS_VARIADIC_TMPL)

            // allocators
            //
            // Stores all the allocators that we're going to need.

            struct allocators
            {
                node_allocator node_alloc_;
                bucket_allocator bucket_alloc_;

                allocators(value_allocator const& a)
                    : node_alloc_(a), bucket_alloc_(a)
                {}

                void destroy(link_ptr ptr)
                {
                    node_ptr n(node_alloc_.address(*static_cast<node*>(&*ptr)));
                    node_alloc_.destroy(n);
                    node_alloc_.deallocate(n, 1);
                }

                void swap(allocators& x)
                {
                    unordered_detail::hash_swap(node_alloc_, x.node_alloc_);
                    unordered_detail::hash_swap(bucket_alloc_, x.bucket_alloc_);
                }

                bool operator==(allocators const& x)
                {
                    return node_alloc_ == x.node_alloc_;
                }
            };

            // node_constructor
            //
            // Used to construct nodes in an exception safe manner.

            class node_constructor
            {
                allocators& allocators_;

                node_ptr node_;
                bool node_constructed_;

            public:

                node_constructor(allocators& a)
                    : allocators_(a),
                    node_(), node_constructed_(false)
                {
                }

                ~node_constructor()
                {
                    if (node_) {
                        if (node_constructed_)
                            allocators_.node_alloc_.destroy(node_);
                        allocators_.node_alloc_.deallocate(node_, 1);
                    }
                }

                template <typename... Args>
                void construct(Args&&... args)
                {
                    BOOST_ASSERT(!node_);
                    node_constructed_ = false;

                    node_ = allocators_.node_alloc_.allocate(1);
                    allocators_.node_alloc_.construct(node_, std::forward<Args>(args)...);
                    node_constructed_ = true;
                }

                node_ptr get() const
                {
                    BOOST_ASSERT(node_);
                    return node_;
                }

                // no throw
                link_ptr release()
                {
                    node_ptr p = node_;
                    unordered_detail::reset(node_);
                    return link_ptr(allocators_.bucket_alloc_.address(*p));
                }

            private:
                node_constructor(node_constructor const&);
                node_constructor& operator=(node_constructor const&);
            };
#else

            // allocators
            //
            // Stores all the allocators that we're going to need.

            struct allocators
            {
                node_allocator node_alloc_;
                bucket_allocator bucket_alloc_;
                value_allocator value_alloc_;
                node_base_allocator node_base_alloc_;

                allocators(value_allocator const& a)
                    : node_alloc_(a), bucket_alloc_(a),
                    value_alloc_(a), node_base_alloc_(a)
                {}

                void destroy(link_ptr ptr)
                {
                    node_ptr n(node_alloc_.address(*static_cast<node*>(&*ptr)));
                    value_alloc_.destroy(value_alloc_.address(n->value_));
                    node_base_alloc_.destroy(node_base_alloc_.address(*n));
                    node_alloc_.deallocate(n, 1);
                }

                void swap(allocators& x)
                {
                    unordered_detail::hash_swap(node_alloc_, x.node_alloc_);
                    unordered_detail::hash_swap(bucket_alloc_, x.bucket_alloc_);
                    unordered_detail::hash_swap(value_alloc_, x.value_alloc_);
                    unordered_detail::hash_swap(node_base_alloc_, x.node_base_alloc_);
                }

                bool operator==(allocators const& x)
                {
                    return value_alloc_ == x.value_alloc_;
                }
            };

            // node_constructor
            //
            // Used to construct nodes in an exception safe manner.

            class node_constructor
            {
                allocators& allocators_;

                node_ptr node_;
                bool value_constructed_;
                bool node_base_constructed_;

            public:

                node_constructor(allocators& a)
                    : allocators_(a),
                    node_(), value_constructed_(false), node_base_constructed_(false)
                {
                    BOOST_UNORDERED_MSVC_RESET_PTR(node_);
                }

                ~node_constructor()
                {
                    if (node_) {
                        if (value_constructed_)
                            allocators_.value_alloc_.destroy(
                                allocators_.value_alloc_.address(node_->value_));
                        if (node_base_constructed_)
                            allocators_.node_base_alloc_.destroy(
                                allocators_.node_base_alloc_.address(*node_));

                        allocators_.node_alloc_.deallocate(node_, 1);
                    }
                }

                template <typename V>
                void construct(V const& v)
                {
                    BOOST_ASSERT(!node_);
                    value_constructed_ = false;
                    node_base_constructed_ = false;

                    node_ = allocators_.node_alloc_.allocate(1);

                    allocators_.node_base_alloc_.construct(
                            allocators_.node_base_alloc_.address(*node_),
                            node_base());
                    node_base_constructed_ = true;

                    allocators_.value_alloc_.construct(
                            allocators_.value_alloc_.address(node_->value_), v);
                    value_constructed_ = true;
                }

                node_ptr get() const
                {
                    BOOST_ASSERT(node_);
                    return node_;
                }

                // no throw
                link_ptr release()
                {
                    node_ptr p = node_;
                    unordered_detail::reset(node_);
                    return link_ptr(allocators_.bucket_alloc_.address(*p));
                }

            private:
                node_constructor(node_constructor const&);
                node_constructor& operator=(node_constructor const&);
            };
#endif

            // Methods for navigating groups of elements with equal keys.

#if BOOST_UNORDERED_EQUIVALENT_KEYS
            static inline link_ptr& prev_in_group(link_ptr n) {
                return static_cast<node*>(&*n)->group_prev_;
            }

            // pre: Must be pointing to the first node in a group.
            static inline link_ptr& next_group(link_ptr n) {
                BOOST_ASSERT(BOOST_UNORDERED_BORLAND_BOOL(n) && n != prev_in_group(n)->next_);
                return prev_in_group(n)->next_;
            }
#else
            static inline link_ptr& next_group(link_ptr n) {
                BOOST_ASSERT(n);
                return n->next_;
            }
#endif

            // pre: Must be pointing to a node
            static inline node& get_node(link_ptr p) {
                BOOST_ASSERT(p);
                return *static_cast<node*>(&*p);
            }

            // pre: Must be pointing to a node
            static inline reference get_value(link_ptr p) {
                BOOST_ASSERT(p);
                return static_cast<node*>(&*p)->value_;
            }

            class iterator_base
            {
                typedef BOOST_UNORDERED_TABLE_DATA<Alloc> data;
            public:
                bucket_ptr bucket_;
                link_ptr node_;

                iterator_base()
                    : bucket_(), node_()
                {
                    BOOST_UNORDERED_MSVC_RESET_PTR(bucket_);
                    BOOST_UNORDERED_MSVC_RESET_PTR(node_);
                }

                explicit iterator_base(bucket_ptr b)
                    : bucket_(b), node_(b->next_) {}

                iterator_base(bucket_ptr b, link_ptr n)
                    : bucket_(b), node_(n) {}

                bool operator==(iterator_base const& x) const
                {
                    return node_ == x.node_;
                }

                bool operator!=(iterator_base const& x) const
                {
                    return node_ != x.node_;
                }

                reference operator*() const
                {
                    return get_value(node_);
                }

                void increment()
                {
                    BOOST_ASSERT(bucket_);
                    node_ = node_->next_;

                    while (!node_) {
                        ++bucket_;
                        node_ = bucket_->next_;
                    }
                }

                void increment_group()
                {
                    node_ = data::next_group(node_);

                    while (!node_) {
                        ++bucket_;
                        node_ = bucket_->next_;
                    }
                }
            };

            // Member Variables

            allocators allocators_;
            bucket_ptr buckets_;
            bucket_manager bucket_manager_;
            bucket_ptr cached_begin_bucket_;
            size_type size_;           

            // Constructors/Deconstructor

            BOOST_UNORDERED_TABLE_DATA(size_type n, value_allocator const& a)
              : allocators_(a),
                buckets_(), bucket_manager_(n),
                cached_begin_bucket_(), size_(0)
            {
                BOOST_UNORDERED_MSVC_RESET_PTR(buckets_);
                create_buckets();
            }

            BOOST_UNORDERED_TABLE_DATA(BOOST_UNORDERED_TABLE_DATA const& x, size_type n)
              : allocators_(x.allocators_),
                buckets_(), bucket_manager_(n),
                cached_begin_bucket_(), size_(0)
            {
                BOOST_UNORDERED_MSVC_RESET_PTR(buckets_);
                create_buckets();
            }

            BOOST_UNORDERED_TABLE_DATA(BOOST_UNORDERED_TABLE_DATA& x, move_tag)
                : allocators_(x.allocators_),
                buckets_(x.buckets_), bucket_manager_(x.bucket_manager_),
                cached_begin_bucket_(x.cached_begin_bucket_), size_(x.size_)
            {
                unordered_detail::reset(x.buckets_);
            }

            BOOST_UNORDERED_TABLE_DATA(BOOST_UNORDERED_TABLE_DATA& x,
                    value_allocator const& a, size_type n, move_tag)
                : allocators_(a), buckets_(), bucket_manager_(),
                cached_begin_bucket_(), size_(0)
            {
                if(allocators_ == x.allocators_) {
                    buckets_ = x.buckets_;
                    bucket_manager_ = x.bucket_manager_;
                    cached_begin_bucket_ = x.cached_begin_bucket_;
                    size_ = x.size_;
                    unordered_detail::reset(x.buckets_);
                }
                else {
                    BOOST_UNORDERED_MSVC_RESET_PTR(buckets_);
                    bucket_manager_ = bucket_manager(n);
                    create_buckets();
                }
            }

            // no throw
            ~BOOST_UNORDERED_TABLE_DATA()
            {
                delete_buckets();
            }

            void create_buckets() {
                size_type bucket_count = bucket_manager_.bucket_count();
            
                // The array constructor will clean up in the event of an
                // exception.
                allocator_array_constructor<bucket_allocator>
                    constructor(allocators_.bucket_alloc_);

                // Creates an extra bucket to act as a sentinel.
                constructor.construct(bucket(), bucket_count + 1);

                cached_begin_bucket_ = constructor.get() + static_cast<difference_type>(bucket_count);

                // Set up the sentinel.
                cached_begin_bucket_->next_ = link_ptr(cached_begin_bucket_);

                // Only release the buckets once everything is successfully
                // done.
                buckets_ = constructor.release();
            }

            // no throw
            void delete_buckets()
            {
                if(buckets_) {
                    bucket_ptr begin = cached_begin_bucket_;
                    bucket_ptr end = buckets_end();
                    while(begin != end) {
                        clear_bucket(begin);
                        ++begin;
                    }

                    // Destroy an extra bucket for the sentinels.
                    ++end;
                    for(begin = buckets_; begin != end; ++begin)
                        allocators_.bucket_alloc_.destroy(begin);

                    allocators_.bucket_alloc_.deallocate(buckets_,
                        bucket_manager_.bucket_count() + 1);
                }
            }

        private:

            BOOST_UNORDERED_TABLE_DATA(BOOST_UNORDERED_TABLE_DATA const&);
            BOOST_UNORDERED_TABLE_DATA& operator=(BOOST_UNORDERED_TABLE_DATA const&);

        public:

            // no throw
            void swap(BOOST_UNORDERED_TABLE_DATA& other)
            {
                std::swap(buckets_, other.buckets_);
                std::swap(bucket_manager_, other.bucket_manager_);
                std::swap(cached_begin_bucket_, other.cached_begin_bucket_);
                std::swap(size_, other.size_);
            }

            // no throw
            void move(BOOST_UNORDERED_TABLE_DATA& other)
            {
                delete_buckets();
                buckets_ = other.buckets_;
                unordered_detail::reset(other.buckets_);
                bucket_manager_ = other.bucket_manager_;
                cached_begin_bucket_ = other.cached_begin_bucket_;
                size_ = other.size_;
            }

            // Return the bucket number for a hashed value.
            //
            // no throw
            size_type bucket_from_hash(size_type hashed) const
            {
                return bucket_manager_.bucket_from_hash(hashed);
            }

            // Return the bucket for a hashed value.
            //
            // no throw
            bucket_ptr bucket_ptr_from_hash(size_type hashed) const
            {
                return buckets_ + static_cast<difference_type>(
                    bucket_manager_.bucket_from_hash(hashed));
            }

            // Begin & End
            //
            // no throw

            bucket_ptr buckets_end() const
            {
                return buckets_ + static_cast<difference_type>(bucket_manager_.bucket_count());
            }

            iterator_base begin() const
            {
                return size_
                    ? iterator_base(cached_begin_bucket_)
                    : end();
            }

            iterator_base end() const
            {
                return iterator_base(buckets_end());
            }

            link_ptr begin(size_type n) const
            {
                return (buckets_ + static_cast<difference_type>(n))->next_;
            }

            link_ptr end(size_type) const
            {
                return unordered_detail::null_ptr<link_ptr>();
            }

            link_ptr begin(bucket_ptr b) const
            {
                return b->next_;
            }

            // Bucket Size

            // no throw
            static inline size_type node_count(link_ptr it)
            {
                size_type count = 0;
                while(BOOST_UNORDERED_BORLAND_BOOL(it)) {
                    ++count;
                    it = it->next_;
                }
                return count;
            }

            static inline size_type node_count(link_ptr it1, link_ptr it2)
            {
                size_type count = 0;
                while(it1 != it2) {
                    ++count;
                    it1 = it1->next_;
                }
                return count;
            }

            size_type bucket_size(size_type n) const
            {
                return node_count(begin(n));
            }

#if BOOST_UNORDERED_EQUIVALENT_KEYS
            static inline size_type group_count(link_ptr it)
            {
                return node_count(it, next_group(it));
            }
#else
            static inline size_type group_count(link_ptr)
            {
                return 1;
            }
#endif

            // get_for_erase
            //
            // Find the pointer to a node, for use when erasing.
            //
            // no throw

#if BOOST_UNORDERED_EQUIVALENT_KEYS
            static link_ptr* get_for_erase(iterator_base r)
            {
                link_ptr n = r.node_;

                // If the element isn't the first in its group, then
                // the link to it will be found in the previous element
                // in the group.
                link_ptr* it = &prev_in_group(n)->next_;
                if(*it == n) return it;

                // The element is the first in its group, so iterate
                // throught the groups, checking against the first element.
                it = &r.bucket_->next_;
                while(*it != n) it = &BOOST_UNORDERED_TABLE_DATA::next_group(*it);
                return it;
            }
#else
            static link_ptr* get_for_erase(iterator_base r)
            {
                link_ptr n = r.node_;
                link_ptr* it = &r.bucket_->next_;
                while(*it != n) it = &(*it)->next_;
                return it;
            }
#endif

            // Link/Unlink/Move Node
            //
            // For adding nodes to buckets, removing them and moving them to a
            // new bucket.
            //
            // no throw

#if BOOST_UNORDERED_EQUIVALENT_KEYS
            // If n points to the first node in a group, this adds it to the
            // end of that group.
            link_ptr link_node(node_constructor& a, link_ptr pos)
            {
                link_ptr n = a.release();
                node& node_ref = get_node(n);
                node& pos_ref = get_node(pos);
                node_ref.next_ = pos_ref.group_prev_->next_;
                node_ref.group_prev_ = pos_ref.group_prev_;
                pos_ref.group_prev_->next_ = n;
                pos_ref.group_prev_ = n;
                ++size_;
                return n;
            }

            link_ptr link_node_in_bucket(node_constructor& a, bucket_ptr base)
            {
                link_ptr n = a.release();
                node& node_ref = get_node(n);
                node_ref.next_ = base->next_;
                node_ref.group_prev_ = n;
                base->next_ = n;
                ++size_;
                if(base < cached_begin_bucket_) cached_begin_bucket_ = base;
                return n;
            }

            void link_group(link_ptr n, bucket_ptr base, size_type count)
            {
                node& node_ref = get_node(n);
                node& last_ref = get_node(node_ref.group_prev_);
                last_ref.next_ = base->next_;
                base->next_ = n;
                size_ += count;
                if(base < cached_begin_bucket_) cached_begin_bucket_ = base;
            }
#else
            void link_node(link_ptr n, bucket_ptr base)
            {
                n->next_ = base->next_;
                base->next_ = n;
                ++size_;
                if(base < cached_begin_bucket_) cached_begin_bucket_ = base;
            }

            link_ptr link_node_in_bucket(node_constructor& a, bucket_ptr base)
            {
                link_ptr n = a.release();
                link_node(n, base);
                return n;
            }

            void link_group(link_ptr n, bucket_ptr base, size_type)
            {
                link_node(n, base);
            }
#endif

#if BOOST_UNORDERED_EQUIVALENT_KEYS
            void unlink_node(iterator_base it)
            {
                link_ptr* pos = get_for_erase(it);
                node* n = &get_node(it.node_);
                link_ptr next = n->next_;

                if(n->group_prev_ == *pos) {
                    // The deleted node is the sole node in the group, so
                    // no need to unlink it from a group.
                }
                else if(BOOST_UNORDERED_BORLAND_BOOL(next) && prev_in_group(next) == *pos)
                {
                    // The deleted node is not at the end of the group, so
                    // change the link from the next node.
                    prev_in_group(next) = n->group_prev_;
                }
                else {
                    // The deleted node is at the end of the group, so the
                    // first node in the group is pointing to it.
                    // Find that to change its pointer.
                    link_ptr it = n->group_prev_;
                    while(prev_in_group(it) != *pos) {
                        it = prev_in_group(it);
                    }
                    prev_in_group(it) = n->group_prev_;
                }
                *pos = next;
                --size_;
            }

            size_type unlink_group(link_ptr* pos)
            {
                size_type count = group_count(*pos);
                size_ -= count;
                *pos = next_group(*pos);
                return count;
            }
#else
            void unlink_node(iterator_base n)
            {
                link_ptr* pos = get_for_erase(n);
                *pos = (*pos)->next_;
                --size_;
            }

            size_type unlink_group(link_ptr* pos)
            {
                *pos = (*pos)->next_;
                --size_;
                return 1;
            }
#endif

            void unlink_nodes(iterator_base n)
            {
                link_ptr* it = get_for_erase(n);
                split_group(*it);
                unordered_detail::reset(*it);
                size_ -= node_count(n.node_);
            }

            void unlink_nodes(iterator_base begin, iterator_base end)
            {
                BOOST_ASSERT(begin.bucket_ == end.bucket_);
                size_ -= node_count(begin.node_, end.node_);
                link_ptr* it = get_for_erase(begin);
                split_group(*it, end.node_);
                *it = end.node_;
            }

            void unlink_nodes(bucket_ptr base, iterator_base end)
            {
                BOOST_ASSERT(base == end.bucket_);

                split_group(end.node_);

                link_ptr ptr(base->next_);
                base->next_ = end.node_;

                size_ -= node_count(ptr, end.node_);
            }

#if BOOST_UNORDERED_EQUIVALENT_KEYS
            // Break a ciruclar list into two, with split as the beginning
            // of the second group (if split is at the beginning then don't
            // split).
            static inline link_ptr split_group(link_ptr split)
            {
                // If split is at the beginning of the group then there's
                // nothing to split.
                if(prev_in_group(split)->next_ != split)
                    return unordered_detail::null_ptr<link_ptr>();

                // Find the start of the group.
                link_ptr start = split;
                do {
                    start = prev_in_group(start);
                } while(prev_in_group(start)->next_ == start);

                link_ptr last = prev_in_group(start);
                prev_in_group(start) = prev_in_group(split);
                prev_in_group(split) = last;

                return start;
            }

            static inline void split_group(link_ptr split1, link_ptr split2)
            {
                link_ptr begin1 = split_group(split1);
                link_ptr begin2 = split_group(split2);

                if(BOOST_UNORDERED_BORLAND_BOOL(begin1) && split1 == begin2) {
                    link_ptr end1 = prev_in_group(begin1);
                    prev_in_group(begin1) = prev_in_group(begin2);
                    prev_in_group(begin2) = end1;
                }
            }
#else
            static inline void split_group(link_ptr)
            {
            }

            static inline void split_group(link_ptr, link_ptr)
            {
            }
#endif

            // copy_group
            //
            // Basic exception safety.
            // If it throws, it only copies some of the nodes in the group.

#if BOOST_UNORDERED_EQUIVALENT_KEYS
            void copy_group(link_ptr it, bucket_ptr dst)
            {
                node_constructor a(allocators_);

                link_ptr end = next_group(it);

                a.construct(get_value(it));                     // throws
                link_ptr n = link_node_in_bucket(a, dst);

                for(it = it->next_; it != end; it = it->next_) {
                    a.construct(get_value(it));                 // throws
                    link_node(a, n);
                }
            }
#else
            void copy_group(link_ptr it, bucket_ptr dst)
            {
                node_constructor a(allocators_);

                a.construct(get_value(it));                     // throws
                link_node_in_bucket(a, dst);
            }
#endif

            // Delete Node
            //
            // Remove a node, or a range of nodes, from a bucket, and destroy
            // them.
            //
            // no throw

            void delete_to_bucket_end(link_ptr begin)
            {
                while(begin) {
                    link_ptr node = begin;
                    begin = begin->next_;
                    allocators_.destroy(node);
                }
            }

            void delete_nodes(link_ptr begin, link_ptr end)
            {
                while(begin != end) {
                    link_ptr node = begin;
                    begin = begin->next_;
                    allocators_.destroy(node);
                }
            }

#if BOOST_UNORDERED_EQUIVALENT_KEYS
            void delete_group(link_ptr first_node)
            {
                delete_nodes(first_node, prev_in_group(first_node)->next_);
            }
#else
            void delete_group(link_ptr node)
            {
                allocators_.destroy(node);
            }
#endif

            // Clear
            //
            // Remove all the nodes.
            //
            // no throw

            void clear_bucket(bucket_ptr b)
            {
                link_ptr first_node = b->next_;
                unordered_detail::reset(b->next_);
                delete_to_bucket_end(first_node);
            }

            void clear()
            {
                bucket_ptr begin = cached_begin_bucket_;
                bucket_ptr end = buckets_end();

                size_ = 0;
                cached_begin_bucket_ = end;

                while(begin != end) {
                    clear_bucket(begin);
                    ++begin;
                }
            }

            // Erase
            //
            // no throw

            iterator_base erase(iterator_base r)
            {
                BOOST_ASSERT(r != end());
                iterator_base next = r;
                next.increment();
                unlink_node(r);
                allocators_.destroy(r.node_);
                // r has been invalidated but its bucket is still valid
                recompute_begin_bucket(r.bucket_, next.bucket_);
                return next;
            }

            iterator_base erase_range(iterator_base r1, iterator_base r2)
            {
                if(r1 != r2)
                {
                    BOOST_ASSERT(r1 != end());

                    if (r1.bucket_ == r2.bucket_) {
                        unlink_nodes(r1, r2);
                        delete_nodes(r1.node_, r2.node_);

                        // No need to call recompute_begin_bucket because
                        // the nodes are only deleted from one bucket, which
                        // still contains r2 after the erase.
                        BOOST_ASSERT(!r1.bucket_->empty());
                    }
                    else {
                        BOOST_ASSERT(r1.bucket_ < r2.bucket_);

                        unlink_nodes(r1);
                        delete_to_bucket_end(r1.node_);

                        bucket_ptr i = r1.bucket_;
                        for(++i; i != r2.bucket_; ++i) {
                            size_ -= node_count(i->next_);
                            clear_bucket(i);
                        }

                        if(r2 != end()) {
                            link_ptr first = r2.bucket_->next_;
                            unlink_nodes(r2.bucket_, r2);
                            delete_nodes(first, r2.node_);
                        }

                        // r1 has been invalidated but its bucket is still
                        // valid.
                        recompute_begin_bucket(r1.bucket_, r2.bucket_);
                    }
                }

                return r2;
            }

            // recompute_begin_bucket
            //
            // After an erase cached_begin_bucket_ might be left pointing to
            // an empty bucket, so this is called to update it
            //
            // no throw

            void recompute_begin_bucket(bucket_ptr b)
            {
                BOOST_ASSERT(!(b < cached_begin_bucket_));

                if(b == cached_begin_bucket_)
                {
                    if (size_ != 0) {
                        while (cached_begin_bucket_->empty())
                            ++cached_begin_bucket_;
                    } else {
                        cached_begin_bucket_ = buckets_end();
                    }
                }
            }

            // This is called when a range has been erased
            //
            // no throw

            void recompute_begin_bucket(bucket_ptr b1, bucket_ptr b2)
            {
                BOOST_ASSERT(!(b1 < cached_begin_bucket_) && !(b2 < b1));
                BOOST_ASSERT(b2 == buckets_end() || !b2->empty());

                if(b1 == cached_begin_bucket_ && b1->empty())
                    cached_begin_bucket_ = b2;
            }

            size_type erase_group(link_ptr* it, bucket_ptr bucket)
            {
                link_ptr pos = *it;
                size_type count = unlink_group(it);
                delete_group(pos);

                this->recompute_begin_bucket(bucket);

                return count;
            }
        };

#if defined(BOOST_MPL_CFG_MSVC_ETI_BUG)
        template <>
        class BOOST_UNORDERED_TABLE_DATA<int>
        {
        public:
            typedef int size_type;
            typedef int iterator_base;
        };
#endif

        //
        // Hash Table
        //

        template <typename ValueType, typename KeyType,
            typename Hash, typename Pred,
            typename Alloc>
        class BOOST_UNORDERED_TABLE
        {
            typedef BOOST_UNORDERED_TABLE_DATA<Alloc> data;

            typedef BOOST_DEDUCED_TYPENAME data::node_constructor node_constructor;
            typedef BOOST_DEDUCED_TYPENAME data::bucket_ptr bucket_ptr;
            typedef BOOST_DEDUCED_TYPENAME data::link_ptr link_ptr;

        public:

            typedef BOOST_DEDUCED_TYPENAME data::value_allocator value_allocator;
            typedef BOOST_DEDUCED_TYPENAME data::node_allocator node_allocator;

            // Type definitions

            typedef KeyType key_type;
            typedef Hash hasher;
            typedef Pred key_equal;
            typedef ValueType value_type;
            typedef std::size_t size_type;
            typedef std::ptrdiff_t difference_type;

            // iterators

            typedef BOOST_DEDUCED_TYPENAME data::iterator_base iterator_base;

        private:


            typedef boost::unordered_detail::buffered_functions<Hash, Pred>
                function_store;
            typedef BOOST_DEDUCED_TYPENAME function_store::functions functions;
            typedef BOOST_DEDUCED_TYPENAME function_store::functions_ptr
                functions_ptr;

            function_store functions_;
            float mlf_;
            size_type max_load_;

        public:

            data data_;

            // Constructors
            //
            // In the constructors, if anything throws an exception,
            // BOOST_UNORDERED_TABLE_DATA's destructor will clean up.

            BOOST_UNORDERED_TABLE(size_type n,
                    hasher const& hf, key_equal const& eq,
                    value_allocator const& a)
                : functions_(hf, eq), // throws, cleans itself up
                mlf_(1.0f),           // no throw
                data_(n, a)           // throws, cleans itself up
            {
                calculate_max_load(); // no throw
            }

            // Construct from iterators

            // initial_size
            //
            // A helper function for the copy constructor to calculate how many
            // nodes will be created if the iterator's support it. Might get it
            // totally wrong for containers with unique keys.
            //
            // no throw

            template <typename I>
            size_type initial_size(I i, I j, size_type n,
                    boost::forward_traversal_tag)
            {
                // max load factor isn't set yet, but when it is, it'll be 1.0.
                return (std::max)(static_cast<size_type>(unordered_detail::distance(i, j)) + 1, n);
            }

            template <typename I>
            size_type initial_size(I, I, size_type n,
                    boost::incrementable_traversal_tag)
            {
                return n;
            }

            template <typename I>
            size_type initial_size(I i, I j, size_type n)
            {
                BOOST_DEDUCED_TYPENAME boost::iterator_traversal<I>::type
                    iterator_traversal_tag;
                return initial_size(i, j, n, iterator_traversal_tag);
            }

            template <typename I>
            BOOST_UNORDERED_TABLE(I i, I j, size_type n,
                    hasher const& hf, key_equal const& eq,
                    value_allocator const& a)
                : functions_(hf, eq),              // throws, cleans itself up
                  mlf_(1.0f),                      // no throw
                  data_(initial_size(i, j, n), a)  // throws, cleans itself up
            {
                calculate_max_load(); // no throw

                // This can throw, but BOOST_UNORDERED_TABLE_DATA's destructor will clean up.
                insert_range(i, j);
            }

            // Copy Construct

            BOOST_UNORDERED_TABLE(BOOST_UNORDERED_TABLE const& x)
                : functions_(x.functions_), // throws
                  mlf_(x.mlf_),             // no throw
                  data_(x.data_, x.min_buckets_for_size(x.size()))  // throws
            {
                calculate_max_load(); // no throw

                // This can throw, but BOOST_UNORDERED_TABLE_DATA's destructor will clean
                // up.
                copy_buckets(x.data_, data_, functions_.current());
            }

            // Copy Construct with allocator

            BOOST_UNORDERED_TABLE(BOOST_UNORDERED_TABLE const& x,
                    value_allocator const& a)
                : functions_(x.functions_), // throws
                mlf_(x.mlf_),               // no throw
                data_(x.min_buckets_for_size(x.size()), a)
            {
                calculate_max_load(); // no throw

                // This can throw, but BOOST_UNORDERED_TABLE_DATA's destructor will clean
                // up.
                copy_buckets(x.data_, data_, functions_.current());
            }

            // Move Construct

            BOOST_UNORDERED_TABLE(BOOST_UNORDERED_TABLE& x, move_tag m)
                : functions_(x.functions_), // throws
                  mlf_(x.mlf_),             // no throw
                  data_(x.data_, m)         // throws
            {
                calculate_max_load(); // no throw
            }

            BOOST_UNORDERED_TABLE(BOOST_UNORDERED_TABLE& x,
                    value_allocator const& a, move_tag m)
                : functions_(x.functions_), // throws
                  mlf_(x.mlf_),             // no throw
                  data_(x.data_, a,
                        x.min_buckets_for_size(x.size()), m)  // throws
            {
                calculate_max_load(); // no throw

                if(x.data_.buckets_) {
                    // This can throw, but BOOST_UNORDERED_TABLE_DATA's destructor will clean
                    // up.
                    copy_buckets(x.data_, data_, functions_.current());
                }
            }

            // Assign
            //
            // basic exception safety, if buffered_functions::buffer or reserver throws
            // the container is left in a sane, empty state. If copy_buckets
            // throws the container is left with whatever was successfully
            // copied.

            BOOST_UNORDERED_TABLE& operator=(BOOST_UNORDERED_TABLE const& x)
            {
                if(this != &x)
                {
                    data_.clear();                        // no throw
                    functions_.set(functions_.buffer(x.functions_));
                                                          // throws, strong
                    mlf_ = x.mlf_;                        // no throw
                    calculate_max_load();                 // no throw
                    reserve(x.size());                    // throws
                    copy_buckets(x.data_, data_, functions_.current()); // throws
                }

                return *this;
            }

            // Swap
            //
            // Swap's behaviour when allocators aren't equal is in dispute, for
            // details see:
            //
            // http://unordered.nfshost.com/doc/html/unordered/rationale.html#swapping_containers_with_unequal_allocators
            //
            // ----------------------------------------------------------------
            //
            // Strong exception safety (might change unused function objects)
            //
            // Can throw if hash or predicate object's copy constructor throws
            // or if allocators are unequal.

            void swap(BOOST_UNORDERED_TABLE& x)
            {
                // The swap code can work when swapping a container with itself
                // but it triggers an assertion in buffered_functions.
                // At the moment, I'd rather leave that assertion in and add a
                // check here, rather than remove the assertion. I might change
                // this at a later date.
                if(this == &x) return;

                // These can throw, but they only affect the function objects
                // that aren't in use so it is strongly exception safe, via.
                // double buffering.
                functions_ptr new_func_this = functions_.buffer(x.functions_);
                functions_ptr new_func_that = x.functions_.buffer(functions_);

                if(data_.allocators_ == x.data_.allocators_) {
                    data_.swap(x.data_); // no throw
                }
                else {
                    // Create new buckets in separate HASH_TABLE_DATA objects
                    // which will clean up if anything throws an exception.
                    // (all can throw, but with no effect as these are new objects).
                    data new_this(data_, x.min_buckets_for_size(x.data_.size_));
                    copy_buckets(x.data_, new_this, functions_.*new_func_this);

                    data new_that(x.data_, min_buckets_for_size(data_.size_));
                    x.copy_buckets(data_, new_that, x.functions_.*new_func_that);

                    // Start updating the data here, no throw from now on.
                    data_.swap(new_this);
                    x.data_.swap(new_that);
                }

                // We've made it, the rest is no throw.
                std::swap(mlf_, x.mlf_);

                functions_.set(new_func_this);
                x.functions_.set(new_func_that);

                calculate_max_load();
                x.calculate_max_load();
            }

            // Move
            //
            // ----------------------------------------------------------------
            //
            // Strong exception safety (might change unused function objects)
            //
            // Can throw if hash or predicate object's copy constructor throws
            // or if allocators are unequal.

            void move(BOOST_UNORDERED_TABLE& x)
            {
                // This can throw, but it only affects the function objects
                // that aren't in use so it is strongly exception safe, via.
                // double buffering.
                functions_ptr new_func_this = functions_.buffer(x.functions_);

                if(data_.allocators_ == x.data_.allocators_) {
                    data_.move(x.data_); // no throw
                }
                else {
                    // Create new buckets in separate HASH_TABLE_DATA objects
                    // which will clean up if anything throws an exception.
                    // (all can throw, but with no effect as these are new objects).
                    data new_this(data_, x.min_buckets_for_size(x.data_.size_));
                    copy_buckets(x.data_, new_this, functions_.*new_func_this);

                    // Start updating the data here, no throw from now on.
                    data_.move(new_this);
                }

                // We've made it, the rest is no throw.
                mlf_ = x.mlf_;
                functions_.set(new_func_this);
                calculate_max_load();
            }

            // accessors

            // no throw
#if defined(BOOST_HAS_RVALUE_REFS) && defined(BOOST_HAS_VARIADIC_TMPL)
            node_allocator get_allocator() const
            {
                return data_.allocators_.node_alloc_;
            }
#else
            value_allocator get_allocator() const
            {
                return data_.allocators_.value_alloc_;
            }
#endif

            // no throw
            hasher const& hash_function() const
            {
                return functions_.current().hash_function();
            }

            // no throw
            key_equal const& key_eq() const
            {
                return functions_.current().key_eq();
            }

            // no throw
            size_type size() const
            {
                return data_.size_;
            }

            // no throw
            bool empty() const
            {
                return data_.size_ == 0;
            }

            // no throw
            size_type max_size() const
            {
                using namespace std;

                // size < mlf_ * count
                return double_to_size_t(ceil(
                        (double) mlf_ * max_bucket_count())) - 1;
            }

            // strong safety
            size_type bucket(key_type const& k) const
            {
                // hash_function can throw:
                return data_.bucket_from_hash(hash_function()(k));
            }


            // strong safety
            bucket_ptr get_bucket(key_type const& k) const
            {
                return data_.buckets_ + static_cast<difference_type>(bucket(k));
            }

            // no throw
            size_type bucket_count() const
            {
                return data_.bucket_manager_.bucket_count();
            }

            // no throw
            size_type max_bucket_count() const
            {
                // -1 to account for the end marker.
                return prev_prime(data_.allocators_.bucket_alloc_.max_size() - 1);
            }

        private:

            // no throw
            size_type min_buckets_for_size(size_type n) const
            {
                BOOST_ASSERT(mlf_ != 0);

                using namespace std;

                // From 6.3.1/13:
                // size < mlf_ * count
                // => count > size / mlf_
                //
                // Or from rehash post-condition:
                // count > size / mlf_
                return double_to_size_t(floor(n / (double) mlf_)) + 1;
            }

            // no throw
            void calculate_max_load()
            {
                using namespace std;

                // From 6.3.1/13:
                // Only resize when size >= mlf_ * count
                max_load_ = double_to_size_t(ceil(
                        (double) mlf_ * data_.bucket_manager_.bucket_count()));
            }

            // basic exception safety
            bool reserve(size_type n)
            {
                bool need_to_reserve = n >= max_load_;
                // throws - basic:
                if (need_to_reserve) rehash_impl(min_buckets_for_size(n));
                BOOST_ASSERT(n < max_load_ || n > max_size());
                return need_to_reserve;
            }

        public:

            // no throw
            float max_load_factor() const
            {
                return mlf_;
            }

            // no throw
            void max_load_factor(float z)
            {
                BOOST_ASSERT(z > 0);
                mlf_ = (std::max)(z, minimum_max_load_factor);
                calculate_max_load();
            }

            // no throw
            float load_factor() const
            {
                BOOST_ASSERT(data_.bucket_manager_.bucket_count() != 0);
                return static_cast<float>(data_.size_)
                    / static_cast<float>(data_.bucket_manager_.bucket_count());
            }

            // key extractors

            // no throw
            static key_type const& extract_key(value_type const& v)
            {
                return extract(v, (type_wrapper<value_type>*)0);
            }

            static key_type const& extract(value_type const& v,
                    type_wrapper<key_type>*)
            {
                return v;
            }

            static key_type const& extract(value_type const& v,
                    void*)
            {
                return v.first;
            }

#if defined(BOOST_HAS_RVALUE_REFS) && defined(BOOST_HAS_VARIADIC_TMPL)
            struct no_key {};

            template <typename Arg1, typename... Args>
            static typename boost::enable_if<
                boost::mpl::and_<
                    boost::mpl::not_<boost::is_same<key_type, value_type> >,
                    boost::is_same<Arg1, key_type>
                >,
                key_type>::type const& extract_key(Arg1 const& k, Args const&...)
            {
                return k;
            }

            template <typename First, typename Second>
            static typename boost::enable_if<
                boost::mpl::and_<
                    boost::mpl::not_<boost::is_same<key_type, value_type> >,
                    boost::is_same<key_type,
                        typename boost::remove_const<
                            typename boost::remove_reference<First>::type
                        >::type>
                >,
                key_type>::type const& extract_key(std::pair<First, Second> const& v)
            {
                return v.first;
            }

            template <typename... Args>
            static no_key extract_key(Args const&...)
            {
                return no_key();
            }
#endif

        public:

            // if hash function throws, basic exception safety
            // strong otherwise.
            void rehash(size_type n)
            {
                using namespace std;

                // no throw:
                size_type min_size = min_buckets_for_size(size());
                // basic/strong:
                rehash_impl(min_size > n ? min_size : n);

                BOOST_ASSERT((float) bucket_count() > (float) size() / max_load_factor()
                        && bucket_count() >= n);
            }

        private:

            // if hash function throws, basic exception safety
            // strong otherwise
            void rehash_impl(size_type n)
            {
                n = next_prime(n); // no throw

                if (n == bucket_count())  // no throw
                    return;

                data new_buckets(data_, n); // throws, seperate
                move_buckets(data_, new_buckets, hash_function());
                                                        // basic/no throw
                new_buckets.swap(data_);                // no throw
                calculate_max_load();                   // no throw
            }

            // move_buckets & copy_buckets
            //
            // if the hash function throws, basic excpetion safety
            // no throw otherwise

            static void move_buckets(data& src, data& dst, hasher const& hf)
            {
                BOOST_ASSERT(dst.size_ == 0);
                //BOOST_ASSERT(src.allocators_.node_alloc_ == dst.allocators_.node_alloc_);

                bucket_ptr end = src.buckets_end();

                for(; src.cached_begin_bucket_ != end;
                        ++src.cached_begin_bucket_) {
                    bucket_ptr src_bucket = src.cached_begin_bucket_;
                    while(src_bucket->next_) {
                        // Move the first group of equivalent nodes in
                        // src_bucket to dst.

                        // This next line throws iff the hash function throws.
                        bucket_ptr dst_bucket = dst.bucket_ptr_from_hash(
                                hf(extract_key(data::get_value(src_bucket->next_))));

                        link_ptr n = src_bucket->next_;
                        size_type count = src.unlink_group(&src_bucket->next_);
                        dst.link_group(n, dst_bucket, count);
                    }
                }
            }

            // basic excpetion safety. If an exception is thrown this will
            // leave dst partially filled.

            static void copy_buckets(data const& src, data& dst, functions const& f)
            {
                BOOST_ASSERT(dst.size_ == 0);
                // no throw:
                bucket_ptr end = src.buckets_end();
                hasher const& hf = f.hash_function();

                // no throw:
                for(bucket_ptr i = src.cached_begin_bucket_; i != end; ++i) {
                    // no throw:
                    for(link_ptr it = src.begin(i);
                            BOOST_UNORDERED_BORLAND_BOOL(it); it = data::next_group(it)) {
                        // hash function can throw.
                        bucket_ptr dst_bucket = dst.bucket_ptr_from_hash(
                                hf(extract_key(data::get_value(it))));
                        // throws, strong
                        dst.copy_group(it, dst_bucket);
                    }
                }
            }

        public:

            // Insert functions
            //
            // basic exception safety, if hash function throws
            // strong otherwise.

#if BOOST_UNORDERED_EQUIVALENT_KEYS

#if !(defined(BOOST_HAS_RVALUE_REFS) && defined(BOOST_HAS_VARIADIC_TMPL))
            // Insert (equivalent key containers)

            // if hash function throws, basic exception safety
            // strong otherwise
            iterator_base insert(value_type const& v)
            {
                // Create the node before rehashing in case it throws an
                // exception (need strong safety in such a case).
                node_constructor a(data_.allocators_);
                a.construct(v);

                return insert_impl(a);
            }

            // Insert (equivalent key containers)

            // if hash function throws, basic exception safety
            // strong otherwise
            iterator_base insert_hint(iterator_base const& it, value_type const& v)
            {
                // Create the node before rehashing in case it throws an
                // exception (need strong safety in such a case).
                node_constructor a(data_.allocators_);
                a.construct(v);

                return insert_hint_impl(it, a);
            }

#else

            // Insert (equivalent key containers)
            // (I'm using an overloaded insert for both 'insert' and 'emplace')

            // if hash function throws, basic exception safety
            // strong otherwise
            template <class... Args>
            iterator_base insert(Args&&... args)
            {
                // Create the node before rehashing in case it throws an
                // exception (need strong safety in such a case).
                node_constructor a(data_.allocators_);
                a.construct(std::forward<Args>(args)...);

                return insert_impl(a);
            }

            // Insert (equivalent key containers)
            // (I'm using an overloaded insert for both 'insert' and 'emplace')

            // if hash function throws, basic exception safety
            // strong otherwise
            template <class... Args>
            iterator_base insert_hint(iterator_base const& it, Args&&... args)
            {
                // Create the node before rehashing in case it throws an
                // exception (need strong safety in such a case).
                node_constructor a(data_.allocators_);
                a.construct(std::forward<Args>(args)...);

                return insert_hint_impl(it, a);
            }

#endif

            iterator_base insert_impl(node_constructor& a)
            {
                key_type const& k = extract_key(a.get()->value_);
                size_type hash_value = hash_function()(k);
                bucket_ptr bucket = data_.bucket_ptr_from_hash(hash_value);
                link_ptr position = find_iterator(bucket, k);

                // reserve has basic exception safety if the hash function
                // throws, strong otherwise.
                if(reserve(size() + 1))
                    bucket = data_.bucket_ptr_from_hash(hash_value);

                // I'm relying on link_ptr not being invalidated by
                // the rehash here.
                return iterator_base(bucket,
                    (BOOST_UNORDERED_BORLAND_BOOL(position)) ?
                    data_.link_node(a, position) :
                    data_.link_node_in_bucket(a, bucket)
                );
            }

            iterator_base insert_hint_impl(iterator_base const& it, node_constructor& a)
            {
                // equal can throw, but with no effects
                if (it == data_.end() || !equal(extract_key(a.get()->value_), *it)) {
                    // Use the standard insert if the iterator doesn't point
                    // to a matching key.
                    return insert_impl(a);
                }
                else {
                    // Find the first node in the group - so that the node
                    // will be inserted at the end of the group.

                    link_ptr start(it.node_);
                    while(data_.prev_in_group(start)->next_ == start)
                        start = data_.prev_in_group(start);

                    // reserve has basic exception safety if the hash function
                    // throws, strong otherwise.
                    bucket_ptr base = reserve(size() + 1) ?
                        get_bucket(extract_key(a.get()->value_)) : it.bucket_;

                    // Nothing after this point can throw

                    return iterator_base(base,
                            data_.link_node(a, start));
                }
            }

            // Insert from iterator range (equivalent key containers)

        private:

            // if hash function throws, or inserting > 1 element, basic exception safety
            // strong otherwise
            template <typename I>
            void insert_for_range(I i, I j, forward_traversal_tag)
            {
                size_type distance = unordered_detail::distance(i, j);
                if(distance == 1) {
                    insert(*i);
                }
                else {
                    // Only require basic exception safety here
                    reserve(size() + distance);
                    node_constructor a(data_.allocators_);

                    for (; i != j; ++i) {
                        a.construct(*i);

                        key_type const& k = extract_key(a.get()->value_);
                        bucket_ptr bucket = get_bucket(k);
                        link_ptr position = find_iterator(bucket, k);

                        if(BOOST_UNORDERED_BORLAND_BOOL(position))
                            data_.link_node(a, position);
                        else
                            data_.link_node_in_bucket(a, bucket);
                    }
                }
            }

            // if hash function throws, or inserting > 1 element, basic exception safety
            // strong otherwise
            template <typename I>
            void insert_for_range(I i, I j,
                    boost::incrementable_traversal_tag)
            {
                // If only inserting 1 element, get the required
                // safety since insert is only called once.
                for (; i != j; ++i) insert(*i);
            }

        public:

            // if hash function throws, or inserting > 1 element, basic exception safety
            // strong otherwise
            template <typename I>
            void insert_range(I i, I j)
            {
                BOOST_DEDUCED_TYPENAME boost::iterator_traversal<I>::type
                    iterator_traversal_tag;
                insert_for_range(i, j, iterator_traversal_tag);
            }
#else
            // if hash function throws, basic exception safety
            // strong otherwise
            value_type& operator[](key_type const& k)
            {
                BOOST_STATIC_ASSERT((
                            !boost::is_same<value_type, key_type>::value));
                typedef BOOST_DEDUCED_TYPENAME value_type::second_type mapped_type;

                size_type hash_value = hash_function()(k);
                bucket_ptr bucket = data_.bucket_ptr_from_hash(hash_value);
                link_ptr pos = find_iterator(bucket, k);

                if (BOOST_UNORDERED_BORLAND_BOOL(pos))
                    return data::get_value(pos);
                else
                {
                    // Side effects only in this block.

                    // Create the node before rehashing in case it throws an
                    // exception (need strong safety in such a case).
                    node_constructor a(data_.allocators_);
                    a.construct(value_type(k, mapped_type()));

                    // reserve has basic exception safety if the hash function
                    // throws, strong otherwise.
                    if(reserve(size() + 1))
                        bucket = data_.bucket_ptr_from_hash(hash_value);

                    // Nothing after this point can throw.

                    return data::get_value(data_.link_node_in_bucket(a, bucket));
                }
            }

#if !(defined(BOOST_HAS_RVALUE_REFS) && defined(BOOST_HAS_VARIADIC_TMPL))

            // Insert (unique keys)

            // if hash function throws, basic exception safety
            // strong otherwise
            std::pair<iterator_base, bool> insert(value_type const& v)
            {
                // No side effects in this initial code
                key_type const& k = extract_key(v);
                size_type hash_value = hash_function()(k);
                bucket_ptr bucket = data_.bucket_ptr_from_hash(hash_value);
                link_ptr pos = find_iterator(bucket, k);

                if (BOOST_UNORDERED_BORLAND_BOOL(pos)) {
                    // Found an existing key, return it (no throw).
                    return std::pair<iterator_base, bool>(
                        iterator_base(bucket, pos), false);

                } else {
                    // Doesn't already exist, add to bucket.
                    // Side effects only in this block.

                    // Create the node before rehashing in case it throws an
                    // exception (need strong safety in such a case).
                    node_constructor a(data_.allocators_);
                    a.construct(v);

                    // reserve has basic exception safety if the hash function
                    // throws, strong otherwise.
                    if(reserve(size() + 1))
                        bucket = data_.bucket_ptr_from_hash(hash_value);

                    // Nothing after this point can throw.

                    link_ptr n = data_.link_node_in_bucket(a, bucket);

                    return std::pair<iterator_base, bool>(
                        iterator_base(bucket, n), true);
                }
            }

            // Insert (unique keys)

            // if hash function throws, basic exception safety
            // strong otherwise
            iterator_base insert_hint(iterator_base const& it, value_type const& v)
            {
                if(it != data_.end() && equal(extract_key(v), *it))
                    return it;
                else
                    return insert(v).first;
            }

#else

            // Insert (unique keys)
            // (I'm using an overloaded insert for both 'insert' and 'emplace')
            //
            // TODO:
            // For sets: create a local key without creating the node?
            // For maps: use the first argument as the key.

            // if hash function throws, basic exception safety
            // strong otherwise
            template<typename... Args>
            std::pair<iterator_base, bool> insert(Args&&... args)
            {
                return insert_impl(
                    extract_key(std::forward<Args>(args)...),
                    std::forward<Args>(args)...);
            }

            template<typename... Args>
            std::pair<iterator_base, bool> insert_impl(key_type const& k, Args&&... args)
            {
                // No side effects in this initial code
                size_type hash_value = hash_function()(k);
                bucket_ptr bucket = data_.bucket_ptr_from_hash(hash_value);
                link_ptr pos = find_iterator(bucket, k);

                if (BOOST_UNORDERED_BORLAND_BOOL(pos)) {
                    // Found an existing key, return it (no throw).
                    return std::pair<iterator_base, bool>(
                        iterator_base(bucket, pos), false);

                } else {
                    // Doesn't already exist, add to bucket.
                    // Side effects only in this block.

                    // Create the node before rehashing in case it throws an
                    // exception (need strong safety in such a case).
                    node_constructor a(data_.allocators_);
                    a.construct(std::forward<Args>(args)...);

                    // reserve has basic exception safety if the hash function
                    // throws, strong otherwise.
                    if(reserve(size() + 1))
                        bucket = data_.bucket_ptr_from_hash(hash_value);

                    // Nothing after this point can throw.

                    return std::pair<iterator_base, bool>(iterator_base(bucket,
                        data_.link_node_in_bucket(a, bucket)), true);
                }
            }

            template<typename... Args>
            std::pair<iterator_base, bool> insert_impl(no_key, Args&&... args)
            {
                // Construct the node regardless - in order to get the key.
                // It will be discarded if it isn't used
                node_constructor a(data_.allocators_);
                a.construct(std::forward<Args>(args)...);

                // No side effects in this initial code
                key_type const& k = extract_key(a.get()->value_);
                size_type hash_value = hash_function()(k);
                bucket_ptr bucket = data_.bucket_ptr_from_hash(hash_value);
                link_ptr pos = find_iterator(bucket, k);
                
                if (BOOST_UNORDERED_BORLAND_BOOL(pos)) {
                    // Found an existing key, return it (no throw).
                    return std::pair<iterator_base, bool>(
                        iterator_base(bucket, pos), false);
                } else {
                    // reserve has basic exception safety if the hash function
                    // throws, strong otherwise.
                    if(reserve(size() + 1))
                        bucket = data_.bucket_ptr_from_hash(hash_value);

                    // Nothing after this point can throw.

                    return std::pair<iterator_base, bool>(iterator_base(bucket,
                        data_.link_node_in_bucket(a, bucket)), true);
                }
            }

            // Insert (unique keys)
            // (I'm using an overloaded insert for both 'insert' and 'emplace')

            // if hash function throws, basic exception safety
            // strong otherwise
            template<typename... Args>
            iterator_base insert_hint(iterator_base const&, Args&&... args)
            {
                // Life is complicated - just call the normal implementation.
                return insert(std::forward<Args>(args)...).first;
            }
#endif

            // Insert from iterators (unique keys)

            template <typename I>
            size_type insert_size(I i, I j, boost::forward_traversal_tag)
            {
                return unordered_detail::distance(i, j);
            }

            template <typename I>
            size_type insert_size(I, I, boost::incrementable_traversal_tag)
            {
                return 1;
            }

            template <typename I>
            size_type insert_size(I i, I j)
            {
                BOOST_DEDUCED_TYPENAME boost::iterator_traversal<I>::type
                    iterator_traversal_tag;
                return insert_size(i, j, iterator_traversal_tag);
            }

            // if hash function throws, or inserting > 1 element, basic exception safety
            // strong otherwise
            template <typename InputIterator>
            void insert_range(InputIterator i, InputIterator j)
            {
                node_constructor a(data_.allocators_);

                for (; i != j; ++i) {
                    // No side effects in this initial code
                    size_type hash_value = hash_function()(extract_key(*i));
                    bucket_ptr bucket = data_.bucket_ptr_from_hash(hash_value);
                    link_ptr pos = find_iterator(bucket, extract_key(*i));

                    if (!BOOST_UNORDERED_BORLAND_BOOL(pos)) {
                        // Doesn't already exist, add to bucket.
                        // Side effects only in this block.

                        // Create the node before rehashing in case it throws an
                        // exception (need strong safety in such a case).
                        a.construct(*i);

                        // reserve has basic exception safety if the hash function
                        // throws, strong otherwise.
                        if(size() + 1 >= max_load_) {
                            reserve(size() + insert_size(i, j));
                            bucket = data_.bucket_ptr_from_hash(hash_value);
                        }

                        // Nothing after this point can throw.
                        data_.link_node_in_bucket(a, bucket);
                    }
                }
            }
#endif
        public:

            // erase_key

            // strong exception safety
            size_type erase_key(key_type const& k)
            {
                // No side effects in initial section
                bucket_ptr bucket = get_bucket(k);
                link_ptr* it = find_for_erase(bucket, k);

                // No throw.
                return *it ? data_.erase_group(it, bucket) : 0;
            }

            // count
            //
            // strong exception safety, no side effects
            size_type count(key_type const& k) const
            {
                link_ptr it = find_iterator(k); // throws, strong
                return BOOST_UNORDERED_BORLAND_BOOL(it) ? data::group_count(it) : 0;
            }

            // find
            //
            // strong exception safety, no side effects
            iterator_base find(key_type const& k) const
            {
                bucket_ptr bucket = get_bucket(k);
                link_ptr it = find_iterator(bucket, k);

                if (BOOST_UNORDERED_BORLAND_BOOL(it))
                    return iterator_base(bucket, it);
                else
                    return data_.end();
            }

            value_type& at(key_type const& k) const
            {
                bucket_ptr bucket = get_bucket(k);
                link_ptr it = find_iterator(bucket, k);

                if (BOOST_UNORDERED_BORLAND_BOOL(it))
                    return data::get_value(it);
                else
                    throw std::out_of_range("Unable to find key in unordered_map.");
            }

            // equal_range
            //
            // strong exception safety, no side effects
            std::pair<iterator_base, iterator_base> equal_range(key_type const& k) const
            {
                bucket_ptr bucket = get_bucket(k);
                link_ptr it = find_iterator(bucket, k);
                if (BOOST_UNORDERED_BORLAND_BOOL(it)) {
                    iterator_base first(iterator_base(bucket, it));
                    iterator_base second(first);
                    second.increment_group();
                    return std::pair<iterator_base, iterator_base>(first, second);
                }
                else {
                    return std::pair<iterator_base, iterator_base>(
                            data_.end(), data_.end());
                }
            }

            // strong exception safety, no side effects
            bool equal(key_type const& k, value_type const& v) const
            {
                return key_eq()(k, extract_key(v));
            }

            // strong exception safety, no side effects
            link_ptr find_iterator(key_type const& k) const
            {
                return find_iterator(get_bucket(k), k);
            }

            // strong exception safety, no side effects
            link_ptr find_iterator(bucket_ptr bucket,
                    key_type const& k) const
            {
                link_ptr it = data_.begin(bucket);
                while (BOOST_UNORDERED_BORLAND_BOOL(it) && !equal(k, data::get_value(it)))
                    it = data::next_group(it);

                return it;
            }

            // strong exception safety, no side effects
            link_ptr* find_for_erase(bucket_ptr bucket, key_type const& k) const
            {
                link_ptr* it = &bucket->next_;
                while(BOOST_UNORDERED_BORLAND_BOOL(*it) && !equal(k, data::get_value(*it)))
                    it = &data::next_group(*it);

                return it;
            }
        };

        //
        // Equals - unordered container equality comparison.
        //

#if BOOST_UNORDERED_EQUIVALENT_KEYS
        template <typename A, typename KeyType>
        inline bool group_equals(
                BOOST_UNORDERED_TABLE_DATA<A>*,
                typename BOOST_UNORDERED_TABLE_DATA<A>::link_ptr it1,
                typename BOOST_UNORDERED_TABLE_DATA<A>::link_ptr it2,
                KeyType*,
                type_wrapper<KeyType>*)
        {
            typedef BOOST_UNORDERED_TABLE_DATA<A> data;
            return data::group_count(it1) == data::group_count(it2);
        }

        template <typename A, typename KeyType>
        inline bool group_equals(
                BOOST_UNORDERED_TABLE_DATA<A>*,
                typename BOOST_UNORDERED_TABLE_DATA<A>::link_ptr it1,
                typename BOOST_UNORDERED_TABLE_DATA<A>::link_ptr it2,
                KeyType*,
                void*)
        {
            typedef BOOST_UNORDERED_TABLE_DATA<A> data;
            typename BOOST_UNORDERED_TABLE_DATA<A>::link_ptr end1 = data::next_group(it1);
            typename BOOST_UNORDERED_TABLE_DATA<A>::link_ptr end2 = data::next_group(it2);

            do {
                if(data::get_value(it1).second != data::get_value(it2).second) return false;
                it1 = it1->next_;
                it2 = it2->next_;
            } while(it1 != end1 && it2 != end2);
            return it1 == end1 && it2 == end2;
        }
#else
        template <typename A, typename KeyType>
        inline bool group_equals(
                BOOST_UNORDERED_TABLE_DATA<A>*,
                typename BOOST_UNORDERED_TABLE_DATA<A>::link_ptr,
                typename BOOST_UNORDERED_TABLE_DATA<A>::link_ptr,
                KeyType*,
                type_wrapper<KeyType>*)
        {
            return true;
        }

        template <typename A, typename KeyType>
        inline bool group_equals(
                BOOST_UNORDERED_TABLE_DATA<A>*,
                typename BOOST_UNORDERED_TABLE_DATA<A>::link_ptr it1,
                typename BOOST_UNORDERED_TABLE_DATA<A>::link_ptr it2,
                KeyType*,
                void*)
        {
            typedef BOOST_UNORDERED_TABLE_DATA<A> data;
            return data::get_value(it1).second == data::get_value(it2).second;
        }
#endif

        template <typename V, typename K, typename H, typename P, typename A>
        bool equals(BOOST_UNORDERED_TABLE<V, K, H, P, A> const& t1,
                BOOST_UNORDERED_TABLE<V, K, H, P, A> const& t2)
        {
            typedef BOOST_UNORDERED_TABLE_DATA<A> data;
            typedef typename data::bucket_ptr bucket_ptr;
            typedef typename data::link_ptr link_ptr;

            if(t1.size() != t2.size()) return false;

            for(bucket_ptr i = t1.data_.cached_begin_bucket_,
                    j = t1.data_.buckets_end(); i != j; ++i)
            {
                for(link_ptr it(i->next_); BOOST_UNORDERED_BORLAND_BOOL(it); it = data::next_group(it))
                {
                    link_ptr other_pos = t2.find_iterator(t2.extract_key(data::get_value(it)));
                    if(!BOOST_UNORDERED_BORLAND_BOOL(other_pos) ||
                        !group_equals((data*)0, it, other_pos, (K*)0, (type_wrapper<V>*)0))
                        return false;
                }
            }

            return true;
        }

        // Iterators

        template <typename Alloc> class BOOST_UNORDERED_ITERATOR;
        template <typename Alloc> class BOOST_UNORDERED_CONST_ITERATOR;
        template <typename Alloc> class BOOST_UNORDERED_LOCAL_ITERATOR;
        template <typename Alloc> class BOOST_UNORDERED_CONST_LOCAL_ITERATOR;
        class iterator_access;

        // Local Iterators
        //
        // all no throw

        template <typename Alloc>
        class BOOST_UNORDERED_LOCAL_ITERATOR
            : public boost::iterator <
                std::forward_iterator_tag,
                BOOST_DEDUCED_TYPENAME allocator_value_type<Alloc>::type,
                std::ptrdiff_t,
                BOOST_DEDUCED_TYPENAME allocator_pointer<Alloc>::type,
                BOOST_DEDUCED_TYPENAME allocator_reference<Alloc>::type >
        {
        public:
            typedef BOOST_DEDUCED_TYPENAME allocator_value_type<Alloc>::type value_type;

        private:
            typedef BOOST_UNORDERED_TABLE_DATA<Alloc> data;
            typedef BOOST_DEDUCED_TYPENAME data::link_ptr ptr;
            typedef BOOST_UNORDERED_CONST_LOCAL_ITERATOR<Alloc> const_local_iterator;

            friend class BOOST_UNORDERED_CONST_LOCAL_ITERATOR<Alloc>;
            ptr ptr_;

        public:
            BOOST_UNORDERED_LOCAL_ITERATOR() : ptr_() {
                BOOST_UNORDERED_MSVC_RESET_PTR(ptr_);
            }
            explicit BOOST_UNORDERED_LOCAL_ITERATOR(ptr x) : ptr_(x) {}
            BOOST_DEDUCED_TYPENAME allocator_reference<Alloc>::type operator*() const
                { return data::get_value(ptr_); }
            value_type* operator->() const { return &data::get_value(ptr_); }
            BOOST_UNORDERED_LOCAL_ITERATOR& operator++() { ptr_ = ptr_->next_; return *this; }
            BOOST_UNORDERED_LOCAL_ITERATOR operator++(int) { BOOST_UNORDERED_LOCAL_ITERATOR tmp(ptr_); ptr_ = ptr_->next_; return tmp; }
            bool operator==(BOOST_UNORDERED_LOCAL_ITERATOR x) const { return ptr_ == x.ptr_; }
            bool operator==(const_local_iterator x) const { return ptr_ == x.ptr_; }
            bool operator!=(BOOST_UNORDERED_LOCAL_ITERATOR x) const { return ptr_ != x.ptr_; }
            bool operator!=(const_local_iterator x) const { return ptr_ != x.ptr_; }
        };

        template <typename Alloc>
        class BOOST_UNORDERED_CONST_LOCAL_ITERATOR
            : public boost::iterator <
                std::forward_iterator_tag,
                BOOST_DEDUCED_TYPENAME allocator_value_type<Alloc>::type,
                std::ptrdiff_t,
                BOOST_DEDUCED_TYPENAME allocator_const_pointer<Alloc>::type,
                BOOST_DEDUCED_TYPENAME allocator_const_reference<Alloc>::type >
        {
        public:
            typedef BOOST_DEDUCED_TYPENAME allocator_value_type<Alloc>::type value_type;

        private:
            typedef BOOST_UNORDERED_TABLE_DATA<Alloc> data;
            typedef BOOST_DEDUCED_TYPENAME data::link_ptr ptr;
            typedef BOOST_UNORDERED_LOCAL_ITERATOR<Alloc> local_iterator;
            friend class BOOST_UNORDERED_LOCAL_ITERATOR<Alloc>;
            ptr ptr_;

        public:
            BOOST_UNORDERED_CONST_LOCAL_ITERATOR() : ptr_() {
                BOOST_UNORDERED_MSVC_RESET_PTR(ptr_);
            }
            explicit BOOST_UNORDERED_CONST_LOCAL_ITERATOR(ptr x) : ptr_(x) {}
            BOOST_UNORDERED_CONST_LOCAL_ITERATOR(local_iterator x) : ptr_(x.ptr_) {}
            BOOST_DEDUCED_TYPENAME allocator_const_reference<Alloc>::type
                operator*() const { return data::get_value(ptr_); }
            value_type const* operator->() const { return &data::get_value(ptr_); }
            BOOST_UNORDERED_CONST_LOCAL_ITERATOR& operator++() { ptr_ = ptr_->next_; return *this; }
            BOOST_UNORDERED_CONST_LOCAL_ITERATOR operator++(int) { BOOST_UNORDERED_CONST_LOCAL_ITERATOR tmp(ptr_); ptr_ = ptr_->next_; return tmp; }
            bool operator==(local_iterator x) const { return ptr_ == x.ptr_; }
            bool operator==(BOOST_UNORDERED_CONST_LOCAL_ITERATOR x) const { return ptr_ == x.ptr_; }
            bool operator!=(local_iterator x) const { return ptr_ != x.ptr_; }
            bool operator!=(BOOST_UNORDERED_CONST_LOCAL_ITERATOR x) const { return ptr_ != x.ptr_; }
        };

        // iterators
        //
        // all no throw


        template <typename Alloc>
        class BOOST_UNORDERED_ITERATOR
            : public boost::iterator <
                std::forward_iterator_tag,
                BOOST_DEDUCED_TYPENAME allocator_value_type<Alloc>::type,
                std::ptrdiff_t,
                BOOST_DEDUCED_TYPENAME allocator_pointer<Alloc>::type,
                BOOST_DEDUCED_TYPENAME allocator_reference<Alloc>::type >
        {
        public:
            typedef BOOST_DEDUCED_TYPENAME allocator_value_type<Alloc>::type value_type;

        private:
            typedef BOOST_DEDUCED_TYPENAME BOOST_UNORDERED_TABLE_DATA<Alloc>::iterator_base base;
            typedef BOOST_UNORDERED_CONST_ITERATOR<Alloc> const_iterator;
            friend class BOOST_UNORDERED_CONST_ITERATOR<Alloc>;
            base base_;

        public:

            BOOST_UNORDERED_ITERATOR() : base_() {}
            explicit BOOST_UNORDERED_ITERATOR(base const& x) : base_(x) {}
            BOOST_DEDUCED_TYPENAME allocator_reference<Alloc>::type
                operator*() const { return *base_; }
            value_type* operator->() const { return &*base_; }
            BOOST_UNORDERED_ITERATOR& operator++() { base_.increment(); return *this; }
            BOOST_UNORDERED_ITERATOR operator++(int) { BOOST_UNORDERED_ITERATOR tmp(base_); base_.increment(); return tmp; }
            bool operator==(BOOST_UNORDERED_ITERATOR const& x) const { return base_ == x.base_; }
            bool operator==(const_iterator const& x) const { return base_ == x.base_; }
            bool operator!=(BOOST_UNORDERED_ITERATOR const& x) const { return base_ != x.base_; }
            bool operator!=(const_iterator const& x) const { return base_ != x.base_; }
        };

        template <typename Alloc>
        class BOOST_UNORDERED_CONST_ITERATOR
            : public boost::iterator <
                std::forward_iterator_tag,
                BOOST_DEDUCED_TYPENAME allocator_value_type<Alloc>::type,
                std::ptrdiff_t,
                BOOST_DEDUCED_TYPENAME allocator_const_pointer<Alloc>::type,
                BOOST_DEDUCED_TYPENAME allocator_const_reference<Alloc>::type >
        {
        public:
            typedef BOOST_DEDUCED_TYPENAME allocator_value_type<Alloc>::type value_type;

        private:
            typedef BOOST_DEDUCED_TYPENAME BOOST_UNORDERED_TABLE_DATA<Alloc>::iterator_base base;
            typedef BOOST_UNORDERED_ITERATOR<Alloc> iterator;
            friend class BOOST_UNORDERED_ITERATOR<Alloc>;
            friend class iterator_access;
            base base_;

        public:

            BOOST_UNORDERED_CONST_ITERATOR() : base_() {}
            explicit BOOST_UNORDERED_CONST_ITERATOR(base const& x) : base_(x) {}
            BOOST_UNORDERED_CONST_ITERATOR(iterator const& x) : base_(x.base_) {}
            BOOST_DEDUCED_TYPENAME allocator_const_reference<Alloc>::type
                operator*() const { return *base_; }
            value_type const* operator->() const { return &*base_; }
            BOOST_UNORDERED_CONST_ITERATOR& operator++() { base_.increment(); return *this; }
            BOOST_UNORDERED_CONST_ITERATOR operator++(int) { BOOST_UNORDERED_CONST_ITERATOR tmp(base_); base_.increment(); return tmp; }
            bool operator==(iterator const& x) const { return base_ == x.base_; }
            bool operator==(BOOST_UNORDERED_CONST_ITERATOR const& x) const { return base_ == x.base_; }
            bool operator!=(iterator const& x) const { return base_ != x.base_; }
            bool operator!=(BOOST_UNORDERED_CONST_ITERATOR const& x) const { return base_ != x.base_; }
        };
    }
}

#undef BOOST_UNORDERED_TABLE
#undef BOOST_UNORDERED_TABLE_DATA
#undef BOOST_UNORDERED_ITERATOR
#undef BOOST_UNORDERED_CONST_ITERATOR
#undef BOOST_UNORDERED_LOCAL_ITERATOR
#undef BOOST_UNORDERED_CONST_LOCAL_ITERATOR
