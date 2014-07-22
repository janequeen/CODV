/*
Copyright (c) 1997, 1998 Carnegie Mellon University.  All Rights
Reserved. 

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:

1. Redistributions of source code must retain the above copyright notice,
this list of conditions and the following disclaimer.
2. Redistributions in binary form must reproduce the above copyright notice,
this list of conditions and the following disclaimer in the documentation
and/or other materials provided with the distribution.
3. The name of the author may not be used to endorse or promote products
derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF
ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

The AODV code developed by the CMU/MONARCH group was optimized and tuned by Samir Das and Mahesh Marina, University of Cincinnati. The work was partially done in Sun Microsystems.
*/


#ifndef __aodv_rtable_h__
#define __aodv_rtable_h__

#include <assert.h>
#include <sys/types.h>
#include <config.h>
#include <lib/bsd-list.h>
#include <scheduler.h>

#define CURRENT_TIME    Scheduler::instance().clock()
#define INFINITY2        0xff

/*
   AODV Neighbor Cache Entry
*/
class AODV_Neighbor {
        friend class AODV;
        friend class aodv_rt_entry;
 public:
        AODV_Neighbor(u_int32_t a) { nb_addr = a; }

 protected:
        LIST_ENTRY(AODV_Neighbor) nb_link;
        nsaddr_t        nb_addr;
        double          nb_expire;      // ALLOWED_HELLO_LOSS * HELLO_INTERVAL
};

LIST_HEAD(aodv_ncache, AODV_Neighbor);

/*
   AODV Precursor list data structure
*/
class AODV_Precursor {
        friend class AODV;
        friend class aodv_rt_entry;
 public:
        AODV_Precursor(u_int32_t a) { pc_addr = a; }

 protected:
        LIST_ENTRY(AODV_Precursor) pc_link;
        nsaddr_t        pc_addr;	// precursor address
};

LIST_HEAD(aodv_precursors, AODV_Precursor);


/*
  Route Table Entry
*/

class aodv_rt_entry {
        friend class aodv_rtable;
        friend class AODV;
	friend class LocalRepairTimer;
 public:
        aodv_rt_entry();
        ~aodv_rt_entry();

        void            nb_insert(nsaddr_t id);
        AODV_Neighbor*  nb_lookup(nsaddr_t id);

        void            pc_insert(nsaddr_t id);
        AODV_Precursor* pc_lookup(nsaddr_t id);
        void 		pc_delete(nsaddr_t id);
        void 		pc_delete(void);
        bool 		pc_empty(void);

        double          rt_req_timeout;         // when I can send another req
        u_int8_t        rt_req_cnt;             // number of route requests
	
 protected:
        LIST_ENTRY(aodv_rt_entry) rt_link;

	

        nsaddr_t        rt_dst;
        u_int32_t       rt_seqno;
	/* u_int8_t 	rt_interface; */
        u_int16_t       rt_hops;       		// hop count
	int 		rt_last_hop_count;	// last valid hop count
        nsaddr_t        rt_nexthop;    		// next hop IP address
	/* list of precursors */ 
        aodv_precursors rt_pclist;
        double          rt_expire;      		// when entry expires
        u_int8_t        rt_flags;
	u_int32_t       piece_hash;//qi add


#define RTF_DOWN 0
#define RTF_UP 1
#define RTF_IN_REPAIR 2

        /*
         *  Must receive 4 errors within 3 seconds in order to mark
         *  the route down.
        u_int8_t        rt_errors;      // error count
        double          rt_error_time;
#define MAX_RT_ERROR            4       // errors
#define MAX_RT_ERROR_TIME       3       // seconds
         */

#define MAX_HISTORY	3
	double 		rt_disc_latency[MAX_HISTORY];
	char 		hist_indx;
        int 		rt_req_last_ttl;        // last ttl value used
	// last few route discovery latencies
	// double 		rt_length [MAX_HISTORY];
	// last few route lengths

        /*
         * a list of neighbors that are using this route.
         */
        aodv_ncache          rt_nblist;
};


/*
  The Routing Table
*/

class aodv_rtable {
 public:
	aodv_rtable() { LIST_INIT(&rthead); }

        aodv_rt_entry*       head() { return rthead.lh_first; }

	aodv_rt_entry*       rt_add(nsaddr_t id);
        void                 rt_delete(nsaddr_t id);
        aodv_rt_entry*       rt_lookup(nsaddr_t id);
///qi add start
        aodv_rt_entry*       rt_add(u_int32_t piece_hash);
        void                 rt_delete(u_int32_t piece_hash);
        aodv_rt_entry*       rt_lookup(u_int32_t piece_hash);
//qi add end

 private:
        LIST_HEAD(aodv_rthead, aodv_rt_entry) rthead;
};

//qi add start for interest table

class AODV_InterestSrc {
        friend class AODV;
        friend class aodv_in_entry;
 public:
        AODV_InterestSrc(u_int32_t a) { insrc_addr = a; }

 protected:
        LIST_ENTRY(AODV_InterestSrc) insrc_link;
        nsaddr_t       	insrc_addr;//从哪个邻居节点收到的interest分组
        double          in_expire;      // ALLOWED_HELLO_LOSS * HELLO_INTERVAL
};

LIST_HEAD(aodv_insrc, AODV_InterestSrc);

class aodv_in_entry {
        friend class aodv_intable;
        friend class AODV;	
 public:
        aodv_in_entry();
        ~aodv_in_entry();

        void            insrc_insert(nsaddr_t insrc_addr);
        AODV_InterestSrc*  insrc_lookup(nsaddr_t insrc_addr);
	void 		insrc_delete(nsaddr_t insrc_addr);       

        double          in_req_timeout;         // when I can send another interest req
        u_int8_t        in_req_cnt;             // number of interest requests
	
 protected:
        LIST_ENTRY(aodv_in_entry) int_link;

        u_int32_t       in_hash;        
        
        double          int_expire;     		// when entry expires

       	aodv_insrc    int_nblist;//请求该hash的节点列表
};

class aodv_intable {
 public:
	aodv_intable() { LIST_INIT(&inthead); }

        aodv_in_entry*       head() { return inthead.lh_first; }

        aodv_in_entry*       int_add(u_int32_t in_hash,nsaddr_t id);
        void                 int_delete(u_int32_t in_hash,nsaddr_t id);
        aodv_in_entry*       int_lookup(u_int32_t in_hash,nsaddr_t id);
	
	
        void                 int_delete(u_int32_t in_hash);
        aodv_in_entry*       int_lookup(u_int32_t in_hash);

 private:
        LIST_HEAD(aodv_inthead, aodv_in_entry) inthead;
};

//qi add end for interest Entry
////////////////////////////////////////////////////
class AODV_CacheReqSrc {
        friend class AODV;
        friend class aodv_cache_entry;
 public:
        AODV_CacheReqSrc(u_int32_t a) { casrc_addr = a; }

 protected:
        LIST_ENTRY(AODV_CacheReqSrc) casrc_link;
        nsaddr_t       	casrc_addr; //谁请求了该片段，可以作为统计用
        double          ca_expire;      // 片段失效的时间
};

LIST_HEAD(aodv_cachereqsrc, AODV_CacheReqSrc);

class aodv_cache_entry {
        friend class aodv_cachetable;
        friend class AODV;	
 public:
        aodv_cache_entry();
        ~aodv_cache_entry();

        void            casrc_insert(nsaddr_t casrc_addr);
        AODV_CacheReqSrc*  casrc_lookup(nsaddr_t casrc_addr);
	void 		casrc_delete(nsaddr_t casrc_addr);       

        double          in_req_timeout;         // when I can send another req
        u_int8_t        rt_req_cnt;             // number of route requests
	
 protected:
        LIST_ENTRY(aodv_cache_entry) cache_link;

        u_int32_t       cache_hash;        
        
        double          cache_expire;     		// when entry expires

       	aodv_cachereqsrc    cache_nblist;//请求该hash的节点列表
};

class aodv_cachetable {
 public:
	aodv_cachetable() { LIST_INIT(&cthead); }

        aodv_cache_entry*       head() { return cthead.lh_first; }

        aodv_cache_entry*       cache_add(u_int32_t cache_hash);
        void                  cache_delete(u_int32_t cache_hash);
        aodv_cache_entry*       cache_lookup(u_int32_t cache_hash);

 private:
        LIST_HEAD(aodv_cachethead, aodv_cache_entry) cthead;
};

#endif /* _aodv__rtable_h__ */
