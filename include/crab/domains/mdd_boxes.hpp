#pragma once 

#include <crab/config.h>
#include <crab/common/debug.hpp>
#include <crab/common/stats.hpp>
#include <crab/common/types.hpp>
#include <crab/domains/operators_api.hpp>
#include <crab/domains/domain_traits.hpp>
#include <crab/domains/intervals.hpp>

#ifndef HAVE_MDD
/*
 * Dummy implementation if Mdd not found 
 */
#define MDD_NOT_FOUND "No Mdd. Run cmake with -DUSE_MDD=ON"

namespace crab {
namespace domains {

    template<typename Number, typename VariableName>
    class mdd_boxes_domain: 
    public abstract_domain<Number, VariableName,
			   mdd_boxes_domain<Number,VariableName>> {
    public:
      typedef mdd_boxes_domain<Number, VariableName> mdd_boxes_domain_t;
      typedef abstract_domain<Number, VariableName, mdd_boxes_domain_t> abstract_domain_t;
      using typename abstract_domain_t::variable_t;
      using typename abstract_domain_t::number_t;
      using typename abstract_domain_t::varname_t;      
      using typename abstract_domain_t::linear_expression_t;
      using typename abstract_domain_t::linear_constraint_t;
      using typename abstract_domain_t::linear_constraint_system_t;
      typedef disjunctive_linear_constraint_system<number_t, varname_t>
      disjunctive_linear_constraint_system_t;
      typedef interval<number_t> interval_t;
      
      mdd_boxes_domain() {}    
      static mdd_boxes_domain_t top()    { CRAB_ERROR(MDD_NOT_FOUND); }
      static mdd_boxes_domain_t bottom() { CRAB_ERROR(MDD_NOT_FOUND); }
      mdd_boxes_domain(const mdd_boxes_domain_t& o) {}
      bool is_bottom() { CRAB_ERROR(MDD_NOT_FOUND); }
      bool is_top()    { CRAB_ERROR(MDD_NOT_FOUND); }
      bool operator<=(mdd_boxes_domain_t other) { CRAB_ERROR(MDD_NOT_FOUND); }
      void operator|=(mdd_boxes_domain_t other)
      { CRAB_ERROR(MDD_NOT_FOUND); }
      mdd_boxes_domain_t operator|(mdd_boxes_domain_t other)
      { CRAB_ERROR(MDD_NOT_FOUND); }
      mdd_boxes_domain_t operator&(mdd_boxes_domain_t other) 
      { CRAB_ERROR(MDD_NOT_FOUND); }
      mdd_boxes_domain_t operator||(mdd_boxes_domain_t other)
      { CRAB_ERROR(MDD_NOT_FOUND); }
      template<typename Thresholds>
      mdd_boxes_domain_t widening_thresholds(mdd_boxes_domain_t e, const Thresholds &ts) 
      { CRAB_ERROR(MDD_NOT_FOUND); }
      mdd_boxes_domain_t operator&&(mdd_boxes_domain_t other) 
      { CRAB_ERROR(MDD_NOT_FOUND); }
      void operator-=(variable_t var) { CRAB_ERROR (MDD_NOT_FOUND); } 
      interval_t operator[](variable_t v)  { CRAB_ERROR (MDD_NOT_FOUND); }
      void set(variable_t v, interval_t ival) { CRAB_ERROR (MDD_NOT_FOUND); } 
      void operator+=(linear_constraint_system_t csts) { CRAB_ERROR (MDD_NOT_FOUND); }
      void assign(variable_t x, linear_expression_t e) { CRAB_ERROR (MDD_NOT_FOUND); } 
      void apply(operation_t op, variable_t x, variable_t y, Number z) 
      { CRAB_ERROR(MDD_NOT_FOUND); }
      void apply(operation_t op, variable_t x, variable_t y, variable_t z) 
      { CRAB_ERROR(MDD_NOT_FOUND); }
      void apply(operation_t op, variable_t x, Number k) 
      { CRAB_ERROR(MDD_NOT_FOUND); }
      void apply(int_conv_operation_t op, variable_t dst, variable_t src) 
      { CRAB_ERROR(MDD_NOT_FOUND); }
      void apply(bitwise_operation_t op, variable_t x, variable_t y, variable_t z) 
      { CRAB_ERROR(MDD_NOT_FOUND); }
      void apply(bitwise_operation_t op, variable_t x, variable_t y, Number k) 
      { CRAB_ERROR(MDD_NOT_FOUND); }
      void apply(div_operation_t op, variable_t x, variable_t y, variable_t z) 
      { CRAB_ERROR(MDD_NOT_FOUND); }
      void apply(div_operation_t op, variable_t x, variable_t y, Number k) 
      { CRAB_ERROR(MDD_NOT_FOUND); }
      void backward_assign (variable_t x, linear_expression_t e, mdd_boxes_domain_t invariant) 
      { CRAB_ERROR(MDD_NOT_FOUND); }
      void backward_apply (operation_t op,
			   variable_t x, variable_t y, Number z, mdd_boxes_domain_t invariant) 
      { CRAB_ERROR(MDD_NOT_FOUND); }
      void backward_apply(operation_t op,
			 variable_t x, variable_t y, variable_t z, mdd_boxes_domain_t invariant) 
      { CRAB_ERROR(MDD_NOT_FOUND); }
      linear_constraint_system_t to_linear_constraint_system () { CRAB_ERROR(MDD_NOT_FOUND); }
      disjunctive_linear_constraint_system_t to_disjunctive_linear_constraint_system ()
      { CRAB_ERROR(MDD_NOT_FOUND); }
      void write(crab_os& o) { CRAB_ERROR(MDD_NOT_FOUND); } 
      static std::string getDomainName () { return "Dummy Mdd-boxes"; }
    }; 
} // namespace domains
}// namespace crab
#else

/* 
 *  Real implementation starts here 
 */

#include "include/MDD.hh"
#include "include/MDD.hpp"
#include "include/mdd_builtin_cache.hh"
#include "include/mdd_ext_cache.hh"
#include "include/MDD_ops.hh"
#include "include/MDD_vis.hh"
#include "include/MDD_visit.hh"
#include "include/MDD_arith.hh"
#include "include/interval.hh"
#include "util/hc-list.h"

#include <boost/optional.hpp>
#include <boost/bimap.hpp>
#include <boost/range/iterator_range.hpp>
#include "boost/range/algorithm/set_algorithm.hpp"
#include <set>
#include <vector>

namespace crab {
namespace domains {
namespace mdd_boxes_impl {

struct int64_repr {
  int64_repr(void) : x(INT64_MIN) { }
  int64_repr(int64_t _x) : x(_x) { }
  bool operator==(const int64_repr& o) const { return x == o.x; }

  bool is_finite(void) const { return x != INT64_MIN; }
  int64_repr operator+(int64_t r) const { return int64_repr(x+r); }
  int64_repr operator*(int64_t c) const { return int64_repr(x*c); }
  bool operator<(const int64_repr& o) const { return x < o.x; }

  static int64_repr minus_infty(void) { return INT64_MIN; }
  static int64_t succ(int64_t k) { return k+1; }
  static int64_t pred(int64_t k) { return k-1; }
  int64_t value(void) const { return x; }
  
  int64_t x;
};
  
}
}  
}

namespace std {
  template<>
  struct hash<crab::domains::mdd_boxes_impl::int64_repr> {
    size_t operator()(const crab::domains::mdd_boxes_impl::int64_repr& r) { return r.x; }
  };
};

namespace crab {
  namespace domains {
    
    template<typename N>
    static mdd_boxes::mdd_mgr<N>& create_mdd_man(void) {
      static mdd_boxes::mdd_mgr<N> man;
      return man;
    }
    
    template<typename Number, typename VariableName>
    class mdd_boxes_domain:
      public abstract_domain<Number,VariableName,
			     mdd_boxes_domain<Number,VariableName>> {
    
      typedef mdd_boxes_domain<Number, VariableName> mdd_boxes_domain_t;
      typedef abstract_domain<Number, VariableName, mdd_boxes_domain_t> abstract_domain_t;
    
    public:
      using typename abstract_domain_t::variable_t;
      using typename abstract_domain_t::number_t;
      using typename abstract_domain_t::varname_t;
      using typename abstract_domain_t::variable_vector_t;	      
      using typename abstract_domain_t::linear_expression_t;
      using typename abstract_domain_t::linear_constraint_t;
      using typename abstract_domain_t::linear_constraint_system_t;
      typedef disjunctive_linear_constraint_system<number_t, varname_t>
      disjunctive_linear_constraint_system_t;
      typedef interval<number_t> interval_t;
    
    private:
      typedef interval_domain<number_t, varname_t> interval_domain_t;
      
      /// -- MDD basic typedefs
      // TODO: make mdd_number_t an user parameter
      typedef int64_t mdd_number_t;
      typedef mdd_boxes_impl::int64_repr mdd_bound_t; 
      typedef mdd_boxes::num_interval<mdd_number_t, mdd_bound_t> mdd_interval_t;
      typedef mdd_boxes::mdd_mgr<mdd_number_t>  mdd_mgr_t;
      typedef mdd_boxes::mdd_node<mdd_number_t> mdd_node_t;
      typedef mdd_boxes::mdd_ref<mdd_number_t>  mdd_ref_t;

      // map crab variables to mdd variables
      typedef unsigned int mdd_var_t;
      typedef boost::bimap<variable_t, mdd_var_t> var_map_t;
      typedef typename var_map_t::value_type binding_t;

      /// -- MDD typedefs for transformers
      typedef ::vec<mdd_var_t> mdd_var_vector;
      typedef mdd_boxes::linterm<mdd_number_t> linterm_t;
      typedef ::vec<linterm_t> linterm_vector;
      template<class Op>
      // parameterized mdd transformer
      using mdd_transformer_t = mdd_boxes::mdd_transformer<mdd_number_t, Op>;
      typedef mdd_boxes::MDD_ops<mdd_number_t> mdd_op_t;
      // rename
      typedef mdd_boxes::var_pair rename_pair_t;
      typedef ::vec<rename_pair_t> renaming_vector;
      typedef mdd_boxes::mdd_rename<mdd_number_t, mdd_bound_t> _mdd_rename_t;
      typedef mdd_transformer_t<_mdd_rename_t> mdd_rename_t;
      // add linear leq
      typedef mdd_boxes::mdd_lin_leq<mdd_number_t, mdd_bound_t> _mdd_lin_leq_t;
      typedef mdd_transformer_t<_mdd_lin_leq_t> mdd_lin_leq_t;
      // eval linear expression
      typedef mdd_boxes::mdd_eval_linexpr<mdd_number_t, mdd_bound_t, mdd_interval_t> _mdd_eval_linexpr_t;
      typedef mdd_transformer_t<_mdd_eval_linexpr_t> mdd_linex_eval_t;
      // assign interval
      typedef mdd_boxes::mdd_assign_interval<mdd_number_t, mdd_interval_t> _mdd_assign_interval_t;
      typedef mdd_transformer_t<_mdd_assign_interval_t> mdd_assign_interval_t;
      // assign linear expression
      typedef mdd_boxes::mdd_assign_linexpr<mdd_number_t, mdd_bound_t, mdd_interval_t> _mdd_assign_linexpr_t;
      typedef mdd_transformer_t<_mdd_assign_linexpr_t> mdd_assign_linexpr_t;
      // assign multiplication
      typedef mdd_boxes::mdd_assign_prod<mdd_number_t, mdd_bound_t, mdd_interval_t> _mdd_assign_prod_t;
      typedef mdd_transformer_t<_mdd_assign_prod_t> mdd_assign_prod_t;
      // assign division
      typedef mdd_boxes::mdd_assign_div<mdd_number_t, mdd_bound_t, mdd_interval_t> _mdd_assign_div_t;
      typedef mdd_transformer_t<_mdd_assign_div_t> mdd_assign_div_t;

      // reference to a mdd node
      mdd_ref_t m_state;
      // map between crab variables and mdd variables
      var_map_t m_var_map;
      
      // return the mdd manager
      mdd_mgr_t* get_man(void) { return &create_mdd_man<mdd_number_t>(); }
      
      static boost::optional<mdd_var_t> get_mdd_var(const var_map_t& m, variable_t v) {
	auto it = m.left.find (v);
	if (it != m.left.end ())
	  return it->second;
	else
	  return boost::optional<mdd_var_t>();
      }

      static mdd_var_t get_mdd_var_insert(var_map_t& m, variable_t v) {
	if (auto opt_mdd_v = get_mdd_var(m, v)) {
	  return *opt_mdd_v;
	} else {
	  mdd_var_t mdd_v = m.size ();
	  m.insert(binding_t (v, mdd_v));
	  return mdd_v;
	}
      }
                  
      static bool has_crab_var(const var_map_t& m, mdd_var_t i) {
	return m.right.find(i) != m.right.end ();
      }

      static variable_t get_crab_var(const var_map_t& m, mdd_var_t i) {
	auto it = m.right.find (i);
	if (it != m.right.end ()) {
	  return it->second;
	}
	CRAB_ERROR ("mdd internal variable id ", i, " is not used!");
      }

      
      boost::optional<mdd_var_t> get_mdd_var(variable_t v) const {
	return get_mdd_var(m_var_map, v);
      }

      mdd_var_t get_mdd_var_insert(variable_t v) {
	return get_mdd_var_insert(m_var_map, v);
      }
      
      bool has_crab_var(mdd_var_t i) const {
	return has_crab_var(m_var_map, i);
      }
            
      variable_t get_crab_var(mdd_var_t i) const {
	return get_crab_var(m_var_map, i);
      }

      var_map_t merge_var_map (const var_map_t& vm1, mdd_ref_t& m1,
			       const var_map_t& vm2, mdd_ref_t& m2) {

	// -- collect all vars from the two maps
	std::set<variable_t> vars;
	for (auto const& p: vm1.left){
	  vars.insert (p.first);
	}
	for (auto const& p: vm2.left) {
	  vars.insert (p.first);
	}
	
	// -- create a fresh map 
	var_map_t res;
	for (auto v: vars) {
	  mdd_var_t i = res.size ();
	  res.insert(binding_t(v, i));
	}

	// -- build the renaming maps	
	renaming_vector rv1, rv2;
	for (auto const &p: vm1.left) {
	  mdd_var_t mdd_var_id = res.left.at(p.first);
	  rv1.push(rename_pair_t { p.second, mdd_var_id });
	}
	for (auto const &p: vm2.left) {
	  mdd_var_t mdd_var_id = res.left.at(p.first);
	  rv2.push(rename_pair_t { p.second, mdd_var_id });
	}

	CRAB_LOG("mdd-boxes-merge-vars",
		 crab::outs () << "Renaming map:\n";
		 for(unsigned i=0, e=rv1.size();i<e;++i) {
		   crab::outs () << "\t" << rv1[i].from << " --> "
				 << rv1[i].to << "\n";
		 }
		 crab::outs () << "Renaming map:\n";
		 for(unsigned i=0, e=rv2.size();i<e;++i) {
		   crab::outs () << "\t" << rv2[i].from << " --> "
				 << rv2[i].to << "\n";
		 });


	auto hrv1 = hc::lookup(rv1);
	assert(hrv1);
	auto hrv2 = hc::lookup(rv2);
	assert(hrv2);
	
	mdd_ref_t ren_m1(get_man(), mdd_rename_t::apply(get_man(), m1.get(), hrv1));
	mdd_ref_t ren_m2(get_man(), mdd_rename_t::apply(get_man(), m2.get(), hrv2));
							
	std::swap(m1, ren_m1);
	std::swap(m2, ren_m2);
	
	return res;
      }

      
      // --- from crab to mdd

      static void convert_crab_number(ikos::z_number n, mdd_number_t &res){
	if (n.fits_slong()) {
	  res = (long) n;
	} else {
	  CRAB_ERROR(n.get_str(), " does not fit into a mdd number");
	}
      }
      static void convert_crab_number(ikos::q_number n, mdd_number_t &res)
      { CRAB_ERROR("mdd-boxes not implemented for rationals");}
      static void convert_crab_number(ikos::z_number n, ikos::z_number &res) 
      { std::swap(res, n); }
      static void convert_crab_number(ikos::q_number n, ikos::q_number &res)
      { std::swap(res, n); }
      static void convert_crab_interval(interval<ikos::z_number> i, mdd_interval_t& res) {
	if (i.is_bottom()) {
	  res = mdd_interval_t::empty();
	} else if (i.is_top()) {
	  res = mdd_interval_t::top();	  
	} else if (i.lb().is_finite() && i.ub().is_finite()) {
	  mdd_number_t x, y;
	  convert_crab_number(*(i.lb().number()), x);
	  convert_crab_number(*(i.ub().number()), y);
	  res = mdd_interval_t::range(x, y);
	} else if (!i.lb().is_finite()) {
	  mdd_number_t y;
	  convert_crab_number(*(i.ub().number()), y);
	  res = mdd_interval_t::ge(y);
	} else if (!i.ub().is_finite()) {
	  mdd_number_t x;
	  convert_crab_number(*(i.lb().number()), x);
	  res = mdd_interval_t::le(x);	  
	} else {
	  CRAB_ERROR("cannot convert to mdd interval");
	}
      }
      static void convert_crab_interval(interval<ikos::q_number> i, mdd_interval_t& res)
      { CRAB_ERROR("mdd-boxes not implemented for rationals");}	
      
      std::pair<linterm_vector, mdd_number_t> expr2linterms(const linear_expression_t& e) const {
	linterm_vector linterms;
	for (auto p: e) {
	  mdd_number_t c;
	  convert_crab_number(p.first, c);
	  boost::optional<mdd_var_t> v = get_mdd_var(p.second);
	  if (!v) CRAB_ERROR(p.second, " is not in the mdd.");
	  linterms.push(linterm_t {c, *v});
	}
	mdd_number_t cst;
	convert_crab_number(e.constant(), cst);
	return {linterms, cst} ;
      }
      
      // --- from mdd to crab 
      static void convert_mdd_number(mdd_number_t n, ikos::z_number& res)
      { res = ikos::z_number(n); }
      static void convert_mdd_number(mdd_number_t n, ikos::q_number& res) 
      { CRAB_ERROR("mdd-boxes not implemented for rationals");}
      static void convert_mdd_interval(mdd_interval_t i, interval<ikos::z_number>& res) {
	if (i.is_empty()) {
	  res = interval<ikos::z_number>::bottom();
	} else if (i.is_top()) { 
	  res = interval<ikos::z_number>::top();
	} else if (i.ub_is_finite() && i.lb_is_finite()) {
	  ikos::z_number lb, ub;
	  convert_mdd_number(i.lb(), lb);
	  convert_mdd_number(i.ub(), ub);
	  interval<ikos::z_number> r(lb, ub -1);
	  std::swap(res, r);
	} else if (!i.ub_is_finite()) {
	  // return [i.lb, +oo]
	  ikos::z_number lb;
	  convert_mdd_number(i.lb(), lb);
	  interval_t r(lb);
	  res = r.upper_half_line();
	} else if (!i.lb_is_finite()) {
	  // return [-oo, i.ub()]
	  ikos::z_number ub;
	  convert_mdd_number(i.ub(), ub);
	  interval_t r(ub);
	  res = r.lower_half_line();
	} else {
	  CRAB_ERROR("cannot convert from mdd interval");
	}
      }
      static void convert_mdd_interval(mdd_interval_t i, interval<ikos::q_number>& res)
      { CRAB_ERROR("mdd-boxes not implemented for rationals");}	

      
      // Convert a MDD to DNF
      class mdd_to_dnf {
	typedef std::vector<std::pair<mdd_node_t*, mdd_interval_t>> stack_t;
	disjunctive_linear_constraint_system_t& m_csts;	
	const var_map_t& m_var_map; 

	void to_linear_constraint_system(variable_t v, interval_t i,
					 linear_constraint_system_t& csts) {
	  // -- add constraints v >= lb and v <= ub
	  auto lb = i.lb ();
	  if (lb.is_finite ())  {
	    // v >= lb <--> -v + lb <= 0
	    assert (lb.number ());
	    linear_expression_t e = (Number(-1) * v) + *(lb.number ());
	    csts += (linear_constraint_t (e, linear_constraint_t::kind_t::INEQUALITY));
	  }
	  auto ub = i.ub ();
	  if (ub.is_finite ()) {
	    // v <= ub <--> v - ub <= 0
	    assert (ub.number ());
	    linear_expression_t e = (v - *(ub.number ()));
	    csts += (linear_constraint_t (e, linear_constraint_t::kind_t::INEQUALITY));
	  }
	}
		
	void mdd_to_dnf_rec(mdd_node_t* n, stack_t& stack) {
	  
	  if (n == mdd_op_t::MDD_TRUE) {
	    // -- end of the path: we convert the path into a crab
	    // -- linear constraint system.
	    linear_constraint_system_t path;
	    for (auto p: stack) {
	      auto it = m_var_map.right.find(p.first->var);
	      if (it == m_var_map.right.end()) {
		CRAB_ERROR("cannot convert mdd var");
	      }
	      variable_t v = it->second;
	      interval_t i = interval_t::top();
	      convert_mdd_interval(p.second, i);
	      to_linear_constraint_system(v, i, path);
	    }
	    m_csts += path;
	    return;
	  }
	  
	  auto it = n->edges.begin();
	  auto en = n->edges.end();
	  if(n->dest0 != mdd_op_t::MDD_FALSE) {
	    // -- there is an edge from n to dest0 with interval (-oo, (*it).lb]
	    mdd_interval_t i = mdd_interval_t::le((*it).lb);
	    stack.push_back({n, i});
	    mdd_to_dnf_rec(n->dest0, stack);
	    stack.pop_back();      
	  }
	  
	  mdd_number_t lb((*it).lb);
	  mdd_node_t* dest((*it).dest);    
	  for(++it; it != en; ++it) {
	    if(dest != mdd_op_t::MDD_FALSE) {
	      // -- there is an edge from n to dest with interval [lb, (*it).lb)
	      mdd_interval_t i = mdd_interval_t::range(lb, (*it).lb);
	      stack.push_back({n, i});	
	      mdd_to_dnf_rec(dest, stack);
	      stack.pop_back();
	    }
	    lb = (*it).lb;
	    dest = (*it).dest;
	  }
	  
	  if (dest != mdd_op_t::MDD_FALSE) {
	    // -- there is an edge from n to dest with interval [(*it).lb, +oo)
	    mdd_interval_t i = mdd_interval_t::ge((*it).lb); // DOUBLE CHECK: isn't lb?
	    stack.push_back({n, i});      
	    mdd_to_dnf_rec(dest, stack);
	    stack.pop_back();
	  }
	}

      public:
	
	mdd_to_dnf(disjunctive_linear_constraint_system_t& csts, const var_map_t& varmap)
	  : m_csts(csts), m_var_map(varmap) { }
	
	void operator()(mdd_node_t* n) {
	  m_csts.clear();
	  if(n == mdd_op_t::MDD_FALSE) {
	    m_csts += linear_constraint_t::get_false();
	  } else if(n == mdd_op_t::MDD_TRUE) {
	    // do nothing
	  } else {
	    stack_t stack;
	    mdd_to_dnf_rec(n, stack);
	  }
	}
      };

      // adapted from split_dbm.hpp
      void unitcsts_of_lin_leq(const linear_expression_t& exp, 
			       std::vector<std::pair<variable_t, number_t>>& lbs,
			       std::vector<std::pair<variable_t, number_t>>& ubs) {
	
        number_t unbounded_lbcoeff;
        number_t unbounded_ubcoeff;
        boost::optional<variable_t> unbounded_lbvar;
        boost::optional<variable_t> unbounded_ubvar;
        number_t exp_ub = - (exp.constant());
        std::vector<std::pair<std::pair<number_t,variable_t>, number_t>> pos_terms;
        std::vector<std::pair<std::pair<number_t,variable_t>, number_t>> neg_terms;
        for(auto p : exp) {
          number_t coeff(p.first);
          if(coeff > number_t(0)) {
            variable_t y(p.second);
	    // evaluate the variable in the domain
            auto y_lb = this->operator[](y).lb();
            if(y_lb.is_infinite()) {
              if(unbounded_lbvar)
                goto diffcst_finish;
              unbounded_lbvar = y;
              unbounded_lbcoeff = coeff;
            } else {
              number_t ymin(*(y_lb.number()));
              // Coeff is negative, so it's still add
              exp_ub -= ymin*coeff;
              pos_terms.push_back({{coeff, y}, ymin});
            }
          } else {
            variable_t y(p.second);
	    // evaluate the variable in the domain	    
            auto y_ub = this->operator[](y).ub(); 
            if(y_ub.is_infinite()) {
	      if(unbounded_ubvar)
                goto diffcst_finish;
              unbounded_ubvar = y;
              unbounded_ubcoeff = -(coeff);
            } else {
              number_t ymax(*(y_ub.number()));
              exp_ub -= ymax*coeff;
              neg_terms.push_back({{-coeff, y}, ymax});
            }
          }
        }

        if(unbounded_lbvar) {
          if(!unbounded_ubvar) {
            // Add bounds for x
	    variable_t x(*unbounded_lbvar);
            ubs.push_back({x, exp_ub/unbounded_lbcoeff});
          }
        } else {
          if(unbounded_ubvar) {
            // Bounds for y
            variable_t y(*unbounded_ubvar);
            lbs.push_back({y, -exp_ub/unbounded_ubcoeff});
          } else {
            for(auto pl : neg_terms)
              lbs.push_back({pl.first.second, -exp_ub/pl.first.first + pl.second});
            for(auto pu : pos_terms)
              ubs.push_back({pu.first.second, exp_ub/pu.first.first + pu.second});
          }
        }
      diffcst_finish:
        return;
      }
      
      bool add_linear_leq(const linear_expression_t& e) {
	#if 1
	auto p = expr2linterms(e);
	linterm_vector xs(p.first);
	mdd_number_t k = -p.second;
	auto hxs = hc::lookup(xs);
	assert(hxs);
	m_state = mdd_ref_t(get_man(),
			    mdd_lin_leq_t::apply(get_man(), m_state.get(), hxs, k));
	return !is_bottom();
	#else
        std::vector<std::pair<variable_t, number_t>> lbs;
        std::vector<std::pair<variable_t, number_t>> ubs;
	linear_expression_t::unitcsts_of_linexpr(e, lbs, ubs);
	for (auto p: lbs) {
          CRAB_LOG("mdd-boxes-add-leq",
                   crab::outs() << "add constraint " << p.first<< ">="<< p.second <<"\n\t"
		                << *this << "\n";);

	  mdd_var_t v = get_mdd_var_insert(p.first);
	  mdd_number_t k;
	  convert_crab_number(p.second, k);
	  linterm_vector xs;
	  xs.push(linterm_t {-1, v});
	  auto hxs = hc::lookup(xs);
	  assert(hxs);
	  m_state = mdd_ref_t(get_man(),
			      mdd_lin_leq_t::apply(get_man(), m_state.get(), hxs, -k));
	  if (is_bottom()) {
	    return false;
	  }
	}

	for (auto p: ubs) {
          CRAB_LOG("mdd-boxes-add-leq",
                   crab::outs() << "add constraint " << p.first<< "<="<< p.second <<"\n\t"
		                << *this << "\n";);

	  mdd_var_t v = get_mdd_var_insert(p.first);
	  mdd_number_t k;
	  convert_crab_number(p.second, k);
	  linterm_vector xs;
	  xs.push(linterm_t {1, v});
	  auto hxs = hc::lookup(xs);
	  assert(hxs);
	  m_state = mdd_ref_t(get_man(),
			      mdd_lin_leq_t::apply(get_man(), m_state.get(), hxs, k));
	  if (is_bottom()) {
	    return false;
	  }
	}
	return true;
	#endif 
      }
	      
    private:
      
      mdd_boxes_domain(mdd_ref_t state, var_map_t varmap):
	m_state(state), m_var_map(varmap) {
      }
      
      mdd_boxes_domain(mdd_ref_t&& state, var_map_t&& varmap):
      	m_state(std::move(state)), m_var_map(std::move(varmap)) {
      }
      
    public:

      mdd_boxes_domain(bool is_bottom = false):
	m_state(is_bottom ?
		get_man()->mdd_false() :
		get_man()->mdd_true()) { }

      mdd_boxes_domain(const mdd_boxes_domain_t& o): 
	  m_state(o.m_state)
	, m_var_map(o.m_var_map) {
	crab::CrabStats::count (getDomainName() + ".count.copy");
	crab::ScopedCrabStats __st__(getDomainName() + ".copy");
      }

      mdd_boxes_domain(mdd_boxes_domain_t&& o): 
	  m_state(std::move(o.m_state))
      	, m_var_map(std::move(o.m_var_map)) { } 
        

      mdd_boxes_domain_t& operator=(const mdd_boxes_domain_t& o) {
      	crab::CrabStats::count (getDomainName() + ".count.copy");
      	crab::ScopedCrabStats __st__(getDomainName() + ".copy");
      	if (this != &o) {
	  m_state = o.m_state;
      	  m_var_map = o.m_var_map;
      	}
      	return *this;
      }

      mdd_boxes_domain_t& operator=(mdd_boxes_domain_t&& o) {
      	if (this != &o) {
      	  m_state = std::move(o.m_state);
      	  m_var_map = std::move(o.m_var_map);
      	}
      	return *this;
      }
        
      static mdd_boxes_domain_t top() { 
	return mdd_boxes_domain_t(false);
      }

      static mdd_boxes_domain_t bottom() { 
	return mdd_boxes_domain_t(true);
      }

      bool is_bottom() { 
	return m_state.get() == get_man()->mdd_false().get();
      }

      bool is_top() { 
	return m_state.get() == get_man()->mdd_true().get();
      }

      bool operator<=(mdd_boxes_domain_t o) { 
	crab::CrabStats::count (getDomainName() + ".count.leq");
	crab::ScopedCrabStats __st__(getDomainName() + ".leq");

	if (is_bottom()) { 
	  return true;
	} else if(o.is_bottom()) {
	  return false;
	} else if (o.is_top ()) {
	  return true;
	} else if (is_top () && !o.is_top ()) {
	  return false;
	} else if (is_top () && o.is_top ()) {
	  return true;
	} else {
	  return mdd_op_t::leq(get_man(), m_state.get(), o.m_state.get());
	}
      }

      void operator|=(mdd_boxes_domain_t o) {
	crab::CrabStats::count (getDomainName() + ".count.join");
	crab::ScopedCrabStats __st__(getDomainName() + ".join");
	if (is_bottom() || o.is_top ()) {
	  *this = o;
	} else if (is_top () || o.is_bottom()) {
	  return ;
	} else {
	  m_var_map = merge_var_map(m_var_map, m_state, o.m_var_map, o.m_state);
	  m_state = mdd_ref_t(get_man(),
	  		      mdd_op_t::join(get_man(), m_state.get(), o.m_state.get()));
	}
      }
        
      mdd_boxes_domain_t operator|(mdd_boxes_domain_t o) {
	crab::CrabStats::count (getDomainName() + ".count.join");
	crab::ScopedCrabStats __st__(getDomainName() + ".join");

	if (is_bottom() || o.is_top ()) {
	  return o;
	} else if (is_top () || o.is_bottom()) {
	  return *this;
	} else {
	  var_map_t m = merge_var_map(m_var_map, m_state, o.m_var_map, o.m_state);
	  mdd_ref_t r(get_man(), mdd_op_t::join(get_man(), m_state.get(), o.m_state.get()));
	  return mdd_boxes_domain_t(r, m);
	}
      }        
        
      mdd_boxes_domain_t operator&(mdd_boxes_domain_t o) {
	crab::CrabStats::count (getDomainName() + ".count.meet");
	crab::ScopedCrabStats __st__(getDomainName() + ".meet");

	if (is_bottom() || o.is_bottom()) {
	  return bottom();
	} else if (is_top()) {
	  return o;
	} else if (o.is_top()) {
	  return *this;
	} else{
	  var_map_t m = merge_var_map(m_var_map, m_state, o.m_var_map, o.m_state);
	  mdd_ref_t r(get_man(), mdd_op_t::meet(get_man(), m_state.get(), o.m_state.get()));
	  return mdd_boxes_domain_t(r, m);
	}
      }        
        
      mdd_boxes_domain_t operator||(mdd_boxes_domain_t o) {
	crab::CrabStats::count (getDomainName() + ".count.widening");
	crab::ScopedCrabStats __st__(getDomainName() + ".widening");

	if (is_bottom()) {
	  return o;
	} else if (o.is_bottom()) {
	  return *this;
	} else {
	  var_map_t m = merge_var_map(m_var_map, m_state, o.m_var_map, o.m_state);
	  mdd_ref_t r(get_man(), mdd_op_t::widen(get_man(), m_state.get(), o.m_state.get()));
	  return mdd_boxes_domain_t(r, m);	  
	}
      }        

      template<typename Thresholds>
      mdd_boxes_domain_t widening_thresholds(mdd_boxes_domain_t o, const Thresholds &ts) {
	crab::CrabStats::count (getDomainName() + ".count.widening");
	crab::ScopedCrabStats __st__(getDomainName() + ".widening");

	if (is_bottom()) {
	  return o;
	} else if (o.is_bottom()) {
	  return *this;
	} else {
	  // TODO: consider thresholds
	  var_map_t m = merge_var_map(m_var_map, m_state, o.m_var_map, o.m_state);
	  mdd_ref_t r(get_man(), mdd_op_t::widen(get_man(), m_state.get(), o.m_state.get()));
	  return mdd_boxes_domain_t(r, m);	  
	}
      }

      // narrowing replace with meet: watch out for infinite descending iterations.
      mdd_boxes_domain_t operator&&(mdd_boxes_domain_t o) {
	return *this & o;
      }        

      template<typename VarRange>
      void forget(const VarRange &vars) {
	crab::CrabStats::count (getDomainName() + ".count.forget");
	crab::ScopedCrabStats __st__(getDomainName() + ".forget");

	if (is_bottom() || is_top()) {
	  return;
	}
	
	mdd_var_vector pvars;
	std::set<mdd_var_t> pvars_set;
	for(auto v: vars) {
	  if (auto mdd_v = get_mdd_var(v)) {
	    pvars.push(*mdd_v);
	    pvars_set.insert(*mdd_v);
	  }
	}

	if (pvars.size() == 0) {
	  return;
	}
	
	m_state = mdd_ref_t(get_man(),
			    mdd_op_t::forget(get_man(), m_state.get(),
					     pvars.begin(), pvars.end()));
	
	var_map_t res;
	for (auto const& p: m_var_map.right) {  
	  if (pvars_set.count (p.first) <= 0) {
	    mdd_var_t i = res.size ();
	    res.insert(binding_t(p.second, i));
	  }
	}
	
	std::swap (m_var_map, res);
      }

      void operator-=(variable_t var) {
	std::vector<variable_t> vars({var});
	forget(vars);
      }
      
      // remove all variables except vars
      template<typename VarRange>
      void project(const VarRange& vars) {
	crab::CrabStats::count (getDomainName() + ".count.project");
	crab::ScopedCrabStats __st__(getDomainName() + ".project");

	if (is_bottom ()) return;
	std::set<variable_t> s1,s2,s3;
	for (auto p: m_var_map.left) s1.insert (p.first);
	s2.insert (vars.begin (), vars.end ());
	boost::set_difference (s1,s2,std::inserter (s3, s3.end ()));
	forget(s3);
      }

      interval_t operator[](variable_t v) {
	crab::CrabStats::count (getDomainName() + ".count.to_intervals");
	crab::ScopedCrabStats __st__(getDomainName() + ".to_intervals");

	if (is_bottom ()) {
	  return interval_t::bottom ();
	}

	// p.second must be 0	
	auto p = expr2linterms(v);
	auto hlinterm = hc::lookup(p.first);
	assert(hlinterm);
	mdd_interval_t i = mdd_linex_eval_t::apply(get_man(), m_state.get(), hlinterm);
						   
						   
	interval_t res = interval_t::top();
	convert_mdd_interval(i, res);
	return res;
      }

      void set(variable_t v, interval_t ival) {
	crab::CrabStats::count (getDomainName() + ".count.assign");
	crab::ScopedCrabStats __st__(getDomainName() + ".assign");

	if (is_bottom ()) return;

	mdd_var_t mdd_v = get_mdd_var_insert(v);
	mdd_interval_t mdd_i;
	convert_crab_interval(ival, mdd_i);

        m_state = mdd_ref_t(get_man(),
			    mdd_assign_interval_t::apply(get_man(),
							 m_state.get(), mdd_v, mdd_i));
	
	CRAB_LOG("mdd-boxes",
		 crab::outs () << v << ":=" << ival << "--->\n\t" << *this << "\n";);
      }

      void assign(variable_t v, linear_expression_t e) {
	crab::CrabStats::count (getDomainName() + ".count.assign");
	crab::ScopedCrabStats __st__(getDomainName() + ".assign");

	if(is_bottom()) return;

	mdd_var_t mdd_v = get_mdd_var_insert(v);
	auto p = expr2linterms(e);
	linterm_vector linterms = p.first;
	if (linterms.size() == 0) {
	  // v := constant
	  auto k = mdd_interval_t::cst(p.second);
	  m_state = mdd_ref_t(get_man(),
			      mdd_assign_interval_t::apply(get_man(),
							   m_state.get(), mdd_v, k));
	} else {
	  // v := linexpr
	  mdd_number_t k = p.second;
	  auto hlinterms = hc::lookup(linterms);
	  assert(hlinterms);
	  m_state = mdd_ref_t(get_man(),
			      mdd_assign_linexpr_t::apply(get_man(),
							  m_state.get(), mdd_v, hlinterms, k));
	}
	CRAB_LOG("mdd-boxes",
		 crab::outs() << v << ":=" << e << "--->\n\t" << *this << "\n";);	
      }
                
      void operator+=(linear_constraint_system_t csts) {
	crab::CrabStats::count (getDomainName() + ".count.add_constraints");
	crab::ScopedCrabStats __st__(getDomainName() + ".add_constraints");

	if(is_bottom()) return;

	if (csts.is_false()) {
	  *this = bottom();
	  return;
	}

	// XXX: filter out unsigned linear inequalities and disequalities
	for (auto const& c: csts) {
	  if (c.is_inequality() && c.is_unsigned()) {
	    CRAB_WARN("unsigned inequality skipped in mdd-boxes domain");
	    continue;
	  }
	  
	  if (c.is_disequation()) {
	    // TODO: disequality
	    continue;
	  } else if (c.is_inequality()) {
	    if (!add_linear_leq(c.expression())) {
	      break;
	    }
	  } else if (c.is_equality()) {
	    linear_expression_t exp = c.expression();
	    if(!add_linear_leq(exp) || !add_linear_leq(-exp)) {
	      break;
	    }
	  }
	}

        CRAB_LOG("mdd-boxes",
                 crab::outs() << csts << " ---> \n\t" << *this <<"\n");
      }
       
      void apply (operation_t op, variable_t x, variable_t y, Number z) {
	crab::CrabStats::count (getDomainName() + ".count.apply");
	crab::ScopedCrabStats __st__(getDomainName() + ".apply");

	if(is_bottom()) return;

	linear_expression_t e;
	switch (op){
	case OP_ADDITION:
	  e = y + z;
	  break;
	case OP_SUBTRACTION:
	  e = y - z;
	  break;
	case OP_MULTIPLICATION:
	  CRAB_WARN("multiplication not implemented in mdd-boxes domain");
	  // TODO mul
	case OP_DIVISION:
	  CRAB_WARN("division not implemented in mdd-boxes domain");
	  // TODO div
	default:
	  //CRAB_ERROR("mdd-boxes operation not supported");	  
	  *this -= x;
	  goto apply_end;
	}
	assign(x, e);

      apply_end:
	CRAB_LOG("mdd-boxes",
	 	 crab::outs() << x << ":= "<< y<< " " << op << " " << z << " --> "
		              << *this <<"\n";);
      }
        
      void apply(operation_t op, variable_t x, variable_t y, variable_t z) {
	crab::CrabStats::count (getDomainName() + ".count.apply");
	crab::ScopedCrabStats __st__(getDomainName() + ".apply");

	if(is_bottom()) return;

	linear_expression_t e;
	switch (op){
	case OP_ADDITION:
	  e = y + z;
	  break;
	case OP_SUBTRACTION:
	  e = y - z;
	  break;
	case OP_MULTIPLICATION:
	  CRAB_WARN("multiplication not implemented in mdd-boxes domain");
	  // TODO mul
	case OP_DIVISION:
	  CRAB_WARN("division not implemented in mdd-boxes domain");
	  // TODO div
	default:
	  //CRAB_ERROR("mdd-boxes operation not supported");
	  *this -= x;
	  goto apply_end;
	}
	assign(x, e);

      apply_end:
	CRAB_LOG("mdd-boxes",
	 	 crab::outs() << x << ":= "<< y<< " " << op << " " << z << " --> "
		              << *this <<"\n";);
      }
        
      void apply(int_conv_operation_t op, variable_t dst, variable_t src) {
	// since reasoning about infinite precision we simply assign and
	// ignore the widths.
	assign(dst, src);
      }

      void apply(bitwise_operation_t op, variable_t x, variable_t y, variable_t z) {
	crab::CrabStats::count (getDomainName() + ".count.apply");
	crab::ScopedCrabStats __st__(getDomainName() + ".apply");

	// Convert to intervals and perform the operation
	interval_t yi = operator[](y);
	interval_t zi = operator[](z);
	interval_t xi = interval_t::top();
	switch (op) {
	case OP_AND: xi = yi.And(zi); break;
	case OP_OR: xi = yi.Or(zi); break;
	case OP_XOR: xi = yi.Xor(zi); break; 
	case OP_SHL: xi = yi.Shl(zi); break; 
	case OP_LSHR: xi = yi.LShr(zi); break;
	case OP_ASHR: xi = yi.AShr(zi); break;
	default: CRAB_ERROR("mdd-boxes operation not supported");
	}
	set(x, xi);
      }
        
      void apply(bitwise_operation_t op, variable_t x, variable_t y, Number k) {
	crab::CrabStats::count (getDomainName() + ".count.apply");
	crab::ScopedCrabStats __st__(getDomainName() + ".apply");

	// Convert to intervals and perform the operation
	interval_t yi = operator[](y);
	interval_t zi(k);
	interval_t xi = interval_t::top();
	switch (op) {
	case OP_AND: xi = yi.And(zi); break;
	case OP_OR: xi = yi.Or(zi); break;
	case OP_XOR: xi = yi.Xor(zi); break; 
	case OP_SHL: xi = yi.Shl(zi); break; 
	case OP_LSHR: xi = yi.LShr(zi); break;
	case OP_ASHR: xi = yi.AShr(zi); break;
	default: CRAB_ERROR("mdd-boxes operation not supported");
	}
	set(x, xi);
      }
        
      void apply(div_operation_t op, variable_t x, variable_t y, variable_t z) {
	crab::CrabStats::count (getDomainName() + ".count.apply");
	crab::ScopedCrabStats __st__(getDomainName() + ".apply");

	if (op == OP_SDIV){
	  apply(OP_DIVISION, x, y, z);
	}
	else {
	  // Convert to intervals and perform the operation
	  interval_t yi = operator[](y);
	  interval_t zi = operator[](z);
	  interval_t xi = interval_t::top ();
            
	  switch (op) {
	  case OP_UDIV: xi = yi.UDiv(zi); break;
	  case OP_SREM: xi = yi.SRem(zi); break;
	  case OP_UREM: xi = yi.URem(zi); break;
	  default: CRAB_ERROR("mdd-boxes operation not supported");
	  }
	  set(x, xi);
	}
      }
        
      void apply(div_operation_t op, variable_t x, variable_t y, Number k) {
	crab::CrabStats::count (getDomainName() + ".count.apply");
	crab::ScopedCrabStats __st__(getDomainName() + ".apply");

	if (op == OP_SDIV){
	  apply(OP_DIVISION, x, y, k);
	}
	else {
	  // Convert to intervals and perform the operation
	  interval_t yi = operator[](y);
	  interval_t zi(k);
	  interval_t xi = interval_t::top ();
	  switch (op) {
	  case OP_UDIV: xi = yi.UDiv(zi); break;
	  case OP_SREM: xi = yi.SRem(zi); break;
	  case OP_UREM: xi = yi.URem(zi); break;
	  default: CRAB_ERROR("mdd-boxes operation not supported");
	  }
	  set(x, xi);
	}
      }

      void backward_assign (variable_t x, linear_expression_t e,
			    mdd_boxes_domain_t invariant) {
	crab::CrabStats::count (getDomainName() + ".count.backward_assign");
	crab::ScopedCrabStats __st__(getDomainName() + ".backward_assign");
	CRAB_WARN("backward assign not implemented in mdd-boxes domain");
      }
          
      void backward_apply (operation_t op,
			   variable_t x, variable_t y, Number z,
			   mdd_boxes_domain_t invariant) {
	crab::CrabStats::count (getDomainName() + ".count.backward_apply");
	crab::ScopedCrabStats __st__(getDomainName() + ".backward_apply");
	CRAB_WARN("backward apply not implemented in mdd-boxes domain");
      }
        
      void backward_apply(operation_t op,
			  variable_t x, variable_t y, variable_t z,
			  mdd_boxes_domain_t invariant)  {
	crab::CrabStats::count (getDomainName() + ".count.backward_apply");
	crab::ScopedCrabStats __st__(getDomainName() + ".backward_apply");
	CRAB_WARN("backward apply not implemented in mdd-boxes domain");	
      }
        	
      linear_constraint_system_t to_linear_constraint_system () {
	linear_constraint_system_t csts;
	
	if(is_bottom ())  {
	  csts += linear_constraint_t::get_false();
	} else if(is_top ()) {
	  csts += linear_constraint_t::get_true();
	} else {
	  CRAB_WARN("to_linear_constraint_system is not implemented");
	  // TODO to_linear_constraint_system
	  // use mdd_lower_bounds and mdd_upper_bounds or convexify
	}
	return csts;
      }

      void rename(const variable_vector_t &from, const variable_vector_t &to) {
	if (is_top () || is_bottom()) return;

	CRAB_WARN("rename operation not implemented in mdd-boxes");
	// TODO rename
	
	// // renaming m_var_map by creating a new map 
	// CRAB_LOG("mdd-boxes",
	// 	 crab::outs() << "Replacing {";
	// 	 for (auto v: from) crab::outs() << v << ";";
	// 	 crab::outs() << "} with ";
	// 	 for (auto v: to) crab::outs() << v << ";";
	// 	 crab::outs() << "}:\n";
	// 	 crab::outs() << *this << "\n";);
	
	// var_map_t new_var_map;
	// for (auto kv: m_var_map.left) {
	//   ptrdiff_t pos = std::distance(from.begin(),
	// 			 std::find(from.begin(), from.end(), kv.first));
	//   if (pos < (int) from.size()) {
	//     new_var_map.insert(binding_t(to[pos], kv.second));
	//   } else {
	//     new_var_map.insert(binding_t(kv.first, kv.second));
	//   }
	// }
	// std::swap(m_var_map, new_var_map);

	// CRAB_LOG("mdd-boxes",
	// 	 crab::outs () << "RESULT=" << *this << "\n");
      }
	
      void expand (variable_t x, variable_t dup) {
	if (is_bottom() || is_top()) return;

	CRAB_WARN("expand operation not implemented in mdd-boxes");	
	// TODO expand
      
	// if (get_var_dim(dup)) {
	//   CRAB_ERROR("expand second parameter ", dup,
	// 	       " cannot be already a variable in the apron domain ", *this);
	// }
      
	// // --- increases number of dimensions by one
	// auto dim_x = get_var_dim_insert (x);
	// m_apstate = apPtr (get_man(),
	//                    ap_abstract0_expand(get_man (), false, &* m_apstate, 
	//                                        dim_x, 1));
      
	// // --- the additional dimension is put at the end of integer
	// //     dimensions.
	// m_var_map.insert (binding_t (dup, get_dims () - 1));
      }
    
      disjunctive_linear_constraint_system_t to_disjunctive_linear_constraint_system() {
	disjunctive_linear_constraint_system_t res;
	mdd_to_dnf(res, m_var_map);
	return res;
      }
      
      void write(crab_os& o) {
	if(is_bottom()){
	  o << "_|_";
	} else if (is_top()){
	  o << "{}";
	} else {
	  auto csts = to_disjunctive_linear_constraint_system();
	  o << csts;
	}
      }          
    
      static std::string getDomainName () { return "Mdd-boxes"; }

    }; 

    // -- domain traits
    template<typename Number, typename VariableName>
    class domain_traits<mdd_boxes_domain<Number, VariableName>> {
    public:
        
      typedef mdd_boxes_domain<Number, VariableName> mdd_boxes_domain_t;
      typedef ikos::variable<Number, VariableName> variable_t;
	
      template<class CFG>
      static void do_initialization(CFG cfg) {}

      static void normalize(mdd_boxes_domain_t& inv) {}
	
      template <typename Iter>
      static void forget(mdd_boxes_domain_t& inv, Iter it, Iter end) {
	inv.forget(boost::make_iterator_range (it, end));
      }
        
      template <typename Iter>
      static void project(mdd_boxes_domain_t& inv, Iter it, Iter end) {
	inv.project(boost::make_iterator_range (it, end));
      }
	
      static void expand(mdd_boxes_domain_t& inv, variable_t x, variable_t new_x) {
	inv.expand(x, new_x);
      }		
    };
  } // namespace domains
}// namespace crab
#endif 