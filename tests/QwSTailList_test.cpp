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
#include "QwSTailList.h"

#include "catch.hpp"

#include "Qw_Lists_adhocTestsShared.h"
#include "Qw_Lists_axiomaticTestsShared.h"
#include "Qw_Lists_randomisedTestShared.h"

/*
compared to QwSList, QwSTailList adds:

    - a back_ member
    - a push_back() method
    - a back() accessor method

Therefore test back() and push_back()

back_ is modified by:

    clear()
    swap()

    push_front()
    pop_front()
    push_back()
    insert_after()
    remove_after()
    erase_after()

back_ is tested by:

    size_is_1
    size_is_greater_than_1

the axiomatic tests below validate back() and there is a test for 
list construction using push_back()
*/

namespace {

    struct TestNode{
        TestNode *links_[2];
        enum { LINK_INDEX_1, LINK_INDEX_2, LINK_COUNT };

        int value;

        TestNode()
            : value( 0 )
        {
            for( int i=0; i < LINK_COUNT; ++i )
                links_[i] = 0;
        }
    };

    typedef QwSTailList<TestNode*, TestNode::LINK_INDEX_1> staillist_1_t;

} // end anonymous namespace


TEST_CASE( "qw/staillist/empty", "QwSTailList operations on empty lists" ) {

    staillist_1_t a, b;
    emptyListTest( a, b );

    REQUIRE( a.back() == 0 );
}

TEST_CASE( "qw/staillist/one", "QwSTailList list operations with 1 node/element" ) {

    TestNode node;
    node.value = 42;

    staillist_1_t a, b;
    a.push_back( &node );

    REQUIRE( a.empty() == false );
    REQUIRE( a.back() == a.front() );

    singleItemSListTest( a, b, &node );
}

TEST_CASE( "qw/staillist/two", "QwSTailList list operations with 2 nodes/elements" ) {

    TestNode node1;
    node1.value = 0;

    TestNode node2;
    node2.value = 1;

    staillist_1_t a;
    a.push_back( &node2 );
    staillist_1_t b;

    twoItemSListTest( a, b, &node1, &node2 );
}

TEST_CASE( "qw/staillist/back-and-push_back", "QwSTailList test back() and push_back()" ) {

    // back() and push_back() are the only two members that IoSList doesn't have that QwSTailList does.
    
    TestNode node1;
    node1.value = 0;

    TestNode node2;
    node2.value = 1;

    TestNode node3;
    node3.value = 2;

    staillist_1_t a;
    staillist_1_t b;

    backAndPushBackListTest( a, b, &node1, &node2, &node3 );
}

TEST_CASE( "qw/staillist/many", "QwSTailList list operations with many nodes/elements" ) {

    const int NODE_COUNT = 5;
    TestNode nodes[NODE_COUNT];

    manyItemsSListTest<staillist_1_t,NODE_COUNT>( nodes );
}

TEST_CASE( "qw/staillist/front-stack", "QwSTailList front stack test" ) {
    
    staillist_1_t a;

    const int NODE_COUNT = 10;

    TestNode nodes[NODE_COUNT];
    for( int i=0; i < NODE_COUNT; ++i )
        nodes[i].value = i;

    frontStackTest_withBackChecks( a, nodes, NODE_COUNT );
}

TEST_CASE( "qw/staillist/back-queue", "QwSTailList back queue test" ) {

    staillist_1_t a;

    const int NODE_COUNT = 10;

    TestNode nodes[NODE_COUNT];
    for( int i=0; i < NODE_COUNT; ++i )
        nodes[i].value = i;

    backQueueTest( a, nodes, NODE_COUNT );
}

/*
    The following "axiomatic" tests aim to build base invariants
    for a list constructed with push_back: size functions operate correctly,
    constructed list contains exactly the expected items in the expected order.
    we then test each other function separately by constructing all of
    the different scenarios, which amount to treating lists of
    zero, one or many items, and inserting at the beginning, middle or end 
    of the list.
*/

// requireEmptyInvariants, requireSingleNodeInvariants, requireMoreThanOneNodeInvariants
//
// used to test accessors
// 
// iterator construction, prefix and postfix ++
//
// empty, size_is_1, size_is_greater_than_1
// front
// before_begin, begin, end
// next

static void requireEmptyInvariants( staillist_1_t& a )
{
    // size related attributes

    REQUIRE( a.empty() == true );
    REQUIRE( a.size_is_1() == false );
    REQUIRE( a.size_is_greater_than_1() == false );

    // front(), back()

    REQUIRE( a.front() == 0 );
    REQUIRE( a.back() == 0 );

    // iterator sequence invariants

    REQUIRE( a.begin() == a.end() );

    { // begin comes directly after before_begin (pre-increment)
        staillist_1_t::iterator i = a.before_begin();
        ++i;
        REQUIRE( i == a.begin() );
    }

    { // begin comes directly after before_begin (post-increment)
        staillist_1_t::iterator i = a.before_begin();
        i++;
        REQUIRE( i == a.begin() );
    }
}

static void requireSingleNodeInvariants( staillist_1_t& a, TestNode *node )
{
    // size related attributes

    REQUIRE( a.empty() == false );
    REQUIRE( a.size_is_1() == true );
    REQUIRE( a.size_is_greater_than_1() == false );

    // front(), back() invariants

    REQUIRE( a.front() != 0 );
    REQUIRE( a.front() == node );

    REQUIRE( a.back() != 0 );
    REQUIRE( a.back() == node );

    REQUIRE( *a.begin() == node );

    // next() of front() is 0 in a one item list

    REQUIRE( a.next(a.front()) == 0 );

    REQUIRE( a.next( a.back() ) == 0 );

	// next() of front() is 0 in a one item list
    REQUIRE( a.next(a.front()) == 0 );

    // iterator sequence invariants

    REQUIRE( a.begin() != a.end() );

    { // begin comes directly after before_begin (pre-increment)
        staillist_1_t::iterator i = a.before_begin();
        ++i;
        REQUIRE( i == a.begin() );
    }

    { // begin comes directly after before_begin (post-increment)
        staillist_1_t::iterator i = a.before_begin();
        i++;
        REQUIRE( i == a.begin() );
    }

    { // end comes directly after begin in a one item list (pre-increment)
        staillist_1_t::iterator i = a.begin();
        REQUIRE( *i == node );
        ++i;
        REQUIRE( i == a.end() );
    }

    { // end comes directly after begin in a one item list (post-increment)
        staillist_1_t::iterator i = a.begin();
        REQUIRE( *i == node );
        i++;
        REQUIRE( i == a.end() );
    }
}

static void requireMoreThanOneNodeInvariants( staillist_1_t& a, TestNode *nodes, int nodeCount )
{
    // size related attributes

    REQUIRE( a.empty() == false );
    REQUIRE( a.size_is_1() == false );
    REQUIRE( a.size_is_greater_than_1() == true );

    // front(), back() invariants

    REQUIRE( a.front() != 0 );
    REQUIRE( a.front() == &nodes[0] );

    REQUIRE( a.back() != 0 );
    REQUIRE( a.back() == &nodes[nodeCount-1] );

    REQUIRE( *a.begin() == &nodes[0] );

    REQUIRE( a.next(a.front()) != 0 );

    REQUIRE( a.next( a.back() ) == 0 );

    // iterator sequence invariants

    REQUIRE( a.begin() != a.end() );

    { // begin comes directly after before_begin (pre-increment)
        staillist_1_t::iterator i = a.before_begin();
        ++i;
        REQUIRE( i == a.begin() );
    }

    { // begin comes directly after before_begin (post-increment)
        staillist_1_t::iterator i = a.before_begin();
        i++;
        REQUIRE( i == a.begin() );
    }

    // nodes are in expected sequence according to iterators
    // and also according to a.next()

    // walk forwards through the list using both an iterator and a.next(n)
    // in each case check that the sequence matches the array passed in
    // and that the list is terminated correctly.

    TestNode *n = a.front();
    staillist_1_t::iterator i_pre = a.begin();
    staillist_1_t::iterator i_post = a.begin();
    for( int j = 0; j < nodeCount; ++j ){

        REQUIRE( n == &nodes[j] );
        n = a.next(n);

        REQUIRE( *i_pre == &nodes[j] );
        ++i_pre;

        REQUIRE( *i_post == &nodes[j] );
        i_post++;
    }

    REQUIRE( n == 0 );
    REQUIRE( i_pre == a.end() );
    REQUIRE( i_post == a.end() );
}

TEST_CASE( "qw/staillist/axiomatic/baseline", "QwSTailList baseline axiomatic tests" ) {
    axiomaticBaselineTest<staillist_1_t>( requireEmptyInvariants, requireSingleNodeInvariants, requireMoreThanOneNodeInvariants );
}

TEST_CASE( "qw/staillist/axiomatic/push_back", "QwSTailList push_back" ) {
    axiomaticPushBackTest<staillist_1_t>( requireEmptyInvariants, requireSingleNodeInvariants, requireMoreThanOneNodeInvariants );
}

TEST_CASE( "qw/staillist/axiomatic/swap", "QwSTailList axiomatic test of swap" ) {
    axiomaticSwapTest<staillist_1_t>( requireEmptyInvariants, requireSingleNodeInvariants, requireMoreThanOneNodeInvariants );
}

TEST_CASE( "qw/staillist/axiomatic/pop_front", "QwSTailList axiomatic test of pop_front" ) {
    axiomaticPopFrontTest<staillist_1_t>( requireEmptyInvariants, requireSingleNodeInvariants, requireMoreThanOneNodeInvariants );
}

TEST_CASE( "qw/staillist/axiomatic/insert_after/node_ptr/empty", "QwSTailList axiomatic test of insert_after(node_ptr,n) for empty list" ) {

    SECTION( "empty", "initially empty list, insert after before_begin")
    { // empty
        staillist_1_t a;
        TestNode node;
        a.insert_after( *a.before_begin(), &node ); // strictly you shouldn't de-ref the before_begin iterator, it doesn't point to a node
        requireSingleNodeInvariants( a, &node );
    }
}

TEST_CASE( "qw/staillist/axiomatic/insert_after/node_ptr", "QwSTailList axiomatic test of insert_after(node_ptr,n)" ) {
    axiomaticInsertAfterNodePtrTest<staillist_1_t>( requireEmptyInvariants, requireSingleNodeInvariants, requireMoreThanOneNodeInvariants );
}

TEST_CASE( "qw/staillist/axiomatic/insert_after/iter", "QwSTailList axiomatic test of insert_after(iter,n)" ) {
    axiomaticInsertAfterIterTest<staillist_1_t>( requireEmptyInvariants, requireSingleNodeInvariants, requireMoreThanOneNodeInvariants );
}

TEST_CASE( "qw/staillist/axiomatic/remove_after/before_begin/node_ptr", "QwSTailList axiomatic test of remove_after(*before_begin())" ) {
    axiomaticRemoveAfterBeforeBeginTest<staillist_1_t>( requireEmptyInvariants, requireSingleNodeInvariants, requireMoreThanOneNodeInvariants );
}

TEST_CASE( "qw/staillist/axiomatic/remove_after/2/node_ptr", "QwSTailList axiomatic test of remove_after(node_ptr) using front" ) {
    axiomaticRemoveAfter2NodePtrTest<staillist_1_t>( requireEmptyInvariants, requireSingleNodeInvariants, requireMoreThanOneNodeInvariants );
}

TEST_CASE( "qw/staillist/axiomatic/remove_after/iter", "QwSTailList axiomatic test of remove_after(iter)" ) {
    axiomaticRemoveAfterIterTest<staillist_1_t>( requireEmptyInvariants, requireSingleNodeInvariants, requireMoreThanOneNodeInvariants );
}

TEST_CASE( "qw/staillist/axiomatic/erase_after/iter", "QwSTailList axiomatic test of erase_after(iter)" ) {
    axiomaticEraseAfterIterTest<staillist_1_t>( requireEmptyInvariants, requireSingleNodeInvariants, requireMoreThanOneNodeInvariants );
}

/* fuzz test */

static void verify( staillist_1_t& list, int expectedCount )
{
    verifyForwards( list, expectedCount );
}

static void randomisedInsert( staillist_1_t& list, TestNode* node, int currentCount )
{
    switch( list.empty() ? rand() % 2 : rand() % 3 ){
    case 0:
        list.push_front( node );
        break;
    case 1:
        list.push_back( node );
        break;
    case 2:
    default:
        {
            int atj = rand() % currentCount;
            staillist_1_t::node_ptr_type at = list.front();
            for( int i=0; i<atj; ++i )
                at = list.next(at);
            list.insert_after( at, node ); // insert n after node before
            break;
        }
    }
}

static TestNode* randomisedRemove( staillist_1_t& list, int currentCount )
{
    switch( currentCount > 1 ? rand() % 2 : 0 ){
    case 0:
        return list.pop_front();
        break;
    case 1:
    default:
        {
            int atj = rand() % (currentCount - 1); // -1 because we can't remove after the last item
            staillist_1_t::node_ptr_type at = list.front();
            for( int i=0; i<atj; ++i )
                at = list.next(at);
            return list.remove_after( at ); // returns the removed node
            break;
        }
    }
}

TEST_CASE( "qw/staillist/fuzz", "[fuzz] QwSTailList fuzz test" ) {
    fuzzTest<staillist_1_t>( randomisedInsert, randomisedRemove, verify );
}

/* -----------------------------------------------------------------------
Last reviewed: June 30, 2013
Last reviewed by: Ross B.
Status: OK
-------------------------------------------------------------------------- */