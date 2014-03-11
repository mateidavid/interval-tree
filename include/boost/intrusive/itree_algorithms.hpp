#ifndef __ITREE_ALGORTIHMS_HPP
#define __ITREE_ALGORTIHMS_HPP

#include <boost/intrusive/rbtree_algorithms.hpp>


namespace boost
{
namespace intrusive
{

template <typename Value_Traits>
struct itree_algorithms : public rbtree_algorithms< typename Value_Traits::node_traits >
{
    typedef typename Value_Traits::node_traits Node_Traits;
    typedef typename Value_Traits::key_type key_type;
    typedef typename Node_Traits::node_ptr node_ptr;
    typedef typename Node_Traits::const_node_ptr const_node_ptr;

    static bool possible_intersection_in_left_stree(
        const key_type& int_start, const key_type&, const_node_ptr n)
    {
        return int_start <= Node_Traits::get_max_end(n);
    }

    static bool possible_intersection_in_right_stree(
        const key_type& int_start, const key_type& int_end, const_node_ptr n)
    {
        return (int_start <= Node_Traits::get_max_end(n)
                and Value_Traits::get_start(Value_Traits::to_value_ptr(n)) <= int_end);
    }

    static bool intersect_node(
        const key_type& int_start, const key_type& int_end, const_node_ptr n)
    {
        key_type n_start = Value_Traits::get_start(Value_Traits::to_value_ptr(n));
        key_type n_end = Value_Traits::get_end(Value_Traits::to_value_ptr(n));
        return ((int_start <= n_start and n_start <= int_end)
                or (n_start <= int_start and int_start <= n_end));
    }

    static node_ptr get_next_interval(
        const key_type& int_start, const key_type& int_end, node_ptr n, int stage)
    {
        while (true)
        {
            if (not n)
            {
                stage = 3;
            }
            if (stage == 0)
            {
                // arrived from parent; try left stree
                if (possible_intersection_in_left_stree(int_start, int_end, n) and Node_Traits::get_left(n))
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
                if (intersect_node(int_start, int_end, n))
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
                if (possible_intersection_in_right_stree(int_start, int_end, n) and Node_Traits::get_right(n))
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
};

}
}

#endif
