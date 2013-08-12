/*******************************************
    Yet Another map generator
    Implementation file.
********************************************/

#include "yamg_hexheap.hpp"

/**
*   Constructor
*/
yamg_hexheap::yamg_hexheap(size_t taille)
    : last_(0)
    , max_(taille - 2)
    , table_(new yamg_hex *[taille * sizeof(yamg_hex *)])
{
}

/**
*   Destructor
*/
yamg_hexheap::~yamg_hexheap()
{
    //dtor
    delete table_;
}

/**
    Add an hex to the heap
    Note this function sets the done flag of the hex. This not only prevent heap overflow, but helps in various cases: no hex can be inserted more than once.

    -> ptr to the hex to add
*/
void yamg_hexheap::add_hex(yamg_hex *h) {

	if((last_ >= max_) || (h->done)) // overflow shield
        return;

	int i = last_++;

	while(i > 0)
	{
		int j = i / 2; // searching father of this element
		if( table_[j]->key <= h->key)
			break; // insert is finished

		table_[i]   = table_[j];
		i   =  j;
	}
	table_[i] = h;
    h->done = true;
}

/**
    Pick (and extract) first element from the heap

    <- ptr on the element
*/
yamg_hex *yamg_hexheap::pick_hex() {
yamg_hex *h, *res;

	  if( table_ == NULL || last_ == 0 )
		  return NULL;

      res = table_[0];

	  h = table_[--last_];
	  int i = 0;
	  int j = 1;
	  int k;
	  while( j < last_ ) {
		  // select the correct son
		  k = j + 1;
		  if( (k < last_) && ( table_[j]->key > table_[k]->key) )
			  j = k;
		  if(h->key <= table_[j]->key) break;
		  table_[i] = table_[j];
		  i = j; j *= 2;
	  }
	  table_[i] = h;
      return res;
}

/**
    Update all keys values
    (of course, this don't invalidate sort)
    -> the value to add
*/
void yamg_hexheap::update_hexes(int val) {

    for(int i = 0; i < last_; i++)
        table_[i]->key += val;
}

/**
    Returns the first hex key value
    <- key member of the hex
*/
int yamg_hexheap::test_hex() {
    if(last_ > 0)
        return table_[0]->key;
    else
        return -1;
}

/**
    Clear all members flag and reset the heap itself
*/
void yamg_hexheap::clear_heap() {
    for(int i = 0; i < last_; i++)
        table_[i]->done = false;
    last_ = 0;
}
