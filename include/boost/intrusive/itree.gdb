#
# Additions to the boost_print python package to allow printing of interval trees.
#
# Copyright (C) 2014 Matei David, Ontario Institute for Cancer Research
#
# Load this from gdb with:
#   (gdb) source <file>
#
python
assert 'boost_print' in sys.modules, 'boost_print not found'

#from boost_print import *
#from boost_print.utils import _add_to_dict

# resolve itree_value_traits::node_traits
#
@boost_print.utils._add_to_dict(boost_print.inner_type,
                                ('boost::intrusive::detail::itree_value_traits', 'node_traits'))
def __aux_itree_inner_type(vtt):
    return gdb.lookup_type(
        'boost::intrusive::detail::itree_node_traits<' + str(vtt.template_argument(0)) + '>')

# resolve itree_value_traits::to_value_ptr
#   pass call to real value_traits
#
@boost_print.utils._add_to_dict(boost_print.static_method,
                                ('boost::intrusive::detail::itree_value_traits', 'to_value_ptr'))
def __aux_itree_to_value_ptr(vtt, *args):
    return boost_print.call_static_method(vtt.fields()[0].type, 'to_value_ptr', *args)

# resolve itree_node_traits::get_parent
#   pass call to real node_traits
#
@boost_print.utils._add_to_dict(boost_print.static_method,
                                ('boost::intrusive::detail::itree_node_traits', 'get_parent'))
def __aux_itree_get_parent(t, *args):
    return boost_print.call_static_method(t.fields()[0].type, 'get_parent', *args)

# resolve itree_node_traits::get_left
#   pass call to real node_traits
#
@boost_print.utils._add_to_dict(boost_print.static_method,
                                ('boost::intrusive::detail::itree_node_traits', 'get_left'))
def __aux_itree_get_left(t, *args):
    return boost_print.call_static_method(t.fields()[0].type, 'get_left', *args)

# resolve itree_node_traits::get_right
#   pass call to real node_traits
#
@boost_print.utils._add_to_dict(boost_print.static_method,
                                ('boost::intrusive::detail::itree_node_traits', 'get_right'))
def __aux_itree_get_right(t, *args):
    return boost_print.call_static_method(t.fields()[0].type, 'get_right', *args)
