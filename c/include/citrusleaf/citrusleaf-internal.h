/*
 * The Aerospike C interface. A good, basic library that many clients can be based on.
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

#include "citrusleaf/citrusleaf.h"
#include "citrusleaf/cl_cluster.h"
#include "citrusleaf/proto.h"
#include "citrusleaf/cf_atomic.h"
#include "citrusleaf/cf_ll.h"
#include "citrusleaf/cf_vector.h"
#include "citrusleaf/cf_queue.h"
#include "citrusleaf/cf_alloc.h"
#include "citrusleaf/cf_digest.h"
#include "citrusleaf/cf_log_internal.h"

#ifdef __cplusplus
extern "C" {
#endif

// 30-39 RESEVED FOR UDF
#define CL_MSG_FIELD_TYPE_UDF_FILENAME          30
#define CL_MSG_FIELD_TYPE_UDF_FUNCTION          31
#define CL_MSG_FIELD_TYPE_UDF_ARGLIST           32

/**
 *  Byte Buffer.
 */
typedef struct as_buffer_s {

    /**
     *  Number of bytes allocated to the buffer
     */
    uint32_t capacity;
    
    /**
     *  Number of bytes used
     */                     
    uint32_t size;          
    
    /**
     *  Bytes of the buffer
     */
    uint8_t * data;
    
} as_buffer;

/**
 *  as_val types
 */
typedef enum as_val_t {
    AS_UNDEF        = 0,
    AS_UNKNOWN      = 0,    //<! @deprecated
    AS_NIL          = 1,
    AS_BOOLEAN      = 2,
    AS_INTEGER      = 3,
    AS_STRING       = 4,
    AS_LIST         = 5,
    AS_MAP          = 6,
    AS_REC          = 7,
    AS_PAIR         = 8,
    AS_BYTES        = 9,
    AS_VAL_T_MAX
} __attribute__((packed)) as_val_t;

/**
 *  Represents a value
 *  @ingroup aerospike_t
 */
typedef struct as_val_s {
    /**
     *  Value type
     */
    enum as_val_t type;

    /**
     *  Value can be freed.
     *  Should be false for stack allocated values.
     */
    bool free;

    /**
     *  Reference count
     *  Values are ref counted.
     *  To increment the count, use `as_val_reserve()`
     */
    cf_atomic32 count;

} as_val;

typedef struct as_string_s {

    /**
     *  @private
     *  as_boolean is a subtype of as_val.
     *  You can cast as_boolean to as_val.
     */
    as_val _;

    /**
     *  If true, then `as_string.value` can be freed.
     */
    bool free;

    /**
     *  The string value.
     */
    char * value;

    /**
     *  The length of the string.
     */
    size_t len;

} as_string;

/**
 *  The length of the string
 *
 *  @param string The string to get the length of. 
 *
 *  @return the length of the string in bytes.
 *
 *  @relatesalso as_string
 */
size_t as_string_len(as_string * string);

/**
 *  Get the string value. If string is NULL, then return the fallback value.
 *
 *  @relatesalso as_string
 */
static inline char * as_string_getorelse(const as_string * string, char * fallback)
{
    return string ? string->value : fallback;
}

/**
 *  Get the string value.
 *  @deprecated Use as_string_get() instead
 *
 *  @relatesalso as_string
 */
static inline char * as_string_tostring(const as_string * string)
{
    return as_string_getorelse(string, NULL);
}

// citrusleaf.c used by cl_batch
int
cl_value_to_op_get_size(cl_bin *v, size_t *sz);

uint8_t *
cl_write_header(uint8_t *buf, size_t msg_sz, uint info1, uint info2, uint info3, uint32_t generation, uint32_t record_ttl, uint32_t transaction_ttl, uint32_t n_fields, uint32_t n_ops );

int
cl_value_to_op(cl_bin *v, cl_operator clOperator, cl_operation *operation, cl_msg_op *op);

void
cl_set_value_particular(cl_msg_op *op, cl_bin *value);

extern shash		  		*g_cl_async_hashtab;

typedef struct as_call_s as_call;

typedef struct cl_async_work {
	uint64_t	trid;		//Transaction-id of the submitted work
	uint64_t	deadline;	//Deadline time for this work item
	uint64_t	starttime;	//Start time for this work item
	cl_cluster_node *node;		//Node to which the work item was sent
	int		fd;		//FD used to send the command asynchronously
	void		*udata;
} cl_async_work;

int cl_del_node_asyncworkitems(void *key, void *value, void *clnode);

struct as_call_s {
    as_string * file;
    as_string * func;
    as_buffer * args;
};

// scan fields
// left-to-right bits
// 0-3 priority hint = ClScanningPriority
// 4 = failOnClusterChange
// 5-7 = unused 
// 8-15 = scan_pct
 typedef struct cl_scan_param_field_s {
	uint8_t	byte1;
	uint8_t scan_pct;
} cl_scan_param_field;

// For XDS...
extern cf_queue	   *g_cl_async_q;
extern cf_queue	   *g_cl_workitems_freepool_q;


int
cl_do_async_monte(cl_cluster *asc, int info1, int info2, const char *ns, const char *set, const cl_object *key,
			const cf_digest *digest, cl_bin **values, cl_operator operator, cl_operation **operations,
			int *n_values, uint32_t *cl_gen, const cl_write_parameters *cl_w_p, uint64_t *trid, void *udata);

/*
int
do_the_full_monte(cl_cluster *asc, int info1, int info2, const char *ns, const char *set, const cl_object *key, 
	const cf_digest *digest, cl_bin **values, cl_operator operator, cl_operation **operations, 
	int *n_values, uint32_t *cl_gen, const cl_write_parameters *cl_w_p, uint64_t *trid);

int
do_many_monte(cl_cluster *asc, uint operation_info, uint operation_info2, const char *ns, const char *set, 
	const cf_digest *digests, int n_digests, citrusleaf_get_many_cb cb, void *udata);

int
do_async_monte(cl_cluster *asc, int info1, int info2, const char *ns, const char *set, const cl_object *key, 
	const cf_digest *digest, cl_bin **values, cl_operator operator, cl_operation **operations, 
	int *n_values, uint32_t *cl_gen, const cl_write_parameters *cl_w_p, uint64_t *trid, void *udata);
*/

//
// cl_batch.c
//
void
citrusleaf_batch_shutdown();

void
citrusleaf_async_shutdown();

int
citrusleaf_info_host_limit(struct sockaddr_in *sa_in, char *names, char **values, int timeout_ms, bool send_asis, uint64_t max_response_length);

int
cl_compile(uint info1, uint info2, uint info3, const char *ns, const char *set, const cl_object *key, const cf_digest *digest,
	cl_bin *values, cl_operator operator, cl_operation *operations, int n_values,  
	uint8_t **buf_r, size_t *buf_sz_r, const cl_write_parameters *cl_w_p, cf_digest *d_ret, uint64_t trid, cl_scan_param_field *scan_field, as_call * call, uint8_t udf_type);

int
cl_parse(cl_msg *msg, uint8_t *buf, size_t buf_len, cl_bin **values_r, cl_operation **operations_r, 
	int *n_values_r, uint64_t *trid, char **setname_r);

#ifdef DEBUG_VERBOSE
void
dump_buf(char *info, uint8_t *buf, size_t buf_len);
#endif

#ifdef __cplusplus
} // end extern "C"
#endif


