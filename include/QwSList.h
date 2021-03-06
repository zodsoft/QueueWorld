/* 
    Queue World is copyright (c) 2014 Ross Bencina

    Permission is hereby granted, free of charge, to any person obtaining a copy
    of this software and associated documentation files (the "Software"), to deal
    in the Software without restriction, including without limitation the rights
    to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
    copies of the Software, and to permit persons to whom the Software is
    furnished to do so, subject to the following conditions:

    The above copyright notice and this permission notice shall be included in all
    copies or substantial portions of the Software.

    THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
    IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
    FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
    AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
    LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
    OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
    SOFTWARE.
*/
#ifndef INCLUDED_QWSLIST_H
#define INCLUDED_QWSLIST_H

#include <algorithm> // swap
#include <cassert>

#include "QwConfig.h"
#include "QwSingleLinkNodeInfo.h"

/*
    QwSList is a single-threaded singly linked list.

    Can be used as a LIFO stack (push and pop to front).

    The list is internally terminated with a 0 (NULL) next ptr.

    Constraints:
        - Don't call pop_front() on an empty list.

        - No back(), push_back()

    Properties:
        - All operation implementations are conditional free.

        - Many operations could be free functions (insert_after_node,
            remove_node_after, node_is_back, node is end, end).

        - O(1) swap contents of two lists

    Nodes must contain a links_ field that is an array of pointers to nodes.
    NEXT_LINK_INDEX specifies the element of this array that is used for the 
	next ptr.

    struct ExampleNodeType{
        ExampleNodeType *links_[2]; // links isn't required to be the first field
                                    // but it is required to be called links_[]
        enum { EXAMPLE_LINK_INDEX_1, EXAMPLE_LINK_INDEX_2 }

        // ... other fields
    }
    
    typedef IoSList<ExampleNodeType*, ExampleNodeType::EXAMPLE_LINK_INDEX_1> list_1_t;

    see forward_list for an interface reference:
    http://en.cppreference.com/w/cpp/container/forward_list
*/

/*
    Some desirable properties of endogenous linked list:

    - errors should be easily caught when you try to add the same 
        list/item to multiple containers
        (nodeinfo::check_node_is_unlinked handles this)

    - invariants involving membership should be easily caught:
        e.g. when adding the list to a freelist should be easy to 
        check that it has been removed from other lists

    - which pointer field(s) are used to link the list together 
        should be encoded in the container and in the list, not in 
        the individual calls (as it is in the BSD version)
*/

template<typename NodePtrT, int NEXT_LINK_INDEX>
class QwSList{
    typedef QwSingleLinkNodeInfo<NodePtrT,NEXT_LINK_INDEX> nodeinfo;

public:
    typedef typename nodeinfo::node_type node_type;
    typedef typename nodeinfo::node_ptr_type node_ptr_type;
    typedef typename nodeinfo::const_node_ptr_type const_node_ptr_type;

private:
    
    node_ptr_type front_; // aka head. first link in list

public:

    class iterator{
        node_ptr_type p_;
    public:
#ifdef QW_VALIDATE_NODE_LINKS
        iterator() : p_( 0 ) {}
#else
        iterator() {}
#endif

        explicit iterator( node_ptr_type p ) : p_( p ) {}
      
        iterator& operator++ ()     // prefix ++
        {
            p_ = nodeinfo::next_ptr(p_);
            return *this;
        }

        iterator operator++ (int)  // postfix ++
        {
            iterator result(*this);
            ++(*this);
            return result;
        }

        // it's a container of pointers so dereferencing the iterator gives a pointer
        node_ptr_type operator*() { return p_; }
        const node_ptr_type* operator->() { return &p_; }

        bool operator!=(const iterator& rhs) const { return rhs.p_ != p_; }
        bool operator==(const iterator& rhs) const { return rhs.p_ == p_; }
    };

    typedef iterator const_iterator;

    // TODO also provides const_iterator?

    QwSList() : front_( 0 ) {}
    explicit QwSList( node_ptr_type front ) // construct from raw head pointer
        : front_( front ) {}

    void clear() { 
#ifdef QW_VALIDATE_NODE_LINKS
        while( !empty() ) pop_front();
#else
        // this doesn't mark nodes as unlinked
        front_ = 0;
#endif
    }

    void swap( QwSList& other ) { std::swap( front_, other.front_ ); }
    // see also void swap( IoSList& a, IoSList &b );

    bool empty() const
    {
        return (front_ == 0);
    }

    bool size_is_1() const
    {
        return (front_ != 0 && nodeinfo::next_ptr(front_) == 0);
    }

    bool size_is_greater_than_1() const
    {
        return (front_ != 0 && nodeinfo::next_ptr(front_) != 0);
    }

    // front returns 0 (NULL) when list is empty
	node_ptr_type front() { return front_; }
    const_node_ptr_type front() const { return front_; }

    void push_front( node_ptr_type n )
    {
        nodeinfo::check_node_is_unlinked( n );

        nodeinfo::next_ptr(n) = front_; // this works even if front_ is 0 when the list is empty.
        front_ = n;
    }

    node_ptr_type pop_front()
    {
        assert( !empty() ); // this version of pop_front doesn't work on an empty list. 
							// caller should check is_empty() first.

        node_ptr_type result = front_;
        front_ = nodeinfo::next_ptr(front_);

        nodeinfo::clear_node_link_for_validation( result );
        return result;
    }

    void insert_after( node_ptr_type before, node_ptr_type n ) // insert n after node before
    {
        assert( before != 0 );
        assert( n != 0 );
        nodeinfo::check_node_is_unlinked( n );

        nodeinfo::next_ptr(n) = nodeinfo::next_ptr(before);
        nodeinfo::next_ptr(before) = n;
    }

    void insert_after( iterator before, node_ptr_type n ) // insert n after node before. 
                                                           // works even with before_begin() on an empty list.
    {
        insert_after( *before, n );
    }

    node_ptr_type remove_after( node_ptr_type before ) // returns the removed node
    {
        assert( nodeinfo::next_ptr(before) != 0 ); // can't remove an item after the last item
        
        node_ptr_type result = nodeinfo::next_ptr(before);
        nodeinfo::next_ptr(before) = nodeinfo::next_ptr(result);
       
        nodeinfo::clear_node_link_for_validation( result );
        return result;
    }

    void remove_after( iterator before )
    {
        remove_after( *before );
    }

    // erase_after returns an iterator to the item past the 
    // item that was erased or end() if it was the last item
    iterator erase_after( iterator before )
    {
        assert( before != end() );

        node_ptr_type before_node_ptr = *before;

        node_ptr_type erased_node_ptr = nodeinfo::next_ptr(before_node_ptr);
        nodeinfo::next_ptr(before_node_ptr) = nodeinfo::next_ptr(erased_node_ptr);

        nodeinfo::clear_node_link_for_validation( erased_node_ptr );
        return iterator( nodeinfo::next_ptr(before_node_ptr) );
    }

    // forward_list provides remove() and remove_if()

    iterator before_begin()
    { 
        // pretend our front_ field is actually the next link field in a node struct
        // offset backwards from front_ then cast to a node ptr and wrap in an iterator
        // this is probably not strictly portable but it allows us to insert at the beginning.
        return iterator( reinterpret_cast<node_ptr_type>(reinterpret_cast<char*>(&front_) - nodeinfo::offsetof_next_ptr()) );
    }

    iterator begin() const { return iterator(front_); }

    const iterator end() const { return iterator(0); }

    // forward_list also provides const iterator and const iterator accessors

    static node_ptr_type next( node_ptr_type n ) { return nodeinfo::next_ptr(n); }

/*
    bool is_front( const node_ptr_type node ) const
    {
        return node == front_;
    }

    bool is_back( const node_ptr_type node ) const
    {
        return nodeinfo::next_ptr(node) == 0;
    }

    // identify terminator pointer (it's just a NULL ptr)

    bool is_end( const node_ptr_type node ) const // node points to the element past back
    {
        return (node == 0);
    }
*/
};

template<typename NodePtrT, int NEXT_LINK_INDEX>
inline void swap( QwSList<NodePtrT,NEXT_LINK_INDEX>& a, QwSList<NodePtrT,NEXT_LINK_INDEX>& b )
{
    a.swap(b);
}

#endif /* INCLUDED_QWSLIST_H */

/* -----------------------------------------------------------------------
Last reviewed: June 30, 2013
Last reviewed by: Ross B.
Status: OK
-------------------------------------------------------------------------- */
