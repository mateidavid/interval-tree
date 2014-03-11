### Intrusive Interval Trees


#### Overview

This package provides a basic implementation of **intrusive interval
trees**, as decribed, e.g., in *Introduction to Algorithms* by Cormen et
al.

The package depends on a version of Boost Intrusive red-black trees
that provide hooks for maintaining extra data within the nodes of the
tree. Specifically, the code assumes the red-black trees know how to
use a `recompute_data()` method in order to maintain extra data in the
node of the tree.

*This package does not work with the stock Boost Intrusive library*. A
separate package provides a hook-enabled version of Boost Intrusive.


#### Usage

This is a header-only package, there is no need to compile
anything. The test file `examples/test-itree.cpp` demonstrates the
intended usage.

To properly compile and use an `itree`, the include path must contain
*in order*:

1. The path to the hook-enabled version of Boost Intrusive;
2. The path to the `include/` folder in this package;
3. The path to the rest of the Boost header files. The Boost Range
   library is needed. The Boost Program Options library is only needed
   to compile the test program, it is not otherwise needed to use an
   `itree`.

The `itree` struct is not as polished as other intrusive data
structures provided by Boost Intrusive. Notably, an `itree` can only
be constructed using explicit `Node_Traits`/`Value_Traits` structs.

In addition to the requirements for red-black trees specified here
http://www.boost.org/doc/libs/1_55_0/doc/html/intrusive/value_traits.html,
to define an `itree`, we have the following extra requirements:

- `Node_Traits` must contain the following:

        typedef (implementation_defined) key_type;
        static key_type get_max_end(const_node_ptr);
        static void set_max_end(node_ptr, key_type);

- `Value_Traits` must contain the following:

        typedef (implementation_defined) key_type;
        static key_type get_start(const_pointer);
        static key_type get_end(const_pointer);
        // if const_pointer != const value_type*, we also need:
        static key_type get_start(const value_type*);
        static key_type get_end(const value_type*);


#### Internals

An intrusive interval tree `itree` is an intrusive red-black tree
`rbtree` where the key is the left end of each interval, and the extra
data maintained at each node `n` of the tree is the right-most end of
all intervals in the subtree rooted at `n`.

In addition to all methods provided by `rbtree`, an `itree` also
implements the following method:

    intersection_iterator_range interval_intersect(const key_type& int_start, const key_type& int_end);

This produces a `boost::iterator_range` object that can be used to
efficiently iterate over the intervals stored in the tree that
intersect the interval given in the query.

For the purposes of computing intersections, all endpoints of all
intervals are considered to be closed.
