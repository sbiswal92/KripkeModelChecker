#ifndef __MODEL_H__
#define __MODEL_H__

#include <set>
#include <stdlib.h>

typedef int state_id;

/**
  Class used to store subsets of states of a Kripke structure.
  
  Students must define this as they see fit, to support the required
  model checking operations.
*/
//class state_set;

typedef std::set<int> state_set;

class model;  // see below

/**

  Returns a new and empty model.

  Students must provide this function, and normally it should
  return a new instance of the derived class of class model.
*/
model* makeEmptyModel(int debug_level);

/**

  Abstract class - interface for models and model checking operations.

  Students must derive a class from this one, and provide all the
  virtual functions.  These are called as needed by the parser.

*/
class model {


  	
  public:
    // Empty constructor.
    model() { }

    // Empty destructor (required).
    virtual ~model() {  }

    /**
        Finalize the Kripke structure.
        The parser will call this after all arcs have been processed,
        in case any post-processing must be done on the data structure
        used to store the Kripke structure.
        This method will be called exactly once by the parser.
          @return true on success, false on failure (stop parsing).
    */
    virtual bool finish() = 0;
	
	/**
        Set the number of states in the model.
        This method is called once by the parser,
        before any arcs are added to the model,
        and before the call to finish().
        
          @param  n   Number of states in the Kripke structure.
                      Once set, valid states are from 0 to n-1.
    */
    virtual void setNumStates(int n) = 0;

    /**
        Check if the given state is valid.
        Will be called after setNumStates().

          @param  s   State id to check
          @return     True, iff s is within range 0..n-1.
    */
    virtual bool isValidState(state_id s) = 0;

    /**
        Add an arc in the Kripke structure,
        from state s1 to state s2.
        Will not be called after finish() has been called.

          @param  s1    Source state id
          @param  s2    Destination state id

    */
    virtual void addArc(state_id s1, state_id s2) = 0;

    /**
        Create a new, empty, state_set for this model.
    */
    virtual state_set* makeEmptySet() = 0;
    /**
        Destroy a state_set for this model.
    */
    virtual void deleteSet(state_set* sset) = 0;

    /**
        Add a state to a state_set.
        The parser calls this for state labels.
        
          @param  s     State id to add
          @param  sset  State_set to add s into
    */
    virtual void addState(state_id s, state_set* sset) = 0;

    /**
        Copy a set of states.
        
          @param  sset    Source set of states.
          @param  rset    Destination set of states;
                          on return of this method, set rset
                          should contain the same states as set sset.
    */
    virtual void copy(const state_set* sset, state_set* rset) = 0;

    //
    // Unary operations
    //

    /**
        Take the complement of a set of states.
          @param  sset  On input: a set of states Y.
          @param  rset  On output: complement of Y is stored here.
                        Note that rset and sset may point to the
                        same object!
    */
    virtual void NOT(const state_set* sset, state_set* rset) = 0;

    /**
        Labeling for EX.
          @param  sset  On input: a set of states satisfying p.
          @param  rset  On output: the set of states satisfying EX p
                        is stored here.
                        Note that rset and sset may point to the 
                        same object!
    */
    virtual void EX(const state_set* sset, state_set* rset) = 0;

    /**
        Labeling for EF.
          @param  sset  On input: a set of states satisfying p.
          @param  rset  On output: the set of states satisfying EF p
                        is stored here.
                        Note that rset and sset may point to the 
                        same object!
    */
    virtual void EF(const state_set* sset, state_set* rset) = 0;

    /**
        Labeling for EG.
          @param  sset  On input: a set of states satisfying p.
          @param  rset  On output: the set of states satisfying EG p
                        is stored here.
                        Note that rset and sset may point to the 
                        same object!
    */
    virtual void EG(const state_set* sset, state_set* rset) = 0;

    /**
        Labeling for AX.
          @param  sset  On input: a set of states satisfying p.
          @param  rset  On output: the set of states satisfying AX p
                        is stored here.
                        Note that rset and sset may point to the 
                        same object!
    */
    virtual void AX(const state_set* sset, state_set* rset) = 0;

    /**
        Labeling for AF.
          @param  sset  On input: a set of states satisfying p.
          @param  rset  On output: the set of states satisfying AF p
                        is stored here.
                        Note that rset and sset may point to the 
                        same object!
    */
    virtual void AF(const state_set* sset, state_set* rset) = 0;

    /**
        Labeling for AG.
          @param  sset  On input: a set of states satisfying p.
          @param  rset  On output: the set of states satisfying AG p
                        is stored here.
                        Note that rset and sset may point to the 
                        same object!
    */
    virtual void AG(const state_set* sset, state_set* rset) = 0;

    //
    // Binary operations
    //

    /**
        Labeling for ^ (and).

          @param  sset1   On input: a set of states satisfying p.
          @param  sset2   On input: a set of states satisfying q.
          @param  rset    On output: the set of states satisfying p ^ q
                          is stored here.
                          Note that pointers sset1, sset2, and rset
                          may be the same!
    */
    virtual void AND(const state_set* sset1, const state_set* sset2, state_set* rset) = 0;

    /**
        Labeling for v (or).

          @param  sset1   On input: a set of states satisfying p.
          @param  sset2   On input: a set of states satisfying q.
          @param  rset    On output: the set of states satisfying p v q
                          is stored here.
                          Note that pointers sset1, sset2, and rset
                          may be the same!
    */
    virtual void OR(const state_set* sset1, const state_set* sset2, state_set* rset) = 0;

    /**
        Labeling for -> (implies).

          @param  sset1   On input: a set of states satisfying p.
          @param  sset2   On input: a set of states satisfying q.
          @param  rset    On output: the set of states satisfying p -> q
                          is stored here.
                          Note that pointers sset1, sset2, and rset
                          may be the same!
    */
    virtual void IMPLIES(const state_set* sset1, const state_set* sset2, state_set* rset) = 0;

    /**
        Labeling for EU.

          @param  sset1   On input: a set of states satisfying p.
          @param  sset2   On input: a set of states satisfying q.
          @param  rset    On output: the set of states satisfying E p U q
                          is stored here.
                          Note that pointers sset1, sset2, and rset
                          may be the same!
    */
    virtual void EU(const state_set* sset1, const state_set* sset2, state_set* rset) = 0;

    /**
        Labeling for AU.

          @param  sset1   On input: a set of states satisfying p.
          @param  sset2   On input: a set of states satisfying q.
          @param  rset    On output: the set of states satisfying A p U q
                          is stored here.
                          Note that pointers sset1, sset2, and rset
                          may be the same!
    */
    virtual void AU(const state_set* sset1, const state_set* sset2, state_set* rset) = 0;

    /**
        Check if a state is contained in a set.

          @param  s     State id to check
          @param  sset  Set of states

          @return true  iff state s is contained in set sset.
    */
    virtual bool elementOf(state_id s, const state_set* sset) = 0;

    /**
        Display all states contained in a set to standard output.
        Output should be a comma separated list of state ids, in order,
        contained in the set.
        For example, output should be
          "{}"      for an empty set,
          "{S42}"   for a set containing state id 42,
          "{S0, S1, S7}"  for a set containing state ids 0, 1, and 7.

          @param  sset    Set to display.
    **/
    virtual void display(const state_set* sset) = 0;
    
    
};

#endif
