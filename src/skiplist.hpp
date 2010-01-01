/*
   Copyright (C) 2009 - 2010 by Chris Hopman <cjhopman@gmail.com>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License version 2
   or at your option any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

#ifndef SKIPLIST_HPP_INCLUDED
#define SKIPLIST_HPP_INCLUDED

#include <boost/pool/detail/singleton.hpp>
#include <boost/random/linear_congruential.hpp>
#include <boost/type_traits/is_pod.hpp>
#include <cassert>
#include <algorithm>

#include "ct_math.hpp"

#ifndef SKIPLIST_MAX_HEIGHT
#define SKIPLIST_MAX_HEIGHT 15
#endif

#include <set>

template <typename Compare, typename Key, bool IsPod = boost::is_pod<Compare>::value>
struct base_compare : public Compare {
	base_compare(const Compare& o) : Compare(o) { }
};

template <typename Compare, typename Key>
struct base_compare<Compare, Key, true> {
	Compare comp;
	base_compare() : comp() { }
	base_compare(const base_compare& o) : comp(o.comp) { }
	base_compare(const Compare& o) : comp(o) { }
	bool operator()(const Key& a, const Key& b) const { return comp(a, b); }
	operator Compare() const { return comp; }
};

template <typename T>
struct key_extractor {
	const typename T::first_type& operator()(const T& p) const { return p.first; }
};

template <typename T, typename Compare>
struct map_value_compare {
	Compare comp;
	explicit map_value_compare(const Compare& o) : comp(o) { }
	bool operator()(const T& a, const T& b) {
		return comp(a.first, b.first);
	}
};

/** Blocks of memory are laid out as
 * [ value_type | (alignment padding) | node* | (optional additional node*s) | (alignment padding)]
 * we generally pass around a pointer to one of the node*s and value_offset is the
 * offset from the first node* to the value_type. We calculate here how much padding between
 * value_type and the first node* is needed to ensure proper alignment
 **/
template <typename value_type, typename node>
struct value_offset {
	enum { value = -ct_lmgte<sizeof(value_type), strideof<node>::value>::value };
};

/** calculates the node size for a node of height H.
 *  we need to ensure that node size is a common multiple
 *  of the stride of both value_type and node* so that each
 *  request is aligned correctly. it must be at least large
 *  enough for the value_offset and H nodes.
 **/
template <typename value_type, typename node, size_t H>
struct node_size {
	enum {
		value =
			ct_lmgte<
				-value_offset<value_type, node>::value + H * sizeof(node),
				ct_lcm<
					strideof<node>::value,
					strideof<value_type>::value
				>::value
			>::value
	};
};

template <typename value_type>
struct node_base {
	node_base* next_;
	node_base(node_base* n) : next_(n) { }
	node_base* next() { return next_; }
	const node_base* next() const { return next_; }
	node_base* lower(int i = 1) { return this - i; }
	const node_base* lower(int i = 1) const { return this - i; }
	node_base* upper(int i = 1) { return this + i; }
	const node_base* upper(int i = 1) const { return this + i; }

	static void destruct_pointers(node_base* ptrs, size_t h) {
		for (size_t i = 0; i < h; ++i, ++ptrs)
			ptrs->~node_base();
	}

	static void construct_pointers(node_base* ptrs, size_t h) {
		for (size_t i = 0; i < h; i++, ++ptrs)
			new (ptrs) node_base(ptrs);
	}

	static void link(node_base* a, node_base* b) {
		std::swap(a->next_, b->next_);
	}

	static void unlink(node_base* a) {
		std::swap(a->next_, a->next_->next_);
	}

	const value_type& get_value(size_t level) const {
		return *static_cast<const value_type*>(advance(this, value_offset<value_type, node_base>::value - level * sizeof(node_base)));
	}

	value_type& get_value(size_t level) {
		return *static_cast<value_type*>(advance(this, value_offset<value_type, node_base>::value - level * sizeof(node_base)));
	}

	template <typename T>
	static void* advance(T* o, ptrdiff_t d) {
		return static_cast<char*>(static_cast<void*>(o)) + d;
	}

	template <typename T>
	static const void* advance(const T* o, ptrdiff_t d) {
		return static_cast<const char*>(static_cast<const void*>(o)) + d;
	}
};

template <typename Alloc, typename Value, size_t Height>
struct alloc_type_base : public alloc_type_base<Alloc, Value, Height - 1>,
	private Alloc::template rebind<char[node_size<Value, node_base<Value>, Height>::value]>::other,
	private Alloc::template rebind<node_base<Value>[Height]>::other
{
	typedef alloc_type_base<Alloc, Value, Height - 1> super;
	typedef typename Alloc::template rebind<char[node_size<Value, node_base<Value>, Height>::value]>::other node_alloc_type;
	typedef typename Alloc::template rebind<node_base<Value>[Height]>::other head_alloc_type;

	void* allocate_node(size_t h) {
		if (h == Height)
			return node_alloc_type::allocate(1);
		return super::allocate_node(h);
	}

	void deallocate_node(void* n, size_t h) {
		if (h == Height) node_alloc_type::deallocate(static_cast<typename node_alloc_type::pointer>(n), 1);
		else super::deallocate_node(n, h);
	}

	node_base<Value>* allocate_head(size_t h) {
		if (h == Height) return static_cast<node_base<Value>*>(static_cast<void*>(head_alloc_type::allocate(1)));
		return super::allocate_head(h);
	}

	void deallocate_head(node_base<Value>* ptr, size_t h) {
		if (h == Height) head_alloc_type::deallocate(static_cast<typename head_alloc_type::pointer>(static_cast<void*>(ptr)), 1);
		else super::deallocate_head(ptr, h);
	}
};

template <typename Alloc, typename Value>
struct alloc_type_base<Alloc, Value, 0> {
	void* allocate_node(size_t) {
		assert(0);
	}

	void deallocate_node(void*, size_t) {
		assert(0);
	}

	node_base<Value>* allocate_head(size_t) {
		assert(0);
	}

	void deallocate_head(void*, size_t) {
		assert(0);
	}
};

template <typename Value, typename Key, typename ExtractKey, typename Compare, typename Alloc>
class skiplist :
		private alloc_type_base<Alloc, Value, SKIPLIST_MAX_HEIGHT>,
		private base_compare<Compare, Key>,
		private ExtractKey
{
public:
	typedef Key key_type;
	typedef Value value_type;
	typedef value_type& reference;
	typedef const value_type& const_reference;
	typedef value_type* pointer;
	typedef const value_type* const_pointer;
	typedef Compare key_compare;
	typedef Alloc allocator_type;
private:
	typedef alloc_type_base<Alloc, Value, SKIPLIST_MAX_HEIGHT> base_allocator;
	typedef ExtractKey key_extractor;
	typedef node_base<Value> node;
	typedef base_compare<Compare, Key> compare_type;
public:
	struct const_iterator {
		typedef Value value_type;
		typedef const value_type& reference;
		typedef const value_type* pointer;
		typedef std::bidirectional_iterator_tag iterator_category;
		typedef int difference_type;

		explicit const_iterator(const skiplist* sl = 0, const node* o = 0) : list(sl), n(o) { }
		reference operator*() const { return n->get_value(0); }
		pointer operator->() const { return &operator*(); }
		const_iterator& operator++() { n = n->next(); return *this; }
		const_iterator operator++(int) { const_iterator ret(*this); operator++(); return ret; }
		const_iterator& operator--() { const node* ptrs[SKIPLIST_MAX_HEIGHT]; list->find_iter(ptrs, *this); n = ptrs[0]; return *this; }
		const_iterator operator--(int) { const_iterator ret(*this); operator--(); return ret; }
		bool operator==(const const_iterator& o) const { return n == o.n; }
		bool operator!=(const const_iterator& o) const { return !operator==(o); }

		friend class skiplist;
	private:
		const skiplist* list;
		const node* n;
	};

	struct iterator {
		typedef Value value_type;
		typedef value_type& reference;
		typedef value_type* pointer;
		typedef std::bidirectional_iterator_tag iterator_category;
		typedef int difference_type;

		explicit iterator(const skiplist* sl = 0, node* o = 0) : list(sl), n(o) { }
		operator const_iterator() { return const_iterator(list, n); }
		reference operator*() const { return n->get_value(0); }
		pointer operator->() const { return &operator*(); }
		iterator& operator++() { n = n->next(); return *this; }
		iterator operator++(int) { iterator ret(*this); operator++(); return ret; }
		iterator& operator--() { const node* ptrs[SKIPLIST_MAX_HEIGHT]; list->find_iter(ptrs, *this); n = const_cast<node*>(ptrs[0]); return *this; }
		iterator operator--(int) { iterator ret(*this); operator--(); return ret; }
		bool operator==(const iterator& o) const { return n == o.n; }
		bool operator!=(const iterator& o) const { return !operator==(o); }

		friend class skiplist;
	private:
		const skiplist* list;
		node* n;
	};

	typedef std::reverse_iterator<iterator> reverse_iterator;
	typedef std::reverse_iterator<const_iterator> const_reverse_iterator;

	skiplist(const skiplist& o) : compare_type(o), head(0), height(o.height) {
		construct_head(head, height);
		insert(o.begin(), o.end());
	}

	skiplist(const Compare& oc = Compare(), const Alloc& = Alloc()) : compare_type(oc), head(0), height(1) {
		construct_head(head, 1);
	}

	skiplist(const skiplist& o, const Compare& oc, const Alloc&) : compare_type(oc), head(0), height(o.height) {
		construct_head(head, height);
		insert(o.begin(), o.end());
	}

	template <typename InputIterator>
	skiplist(InputIterator first, InputIterator last, const Compare& oc, const Alloc&) : compare_type(oc), head(0), height(1) {
		construct_head(head, height);
		insert(first, last);
	}

	skiplist& operator=(const skiplist& o) {
		if (this == &o) return *this;
		skiplist t(o);
		swap(t);
		return *this;
	}

	~skiplist() {
		erase(begin(), end());
		destruct_head(head, height);
	}

	key_compare key_comp() const {
		return *this;
	}

	void clear() {
		skiplist t(key_comp(), get_allocator());
		swap(t);
	}

	bool empty() const {
		return head == head->next();
	}

	size_t max_size() const {
		return std::numeric_limits<size_t>::max();
	}

	size_t size() const {
		return std::distance(begin(), end());
	}

	void erase(const iterator& iter) {
		node* ptrs[SKIPLIST_MAX_HEIGHT];
		find_iter(const_cast<const node**>(ptrs), iter);
		assert(iter.n == ptrs[0]->next());
		size_t h = 1;
		while (h < height && ptrs[h]->next() == iter.n->upper(h)) h++;
		unlink_node_full(ptrs, h);
		destroy_node(iter.n, h);
	}

	size_t erase(iterator first, const iterator& last) {
		assert(this == first.list && this == last.list && (last.n == head || !compare(key_extract(*last), key_extract(*first))));
		size_t c = 0;
		node* ptrs[SKIPLIST_MAX_HEIGHT];
		find_iter(const_cast<const node**>(ptrs), first);
		while (ptrs[0]->next() != last.n) {
			size_t h = 0;
			node* n = ptrs[0]->next();
			while (h < height && ptrs[h]->next() == n->upper(h)) {
				h++;
			}

			unlink_node_full(ptrs, h);
			destroy_node(n, h);
			c++;
		}
		return c;
	}

	size_t erase(const key_type& k) {
		std::pair<iterator, iterator> range = equal_range(k);
		return erase(range.first, range.second);
	}

	iterator lower_bound(const key_type& k) {
		const node* ptrs[SKIPLIST_MAX_HEIGHT];
		find(ptrs, k, lower_compare(*this));
		return iterator(this, const_cast<node*>(ptrs[0])->next());
	}

	const_iterator lower_bound(const key_type& k) const {
		const node* ptrs[SKIPLIST_MAX_HEIGHT];
		find(ptrs, k, lower_compare(*this));
		return const_iterator(this, ptrs[0]->next());
	}

	iterator upper_bound(const key_type& k) {
		const node* ptrs[SKIPLIST_MAX_HEIGHT];
		find(ptrs, k, upper_compare(*this));
		return iterator(this, const_cast<node*>(ptrs[0])->next());
	}

	const_iterator upper_bound(const key_type& k) const {
		const node* ptrs[SKIPLIST_MAX_HEIGHT];
		find(ptrs, k, upper_compare(*this));
		return const_iterator(this, ptrs[0]->next());
	}

	std::pair<iterator, iterator> equal_range(const key_type& k) {
		return std::make_pair(lower_bound(k), upper_bound(k));
	}

	std::pair<const_iterator, const_iterator> equal_range(const key_type& k) const {
		return std::make_pair(lower_bound(k), upper_bound(k));
	};

	/** iterators do not contain enough information for a fast hinted insert **/
	iterator insert(iterator, const value_type& v) {
		return insert(v);
	}

	iterator insert(const value_type& v) {
		const node* ptrs[SKIPLIST_MAX_HEIGHT];
		size_t h = next_height();
		expand_head(h);
		node* n = construct_node(h, v);
		find(ptrs, key_extract(v), upper_compare(*this));
		link_node_full(const_cast<node**>(ptrs), n, h);
		return iterator(this, n);
	}

	template <typename InputIterator>
	void insert(InputIterator first, InputIterator last) {
		//TODO
		while (first != last) insert(*first++);
		return;
		const node* ptrs[SKIPLIST_MAX_HEIGHT];
		for (size_t i = 0; i < height; i++)
			ptrs[i] = head->upper(i);
		while (first != last) {
			size_t h = next_height(), prev_height = height;
			expand_head(h);
			for (size_t i = prev_height; i < height; i++)
				ptrs[i] = head->upper(i);
			hinted_find(ptrs, first->first, upper_compare(*this));
			node* n = construct_node(h, *first);
			link_node_full(const_cast<node**>(ptrs), n, h);
			for (size_t i = 0; i < h; i++)
				ptrs[i] = ptrs[i]->next();
			++first;
		}
	}

	std::pair<iterator, bool> insert_unique(iterator iter, const value_type& v) {
		return insert_unique(v);
	}

	std::pair<iterator, bool> insert_unique(const value_type& v) {
		const node* ptrs[SKIPLIST_MAX_HEIGHT];
		size_t h = next_height();
		expand_head(h);
		find(ptrs, key_extract(v), upper_compare(*this));
		if (ptrs[0] != head && !compare(key_extract(ptrs[0]->get_value(0)), key_extract(v)))
			return std::make_pair(iterator(this, const_cast<node*>(ptrs[0])), false);
		node* n = construct_node(h, v);
		link_node_full(const_cast<node**>(ptrs), n, h);
		return std::make_pair(iterator(this, n), true);
	}

	template <typename InputIterator>
	void insert_unique(InputIterator first, InputIterator last) {
		// TODO
		while (first != last) insert_unique(*(first++));
	}

	iterator find(const Key& k) {
		const node* ptrs[SKIPLIST_MAX_HEIGHT];
		find(ptrs, k, lower_compare(*this));
		if (ptrs[0]->next() != head && !compare(k, key_extract(ptrs[0]->next()->get_value(0))))
			return iterator(this, const_cast<node*>(ptrs[0])->next());
		return end();
	}

	const_iterator find(const Key& k) const {
		const node* ptrs[SKIPLIST_MAX_HEIGHT];
		find(ptrs, k, lower_compare(*this));
		if (ptrs[0]->next() != head && !compare(k, key_extract(ptrs[0]->next()->get_value(0))))
			return const_iterator(this, ptrs[0]->next());
		return end();
	}

	void swap(skiplist& o) {
		std::swap(head, o.head);
		std::swap(height, o.height);
	}

	allocator_type get_allocator() { return allocator_type(); }

	iterator begin() { return iterator(this, head->next()); }
	iterator end() { return iterator(this, head); }

	const_iterator begin() const { return const_iterator(this, head->next()); }
	const_iterator end() const { return const_iterator(this, head); }

	reverse_iterator rbegin() { return reverse_iterator(end()); }
	reverse_iterator rend() { return reverse_iterator(begin()); }

	const_reverse_iterator rbegin() const { return const_reverse_iterator(end()); }
	const_reverse_iterator rend() const { return const_reverse_iterator(begin()); }


private:
	node* head;
	size_t height;

	node* construct_node(size_t h) {
		return construct(h, Key(), Value());
	}

	node* construct_node(size_t h, const value_type& v) {
		void* mem = allocate_node(h);
		void* val_ptr = node::advance(mem, value_offset<value_type, node>::value);
		try {
			new (val_ptr) value_type (v);
		} catch (...) {
			free_node(mem, h);
		}
		node::construct_pointers(static_cast<node*>(mem), h);
		return static_cast<node*>(mem);
	}

	void destroy_node(node* n, size_t h) {
		n->get_value(0).~value_type();
		node::destruct_pointers(n, h);
		free_node(n, h);
	}


	void* allocate_node(size_t h) {
		return node::advance(base_allocator::allocate_node(h), -value_offset<Value, node>::value);
	}

	void free_node(void* n, size_t h) {
		base_allocator::deallocate_node(node::advance(n, value_offset<Value, node>::value), h);
	}

	const Key& key_extract(const value_type& v) const {
		return ExtractKey::operator()(v);
	}

	bool compare(const Key& a, const Key& b) const {
		return compare_type::operator()(a, b);
	}

	struct head_compare {
		bool operator()(const Key&, const node*) const { return true; }
	};

	void expand_head(size_t h) {
		if (h <= height) return;

		node* ptrs[SKIPLIST_MAX_HEIGHT];
		for (size_t i = 0; i < height; i++) {
			ptrs[i] = head->upper(i);
		}

		node* head_ptrs[SKIPLIST_MAX_HEIGHT];
		std::copy(ptrs, ptrs + SKIPLIST_MAX_HEIGHT, head_ptrs);

		find(const_cast<const node**>(ptrs), head, head_compare());

		node* mem;
		construct_head(mem, h);

		// non-throwing ops from here on
		std::swap(head, mem);
		link_node_full(head_ptrs, head, height);
		unlink_node_full(ptrs, height);
		destruct_head(mem, height);
		height = h;

	}

	static void unlink_node_full(node** ptrs, size_t h) {
		for (size_t i = 0; i < h; ++i, ++ptrs)
			node::unlink(*ptrs);
	}

	static void link_node_full(node** ptrs, node* node, size_t h) {
		for (size_t i = 0; i < h; ++i, ++ptrs, ++node)
			node::link(*ptrs, node);
	}

	template <typename T, typename Pred>
	void hinted_find(const node** const ptrs, const T& t, const Pred& pred = Pred()) const {
		if ((ptrs[0] == head || !pred(t, key_extract(ptrs[0]->get_value(0))))
				&& (ptrs[0]->next() == head || pred(t, key_extract(ptrs[0]->next()->get_value(0)))))
			return;
		find(ptrs, t, pred);
	}

	template <typename T, typename Pred>
	void find(const node** const ptrs, const T& t, const Pred& pred = Pred()) const {
		const node* node = head->upper(height - 1), * end = node, * next;
		for (int curr_level = height - 1; curr_level >= 0; curr_level--) {
			next = node->next();
			ptrs[curr_level] = node;
			while (next != end && pred(key_extract(next->get_value(curr_level)), t)) {
				ptrs[curr_level] = next;
				next = ptrs[curr_level]->next();
			}
			end = next->lower();
			node = ptrs[curr_level]->lower();
		}
	}

	void find_iter(const node** const ptrs, const iterator& iter) const {
		if (iter.n == head) find(ptrs, iter.n, head_compare());
		else find(ptrs, key_extract(*iter), lower_compare(*this));
		while(ptrs[0]->next() != iter.n) {
			ptrs[0] = ptrs[0]->next();
			for (size_t i = 1; i < height && ptrs[i]->next()->lower(i) == ptrs[0]; i++) ptrs[i] = ptrs[i]->next();
		}
	}

	void construct_head(node*& mem, size_t h) {
		mem = base_allocator::allocate_head(h);
		node::construct_pointers(mem, h);
	}

	void destruct_head(node* mem, size_t h) {
		node::destruct_pointers(mem, h);
		base_allocator::deallocate_head(mem, h);
	}


	size_t next_height() {
		boost::rand48::result_type rn = boost::details::pool::singleton_default<boost::rand48>::instance()();
		size_t l = (__builtin_ffs(rn) + 1) / 2 - 1;
		l = std::min(std::min<size_t>(SKIPLIST_MAX_HEIGHT - 1, height), l) + 1;
		return l;
	}

	struct upper_compare {
		const compare_type& comp;
		upper_compare(const compare_type& o) : comp(o) { }
		bool operator()(const Key& c, const Key& k) const {
			return !(comp(k, c));
		}
	};

	struct lower_compare {
		const compare_type& comp;
		lower_compare(const compare_type& o) : comp(o) { }
		bool operator()(const Key& c, const Key& k) const {
			return comp(c, k);
		}
	};

};

template <typename Value, typename Key, typename ExtractKey, typename Compare, typename Alloc>
bool operator==(const skiplist<Value, Key, ExtractKey, Compare, Alloc>& a, const skiplist<Value, Key, ExtractKey, Compare, Alloc>& b) {
	return a.size() == b.size() && std::equal(a.begin(), a.end(), b.begin());
}

template <typename Value, typename Key, typename ExtractKey, typename Compare, typename Alloc>
bool operator!=(const skiplist<Value, Key, ExtractKey, Compare, Alloc>& a, const skiplist<Value, Key, ExtractKey, Compare, Alloc>& b) {
	return !operator==(a, b);
}

#endif
