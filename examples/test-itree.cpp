#include <iostream>
#include <time.h>
#include <boost/program_options.hpp>
#include <boost/intrusive/list.hpp>
#include <boost/intrusive/itree.hpp>
#include <boost/tti/tti.hpp>

using namespace std;

namespace detail
{
    //check Boost Intrusive contains hooks to maintain extra data
    typedef boost::intrusive::detail::extra_data_manager< void > extra_data_manager_check;
}

struct Value
{
    typedef Value* ptr_type;

    size_t _start;
    size_t _end;

    ptr_type _parent;
    ptr_type _l_child;
    ptr_type _r_child;
    int _col;
    size_t _max_end;

    ptr_type _list_next;
    ptr_type _list_prev;
};

typedef Value* ptr_type;
typedef const Value* const_ptr_type;
typedef Value& ref_type;
typedef const Value& const_ref_type;

ostream& operator <<(ostream& os, const Value& rhs)
{
    os << "[_start=" << rhs._start << ",_end=" << rhs._end
       << ",_parent=" << rhs._parent
       << ",_l_child=" << rhs._l_child
       << ",_r_child=" << rhs._r_child
       << ",_col=" << rhs._col
       << ",_max_end=" << rhs._max_end
       << "]";
    return os;
}

bool intersect(const Value& lhs, const Value& rhs)
{
    return (lhs._start <= rhs._start and rhs._start <= lhs._end)
           or (rhs._start <= lhs._start and lhs._start <= rhs._end);
}

template <class T>
struct ITree_Node_Traits
{
    typedef T node;
    typedef node* node_ptr;
    typedef const node* const_node_ptr;
    typedef int color;
    typedef size_t key_type;

    static node_ptr get_parent(const_node_ptr n) { return n->_parent; }
    static void set_parent(node_ptr n, node_ptr ptr) { n->_parent = ptr; }
    static node_ptr get_left(const_node_ptr n) { return n->_l_child; }
    static void set_left(node_ptr n, node_ptr ptr) { n->_l_child = ptr; }
    static node_ptr get_right(const_node_ptr n) { return n->_r_child; }
    static void set_right(node_ptr n, node_ptr ptr) { n->_r_child = ptr; }
    static color get_color(const_node_ptr n) { return n->_col; }
    static void set_color(node_ptr n, color c) { n->_col = c ; }
    static color black() { return 0; }
    static color red() { return 1; }
    static key_type get_max_end(const_node_ptr n) { return n->_max_end; }
    static void set_max_end(node_ptr n, key_type k) { n->_max_end = k ; }
};

template <class T>
struct ITree_Value_Traits
{
    typedef T value_type;
    typedef ITree_Node_Traits< T > node_traits;
    typedef typename node_traits::key_type key_type;
    typedef typename node_traits::node_ptr node_ptr;
    typedef typename node_traits::const_node_ptr const_node_ptr;
    typedef node_ptr pointer;
    typedef const_node_ptr const_pointer;
    typedef value_type& reference;
    typedef const value_type& const_reference;

    static const boost::intrusive::link_mode_type link_mode = boost::intrusive::normal_link;

    static node_ptr to_node_ptr (reference value) { return &value; }
    static const_node_ptr to_node_ptr (const_reference value) { return &value; }
    static pointer to_value_ptr(node_ptr n) { return n; }
    static const_pointer to_value_ptr(const_node_ptr n) { return n; }
    static key_type get_start(const_pointer n) { return n->_start; }
    static key_type get_end(const_pointer n) { return n->_end; }
};

template <class T>
struct List_Node_Traits
{
    typedef T node;
    typedef node* node_ptr;
    typedef const node* const_node_ptr;

    static node_ptr get_next(const_node_ptr n) { return n->_list_next; }
    static void set_next(node_ptr n, node_ptr next) { n->_list_next = next; }
    static node_ptr get_previous(const_node_ptr n) { return n->_list_prev; }
    static void set_previous(node_ptr n, node_ptr prev) { n->_list_prev = prev; }
};

template <class T>
struct List_Value_Traits
{
    typedef T value_type;
    typedef List_Node_Traits< T > node_traits;
    typedef typename node_traits::node_ptr node_ptr;
    typedef typename node_traits::const_node_ptr const_node_ptr;
    typedef node_ptr pointer;
    typedef const_node_ptr const_pointer;
    typedef value_type& reference;
    typedef const value_type& const_reference;

    static const boost::intrusive::link_mode_type link_mode = boost::intrusive::normal_link;

    static node_ptr to_node_ptr (reference value) { return &value; }
    static const_node_ptr to_node_ptr (const_reference value) { return &value; }
    static pointer to_value_ptr(node_ptr n) { return n; }
    static const_pointer to_value_ptr(const_node_ptr n) { return n; }
};

typedef boost::intrusive::itree< ITree_Value_Traits< Value > > itree_type;
typedef itree_type::itree_algo itree_algo;
typedef boost::intrusive::list< Value, boost::intrusive::value_traits< List_Value_Traits< Value > > > list_type;

static_assert(
    boost::intrusive::detail::extra_data_manager<
        boost::intrusive::detail::ITree_Node_Traits < ITree_Value_Traits< Value > >
    >::enabled,
    "Extra data manager is not enabled");

template<class T>
class delete_disposer
{
   public:
   template <class Pointer>
      void operator()(Pointer p)
   {
      delete boost::intrusive::detail::to_raw_pointer(p);
   }
};

template<class T>
class new_cloner
{
   public:
      T *operator()(const T &t)
   {  return new T(t);  }
};

const_ptr_type get_root(itree_type& t)
{
    return itree_algo::get_header(&*t.begin())->_parent;
}

void print_sub_tree(const_ptr_type r, size_t depth)
{
    if (!r)
    {
        return;
    }
    print_sub_tree(r->_l_child, depth + 1);
    for (size_t i = 0; i < depth; ++i)
    {
        clog << "  ";
    }
    clog << '[' << r->_start << ',' << r->_end << "] " << r->_max_end << '\n';
    print_sub_tree(r->_r_child, depth + 1);
}

void print_tree(itree_type& t)
{
    print_sub_tree(get_root(t), 0);
}

bool check_max_ends(const_ptr_type node_ptr, size_t& max_end)
{
    if (!node_ptr)
    {
        max_end = 0;
        return true;
    }
    size_t max_end_left;
    size_t max_end_right;
    if (not check_max_ends(node_ptr->_l_child, max_end_left) or not check_max_ends(node_ptr->_r_child, max_end_right))
    {
        return false;
    }
    if (node_ptr->_max_end != max(node_ptr->_end, max(max_end_left, max_end_right)))
    {
        clog << "_max_end error: " << *node_ptr << '\n';
        return false;
    }
    max_end = node_ptr->_max_end;
    return true;
}

void check_max_ends(itree_type& t)
{
    const_ptr_type root_node = get_root(t);
    size_t max_end;
    if (not check_max_ends(root_node, max_end))
    {
        clog << "tree:\n";
        for (auto const& e :t)
        {
            clog << e << '\n';
        }
        print_tree(t);
        exit(1);
    }
}

struct Program_Options
{
    size_t max_load;
    size_t range_max;
    size_t n_ops;
    size_t seed;
    bool print_tree_each_op;
};


void real_main(const Program_Options& po)
{
    clog << "----- program options:"
         << "\nmax_load=" << po.max_load
         << "\nrange_max=" << po.range_max
         << "\nn_ops=" << po.n_ops
         << "\nseed=" << po.seed << '\n';

    clog << "----- constructing iitree & ilist\n";
    itree_type t;
    list_type l;

    clog << "----- initializing random number generator\n";
    srand48(po.seed);

    clog << "----- main loop\n";
    for (size_t i = 0; i < po.n_ops; ++i)
    {
        int op = int(drand48()*5);
        if (op == 0)
        {
            // insert new element
            if (l.size() >= po.max_load)
            {
                continue;
            }
            ptr_type a = new Value();
            size_t e1 = size_t(drand48() * po.range_max);
            size_t e2 = size_t(drand48() * po.range_max);
            a->_start = min(e1, e2);
            a->_end = max(e1, e2);
            clog << "adding: " << *a << '\n';
            l.push_back(*a);
            t.insert(*a);
        }
        else if (op == 1)
        {
            // delete existing element
            if (l.size() == 0)
            {
                continue;
            }
            size_t idx = size_t(drand48() * l.size());
            auto it = l.begin();
            while (idx > 0)
            {
                ++it;
                --idx;
            }
            ptr_type a = &*it;
            clog << "deleting: " << *a << '\n';
            l.erase(it);
            t.erase(t.iterator_to(*a));
            delete a;
        }
        else if (op == 2)
        {
            // compute intersection with some interval
            size_t e1 = size_t(drand48() * po.range_max);
            size_t e2 = size_t(drand48() * po.range_max);
            if (e1 > e2)
            {
                swap(e1, e2);
            }
            ptr_type a = new Value();
            a->_start = e1;
            a->_end = e2;
            clog << "checking intersection with: " << *a << '\n';
            // first count intersections using list
            size_t res_list = 0;
            for (const auto& v : l)
            {
                if (intersect(v, *a))
                {
                    ++res_list;
                }
            }
            // count with iterator range
            size_t res_iterator_range = 0;
            for (const auto& r : t.interval_intersect(e1, e2))
            {
                (void)r;
                ++res_iterator_range;
            }
            if (res_iterator_range != res_list)
            {
                clog << "wrong intersection with " << *a << '\n';
                clog << "list:\n";
                for (const auto& v : l)
                {
                    clog << v << '\n';
                }
                clog << "itree:\n";
                for (const auto& v : t)
                {
                    clog << v << '\n';
                }
                print_tree(t);
                exit(EXIT_FAILURE);
            }
            clog << "intersection ok, size = " << res_list << " / " << l.size() << '\n';
            delete a;
        }
        else if (op == 3)
        {
            // check max_end fields in the tree
            clog << "checking max_end fields\n";
            check_max_ends(t);
        }
        else if (op == 4)
        {
            // clon tree and check
            clog << "cloning tree of size: " << t.size() << '\n';
            itree_type t2;
            t2.clone_from(t, new_cloner< Value >(), delete_disposer< Value >());
            clog << "checking max_end fields in clone of size: " << t2.size() << '\n';
            check_max_ends(t2);
            clog << "destroying clone\n";
            ptr_type tmp;
            while ((tmp = t2.unlink_leftmost_without_rebalance()))
            {
                delete tmp;
            }
        }
        if (po.print_tree_each_op)
        {
            print_tree(t);
        }
    }
    clog << "----- clearing list\n";
    while (l.size() > 0)
    {
        auto it = l.begin();
        ptr_type a = &*it;
        l.erase(it);
        t.erase(t.iterator_to(*a));
        delete a;
    }
    clog << "----- success\n";
}

int main(int argc, char* argv[])
{
    namespace bo = boost::program_options;
    Program_Options po;
    try
    {
        bo::options_description generic_opts_desc("Generic options");
        bo::options_description config_opts_desc("Configuration options");
        bo::options_description hidden_opts_desc("Hidden options");
        bo::options_description cmdline_opts_desc;
        bo::options_description visible_opts_desc("Allowed options");
        generic_opts_desc.add_options()
            ("help,h", "produce help message")
            ;
        config_opts_desc.add_options()
            ("max-load", bo::value<size_t>(&po.max_load)->default_value(100), "maximum load of the interval tree")
            ("range-max", bo::value<size_t>(&po.range_max)->default_value(20), "maximum endpoint")
            ("n-ops", bo::value<size_t>(&po.n_ops)->default_value(1000), "number of operations")
            ("seed", bo::value<size_t>(&po.seed)->default_value(0), "random number generator seed")
            ("print-tree", "print tree after each operation")
            ;
        cmdline_opts_desc.add(generic_opts_desc).add(config_opts_desc).add(hidden_opts_desc);
        visible_opts_desc.add(generic_opts_desc).add(config_opts_desc);
        bo::variables_map vm;
        store(bo::command_line_parser(argc, argv).options(cmdline_opts_desc).run(), vm);
        notify(vm);
        if (vm.count("help"))
        {
            cout << visible_opts_desc;
            exit(EXIT_SUCCESS);
        }
        if (po.seed == 0)
        {
            po.seed = time(NULL);
        }
        po.print_tree_each_op = vm.count("print-tree") > 0;
    }
    catch(exception& e)
    {
        cout << e.what() << "\n";
        return EXIT_FAILURE;
    }
    real_main(po);
    return EXIT_SUCCESS;
}
