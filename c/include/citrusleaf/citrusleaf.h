/*
 * The Citrusleaf C interface. A good, basic library that many clients can be based on.
 *
 * This is the external, public header file
 *
 * this code currently assumes that the server is running in an ASCII-7 based
 * (ie, utf8 or ISO-LATIN-1)
 * character set, as values coming back from the server are UTF-8. We currently
 * don't bother to convert to the character set of the machine we're running on
 * but we advertise these values as 'strings'
 *
 * All rights reserved
 * Brian Bulkowski, 2009
 * CitrusLeaf
 */

#include <inttypes.h>
#include <stdbool.h>
#include <netinet/in.h>
 
// do this both the new skool and old skool way which gives the highest correctness,
// speed, and compatibility
#pragma once

#ifndef XDS	// Hack for the sake of XDS. XDS includes the main CF libs. 
		// We do not want to include them again from client API
#include "citrusleaf/cf_atomic.h"
#include "citrusleaf/cf_log.h"
#include "citrusleaf/cf_ll.h"
#include "citrusleaf/cf_clock.h"
#include "citrusleaf/cf_vector.h"
#include "citrusleaf/cf_queue.h"
#include "citrusleaf/cf_alloc.h"
#include "citrusleaf/cf_digest.h"
#include "citrusleaf/cf_shash.h"
#endif

#define STACK_BUF_SZ (1024 * 16) // provide a safe number for your system - linux tends to have 8M stacks these days
#define DEFAULT_PROGRESS_TIMEOUT 50

#define NODE_NAME_SIZE 20	

#ifdef __cplusplus
extern "C" {
#endif

//
// Return values for the citrusleaf get and put calls
// 

typedef enum cl_rv {
    CITRUSLEAF_FAIL_ASYNCQ_FULL = -3,
    CITRUSLEAF_FAIL_TIMEOUT = -2,
	CITRUSLEAF_FAIL_CLIENT = -1, 	// an out of memory or similar locally
	CITRUSLEAF_OK = 0,
	CITRUSLEAF_FAIL_UNKNOWN = 1, 	// unknown failure on the server side
	CITRUSLEAF_FAIL_NOTFOUND = 2,	//
	CITRUSLEAF_FAIL_GENERATION = 3,  // likely a CAS write, and the write failed
	CITRUSLEAF_FAIL_PARAMETER = 4,   // you passed in bad parameters
	CITRUSLEAF_FAIL_KEY_EXISTS = 5,
	CITRUSLEAF_FAIL_BIN_EXISTS = 6,
	CITRUSLEAF_FAIL_CLUSTER_KEY_MISMATCH = 7,
	CITRUSLEAF_FAIL_PARTITION_OUT_OF_SPACE = 8,
	CITRUSLEAF_FAIL_SERVERSIDE_TIMEOUT = 9,
	CITRUSLEAF_FAIL_NOXDS = 10,
	CITRUSLEAF_FAIL_UNAVAILABLE = 11,
	CITRUSLEAF_FAIL_INCOMPATIBLE_TYPE = 12,  // specified operation cannot be performed on that data type
	CITRUSLEAF_FAIL_RECORD_TOO_BIG = 13,
	CITRUSLEAF_FAIL_KEY_BUSY = 14,

    // scan was aborted ... but why?
    CITRUSLEAF_FAIL_SCAN_ABORT              = 15,

    // Server does not (yet) support this function
    CITRUSLEAF_FAIL_UNSUPPORTED_FEATURE     = 16,

    // Bin-level replace-only supported on server but not on client.
    CITRUSLEAF_FAIL_BIN_NOT_FOUND           = 17,

    // Storage device(s) can't keep up with the current write load.
    CITRUSLEAF_FAIL_DEVICE_OVERLOAD         = 18,

    // Record key sent with transaction did not match key stored on server.
    CITRUSLEAF_FAIL_KEY_MISMATCH            = 19,

    // Request made with no namespace or unrecognized namespace.
    CITRUSLEAF_FAIL_NAMESPACE               = 20,

    // Write operation made with too-long bin name, or bin name quota exceeded.
    CITRUSLEAF_FAIL_BIN_NAME                = 21,

    CITRUSLEAF_QUERY_END                    = 50,
    CITRUSLEAF_SECURITY_NOT_SUPPORTED       = 51,
    CITRUSLEAF_SECURITY_NOT_ENABLED         = 52,
    CITRUSLEAF_SECURITY_SCHEME_NOT_SUPPORTED = 53,
    CITRUSLEAF_INVALID_COMMAND              = 54,
    CITRUSLEAF_INVALID_FIELD                = 55,
    CITRUSLEAF_ILLEGAL_STATE                = 56,
    CITRUSLEAF_INVALID_USER                 = 60,
    CITRUSLEAF_USER_ALREADY_EXISTS          = 61,
    CITRUSLEAF_INVALID_PASSWORD             = 62,
    CITRUSLEAF_EXPIRED_PASSWORD             = 63,
    CITRUSLEAF_FORBIDDEN_PASSWORD           = 64,
    CITRUSLEAF_INVALID_CREDENTIAL           = 65,
    CITRUSLEAF_INVALID_ROLE                 = 70,
    CITRUSLEAF_INVALID_PRIVILEGE            = 71,
    CITRUSLEAF_NOT_AUTHENTICATED            = 80,
    CITRUSLEAF_ROLE_VIOLATION               = 81,

    // ???
    CITRUSLEAF_FAIL_INVALID_DATA              = 99,

    // UDF RANGE 100-110
    CITRUSLEAF_FAIL_UDF_BAD_RESPONSE    = 100,
    CITRUSLEAF_FAIL_UDF_LUA_EXECUTION   = 101,
    CITRUSLEAF_FAIL_LUA_FILE_NOTFOUND   = 102,

    // Secondary Index Query Codes 200 - 230
    CITRUSLEAF_FAIL_INDEX_FOUND             = 200,
    CITRUSLEAF_FAIL_INDEX_NOTFOUND          = 201,
    CITRUSLEAF_FAIL_INDEX_OOM               = 202,
    CITRUSLEAF_FAIL_INDEX_NOTREADABLE       = 203,
    CITRUSLEAF_FAIL_INDEX_GENERIC           = 204,
    CITRUSLEAF_FAIL_INDEX_NAME_MAXLEN       = 205,
    CITRUSLEAF_FAIL_INDEX_MAXCOUNT          = 206,

    CITRUSLEAF_FAIL_QUERY_ABORTED           = 210,
    CITRUSLEAF_FAIL_QUERY_QUEUEFULL         = 211,
    CITRUSLEAF_FAIL_QUERY_TIMEOUT           = 212,
    CITRUSLEAF_FAIL_QUERY_GENERIC           = 213
} cl_rv;

typedef enum cl_rvclient {
	CITRUSLEAF_FAIL_DC_DOWN = 1,
	CITRUSLEAF_FAIL_DC_UP = 2
} cl_rvclient;


// hidden forward reference
typedef struct cl_conn_s cl_conn;

enum cl_type {
  CL_NULL = 0x00, CL_INT = 0x01, CL_FLOAT = 2, CL_STR = 0x03, CL_BLOB = 0x04,
	CL_TIMESTAMP = 5, CL_DIGEST = 6, CL_JAVA_BLOB = 7, CL_CSHARP_BLOB = 8,
  CL_PYTHON_BLOB = 9, CL_RUBY_BLOB = 10, CL_PHP_BLOB = 11, CL_ERLANG_BLOB = 12,
  CL_APPEND       = 13,
  CL_RTA_LIST     = 14,
  CL_RTA_DICT     = 15,
  CL_RTA_APPEND_DICT = 16,
  CL_RTA_APPEND_LIST = 17,
  CL_LUA_BLOB     = 18,
  CL_MAP          = 19,
  CL_LIST         = 20,
  CL_UNKNOWN = 666666};
typedef enum cl_type cl_type;

enum cl_write_policy { CL_WRITE_ASYNC, CL_WRITE_ONESHOT, CL_WRITE_RETRY, CL_WRITE_ASSURED };

typedef enum cl_write_policy cl_write_policy;

enum cl_scan_priority { CL_SCAN_PRIORITY_AUTO, CL_SCAN_PRIORITY_LOW, CL_SCAN_PRIORITY_MEDIUM, CL_SCAN_PRIORITY_HIGH };

typedef enum cl_scan_priority cl_scan_priority;


//
// An object is the value in a bin, or it is used as a key
// The object is typed according to the citrusleaf typing system
// These are often stack allocated, and are assigned using the 'wrap' calls
//

typedef struct cl_object_s {
	enum cl_type    type;
	size_t			sz; 
	union {
		char 		*str; // note for str: sz is strlen (not strlen+1 
		void 		*blob;
		int64_t		i64;   // easiest to have one large int type
	} u;

	void *free; // if this is set, this must be freed on destructuion	

} cl_object;


typedef enum cl_operator_type { CL_OP_WRITE, CL_OP_READ, CL_OP_INCR, CL_OP_MC_INCR , CL_OP_PREPEND, CL_OP_APPEND, CL_OP_MC_PREPEND, CL_OP_MC_APPEND, CL_OP_TOUCH, CL_OP_MC_TOUCH} cl_operator;

// A bin is the bin name, and the value set or gotten

typedef struct cl_bin_s {
	char		bin_name[32];
	cl_object	object;
} cl_bin;

// A record structure containing the most common fileds of a record
typedef struct cl_rec {
	cf_digest	digest;
	uint32_t	generation;
	uint32_t	record_voidtime;
	cl_bin		*bins;
	int		n_bins;
} cl_rec;

// Structure used by functions which want to return a bunch of records
typedef struct cl_batchresult {
	pthread_mutex_t lock;
	int 		numrecs;
	cl_rec		*records;
} cl_batchresult;

// An operation is the bin, plus the operator (write, read, add, etc)
// This structure is used for the more complex 'operate' call,
// which can specify simultaneous operations on multiple bins

typedef struct cl_operation_s {
	cl_bin                bin;
	enum cl_operator_type op;
}cl_operation;
	
//Structure to map the internal address to the external address
typedef struct cl_addrmap {
	char *orig;
	char *alt;
} cl_addrmap;

//
// All citrusleaf functions return an integer. This integer is 0 if the
// call has succeeded, and a negative number if it has failed.
// All returns of pointers and objects are done through the parameters.
// (When in C++, use & parameters for return, but we're not there yet)
//
// 'void' return functions are only used for functions that are syntactically
// unable to fail.
//

typedef void (*cl_async_fail_cb) (void *udata, int rv, uint64_t);
typedef void (*cl_async_success_cb) (void *udata, int rv, uint64_t);

//
// Call this init function sometime early, create our mutexes and a few other things.
// We'd prefer if this is only called once
//

int
citrusleaf_init(void);

void
citrusleaf_change_tend_speed(int secs);

//
// Initialize async queue and async worker threads.
//
// size_limit: Maximum number of items allowed in queue. Puts are rejected when maximum is reached.
//
// num_receiver_threads: Number of worker threads to create.
//     If running in multi-process mode from python or perl, num_receiver_threads should be 1.
//     The maximum num_receiver_threads is 32.
//
// fail_cb_fn: Callback for failed transactions. Use null if callback is not desired.
// success_cb_fn: Callback for successful transactions. Use null if callback is not desired.
//
int
citrusleaf_async_init(int size_limit, int num_receiver_threads, cl_async_fail_cb fail_cb_fn, cl_async_success_cb success_cb_fn);

int
citrusleaf_async_reinit(int size_limit, unsigned int num_receiver_threads);

void 
citrusleaf_async_getstats(uint64_t *retries, uint64_t *dropouts, int *workitems);

void
citrusleaf_async_set_nw_timeout(int nw_timeout);

//
// If you wish to free up resources used by the citrusleaf client in your process,
// call this - all cl_conn will be invalid, and you'll have to call citrusleaf_init
// again to do anything
//
void citrusleaf_shutdown(void);

void citrusleaf_set_debug(bool debug_flag);

//
// This call will print stats to stderr
//
void citrusleaf_print_stats(void);

//
// cl_object calls
// 

// fill out the object structure with the string in question - no allocs
void citrusleaf_object_init(cl_object *o);
void citrusleaf_object_init_str(cl_object *o, char const *str);
void citrusleaf_object_init_str2(cl_object *o, char const *str, size_t str_len);
void citrusleaf_object_init_blob(cl_object *o, void const *buf, size_t buf_len);
void citrusleaf_object_init_blob2(cl_object *o, void const *buf, size_t buf_len, cl_type type); // several blob types
void citrusleaf_object_init_int(cl_object *o, int64_t i);
void citrusleaf_object_init_null(cl_object *o);
void citrusleaf_object_free(cl_object *o);

// frees all the memory in a bin array that would be returned from get_all but not the bin array itself
void citrusleaf_bins_free(cl_bin *bins, int n_bins);

int citrusleaf_copy_bins(cl_bin **destbins, const cl_bin *srcbins, int n_bins);


// use:
// Calling this will return an cl_conn, which is a structure internal to the library.
// You can use this to do multiple calls to Aerospike instead of returning
// and re-getting each time
// although if you have an error, it might be because that particular cluster
// went down and you might need to get a new connection to the cluster.
// (perhaps we'll hide that detail in future APIs)
//
// You will be required to return your connections! That would be called a leak.
//
// the host-port combo should be some way to get to the cluster (often a DNS name)
// However, it might be done. Using the cluster name allows the clustering
// internals to keep track of other host-ports which are the same cluster,
// and match your request to other hosts

// forward, hidden reference
struct cl_cluster_s;
typedef struct cl_cluster_s cl_cluster;

extern cl_cluster *citrusleaf_cluster_create(void);
extern void citrusleaf_cluster_destroy(cl_cluster *asc);
extern void citrusleaf_cluster_shutdown(void);

extern cl_cluster * citrusleaf_cluster_get_or_create(char *host, short port, int timeout_ms);
extern void citrusleaf_cluster_release_or_destroy(cl_cluster **asc);
extern void citrusleaf_cluster_change_tend_speed(struct cl_cluster_s *asc, int secs);
extern void citrusleaf_cluster_use_nbconnect(struct cl_cluster_s *asc);

// the timeout is how long to wait before the cluster is "settled"
// 0 - a sensible default
// N - some number of MS
// -1 - don't wait this time

extern cl_rv citrusleaf_cluster_add_host(cl_cluster *asc, char const *host, short port, int timeout_ms);

extern void  citrusleaf_cluster_add_addr_map(cl_cluster *asc, char *orig, char *alt);

extern bool citrusleaf_cluster_settled(cl_cluster *asc);

extern int citrusleaf_cluster_get_nodecount(cl_cluster *asc);

// must free node_names when done
extern void cl_cluster_get_node_names(cl_cluster *asc, int *n_nodes, char **node_names);


// in the PHP system, URLs are lingua franca. We expect that
// a single cluster will be created with one name - the URL - and 
// will be used over and over.
//
// URLs are of the form;
// citrusleaf://host:port (or similar)
extern cl_cluster *citrusleaf_cluster_get(char const *url);


// By default, the C client will "follow" the cluster, that is,
// track all the nodes in the cluster and continually update the cluster
// members. If, for testing, you wish to disable this feature, set this
// flag to false. This must be done before any 'addhost' calls, because
// even at the first one, the following of the cluster might start.
//
// Currently, setting this flags after hosts has an undefined effect.

extern void citrusleaf_cluster_follow(cl_cluster *asc, bool flag);

//
// write info structure
// There's a lot of info that can go into a write ---
typedef struct {
	bool unique;  // write unique - means success if didn't exist before
	bool unique_bin;  // write unique bin - means success if the bin didn't exist before
	bool use_generation;     // generation must be exact for write to succeed
	bool use_generation_gt;  // generation must be less - good for backup & restore
	bool use_generation_dup;	// on generation collision, create a duplicat
	uint32_t generation;
	int timeout_ms;
	uint32_t record_ttl;    // seconds, from now, when the record would be auto-removed from the DBcd 
	cl_write_policy w_pol;
} cl_write_parameters;

static inline void
cl_write_parameters_set_default(cl_write_parameters *cl_w_p)
{
	cl_w_p->unique = false;
	cl_w_p->unique_bin = false;
	cl_w_p->use_generation = false;
	cl_w_p->use_generation_gt = false;
	cl_w_p->use_generation_dup = false;
	cl_w_p->timeout_ms = 0;
	cl_w_p->record_ttl = 0;
	cl_w_p->w_pol = CL_WRITE_RETRY;
}

static inline void
cl_write_parameters_set_generation( cl_write_parameters *cl_w_p, uint32_t generation) {
	cl_w_p->generation = generation;
	cl_w_p->use_generation = true;
}

static inline void
cl_write_parameters_set_generation_gt( cl_write_parameters *cl_w_p, uint32_t generation) {
	cl_w_p->generation = generation;
	cl_w_p->use_generation_gt = true;
}

static inline void
cl_write_parameters_set_generation_dup( cl_write_parameters *cl_w_p, uint32_t generation) {
	cl_w_p->generation = generation;
	cl_w_p->use_generation_dup = true;
}


// scan_option info
typedef struct cl_scan_parameters_s {
	bool fail_on_cluster_change;	// honored by server: terminate scan if cluster in fluctuating state
	cl_scan_priority	priority;	// honored by server: priority of scan
	bool concurrent_nodes;	  		// honored on client: work on nodes in parallel or serially
	uint8_t	threads_per_node;		// honored on client: have multiple threads per node. @TODO
} cl_scan_parameters;

static inline void
cl_scan_parameters_set_default(cl_scan_parameters *cl_scan_p)
{
	cl_scan_p->fail_on_cluster_change = false;
	cl_scan_p->concurrent_nodes = false;
	cl_scan_p->threads_per_node = 1;	// not honored currently
	cl_scan_p->priority = CL_SCAN_PRIORITY_AUTO;
}

typedef struct cl_node_response_s {
	char	node_name[NODE_NAME_SIZE];
	cl_rv	node_response;
} cl_node_response;
//
// get-all will malloc an array of values and return all current values for a row.
// thus, it is SELECT * in SQL. So pass in a pointer to cl_value to be filled, and a
// pointer-to-int to know how many.
// 
// The memory contract with get_all is that the bins pointer (*bins) must be freed by the caller.
// The data you've actually retrieved (cl_object->u.str ; cl->object->u.blob) has been allocated using malloc.
// You may use it for your own purposes, transfer it to another program, or you must free it.
// citrusleaf_object_free() will free the internal memory in these cases,
// or you can call citrusleaf_bins_free and it will take care of all memory.
//
// Note this is different from getting a specific set of bins, where the bin array was passed in.
// (the simple 'get') See that call for information there.
 

cl_rv
citrusleaf_get_all(cl_cluster *asc, const char *ns, const char *set, const cl_object *key, cl_bin **bins, int *n_bins, int timeout_ms, uint32_t *cl_gen);

cl_rv
citrusleaf_get_all_digest(cl_cluster *asc, const char *ns, const cf_digest *d, cl_bin **bins, int *n_bins, int timeout_ms, uint32_t *cl_gen);

cl_rv
citrusleaf_get_all_digest_getsetname(cl_cluster *asc, const char *ns, const cf_digest *d, cl_bin **bins, int *n_bins, int timeout_ms, uint32_t *cl_gen, char **setname);

//
// Put is like insert. Create a list of bins, and call this function to set them.
//
	
cl_rv
citrusleaf_put(cl_cluster *asc, const char *ns, const char *set, const cl_object *key, const cl_bin *bins, int n_bins, const cl_write_parameters *cl_w_p);

cl_rv
citrusleaf_put_digest(cl_cluster *asc, const char *ns, const cf_digest *d, const cl_bin *bins, int n_bins, const cl_write_parameters *cl_w_p);

cl_rv
citrusleaf_put_replace(cl_cluster *asc, const char *ns, const char *set, const cl_object *key, const cl_bin *values, int n_values, const cl_write_parameters *cl_w_p);

cl_rv
citrusleaf_restore(cl_cluster *asc, const char *ns, const cf_digest *digest, const char *set, const cl_bin *values, int n_values, const cl_write_parameters *cl_w_p);

//
// Send asynchronous put request to server and return without waiting for response.
// The response is available in callback specified in citrusleaf_async_init().
//
cl_rv
citrusleaf_async_put(cl_cluster *asc, const char *ns, const char *set, const cl_object *key,
	const cl_bin *bins, int n_bins, const cl_write_parameters *cl_w_p, uint64_t trid, void *udata);

//
// Send asynchronous put request with digest key to server and return without waiting for response.
// The response is available in callback specified in citrusleaf_async_init().
//
cl_rv
citrusleaf_async_put_digest(cl_cluster *asc, const char *ns, const cf_digest *d, const char *set,
	const cl_bin *bins, int n_bins, const cl_write_parameters *cl_w_p, uint64_t trid, void *udata);

//
// Send asynchronous put request to server and return without waiting for response.
//
cl_rv
citrusleaf_async_put_forget(cl_cluster *asc, const char *ns, const char *set, const cl_object *key,
	const cl_bin *bins, int n_bins, const cl_write_parameters *cl_w_p);

//
// Send asynchronous put request with digest key to server and return without waiting for response.
//
cl_rv
citrusleaf_async_put_digest_forget(cl_cluster *asc, const char *ns, const cf_digest *d, const char *set,
	const cl_bin *bins, int n_bins, const cl_write_parameters *cl_w_p);

cl_rv
citrusleaf_async_put_digest_xdr(cl_cluster *asc, const char *ns, const cf_digest *d, char *set,
	const cl_bin *bins, int n_bins, const cl_write_parameters *cl_w_p, uint64_t trid, void *udata);

cl_rvclient
citrusleaf_check_cluster_health(cl_cluster *asc);

void
citrusleaf_sleep_for_tender(cl_cluster *asc);

//
// Get is like select in SQL. Create a list of bins to get, and call this function to retrieve
// the values.

cl_rv
citrusleaf_get(cl_cluster *asc, const char *ns, const char *set, const cl_object *key, cl_bin *bins, int n_bins, int timeout_ms, uint32_t *cl_gen);

cl_rv
citrusleaf_get_digest(cl_cluster *asc, const char *ns, const cf_digest *d, cl_bin *bins, int n_bins, int timeout_ms, uint32_t *cl_gen);

//
// Delete simply wipes this single key off the face of the earth.
//

cl_rv
citrusleaf_delete(cl_cluster *asc, const char *ns, const char *set, const cl_object *key, const cl_write_parameters *cl_w_p);

cl_rv
citrusleaf_delete_digest(cl_cluster *asc, const char *ns,  const cf_digest *d, const cl_write_parameters *cl_w_p);

cl_rv
citrusleaf_async_delete_digest_xdr(cl_cluster *asc, const char *ns, const cf_digest *digest, const cl_write_parameters *cl_w_p, void *udata);

//
// Efficiently determine if the key exists.
//  (Note:  The bins are currently ignored but may be testable in the future.)
//

cl_rv
citrusleaf_exists_key(cl_cluster *asc, const char *ns, const char *set, const cl_object *key, cl_bin *bins, int n_bins, int timeout_ms, uint32_t *cl_gen);

cl_rv
citrusleaf_exists_digest(cl_cluster *asc, const char *ns, const cf_digest *d, cl_bin *bins, int n_bins, int timeout_ms, uint32_t *cl_gen);

//
// get many call has the following properties:
// you can pass null either with the namespace or the set, and a large iteration will occur
// Memory available vanishes after 'return', if you want a copy, make a copy
// Non-zero return in the callback aborts the call
typedef int (*citrusleaf_get_many_cb) (char *ns, cf_digest *keyd, char *set, uint32_t generation, uint32_t record_ttl,
	cl_bin *bins, int n_bins, bool is_last, void *udata);

cl_rv
citrusleaf_scan(cl_cluster *asc, char *ns, char *set, cl_bin *bins, int n_bins, bool get_key, citrusleaf_get_many_cb cb, void *udata, bool nobindata);

// response is a vector of cl_node_response
cf_vector *
citrusleaf_scan_all_nodes (cl_cluster *asc, char *ns, char *set, cl_bin *bins, int n_bins, bool nobindata, uint8_t scan_pct,
		citrusleaf_get_many_cb cb, void *udata, cl_scan_parameters *scan_p);

cl_rv
citrusleaf_scan_node (cl_cluster *asc, char *node_name, char *ns, char *set, cl_bin *bins, int n_bins, bool nobindata, uint8_t scan_pct,
		citrusleaf_get_many_cb cb, void *udata, cl_scan_parameters *scan_p);

//
// Initialize batch queue and specified number of worker threads (Maximum thread count is 6).
//
cl_rv
citrusleaf_batch_init(int n_threads);

//
// Get many digest
// This version of the call acts exactly as the get digest call, but takes an array
// of digests. Those digests will be retrieved with the same API conventions as the
// previous ones.
// To come: an array of keys, but it'll just be a wrapper on this.
//
cl_rv
citrusleaf_get_many_digest(cl_cluster *asc, char *ns, const cf_digest *digests, int n_digests, cl_bin *bins, int n_bins, bool get_key /*if true, retrieve key instead of simply digest*/, 
	citrusleaf_get_many_cb cb, void *udata);

//
// Get many digest without a callback
// This version of the batch-get call does not need the callback function. It will return an array of records. 
// The results are returned in an array. No ordering is guaranteed between the input digest array and 
// the returned rows. If the corresponding records for the digests are not found in the cluster, there wont 
// be any corresponding entry in the result array indicating that the records are missing. The caller must 
// call the citrusleaf_free_batchresult() to free the memory allocated during this operation.
cl_rv
citrusleaf_get_many_digest_direct(cl_cluster *asc, char *ns, const cf_digest *digests, int n_digests, cl_batchresult **br);

// Utility function to free the memory allocated by the citrusleaf_get_many_digest_direct() function
void
citrusleaf_free_batchresult(cl_batchresult *br);

//
// Key exists many digest
// This version of the call acts exactly as the exists digest call, but takes an array
// of digests. Those digests will be retrieved with the same API conventions as the
// previous ones.
// To come: an array of keys, but it'll just be a wrapper on this.
//

cl_rv
citrusleaf_exists_many_digest(cl_cluster *asc, char *ns, const cf_digest *digests, int n_digests, cl_bin *bins, int n_bins, bool get_key /*if true, retrieve key instead of simply digest*/, 
	citrusleaf_get_many_cb cb, void *udata);

//
// Use the information system to request data from a given node
// Pass in a '\n' seperated list of names, or no names at all
// [Perhaps a better interface would be an array of pointers-to-names, returning an
//  array of pointers-to-values?]
// Returns a malloc'd string which is the response from the server.
//

int
citrusleaf_info(char *host, short port, char *names, char **values, int timeout_ms);

int 
citrusleaf_info_host(struct sockaddr_in *sa_in, char *names, char **values, int timeout_ms, bool send_asis);

// Do a lookup with this name and port, and add the sockaddr to the
// vector using the unique lookup
int cl_lookup(cl_cluster *asc, char *hostname, short port, cf_vector *sockaddr_in_v);

//
// This call is good for testing. Call it when you think you know the values. If the key doesn't exist, or
// the data is incorrect, then the server that is serving the request will spit a failure, and if you're
// running in the right server debug mode you can examine the error in detail.
//

cl_rv
citrusleaf_verify(cl_cluster *asc, const char *ns, const char *set, const cl_object *key, const cl_bin *bins, int n_bins, int timeout_ms, uint32_t *cl_gen);
cl_rv
citrusleaf_delete_verify(cl_cluster *asc, const char *ns, const char *set, const cl_object *key, const cl_write_parameters *cl_w_p);

//
// This call allows the caller to specify the operation - read, write, add, etc.  Multiple operations
// can be specified in a single call.//

cl_rv
citrusleaf_operate(cl_cluster *asc, const char *ns, const char *set, const cl_object *key, cl_operation *operations, int n_operations, const cl_write_parameters *cl_w_p, int replace, uint32_t *generation);

//
// This debugging call can be useful for tracking down errors and coordinating with server failures
// gets the digest for a particular set and key
int
citrusleaf_calculate_digest(const char *set, const cl_object *key, cf_digest *digest);

/******************************************************************************
 *	FUNCTIONS
 *****************************************************************************/

/**
 *	Add a value into the lset.
 *
 *	~~~~~~~~~~{.c}
 *	as_key key;
 *	as_key_init(&key, "myns", "myset", "mykey");
 *
 *	as_ldt lset;
 *	as_ldt_init(&lset, "mylset", AS_LDT_LSET, NULL);
 *	
 *	as_integer ival;
 *	as_integer_init(&ival, 123);
 *
 *	if ( aerospike_lset_add(&as, &err, NULL, &key, &lset, (as_val *) &ival) != AEROSPIKE_OK ) {
 *		fprintf(stderr, "error(%d) %s at [%s:%d]", err.code, err.message, err.file, err.line);
 *	}
 *	~~~~~~~~~~
 *
 *	@param as			The aerospike instance to use for this operation.
 *	@param err			The as_error to be populated if an error occurs.
 *	@param policy		The policy to use for this operation. If NULL, then the default policy will be used.
 *	@param key			The key of the record.
 *	@param ldt 			The ldt bin to insert values to.
 *	@param val			The value to insert into the lset.
 *
 *	@return AEROSPIKE_OK if successful. Otherwise an error.
 *
 *	@ingroup ldt_operations
 */
cl_rv 
citrusleaf_lset_add(cl_cluster *asc, const char *ns, const char *set, const cl_object *key, const char *ldt, const cl_object *val, const cl_write_parameters *cl_w_p);

/**
 *	Add a list of values into the lset.
 *
 *	~~~~~~~~~~{.c}
 *	as_key key;
 *	as_key_init(&key, "myns", "myset", "mykey");
 *
 *	as_ldt lset;
 *	as_ldt_init(&lset, "mylset", AS_LDT_LSET, NULL);
 *
 *
 *	as_arraylist vals;
 *	as_arraylist_inita(&vals, 2);
 *	as_string s;
 *	as_string_init(s,"a string",false);
 *	as_arraylist_append_string(&vals, s);
 *	as_arraylist_append_int64(&vals, 35);
 *
 *	if ( aerospike_lset_add_all(&as, &err, NULL, &key, &lset, (as_list *)vals) != AEROSPIKE_OK ) {
 *		fprintf(stderr, "error(%d) %s at [%s:%d]", err.code, err.message, err.file, err.line);
 *	}
 *
 *	~~~~~~~~~~
 *
 *	@param as			The aerospike instance to use for this operation.
 *	@param err			The as_error to be populated if an error occurs.
 *	@param policy		The policy to use for this operation. If NULL, then the default policy will be used.
 *	@param key			The key of the record.
 *	@param ldt 			The ldt bin to insert values to.
 *	@param vals			The list of values to insert into the lset.
 *
 *	@return AEROSPIKE_OK if successful. Otherwise an error.
 *
 *	@ingroup ldt_operations
 */
//cl_rv citrusleaf_lset_add_all(
//	aerospike * as, as_error * err, const as_policy_apply * policy,
//	const as_key * key, const as_ldt * ldt, const as_list * vals);

/**
 *	See if a value exists in an lset
 *
 *	~~~~~~~~~~{.c}
 *	as_key key;
 *	as_key_init(&key, "myns", "myset", "mykey");
 *
 *	as_ldt lset;
 *	as_ldt_init(&lset, "mylset", AS_LDT_LSET, NULL);
 *
 *	as_integer ival;
 *	as_integer_init(&ival, 123);
 *
 *  boolean exists = false;
 *	
 *	if ( aerospike_lset_exists(&as, &err, NULL, &key, &lset, &ival, &exists) != AEROSPIKE_OK ) {
 *		fprintf(stderr, "error(%d) %s at [%s:%d]", err.code, err.message, err.file, err.line);
 *	}
 *	else {
 *		// do logic because element exists
 *	}
 *	~~~~~~~~~~
 *
 *	@param as			The aerospike instance to use for this operation.
 *	@param err			The as_error to be populated if an error occurs.
 *	@param policy		The policy to use for this operation. If NULL, then the default policy will be used.
 *	@param key			The key of the record.
 *	@param ldt 			The lset bin to lookup from. If not an lset bin, will return error.
 *	@param val          The value we're searching for.
 *	@param exists 		Returned boolean value to indicate value exists.
 *
 *	@return AEROSPIKE_OK if successful. Otherwise an error.
 *	
 *	@ingroup ldt_operations
 */

cl_rv citrusleaf_lset_exists(cl_cluster *asc, const char *ns, const char *set, const cl_object *key, cl_bin *bins, int n_bins, int timeout_ms, uint32_t *cl_gen);

/**
 *	Fetch (get) a value from the lset.
 *	Note that this is useful mainly in the case where the search criteria for
 *	an object is less than the entire object -- and that is when the standard
 *	defaults are overridden and the unique_identifier() function is employed
 *	to use only part of the object for search and compare. 
 *	The unique_identifier() function is defined on create -- and declared in
 *	the USER_MODULE.
 *
 *	~~~~~~~~~~{.c}
 *	as_key key;
 *	as_key_init(&key, "myns", "myset", "mykey");
 *
 *	as_ldt lset;
 *	as_ldt_init(&lset, "mylset", AS_LDT_LSET, NULL);
 *
 *	as_integer ival;
 *	as_integer_init(&ival, 123);
 *
 *  as_val * p_return_val;
 *	
 *	if ( aerospike_lset_exists(&as, &err, NULL, &key, &lset, &ival, &p_return_value) != AEROSPIKE_OK ) {
 *		fprintf(stderr, "error(%d) %s at [%s:%d]", err.code, err.message, err.file, err.line);
 *	}
 *	else {
 *		// do logic because element exists
 *	}
 *	~~~~~~~~~~
 *
 *	@param as			The aerospike instance to use for this operation.
 *	@param err			The as_error to be populated if an error occurs.
 *	@param policy		The policy to use for this operation. If NULL, then the default policy will be used.
 *	@param key			The key of the record.
 *	@param ldt 			The lset bin to lookup from. If not an lset bin, will return error.
 *	@param val          The value we're searching for.
 *	@param pp_return_val   Returned value.
 *
 *	@return AEROSPIKE_OK if successful. Otherwise an error.
 *	
 *	@ingroup ldt_operations
 */

cl_rv citrusleaf_lset_get(cl_cluster *asc, const char *ns, const char *set, const cl_object *key, const char *ldt, cl_bin **bins, int *n_bins, int timeout_ms, uint32_t *cl_gen);
/**
 *	Given an lset bin, filter the set of objects using the given filter function.
 *	If no filter function is specified, all values in the set will be returned.
 *
 *	~~~~~~~~~~{.c}
 *	as_key key;
 *	as_key_init(&key, "myns", "myset", "mykey");
 *
 *	as_ldt lset;
 *	as_ldt_init(&lset, "mylset", AS_LDT_LSET, NULL);
 *
 *	as_list *list = NULL;
 *
 *	if ( aerospike_lset_filter(&as, &err, NULL, &key, &lset,
 *			"search_filter", NULL, (as_list *) &list) != AEROSPIKE_OK ) {
 *		fprintf(stderr, "error(%d) %s at [%s:%d]", err.code, err.message, err.file, err.line);
 *	}
 *	else {
 *		// process the returned elements
 *		as_arraylist_destroy(list);
 *	}
 *	~~~~~~~~~~
 *
 *	@param as			The aerospike instance to use for this operation.
 *	@param err			The as_error to be populated if an error occurs.
 *	@param policy		The policy to use for this operation. If NULL, then the default policy will be used.
 *	@param key			The key of the record.
 *	@param ldt 			The lset bin to search from. If not an lset bin, will return error.
 *	@param filter		The name of the User-Defined-Function to use as a search filter.
 *	@param fargs		The list of parameters passed in to the User-Defined-Function filter.
 *	@param list			The pointer to a list of elements returned from search function. Pointer should
 *						be NULL passed in.
 *
 *	@return AEROSPIKE_OK if successful. Otherwise an error.
 *
 *	@ingroup ldt_operations
 */
//cl_rv citrusleaf_lset_filter(
//	aerospike * as, as_error * err, const as_policy_apply * policy,
//	const as_key * key, const as_ldt * ldt,
//	const as_udf_function_name filter, const as_list *filter_args,
//	as_list ** elements );

/**
 *	Given an lset bin, scan for all the values in the set
 *
 *	~~~~~~~~~~{.c}
 *	as_key key;
 *	as_key_init(&key, "myns", "myset", "mykey");
 *
 *	as_ldt lset;
 *	as_ldt_init(&lset, "mylset", AS_LDT_LSET, NULL);
 *
 *	as_list *list = NULL;
 *
 *	if ( aerospike_lset_scan(&as, &err, NULL, &key, &lset,
 *			"search_filter", NULL, (as_list *) &list) != AEROSPIKE_OK ) {
 *		fprintf(stderr, "error(%d) %s at [%s:%d]", err.code, err.message, err.file, err.line);
 *	}
 *	else {
 *		// process the returned elements
 *		as_arraylist_destroy(list);
 *	}
 *	~~~~~~~~~~
 *
 *	@param as			The aerospike instance to use for this operation.
 *	@param err			The as_error to be populated if an error occurs.
 *	@param policy		The policy to use for this operation. If NULL, then the default policy will be used.
 *	@param key			The key of the record.
 *	@param ldt 			The lset bin to search from. If not an lset bin, will return error.
 *	@param list			The pointer to a list of elements returned from search function. Pointer should
 *						be NULL passed in.
 *
 *	@return AEROSPIKE_OK if successful. Otherwise an error.
 *
 *	@ingroup ldt_operations
 */

cl_rv citrusleaf_lset_scan(cl_cluster *asc, const char *ns, const char *set, const cl_object *key, const char *ldt, cl_bin **bins, int *n_bins, int timeout_ms, uint32_t *cl_gen);

/**
 *	Look up a lset and find how many elements it contains
 *
 *	~~~~~~~~~~{.c}
 *	as_key key;
 *	as_key_init(&key, "myns", "myset", "mykey");
 *
 *	as_ldt lset;
 *	as_ldt_init(&lset, "mylset", AS_LDT_LSET, NULL);
 *	uint32_t lset_size = 0;
 *
 *	if ( aerospike_lset_size(&as, &err, NULL, &key, &lset, &lset_size) != AEROSPIKE_OK ) {
 *		fprintf(stderr, "error(%d) %s at [%s:%d]", err.code, err.message, err.file, err.line);
 *	}
 *	~~~~~~~~~~
 *
 *	@param as			The aerospike instance to use for this operation.
 *	@param err			The as_error to be populated if an error occurs.
 *	@param policy		The policy to use for this operation. If NULL, then the default policy will be used.
 *	@param key			The key of the record.
 *	@param ldt 			The lset to operate on. If not an lset bin, will return error.
 *	@param n			Return the number of elements in the lset.
 *
 *	@return AEROSPIKE_OK if successful. Otherwise an error.
 *
 *	@ingroup ldt_operations
 */
cl_rv citrusleaf_lset_size(cl_cluster *asc, const char *ns, const char *set, const cl_object *key, const char *ldt, int *n_bins, int timeout_ms, uint32_t *cl_gen);

/**
 *	Delete the given value from the lset
 *
 *	~~~~~~~~~~{.c}
 *	as_key key;
 *	as_key_init(&key, "myns", "myset", "mykey");
 *
 *	as_ldt lset;
 *	as_ldt_init(&lset, "lset", AS_LDT_LSET, NULL);
 *
 *	as_integer ival;
 *	as_integer_init(&ival, 123);
 *
 *	if ( aerospike_lset_remove(&as, &err, NULL, &key, &lset, &ival) != AEROSPIKE_OK ) {
 *		fprintf(stderr, "error(%d) %s at [%s:%d]", err.code, err.message, err.file, err.line);
 *	}
 *	~~~~~~~~~~
 *
 *	@param as			The aerospike instance to use for this operation.
 *	@param err			The as_error to be populated if an error occurs.
 *	@param policy		The policy to use for this operation. If NULL, then the default policy will be used.
 *	@param key			The key of the record.
 *	@param ldt 			The lset bin to delete from. If not an lset bin, will return error.
 *	@param val			The value to delete from the set.
 *
 *	@return AEROSPIKE_OK if successful. Otherwise an error.
 *
 *	@ingroup ldt_operations
 */

cl_rv 
citrusleaf_lset_remove(cl_cluster *asc, const char *ns, const char *set, const cl_object *key, const char *ldt, const cl_object *val, const cl_write_parameters *cl_w_p);

/**
 *	Destroy the lset bin
 *
 *	~~~~~~~~~~{.c}
 *	as_key key;
 *	as_key_init(&key, "myns", "myset", "mykey");
 *
 *	as_ldt lset;
 *	as_ldt_init(&lset, "mylset", AS_LDT_LSET, NULL);
 *
 *	if ( aerospike_lset_destroy(&as, &err, NULL, &key, &lset) != AEROSPIKE_OK ) {
 *		fprintf(stderr, "error(%d) %s at [%s:%d]", err.code, err.message, err.file, err.line);
 *	}
 *	~~~~~~~~~~
 *
 *	@param as			The aerospike instance to use for this operation.
 *	@param err			The as_error to be populated if an error occurs.
 *	@param policy		The policy to use for this operation. If NULL, then the default policy will be used.
 *	@param key			The key of the record.
 *	@param ldt 			The lset bin to destroy. If not an lset bin, will return error.
 *
 *	@return AEROSPIKE_OK if successful. Otherwise an error.
 *
 *	@ingroup ldt_operations
 */
cl_rv citrusleaf_lset_destroy(
        cl_cluster *asc, const char *ns, const char *set, const cl_object *key, const                    cl_write_parameters *cl_w_p);

/**
 *	Change an LDT storage capacity (in number of elements)
 *
 *	~~~~~~~~~~{.c}
 *	as_key key;
 *	as_key_init(&key, "myns", "myset", "mykey");
 *
 *	as_ldt lset;
 *	as_ldt_init(&lset, "mylset", AS_LDT_LMAP, NULL);
 *	uint32_t ldt_capacity = 0;
 *
 *	if ( aerospike_lset_set_capacity(&as, &err, NULL, &key, &lset, ldt_capacity) != AEROSPIKE_OK ) {
 *		fprintf(stderr, "error(%d) %s at [%s:%d]", err.code, err.message, err.file, err.line);
 *	}
 *	~~~~~~~~~~
 *
 *	@param as			The aerospike instance to use for this operation.
 *	@param err			The as_error to be populated if an error occurs.
 *	@param policy		The policy to use for this operation. If NULL, then the default policy will be used.
 *	@param key			The key of the record.
 *	@param ldt 			The LDT to check
 *	@param ldt_capacity	The number of elements cap for the LDT.
 *
 *	@return AEROSPIKE_OK if successful. Otherwise an error.
 *
 *	@ingroup ldt_operations
 */
cl_rv citrusleaf_lset_set_capacity(
        cl_cluster *asc, const char *ns, const char *set, const cl_object *key, const                    cl_write_parameters *cl_w_p);

/**
 *	Get an LDTs storage capacity (in number of elements)
 *
 *	~~~~~~~~~~{.c}
 *	as_key key;
 *	as_key_init(&key, "myns", "myset", "mykey");
 *
 *	as_ldt lset;
 *	as_ldt_init(&lset, "mylset", AS_LDT_LMAP, NULL);
 *	uint32_t ldt_capacity = 0;
 *
 *	if ( aerospike_lset_get_capacity(&as, &err, NULL, &key, &lset, &ldt_capacity) != AEROSPIKE_OK ) {
 *		fprintf(stderr, "error(%d) %s at [%s:%d]", err.code, err.message, err.file, err.line);
 *	}
 *	~~~~~~~~~~
 *
 *	@param as			The aerospike instance to use for this operation.
 *	@param err			The as_error to be populated if an error occurs.
 *	@param policy		The policy to use for this operation. If NULL, then the default policy will be used.
 *	@param key			The key of the record.
 *	@param ldt 			The LDT bin to operate on
 *	@param ldt_capacity The LDT Capacity, in terms of elements, not bytes.
 *
 *	@return AEROSPIKE_OK if successful. Otherwise an error.
 *
 *	@ingroup ldt_operations
 */
cl_rv citrusleaf_lset_get_capacity(
        cl_cluster *asc, const char *ns, const char *set, const cl_object *key, const                    cl_write_parameters *cl_w_p);

/**
 *	Check to see if an LDT object exists in this record bin.
 *
 *	~~~~~~~~~~{.c}
 *	as_key key;
 *	as_key_init(&key, "myns", "myset", "mykey");
 *
 *	as_ldt lset;
 *	as_ldt_init(&lset, "mylset", AS_LDT_LSET, NULL);
 *	uint32_t ldt_exists = 0;
 *
 *	if ( aerospike_lset_ldt_exists(&as, &err, NULL, &key, &lset, &ldt_exists) != AEROSPIKE_OK ) {
 *		fprintf(stderr, "error(%d) %s at [%s:%d]", err.code, err.message, err.file, err.line);
 *	}
 *	~~~~~~~~~~
 *
 *	@param as			The aerospike instance to use for this operation.
 *	@param err			The as_error to be populated if an error occurs.
 *	@param policy		The policy to use for this operation. If NULL, then the default policy will be used.
 *	@param key			The key of the record.
 *	@param ldt 			The LDT to operate on. If not an LMAP bin, will return error.
 *	@param ldt_exists	Ptr to as_boolean: Set to TRUE if ldt exists, otherwise false.
 *
 *	@return AEROSPIKE_OK if successful. Otherwise an error.
 *
 *	@ingroup ldt_operations
 */
cl_rv citrusleaf_lset_ldt_exists(
        cl_cluster *asc, const char *ns, const char *set, const cl_object *key, const                    cl_write_parameters *cl_w_p);


#ifdef __cplusplus
} // end extern "C"
#endif


