#ifndef __ITREE_ALGORTIHMS_HPP
#define __ITREE_ALGORTIHMS_HPP

#include <boost/intrusive/rbtree_algorithms.hpp>


namespace boost
{
namespace intrusive
{

namespace detail
{

template<class ValueTraits, class ExtraChecker>
struct itree_node_extra_checker
      : public ExtraChecker
{
   typedef ExtraChecker                            base_checker_t;
   typedef ValueTraits                             value_traits;
   typedef typename value_traits::node_traits      node_traits;
   typedef typename node_traits::const_node_ptr    const_node_ptr;
   typedef typename value_traits::const_pointer    const_pointer;
   typedef typename node_traits::key_type          key_type;

   typedef typename base_checker_t::return_type    return_type;

   itree_node_extra_checker(const value_traits* vt_p, ExtraChecker extra_checker)
      : base_checker_t(extra_checker), vt_p_(vt_p)
   {}

   void operator () (const const_node_ptr& p,
                     const return_type& check_return_left, const return_type& check_return_right,
                     return_type& check_return)
   {
      const_node_ptr left = node_traits::get_left(p);
      const_node_ptr right = node_traits::get_right(p);
      const_pointer val_p = vt_p_->to_value_ptr(p);
      BOOST_INTRUSIVE_INVARIANT_ASSERT(vt_p_->get_start(val_p) <= vt_p_->get_end(val_p));
      key_type max_end = vt_p_->get_end(val_p);
      if (left)
         max_end = std::max(max_end, node_traits::get_max_end(left));
      if (right)
         max_end = std::max(max_end, node_traits::get_max_end(right));
      BOOST_INTRUSIVE_INVARIANT_ASSERT(node_traits::get_max_end(p) == max_end);
      base_checker_t::operator()(p, check_return_left, check_return_right, check_return);
   }

   const value_traits* const vt_p_;
};

} // namespace detail

template <typename Value_Traits>
struct itree_algorithms : public rbtree_algorithms< typename Value_Traits::node_traits >
{
    typedef typename Value_Traits::node_traits Node_Traits;
    typedef typename Value_Traits::key_type key_type;
    typedef typename Node_Traits::node_ptr node_ptr;
    typedef typename Node_Traits::const_node_ptr const_node_ptr;

    static bool possible_intersection_in_left_stree(const Value_Traits*,
        const key_type& int_start, const key_type&, const_node_ptr n)
    {
        return int_start <= Node_Traits::get_max_end(n);
    }

    static bool possible_intersection_in_right_stree(const Value_Traits* vtp,
        const key_type& int_start, const key_type& int_end, const_node_ptr n)
    {
        return (int_start <= Node_Traits::get_max_end(n)
                and vtp->get_start(vtp->to_value_ptr(n)) <= int_end);
    }

    static bool intersect_node(const Value_Traits* vtp,
        const key_type& int_start, const key_type& int_end, const_node_ptr n)
    {
        key_type n_start = vtp->get_start(vtp->to_value_ptr(n));
        key_type n_end = vtp->get_end(vtp->to_value_ptr(n));
        return ((int_start <= n_start and n_start <= int_end)
                or (n_start <= int_start and int_start <= n_end));
    }

    static node_ptr get_next_interval(const Value_Traits* vtp,
        const key_type& int_start, const key_type& int_end, const_node_ptr _n, int stage)
    {
        node_ptr n = pointer_traits< node_ptr >::const_cast_from(_n);
        while (true)
        {
            if (not n)
            {
                stage = 3;
            }
            if (stage == 0)
            {
                // arrived from parent; try left stree
                if (possible_intersection_in_left_stree(vtp, int_start, int_end, n) and Node_Traits::get_left(n))
                {
                    n = Node_Traits::get_left(n);
                    stage = 0;
                }
                else
                {
                    stage = 1;
                }
            }
            else if (stage == 1)
            {
                // finished visiting left stree; try current node
                if (intersect_node(vtp, int_start, int_end, n))
                {
                    return n;
                }
                else
                {
                    stage = 2;
                }
            }
            else if (stage == 2)
            {
                // visited current node; try right stree
                if (possible_intersection_in_right_stree(vtp, int_start, int_end, n) and Node_Traits::get_right(n))
                {
                    n = Node_Traits::get_right(n);
                    stage = 0;
                }
                else
                {
                    stage = 3;
                }
            }
            else if (stage == 3)
            {
                // finished visiting right stree
                node_ptr p = Node_Traits::get_parent(n);
                if (Node_Traits::get_parent(p) == n)
                {
                    // p is the header; we are done
                    return p;
                }
                else if (Node_Traits::get_left(p) == n)
                {
                    // n is left child
                    n = p;
                    stage = 1;
                }
                else
                {
                    // n is right child
                    n = p;
                    stage = 3;
                }
            }
        }
    }

    static key_type max_end_stree(const Value_Traits* vtp, const_node_ptr n, key_type max_val)
    {
        if (not n)
        {
            return std::numeric_limits< key_type >::min();
        }
        if (Node_Traits::get_max_end(n) <= max_val)
        {
            return Node_Traits::get_max_end(n);
        }
        key_type res = std::numeric_limits< key_type >::min();
        key_type tmp = vtp->get_end(vtp->to_value_ptr(n));
        if (tmp <= max_val)
        {
            res = tmp;
        }
        res = std::max(res, max_end_stree(vtp, Node_Traits::get_left(n), max_val));
        res = std::max(res, max_end_stree(vtp, Node_Traits::get_right(n), max_val));
        return res;
    }
};

}
}

#endif
