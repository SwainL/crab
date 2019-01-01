#pragma once

#include <crab/common/types.hpp>
#include <crab/common/stats.hpp>
#include <crab/domains/abstract_domain.hpp>

#include <crab/domains/intervals.hpp>
#include <crab/domains/combined_domains.hpp>

namespace crab {
  namespace domains {

    // A wrapper for comparing two numerical domains up to intervals.
    // For debugging purposes.
    template<typename Domain1, typename Domain2>
    class diff_domain:
      public abstract_domain<typename Domain1::number_t, typename Domain1::varname_t,
			     diff_domain<Domain1, Domain2> > {
     public:

      typedef abstract_domain<typename Domain1::number_t, typename Domain1::varname_t,
			      diff_domain<Domain1, Domain2> >  abstract_domain_t;
      
      using typename abstract_domain_t::linear_expression_t;
      using typename abstract_domain_t::linear_constraint_t;
      using typename abstract_domain_t::linear_constraint_system_t;
      using typename abstract_domain_t::disjunctive_linear_constraint_system_t;       
      using typename abstract_domain_t::variable_t;
      using typename abstract_domain_t::number_t;
      using typename abstract_domain_t::varname_t;
      using typename abstract_domain_t::variable_vector_t;      

     private:
      
      typedef domain_product2<number_t,varname_t, Domain1, Domain2> domain_product2_t;
      
     public:

      typedef interval<number_t> interval_t;
      typedef diff_domain<Domain1, Domain2> diff_domain_t;
      
     private:
      
      domain_product2_t _product;
      
      diff_domain(domain_product2_t product): _product(product) { }

      typedef interval_domain<number_t,varname_t> interval_domain_t;

      template<typename Dom>
      class to_intervals {
	Dom m_inv;
      public:
	
	to_intervals(Dom &inv): m_inv(inv) {}
	
	interval_domain_t operator()() {
	  interval_domain_t res;
	  res += m_inv.to_linear_constraint_system();
	  return res;
	}
      };
      
      // check that to_intervals(second) <= to_intervals(first)
      // That is, second is always a refinement of first
      // FIXME: very expensive operation
      bool diff(int line) {
	_product.normalize();
	to_intervals<Domain1> inv1(_product.first());
	to_intervals<Domain2> inv2(_product.second());
	auto i1 = inv1();
	auto i2 = inv2();
	bool res = (i2 <= i1);
	if (!res) {
	  if (true) {
	    crab::outs () << "PRECISION LEAK at line " << line << "\n"
			  << "\t" << _product.first().getDomainName() << "=" << i1 << "\n"
			  << "\t" << _product.second().getDomainName() << "=" << i2 << "\n";
	  }
	}
	  
	return res;
      }

      // reduce second from first
      void reduce() {	
	linear_constraint_system_t csts = _product.first().to_linear_constraint_system();
	for (auto c: csts)
	 { _product.second() += c; }       
      }
      
     public:
      
      static diff_domain_t top() {
        return diff_domain_t(domain_product2_t::top());
      }
      
      static diff_domain_t bottom() {
        return diff_domain_t(domain_product2_t::bottom());
      }
      
     public:
      
      diff_domain(): _product() { }
      
      diff_domain(const diff_domain_t& other): 
	_product(other._product) {
	diff(__LINE__);	
      }
      
      diff_domain_t& operator=(const diff_domain_t& other) {
        if (this != &other) {
          this->_product = other._product;
	}
	diff(__LINE__);	  	
        return *this;
      }
      
      bool is_bottom() {
	//diff(__LINE__);	
        return this->_product.is_bottom();
      }
      
      bool is_top() {
	//diff(__LINE__);	
        return this->_product.is_top();
      }
      
      bool operator<=(diff_domain_t other) {
	//diff(__LINE__);	
        return (this->_product <= other._product);
      }
      
      bool operator==(diff_domain_t other) {
	//diff(__LINE__);	
        return (this->_product == other._product);
      }
      
      void operator|=(diff_domain_t other) {
	bool r1 = diff(__LINE__);	
        this->_product |= other._product;
	bool r2 = diff(__LINE__);
	if (r1 && !r2) {
	  CRAB_WARN("PRECISION LEAK of ",
		    _product.second().getDomainName(), " at line ", __LINE__);
	  reduce();
	}
      }
      
      diff_domain_t operator|(diff_domain_t other) {
	//diff(__LINE__);	
        return diff_domain_t(this->_product | other._product);
      }
      
      diff_domain_t operator&(diff_domain_t other) {
	//diff(__LINE__);	
        return diff_domain_t(this->_product & other._product);
      }
      
      diff_domain_t operator||(diff_domain_t other) {
	//diff(__LINE__);		
        diff_domain_t res (this->_product || other._product);
	return res;
      }
      
      template<typename Thresholds>
      diff_domain_t widening_thresholds (diff_domain_t other,
					 const Thresholds& ts) {
	//diff(__LINE__);	
        return diff_domain_t (this->_product.widening_thresholds (other._product, ts));
      }
      
      diff_domain_t operator&&(diff_domain_t other) {
	//diff(__LINE__);	
        return diff_domain_t(this->_product && other._product);
      }
      
      void assign(variable_t x, linear_expression_t e) {
	bool r1 = diff(__LINE__);	
        this->_product.assign(x, e);
	bool r2 = diff(__LINE__);
	if (r1 && !r2) {
	  CRAB_WARN("PRECISION LEAK of ",
		    _product.second().getDomainName(), " at line ", __LINE__);
	  reduce();
	}
	
      }
      
      void apply(operation_t op, variable_t x, variable_t y, variable_t z) {
	bool r1 = diff(__LINE__);	
        this->_product.apply(op, x, y, z);
	bool r2 = diff(__LINE__);
	if (r1 && !r2) {
	  CRAB_WARN("PRECISION LEAK of ",
		    _product.second().getDomainName(), " at line ", __LINE__);
	  reduce();	  
	}
	
      }
      
      void apply(operation_t op, variable_t x, variable_t y, number_t k) {
	bool r1 = diff(__LINE__);	
        this->_product.apply(op, x, y, k);
	bool r2 = diff(__LINE__);
	if (r1 && !r2) {
	  CRAB_WARN("PRECISION LEAK of ",
		    _product.second().getDomainName(), " at line ", __LINE__);
	  reduce();	  
	}
	
      }

      void set (variable_t v, interval_t x) {
	//diff(__LINE__);	
        this->_product.set(v, x);
      }
      
      interval_t operator[](variable_t v) {
	//diff(__LINE__);	
        return this->_product.second()[v];
      }
            
      virtual void backward_assign (variable_t x, linear_expression_t e,
				    diff_domain_t invariant)  {
	bool r1 = diff(__LINE__);	
        this->_product.backward_assign(x, e, invariant._product);
	bool r2 = diff(__LINE__);
	if (r1 && !r2) {
	  CRAB_WARN("PRECISION LEAK of ",
		    _product.second().getDomainName(), " at line ", __LINE__);
	  reduce();	  
	}
	
      }
      
      virtual void backward_apply (operation_t op,
				   variable_t x, variable_t y, number_t k,
				   diff_domain_t invariant)  {
	bool r1 = diff(__LINE__);	
        this->_product.backward_apply(op, x, y, k, invariant._product);
	bool r2 = diff(__LINE__);
	if (r1 && !r2) {
	  CRAB_WARN("PRECISION LEAK of ",
		    _product.second().getDomainName(), " at line ", __LINE__);
	  reduce();	  
	}
	
      }
      
      virtual void backward_apply(operation_t op,
				  variable_t x, variable_t y, variable_t z,
				  diff_domain_t invariant)  {
	bool r1 = diff(__LINE__);	
        this->_product.backward_apply(op, x, y, z, invariant._product);
	bool r2 = diff(__LINE__);
	if (r1 && !r2) {
	  CRAB_WARN("PRECISION LEAK of ",
		    _product.second().getDomainName(), " at line ", __LINE__);
	  reduce();	  
	}
	
      }
      
      void operator+=(linear_constraint_system_t csts) {
	bool r1 = diff(__LINE__);	
        this->_product += csts;
	bool r2 = diff(__LINE__);
	if (r1 && !r2) {
	  CRAB_WARN("PRECISION LEAK of ",
		    _product.second().getDomainName(), " at line ", __LINE__,
		    " with constraints ", csts);
	  if (true) {
	    to_intervals<Domain1> inv1(_product.first());
	    to_intervals<Domain2> inv2(_product.second());
	    auto i1 = inv1();
	    auto i2 = inv2();
	    crab::outs() << "\t" << _product.first().getDomainName() << "=" << i1 << "\n";
	    crab::outs() << "\t" << _product.second().getDomainName() << "=" << i2 << "\n";
	  }
	  reduce();
	}
	
      }
      
      void operator-=(variable_t v) {
	//bool r1 = diff(__LINE__);	
        this->_product -= v;
	//bool r2 = diff(__LINE__);
	// if (r1 && !r2) {
	//   CRAB_WARN("PRECISION LEAK of ",
	// 	    _product.second().getDomainName(), " at line ", __LINE__);
	//   reduce();
	// }
	
      }
      
      // cast_operators_api
      
      void apply(int_conv_operation_t op, variable_t dst, variable_t src) {
	bool r1= diff(__LINE__);	
        this->_product.apply(op, dst, src);
	bool r2 = diff(__LINE__);
	if (r1 && !r2) {
	  CRAB_WARN("PRECISION LEAK of ",
		    _product.second().getDomainName(), " at line ", __LINE__);
	  reduce();
	}
	
      }
      
      // bitwise_operators_api
      
      void apply(bitwise_operation_t op, variable_t x, variable_t y, variable_t z) {
	//bool r1= diff(__LINE__);	
        this->_product.apply(op, x, y, z);
	//bool r2= diff(__LINE__);
	// if (r1 && !r2) {
	//   CRAB_WARN("PRECISION LEAK of ",
	// 	    _product.second().getDomainName(), " at line ", __LINE__);
	//   reduce();
	// }
	
      }
      
      void apply(bitwise_operation_t op, variable_t x, variable_t y, number_t k) {
	//bool r1= diff(__LINE__);	
        this->_product.apply(op, x, y, k);
	//bool r2= diff(__LINE__);
	// if (r1 && !r2) {
	//   CRAB_WARN("PRECISION LEAK of ",
	// 	    _product.second().getDomainName(), " at line ", __LINE__);
	//   reduce();
	// }
	
      }
      
      // array_operators_api
      
      virtual void array_init (variable_t a,
			       linear_expression_t elem_size,
			       linear_expression_t lb_idx,
			       linear_expression_t ub_idx, 
			       linear_expression_t val) override {
	bool r1 = diff(__LINE__);	
        this->_product.array_init (a, elem_size, lb_idx, ub_idx, val);
	bool r2 = diff(__LINE__);
	if (r1 && !r2) {
	  CRAB_WARN("PRECISION LEAK of ",
		    _product.second().getDomainName(), " at line ", __LINE__);
	  reduce();
	}
	
      }
      
      virtual void array_load (variable_t lhs,
			       variable_t a, linear_expression_t elem_size,
                               linear_expression_t i) override {
	bool r1 = diff(__LINE__);        
        this->_product.array_load (lhs, a, elem_size, i);
	bool r2 = diff(__LINE__);
	if (r1 && !r2) {
	  CRAB_WARN("PRECISION LEAK of ",
		    _product.second().getDomainName(), " at line ", __LINE__);
	  reduce();
	}
	
      }
      
      virtual void array_store (variable_t a, linear_expression_t elem_size,
                                linear_expression_t i,
				linear_expression_t val, 
				bool is_singleton) override {
	bool r1 = diff(__LINE__);	
        this->_product.array_store (a, elem_size, i, val, is_singleton);
	bool r2 = diff(__LINE__);
	if (r1 && !r2) {
	  CRAB_WARN("PRECISION LEAK of ",
		    _product.second().getDomainName(), " at line ", __LINE__);
	  reduce();
	}
	
      }
      
      virtual void array_assign (variable_t lhs, variable_t rhs) override {
	bool r1 = diff(__LINE__);	
        this->_product.array_assign (lhs, rhs);
	bool r2 = diff(__LINE__);
	if (r1 && !r2) {
	  CRAB_WARN("PRECISION LEAK of ",
		    _product.second().getDomainName(), " at line ", __LINE__);
	  reduce();
	}
	
      }
      
      // Ignored pointer_operators_api

      // boolean operators
      virtual void assign_bool_cst (variable_t lhs, linear_constraint_t rhs) override {
	bool r1 = diff(__LINE__);	
        this->_product.assign_bool_cst (lhs, rhs);
	bool r2 = diff(__LINE__);
	if (r1 && !r2) {
	  CRAB_WARN("PRECISION LEAK of ",
		    _product.second().getDomainName(), " at line ", __LINE__,
		    " with ", lhs, ":=", rhs);
	  reduce();
	}
	
      }    

      virtual void assign_bool_var(variable_t lhs, variable_t rhs,
				   bool is_not_rhs) override {
	bool r1= diff(__LINE__);	
        this->_product.assign_bool_var (lhs, rhs, is_not_rhs);
	bool r2 = diff(__LINE__);
	if (r1 && !r2) {
	  CRAB_WARN("PRECISION LEAK of ",
		    _product.second().getDomainName(), " at line ", __LINE__,
		    " with ", lhs, ":=",(is_not_rhs ? " not": " "), rhs);
	  reduce();
	}
	
      }    

      virtual void apply_binary_bool (bool_operation_t op,variable_t x,
				      variable_t y,variable_t z) override {
	bool r1= diff(__LINE__);	
        this->_product.apply_binary_bool (op, x, y, z);
	bool r2 = diff(__LINE__);
	if (r1 && !r2) {
	  CRAB_WARN("PRECISION LEAK of ",
		    _product.second().getDomainName(), " at line ", __LINE__,
		    " with ", x, " := ", y, " ", op, " ", z);
	  reduce();
	}
	
      }    

      virtual void assume_bool (variable_t v, bool is_negated) override {
	bool r1 = diff(__LINE__);	
        this->_product.assume_bool (v, is_negated);
	bool r2 = diff(__LINE__);
	if (r1 && !r2) {
	  CRAB_WARN("PRECISION LEAK of ",
		    _product.second().getDomainName(), " at line ", __LINE__,
		    " with ", "assume", (is_negated? "(not ": "("), v, ")");
	  reduce();
	}
	
      }    

      // backward boolean operators
      virtual void backward_assign_bool_cst(variable_t lhs, linear_constraint_t rhs,
					    diff_domain_t inv){
	bool r1 = diff(__LINE__);	
        this->_product.backward_assign_bool_cst(lhs, rhs, inv._product);
	bool r2= diff(__LINE__);
	if (r1 && !r2) {
	  CRAB_WARN("PRECISION LEAK of ",
		    _product.second().getDomainName(), " at line ", __LINE__);
	  reduce();
	}
	
      }
	
      virtual void backward_assign_bool_var(variable_t lhs, variable_t rhs, bool is_not_rhs,
					    diff_domain_t inv) {
	bool r1 = diff(__LINE__);	
	this->_product.backward_assign_bool_var(lhs, rhs, is_not_rhs, inv._product);
	bool r2 = diff(__LINE__);
	if (r1 && !r2) {
	  CRAB_WARN("PRECISION LEAK of ",
		    _product.second().getDomainName(), " at line ", __LINE__);
	  reduce();
	}
	
      }
	
      virtual void backward_apply_binary_bool(bool_operation_t op,
					      variable_t x,variable_t y,variable_t z,
					      diff_domain_t inv) {
	bool r1= diff(__LINE__);	
	this->_product.backward_apply_binary_bool(op, x, y, z, inv._product);
	bool r2= diff(__LINE__);
	if (r1 && !r2) {
	  CRAB_WARN("PRECISION LEAK of ",
		    _product.second().getDomainName(), " at line ", __LINE__);
	  reduce();
	}
	
      }
      
      linear_constraint_system_t to_linear_constraint_system() {
	//diff(__LINE__);	
	linear_constraint_system_t csts;
	csts += this->_product.to_linear_constraint_system();
        return csts;
      }
      
      virtual void rename (const variable_vector_t& from,
			   const variable_vector_t& to) override {
	//bool r1= diff(__LINE__);	
        this->_product.rename(from, to);
	//bool r2= diff(__LINE__);
	// if (r1 && !r2) {
	//   CRAB_WARN("PRECISION LEAK of ",
	// 	    _product.second().getDomainName(), " at line ", __LINE__);
	//   reduce();
	// }
	
      }    
      
      void write(crab::crab_os& o) {
        this->_product.write(o);
      }
      
      static std::string getDomainName () { 
        return domain_product2_t::getDomainName ();
      }

      
      void expand(variable_t x, variable_t new_x) {
	//bool r1 = diff(__LINE__);
	this->_product.expand(x, new_x);
	// bool r2= diff(__LINE__);
	// if (r1 && !r2) {
	//   CRAB_WARN("PRECISION LEAK of ",
	// 	    _product.second().getDomainName(), " at line ", __LINE__);
	//   reduce();
	// }
	
      }
      
      void forget(const variable_vector_t& variables){
	//bool r1= diff(__LINE__);
	this->_product.forget(variables);
	// bool r2 = diff(__LINE__);
	// if (r1 && !r2) {
	//   CRAB_WARN("PRECISION LEAK of ",
	// 	    _product.second().getDomainName(), " at line ", __LINE__);
	//   reduce();
	// }
	
      }
      
      void project(const variable_vector_t& variables) {
	//bool r1 = diff(__LINE__);
	this->_product.project(variables);
	// bool r2 = diff(__LINE__);
	// if (r1 && !r2) {
	//   CRAB_WARN("PRECISION LEAK of ",
	// 	    _product.second().getDomainName(), " at line ", __LINE__);
	//   reduce();
	// }
      }

      void normalize() {
	this->_product.normalize();
      }
    }; // class diff_domain
    
    template<typename Domain1, typename Domain2>
    class domain_traits <diff_domain<Domain1, Domain2> > {
     public:
      typedef diff_domain<Domain1, Domain2> diff_domain_t;
      template<class CFG>
      static void do_initialization (CFG cfg) { }
    };
    
  } // end namespace domains
} // namespace crab

