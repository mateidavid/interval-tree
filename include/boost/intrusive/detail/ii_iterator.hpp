#ifndef __II_ITERATOR_HPP
#define __II_ITERATOR_HPP

#include <boost/intrusive/detail/tree_iterator.hpp>
#include <boost/iterator/iterator_adaptor.hpp>
#include <boost/intrusive/itree_algorithms.hpp>


namespace boost
{
namespace intrusive
{
namespace detail
{

/** Interval intersection iterator. */
template < typename Value_Traits, bool Is_Const >
class ii_iterator
    : public iterator_adaptor< ii_iterator< Value_Traits, Is_Const >,
                               tree_iterator< Value_Traits, Is_Const >,
                               use_default,
                               forward_traversal_tag >
{
public:
    typedef tree_iterator< Value_Traits, Is_Const > regular_iterator;
    typedef typename Value_Traits::key_type key_type;
    typedef typename tree_iterator< Value_Traits, Is_Const >::pointer pointer; // WHY ???

    ii_iterator()
        : ii_iterator::iterator_adaptor_(), _int_start(0), _int_end(0) {}
    explicit ii_iterator(const regular_iterator& other, key_type int_start = 0, key_type int_end = 0)
        : ii_iterator::iterator_adaptor_(other), _int_start(int_start), _int_end(int_end) {}
    ii_iterator(const ii_iterator< Value_Traits, false >& other)
        : ii_iterator::iterator_adaptor_(other), _int_start(other._int_start), _int_end(other._int_end) {}

    regular_iterator operator -> () const { return this->base(); } // WHY ???

private:
    friend class boost::iterator_core_access;
    typedef itree_algorithms< Value_Traits > itree_algo;

    void increment()
    {
        this->base_reference() = itree_algo::get_next_interval(
            this->base().get_value_traits(), _int_start, _int_end, this->base().pointed_node(), 2);
    }

    key_type _int_start;
    key_type _int_end;
}; // class ii_iterator


} // namespace detail
} // namespace intrusive
} // namespace boost


#endif
