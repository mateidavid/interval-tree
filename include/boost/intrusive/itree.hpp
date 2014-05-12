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
template < typename Value_Traits >
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
    using typename Base::node_ptr;
    using typename Base::const_node_ptr;
    using typename Base::key_type;

    static void init_data(node_ptr n)
    {
        Base::set_max_end(n, Value_Traits::get_end(Value_Traits::to_value_ptr(n)));
    }
    static void recompute_extra_data(node_ptr n)
    {
        init_data(n);
        key_type tmp = Base::get_max_end(n);
        if (Base::get_left(n))
        {
            tmp = std::max(tmp, Base::get_max_end(Base::get_left(n)));
        }
        if (Base::get_right(n))
        {
            tmp = std::max(tmp, Base::get_max_end(Base::get_right(n)));
        }
        Base::set_max_end(n, tmp);
    }
    static void clone_extra_data(node_ptr dest, const_node_ptr src)
    {
        Base::set_max_end(dest, Base::get_max_end(src));
    }
}; // struct ITree_Node_Traits

/** Value Traits adaptor class for Interval Tree.
 *
 * The only function is to change the node_traits typedef.
 */
template < typename Value_Traits >
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
}; // struct ITree_Value_Traits

/** Comparator for Interval Tree. */
template < typename Value_Traits >
struct ITree_Compare
{
    typedef typename Value_Traits::value_type value_type;
    bool operator () (const value_type& lhs, const value_type& rhs) const
    {
        return Value_Traits::get_start(&lhs) < Value_Traits::get_start(&rhs);
    }
}; // struct ITree_Compare

template < typename Value_Traits, bool is_const >
class Intersection_Iterator
    : public boost::iterator_facade< Intersection_Iterator< Value_Traits, is_const >,
                                     typename Value_Traits::value_type,
                                     boost::forward_traversal_tag,
                                     typename boost::mpl::if_c< is_const,
                                                                typename Value_Traits::const_reference,
                                                                typename Value_Traits::reference
                                                              >::type
                                   >
{
public:
    typedef boost::iterator_facade< Intersection_Iterator< Value_Traits, is_const >,
                                    typename Value_Traits::value_type,
                                    boost::forward_traversal_tag,
                                    typename boost::mpl::if_c< is_const,
                                                               typename Value_Traits::const_reference,
                                                               typename Value_Traits::reference
                                                             >::type
                                  > Base;
    using typename Base::value_type;
    typedef typename Base::reference qual_reference;
    typedef typename Value_Traits::key_type key_type;
    typedef typename Value_Traits::node_ptr node_ptr;
    typedef typename Value_Traits::const_node_ptr const_node_ptr;
    typedef typename boost::mpl::if_c< is_const,
                                       const_node_ptr,
                                       node_ptr
                                     >::type qual_node_ptr;
    typedef typename boost::mpl::if_c< is_const,
                                       const value_type*,
                                       value_type*
                                     >::type qual_node_raw_ptr;

    Intersection_Iterator() {}
    explicit Intersection_Iterator(qual_node_ptr node, key_type int_start = 0, key_type int_end = 0)
    : _node(pointer_traits< node_ptr >::const_cast_from(node)), _int_start(int_start), _int_end(int_end) {}

    // implicit conversion to const
    operator const Intersection_Iterator< Value_Traits, true >& () const
    { return *reinterpret_cast< const Intersection_Iterator< Value_Traits, true >* >(this); }
    operator Intersection_Iterator< Value_Traits, true >& ()
    { return *reinterpret_cast< Intersection_Iterator< Value_Traits, true >* >(this); }

    // explicit conversion to non-const
    const Intersection_Iterator< Value_Traits, false >& unconst() const
    { return *reinterpret_cast< const Intersection_Iterator< Value_Traits, false >* >(this); }
    Intersection_Iterator< Value_Traits, false >& unconst()
    { return *reinterpret_cast< Intersection_Iterator< Value_Traits, false >* >(this); }

    qual_node_raw_ptr operator -> () const { return (&dereference()).operator ->(); }

private:
    friend class boost::iterator_core_access;

    typedef itree_algorithms< Value_Traits > itree_algo;

    bool equal(const Intersection_Iterator& rhs) const { return _node == rhs._node; }
    void increment() { _node = itree_algo::get_next_interval(_int_start, _int_end, _node, 2); }
    qual_reference dereference() const { return *Value_Traits::to_value_ptr(_node); }

    node_ptr _node;
    key_type _int_start;
    key_type _int_end;
}; // class Intersection_Iterator

} // namespace detail

template < class Value_Traits, class Compare, class Size_Type, bool Constant_Time_Size, typename Node_Allocator >
class itree_impl
    : public multiset_impl< Value_Traits, Compare, Size_Type, Constant_Time_Size, Node_Allocator >
{
public:
    typedef multiset_impl< Value_Traits, Compare, Size_Type, Constant_Time_Size, Node_Allocator > Base;
    using typename Base::value_compare;
    using typename Base::value_traits;
    using typename Base::node_allocator_type;
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

    // disallow copy
    itree_impl(const itree_impl&) = delete;
    itree_impl& operator = (const itree_impl&) = delete;

    explicit itree_impl(const value_compare& cmp = value_compare(),
                        const value_traits& v_traits = value_traits(),
                        const node_allocator_type& alloc = node_allocator_type())
        :  Base(cmp, v_traits, alloc)
    {}

    template < class Iterator >
    itree_impl(Iterator b, Iterator e,
               const value_compare& cmp = value_compare(),
               const value_traits& v_traits = value_traits(),
               const node_allocator_type& alloc = node_allocator_type())
        : Base(false, b, e, cmp, v_traits, alloc)
    {}

    itree_impl(itree_impl&& x)
        :  Base(std::move(static_cast< Base& >(x)))
    {}

    itree_impl& operator = (itree_impl&& other)
    { return static_cast< itree_impl& >(Base::operator = (std::move(static_cast< Base& >(other)))); }

    /** Return intervals in the tree that intersect a given interval.
     * @param int_start Interval start.
     * @param int_end Interval end.
     * @return An iterator range for the intersection (begin, end).
     */
    intersection_const_iterator_range iintersect(const key_type& int_start, const key_type& int_end) const
    {
        return make_iterator_range(iintersect_begin(int_start, int_end),
                                   iintersect_end());
    }

    /** Get maximum right endpoint is the tree. */
    key_type max_end() const
    {
        return Node_Traits::get_max_end(this->header_ptr());
    }

    /** Inform interval tree of an external shift in all interval endpoints.
     * NOTE: This function shifts the internal data stored in the interval tree,
     * but not the elements (intervals) themselves.
     * @param delta Value to add to all endpoints.
     */
    template < typename delta_type >
    void implement_shift(delta_type delta)
    {
        for (auto ref : *this)
        {
            node_ptr n = &ref;
            Node_Traits::set_max_end(n, key_type(delta_type(Node_Traits::get_max_end(n)) + delta));
        }
    }

private:
    intersection_const_iterator iintersect_begin(const key_type& int_start, const key_type& int_end) const
    {
        const_node_ptr header = this->header_ptr();
        if (not Node_Traits::get_parent(header))
        {
            return iintersect_end();
        }
        return intersection_const_iterator(
            itree_algo::get_next_interval(int_start, int_end, Node_Traits::get_parent(header), 0),
            int_start, int_end);
    }
    intersection_const_iterator iintersect_end() const
    {
        const_node_ptr header = this->header_ptr();
        return intersection_const_iterator(header);
    }
}; // class itree_impl

template < class T, class ...Options >
struct make_itree
{
    typedef typename pack_options< rbtree_defaults, Options... >::type packed_options;
    typedef typename detail::get_value_traits< T, typename packed_options::proto_value_traits >::type value_traits;
    typedef itree_impl< detail::ITree_Value_Traits< value_traits >
                      , detail::ITree_Compare< value_traits >
                      , typename packed_options::size_type
                      , packed_options::constant_time_size
                      , typename packed_options::node_allocator_type
                      > type;
}; // class make_itree

template < class T, class ...Options >
class itree
    : public make_itree< T, Options... >::type
{
    typedef typename make_itree< T, Options... >::type Base;

public:
    using typename Base::value_compare;
    using typename Base::value_traits;
    using typename Base::node_allocator_type;
    using typename Base::iterator;
    using typename Base::const_iterator;
    static_assert(std::is_same< typename value_traits::value_type, T >::value, "conflicting value and value traits types");

    // disallow copy
    itree(const itree&) = delete;
    itree& operator = (const itree&) = delete;

    explicit itree(const value_compare& cmp = value_compare(),
                   const value_traits& v_traits = value_traits(),
                   const node_allocator_type& alloc = node_allocator_type())
        : Base(cmp, v_traits, alloc)
    {}

    template < class Iterator >
    itree(Iterator b, Iterator e,
          const value_compare& cmp = value_compare(),
          const value_traits& v_traits = value_traits(),
          const node_allocator_type& alloc = node_allocator_type())
        : Base(false, b, e, cmp, v_traits, alloc)
    {}

    itree(itree&& other)
        : Base(std::move(static_cast< Base& >(other)))
    {}

    itree& operator = (itree&& other)
    { return static_cast< itree& >(Base::operator = (std::move(static_cast< Base& >(other)))); }

    // upcast container_from_iterator return values
    static itree& container_from_end_iterator(iterator end_iterator)
    { return static_cast< itree& >(Base::container_from_end_iterator(end_iterator)); }
    static const itree& container_from_end_iterator(const_iterator end_iterator)
    { return static_cast< const itree& >(Base::container_from_end_iterator(end_iterator)); }
    static itree& container_from_iterator(iterator it)
    { return static_cast< itree& >(Base::container_from_iterator(it)); }
    static const itree& container_from_iterator(const_iterator it)
    { return static_cast< const itree& >(Base::container_from_iterator(it)); }
}; // class itree

} // namespace intrusive
} // namespace boost

#endif
