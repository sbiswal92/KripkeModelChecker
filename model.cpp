
#include "model.h"
#include <stdlib.h>
#include <stdio.h>
#include <set>
#include <algorithm>
#include <iterator>
#include <iostream>


/*

  Student code here

*/
//Source state 

typedef std::pair<state_id, state_id> pairs;
typedef std::set<int> state_set;

//class state_set : public std::set<state_id>{};

class model_derived : public model
{
	private:
		std::set< pairs > state_src;
		int num_srcs;
		
	public:
	model_derived() : model()
	{
		num_srcs=0;	
			
	}

	void setNumStates(int n)
	{
		num_srcs=n;
	}
	
	bool isValidState(state_id s)
	{
		return (s>=0 && s<=num_srcs-1);
	}
	
	void addArc(state_id s1, state_id s2)
	{
	    state_src.insert(std::make_pair (s1,s2));
	}
	
	state_set* makeEmptySet()
	{
		state_set* t = new state_set();
		return t;
	}
	
	
	void deleteSet(state_set* sset)
	{
 		delete sset;	
	}
	
	void addState(state_id s, state_set* sset)
	{
		sset->insert(s);
	}
	
	void copy(const state_set* sset, state_set* rset) // Clears out rset and copies sset into rset
	{
	    state_set* temp= new state_set();
		temp->insert(sset->begin(),sset->end());
		rset->clear();
		rset->insert(temp->begin(),temp->end());
		
		delete temp;
	}
	
	void NOT(const state_set* sset, state_set* rset)
	{
		state_set* temp= new state_set();
	    
	    std::set<pairs> :: iterator it_model ;
		int tag=-1;
	    for(it_model=state_src.begin();it_model!=state_src.end();it_model++) // Insert those states from the KS into temp set which are not found in sset 
	    	{
	    		int f_item = (*it_model).first;
	    		if(f_item > tag)
	    		{	tag=f_item;
	    		     state_set:: iterator it = (*sset).find(tag);
	    		    if(it == sset->end())
	    			{
	    			  	(temp)->insert(tag);
	    			}
	    		}
	    	}
	    rset->clear();	
	    copy(temp,rset);
	    
	    delete temp;	
	   }
	
	bool elementOf(state_id s, const state_set* sset)
	{
		
		state_set :: iterator it_state  = sset->find(s);
		if(it_state != sset->end())
			return true;
		else
			return false;
		
	}
	
	bool finish() 
	{
		 std::set<pairs> :: iterator it_model ;
		 int tag=-1;
	 	 for (it_model=state_src.begin(); it_model!=state_src.end(); ++it_model) // Check if all the states have atleast one outgoing edge
	 	 {
	 		int frt = (*it_model).first;
	 		
	 		if(!isValidState(frt)) // Invalid state
	 		{
	 			std::cout<<"\nState "<< frt <<" does not lie between ["<< 0 <<","<<num_srcs-1<<"] \n";
				return false;
	 		
	 		}
	 		
	 		if(frt==tag+1) // At least one outgoing edge on each.
	 				tag = frt;
	 		else if(frt>tag+1)
	 		{
	 			std::cout<<"\nState "<< tag+1 <<" does not have any outgoing edge \n";
				return false;
		 	}
	 				
	 	}
 		
		return true;
			
	}
	 
	 void  OR(const state_set* sset1, const state_set* sset2, state_set* rset)
	{
		rset->insert(sset1->begin(),sset1->end());
		rset->insert(sset2->begin(),sset2->end());
				
	}
	 
	void AND(const state_set* sset1, const state_set* sset2, state_set* rset)
	{
	 	state_set* temp= new state_set();
	    state_set* rset_not_sset1 = new state_set() ;
	    state_set* rset_not_sset2 = new state_set() ;
	    state_set* rset_or = new state_set() ;
	    
	    NOT(sset1,rset_not_sset1); // !p
	    NOT(sset2,rset_not_sset2); // !q
		
		OR(rset_not_sset1,rset_not_sset2,rset_or); // !p || !q == !(p && q)
		NOT(rset_or,temp); // p && q
		
		rset->clear();
		copy(temp,rset);
		
		delete rset_not_sset1;
		delete rset_not_sset2;
		delete rset_or;
		delete temp;
	}
	 
	void IMPLIES(const state_set* sset1, const state_set* sset2, state_set* rset)
	{
	 	state_set* temp= new state_set();
	    state_set* rset_not_sset1 = new state_set();
	    
	    NOT(sset1,rset_not_sset1); // !p
		
		OR(rset_not_sset1,sset2,temp); // !p || q == p -> q

		rset->clear();
		copy(temp,rset);
		
		delete temp;
		delete rset_not_sset1;
	 }
	 
	void EX(const state_set* sset, state_set* rset) 
	{
		
		state_set* temp= new state_set();
	    state_set :: iterator it_state;
	   
	    for(int i = 0; i<num_srcs;i++)
	    {
	    	for(it_state=sset->begin();it_state!=sset->end();it_state++)
	    	{
	    		std::set<pairs> :: iterator it=state_src.find(std::make_pair (i,(*it_state)));
	    		if(it != state_src.end()) // if i is reachable to a state in sset in 1 step
	    			{
	    				temp->insert(i);
	    				break;
	    			}
	    	}
	    }
	    rset->clear();
	    copy(temp,rset);
	    
	    delete temp;
	}
	 
	void AX(const state_set* sset, state_set* rset) 
	{
		state_set* temp= new state_set();
	    state_set* rset_not_sset = new state_set();
	    state_set* rset_ex_not = new state_set();
	    
	    
	    NOT(sset,rset_not_sset); // !p
	    
	    EX(rset_not_sset,rset_ex_not); // EX !p
	    delete rset_not_sset;
	    
	    NOT(rset_ex_not,temp); // !EX !p = AX p	  
	    
	    rset->clear();
	    copy(temp,rset);
	    
	    delete rset_ex_not;
	    delete temp;  
	}
	 
	void EF(const state_set* sset, state_set* rset) 
	{
		state_set* temp= new state_set();
	    state_set* rset_true = new state_set();
	    
	    for(int i=0;i < num_srcs; i++) // true
	    	rset_true->insert(i);
	    	
	    EU(rset_true,sset,temp); // E tt U p
	    
	    rset->clear();
	    copy(temp,rset); 
	    
	    delete temp;
	    delete rset_true;
	}
	 
	void AF(const state_set* sset, state_set* rset) 
	{
    
	    state_set* temp= new state_set();
	    state_set temp1,temp2;
	    
	    copy(sset,temp);
	    temp2=*temp;
	   
	    state_set :: iterator it_state;
	    std::set<pairs> :: iterator it;
	    int i_new,flag;
	    
	    do // does until no further states can be marked
	    {
		 i_new = -1; // holds the state_id for which AF p is true
		 flag = -1; 
		 temp1 = temp2;
     	 for(it = state_src.begin(); it != state_src.end();it++) // Any state whose all successors are in AF p , is also in AF p
	     {
	        
	    	int s = (*it).first;
	    	int d = (*it).second;
	    	
			if((i_new != flag)&&(s>i_new))
				temp->insert(i_new);
		
			if(s == flag)
				continue;
			
			if(s>i_new)
				i_new=s;
			
			if((*temp).find(d) == (*temp).end())
					flag = i_new;
			 
	      }
	    
	      temp2=*temp; 
	     
	      }while(temp1 != temp2);	
			
	     rset->clear();
	      copy(temp,rset);
	      delete temp;
	  }
	
	void AG(const state_set* sset, state_set* rset) 
	{
		state_set* temp= new state_set();
	    state_set* rset_not_sset = new state_set();
	    
	    
	    NOT(sset,rset_not_sset); // !p
	    
	    state_set* rset_ef_not = new state_set();
	    EF(rset_not_sset,rset_ef_not); // EF !p
	    delete rset_not_sset;
	    
	    NOT(rset_ef_not,temp); // !EF !p = AG p
	    
	    std::cout<<"CHeck AG::";
	    display(temp);
	    rset->clear();
	    copy(temp,rset);
	    
	    delete temp;
	    delete rset_ef_not;
	}
	 
	void EG(const state_set* sset, state_set* rset)
	{
		state_set* temp= new state_set();
	    state_set* rset_not_sset = new state_set();
	    
	    
	    NOT(sset,rset_not_sset); // !p
	    
	    
	    state_set* rset_af_not = new state_set();
	    AF(rset_not_sset,rset_af_not); // AF !p
	    delete rset_not_sset;
	    
	    NOT(rset_af_not,temp); // !AF !p = EG p
	    rset->clear();
	    copy(temp,rset);
	    
	    delete temp;
	    delete rset_af_not;
	    
	}
	 
	void EU(const state_set* sset1, const state_set* sset2, state_set* rset) 
	{
	    state_set* temp= new state_set();
	    state_set temp1,temp2;
	    
	    copy(sset2,temp);  // Any state labelled with q is also E p U q
	    temp2=*temp;
	    

	  
	    state_set :: iterator it_state;
	    std::set<pairs> :: iterator it;
	    do // does until no further states can be marked
	    {
	     temp1 = temp2;
	     for(it_state = (*sset1).begin();it_state!=(*sset1).end();it_state++) // Any state in p whose some successor is in E p U q , is also in E p U q
	     {
	    	for(int j = 0;j < num_srcs;j++)
	    	{
	    		
	    		it=state_src.find(std::make_pair ((*it_state),j));
					if((*temp).find((*it).second) != (*temp).end())
	    			{
	    				temp->insert(*it_state);
	    				break;
	    			}
	    	}
	    	
	      }
	      temp2 = *temp;
	     }while(temp1!=temp2);
	     
	     rset->clear();   
	     display(temp);
	     copy(temp,rset);
	     display(rset);
	     delete temp;
	}
	 
	void AU(const state_set* sset1, const state_set* sset2, state_set* rset) 
	{
	 	state_set* temp= new state_set();
	    state_set* rset_not_sset1 = new state_set(); ; // !p
	    state_set* rset_not_sset2 = new state_set(); ; //!q
	    
	    
	    NOT(sset1,rset_not_sset1); // !p
	    NOT(sset2,rset_not_sset2); // !q
	    
	    state_set* rset_and_pnq = new state_set();; // p && !q
	    state_set* rset_and_npnq = new state_set(); ; // !p && !q
	   
	    AND(sset1,rset_not_sset2,rset_and_pnq); // p && !q
	    AND(rset_not_sset1,rset_not_sset2,rset_and_npnq); // !p && !q
	    
	    state_set* rset_eu = new state_set();
	    EU(rset_and_pnq,rset_and_npnq,rset_eu);
	    
	    delete rset_not_sset1;
	    delete rset_and_npnq;
	    delete rset_and_pnq;
	    
	    state_set* rset_eg_not = new state_set();
	    EG(rset_not_sset2,rset_eg_not); // EG !q
	    delete rset_not_sset2;
	    
	    state_set* rset_union = new state_set();
	    OR(rset_eu,rset_eg_not,rset_union); // rset_eg_not || rset_eu
	    NOT(rset_union,temp);
	    
	    rset->clear();
	    copy(temp,rset);
	    
	    delete rset_union;
	    delete rset_eg_not;
	    delete rset_eu;
	    delete temp;
	    
	    
	}
	
	 
	void display(const state_set* sset)
	{
	 	printf(":");
	 	state_set :: iterator it_state;
	 	for (it_state=(*sset).begin(); it_state!=(*sset).end(); ++it_state)
 			std::cout << (*it_state)<<"  ";
 		std::cout << "\n";
	}
};

model* makeEmptyModel(int debug_level)
{
  model* modelKS = new model_derived();
  
  return modelKS;
}

