#ifndef __ITREE_HPP
#define __ITREE_HPP

#include <boost/intrusive/set.hpp>
#include <boost/iterator/iterator_facade.hpp>
#include <boost/range/iterator_range.hpp>
#include <boost/mpl/if.hpp>
#include <boost/tti/tti.hpp>
#include "itree_algorithms.hpp"


namespace boost
{
namespace intrusive
{
namespace detail
{

// Checks for Node Traits and Value Traits members
BOOST_TTI_HAS_TYPE(key_type)
BOOST_TTI_HAS_STATIC_MEMBER_FUNCTION(get_max_end)
BOOST_TTI_HAS_STATIC_MEMBER_FUNCTION(set_max_end)
BOOST_TTI_HAS_STATIC_MEMBER_FUNCTION(get_start)
BOOST_TTI_HAS_STATIC_MEMBER_FUNCTION(get_end)

/** Node Traits adaptor for Interval Tree.
 *
 * This Traits class defines the node maintenance methods that hook into
 * the rbtree algorithms.
 */
template <typename Value_Traits>
struct ITree_Node_Traits : public Value_Traits::node_traits
{
private:
    typedef typename Value_Traits::node_traits Base;

    static_assert(has_type_key_type< Base >::value,
                  "Node Traits missing key_type");
    static_assert(has_static_member_function_get_max_end< Base, typename Base::key_type (typename Base::const_node_ptr)>::value,
                  "Node Traits missing get_max_end()");
    static_assert(has_static_member_function_set_max_end< Base, void (typename Base::node_ptr, typename Base::key_type)>::value,
                  "Node Traits missing set_max_end()");
public:
    typedef ITree_Node_Traits node_traits;
    using typename Base::node;
    using typename Base::node_ptr;
    using typename Base::const_node_ptr;
    using typename Base::color;
    using typename Base::key_type;
    using Base::get_parent;
    using Base::set_parent;
    using Base::get_left;
    using Base::set_left;
    using Base::get_right;
    using Base::set_right;
    using Base::get_color;
    using Base::set_color;
    using Base::black;
    using Base::red;
    using Base::get_max_end;
    using Base::set_max_end;

    static void init_data(node_ptr n)
    {
        set_max_end(n, Value_Traits::get_end(Value_Traits::to_value_ptr(n)));
    }
    static void recompute_extra_data(node_ptr n)
    {
        init_data(n);
        key_type tmp = get_max_end(n);
        if (get_left(n))
        {
            tmp = std::max(tmp, get_max_end(get_left(n)));
        }
        if (get_right(n))
        {
            tmp = std::max(tmp, get_max_end(get_right(n)));
        }
        set_max_end(n, tmp);
    }
    static void clone_extra_data(node_ptr dest, const_node_ptr src)
    {
        set_max_end(dest, get_max_end(src));
    }
};

/** Value Traits adaptor class for Interval Tree.
 *
 * The only function is to change the node_traits typedef.
 */
template <typename Value_Traits>
struct ITree_Value_Traits : public Value_Traits
{
private:
    typedef Value_Traits Base;

    static_assert(has_type_key_type< Base >::value,
                  "Value Traits missing key_type");
    static_assert(has_static_member_function_get_start< Base, typename Base::key_type (typename Base::const_node_ptr)>::value,
                  "Value Traits missing get_start(const_node_ptr)");
    static_assert(has_static_member_function_get_start< Base, typename Base::key_type (const typename Base::value_type*)>::value,
                  "Value Traits missing get_start(const value_type*)");
    static_assert(has_static_member_function_get_end< Base, typename Base::key_type (typename Base::const_node_ptr)>::value,
                  "Value Traits missing get_end(const_node_ptr)");
    static_assert(has_static_member_function_get_end< Base, typename Base::key_type (const typename Base::value_type*)>::value,
                  "Value Traits missing get_end(const value_type*)");
public:
    typedef ITree_Node_Traits< Value_Traits > node_traits;
    using typename Base::value_type;
    using typename Base::node_ptr;
    using typename Base::const_node_ptr;
    using typename Base::pointer;
    using typename Base::const_pointer;
    using typename Base::reference;
    using typename Base::const_reference;
    using Base::to_node_ptr;
    using Base::to_value_ptr;
    using Base::get_start;
    using Base::get_end;
};

/** Comparator for Interval Tree.
 *
 */
template <typename Value_Traits>
struct ITree_Compare
{
    typedef typename Value_Traits::value_type value_type;
    bool operator () (const value_type& lhs, const value_type& rhs) const
    {
        return Value_Traits::get_start(&lhs) < Value_Traits::get_start(&rhs);
    }
};

template <typename Value_Traits, bool is_const>
class Intersection_Iterator : public boost::iterator_facade<
    Intersection_Iterator< Value_Traits, is_const >,
    typename Value_Traits::value_type,
    boost::forward_traversal_tag,
    typename boost::mpl::if_c< is_const, typename Value_Traits::const_reference, typename Value_Traits::reference >::type
    >
{
public:
    typedef typename Value_Traits::value_type value_type;
    typedef typename Value_Traits::key_type key_type;
    typedef typename Value_Traits::node_ptr node_ptr;
    typedef typename Value_Traits::reference reference;
    typedef typename Value_Traits::const_reference const_reference;
    typedef typename boost::mpl::if_c< is_const, const_reference, reference >::type qual_reference;

    Intersection_Iterator() {}
    explicit Intersection_Iterator(node_ptr node, key_type int_start = 0, key_type int_end = 0)
    : _node(node), _int_start(int_start), _int_end(int_end) {}

    // implicit conversion to const
    operator Intersection_Iterator< Value_Traits, true >& ()
    { return *reinterpret_cast< Intersection_Iterator< Value_Traits, true >* >(this); }
    operator const Intersection_Iterator< Value_Traits, true >& () const
    { return *reinterpret_cast< const Intersection_Iterator< Value_Traits, true >* >(this); }

    // explicit conversion to non-const
    explicit operator Intersection_Iterator< Value_Traits, false >& ()
    { return *reinterpret_cast< Intersection_Iterator< Value_Traits, false >* >(this); }
    explicit operator const Intersection_Iterator< Value_Traits, false >& () const
    { return *reinterpret_cast< const Intersection_Iterator< Value_Traits, false >* >(this); }

    value_type* operator -> () const { return (&dereference()).operator ->(); }

private:
    friend class boost::iterator_core_access;

    typedef itree_algorithms< Value_Traits > itree_algo;

    bool equal(const Intersection_Iterator& rhs) const { return _node == rhs._node; }
    void increment() { _node = itree_algo::get_next_interval(_int_start, _int_end, _node, 2); }
    qual_reference dereference() const { return *Value_Traits::to_value_ptr(_node); }

    node_ptr _node;
    key_type _int_start;
    key_type _int_end;
};

}

template <typename Value_Traits>
class itree
  : public multiset<
        typename Value_Traits::value_type,
        compare< detail::ITree_Compare< Value_Traits > >,
        value_traits< detail::ITree_Value_Traits< Value_Traits > >
    >
{
public:
    typedef multiset<
        typename Value_Traits::value_type,
        compare< detail::ITree_Compare< Value_Traits > >,
        value_traits< detail::ITree_Value_Traits< Value_Traits > >
    > Base;
    typedef itree_algorithms< Value_Traits > itree_algo;
    typedef typename Value_Traits::node_traits Node_Traits;
    typedef typename Value_Traits::key_type key_type;
    typedef typename Value_Traits::pointer pointer;
    typedef typename Value_Traits::const_pointer const_pointer;
    typedef typename Value_Traits::node_ptr node_ptr;
    typedef typename Value_Traits::const_node_ptr const_node_ptr;
    typedef detail::Intersection_Iterator< Value_Traits, false > intersection_iterator;
    typedef detail::Intersection_Iterator< Value_Traits, true > intersection_const_iterator;
    typedef boost::iterator_range< detail::Intersection_Iterator< Value_Traits, false > > intersection_iterator_range;
    typedef boost::iterator_range< detail::Intersection_Iterator< Value_Traits, true > > intersection_const_iterator_range;

    // inherit multiset constructors
    using Base::Base;

    /** Return intervals in the tree that intersect a given interval.
     * @param int_start Interval start.
     * @param int_end Interval end.
     * @return An iterator range for the intersection (begin, end).
     */
    intersection_iterator_range interval_intersect(const key_type& int_start, const key_type& int_end)
    {
        return make_iterator_range(interval_intersect_begin(int_start, int_end),
                                   interval_intersect_end());
    }

private:
    intersection_iterator interval_intersect_begin(const key_type& int_start, const key_type& int_end)
    {
        node_ptr header = this->header_ptr();
        if (not Node_Traits::get_parent(header))
        {
            return intersection_iterator(interval_intersect_end());
        }
        return intersection_iterator(
            itree_algo::get_next_interval(int_start, int_end, Node_Traits::get_parent(header), 0),
            int_start, int_end);
    }
    intersection_iterator interval_intersect_end()
    {
        node_ptr header = this->header_ptr();
        return intersection_iterator(header);
    }
};

}
}

#endif
