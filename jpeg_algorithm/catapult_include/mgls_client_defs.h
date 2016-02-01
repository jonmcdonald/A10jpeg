/*
** HEADER
**    mgls_client_defs.h --  This file contains all #define's needed by 
**                           clients of MGLS.
**
** COPYRIGHT
**
**  Copyright 1992-2013 Mentor Graphics Corporation
**  All Rights Reserved.
**
**  THIS WORK CONTAINS TRADE SECRET AND PROPRIETARY
**  INFORMATION WHICH IS THE PROPERTY OF MENTOR
**  GRAPHICS CORPORATION OR ITS LICENSORS AND IS
**  SUBJECT TO LICENSE TERMS.
**
** WRITTEN BY:
**   Mahmood TahanPesar
**
** DESCRIPTION
**    This file contains the definition of some #define values used by
**    MGLS client applications.
**
** USER GUIDE
**
*/
#ifndef INCLUDED_MGLS_CLIENT_DEFS
#define INCLUDED_MGLS_CLIENT_DEFS

#include <values.h>

/*
 *  Length limit for display passed to the core_Mheap_allocv_attrs function.
 *
 */
#define MGLS_DISPLAY_LENGTH   768

/*
**  Length limit for data returned by the core_Mheap_msg function.
**
*/
#define MGLS_TAMPER_MSG_MAX_SIZE   4096

/*
**  All core_Mheap_... calls return either of the two values
**  defined below
*/

#define MGLS_ERR       -1       /* return this value on error     */
#define MGLS_OK         0       /* this value says "it worked OK" */

/*
**  Strings holding MGLS error message should point to at 256 
**  bytes of allocated memory
*/
#define SIZE_OF_MGLS_ERROR_STRING 256

/*
**  other values returned by core_Mheap_stat system call
*/
#define MGLS_REQ_PENDING    1     /* Indicates request is being worked on    */
#define MGLS_REQ_IN_QUEUE   2     /* Indicates request is waiting in a queue */
#define MGLS_EXP_WARNING    3     /* Indicates license expiration is near    */
#define MGLS_MAXUSERS      -4     /* Indicates all licenses in use           */
#define MGLS_SOME_OK       -5     /* Indicates some licenses in use          */

/*
**  Other values returned by core_Mheap_validv, core_Mheap_valid, and
**  core_Mheap_validv_attrs calls
*/
#define MGLS_LICENSE_INUSE       0

/*
**  Other values returned by core_Mheap_validv, core_Mheap_valid, 
**  core_Mheap_validv_attrs, and core_Mheap_exists calls
*/
#define MGLS_LICENSE_AVAILABLE   3

/*
**  other values returned by core_Mheap_exists call
*/
#define MGLS_LICENSE_UNAVAILABLE   0

/*
**  Following are #define's used as the first parameter of the
**  core_Mheap_set and core_Mheap_get calls that set and return
**  various attributes of MGLS. For names that contain _ENV_,
**  the UNIX environment is searched for a value when MGLS
**  starts. Values from the environment are assumed to be
**  "The User Speaking"; once they are set, core_Mheap_set
**  calls do not change them.
*/
#define MGLS_FIRST_ATTR_NUMBER                    1

#define MGLS_ATTR_CHECK_INTERVAL                  1
#define MGLS_ATTR_RETRY_INTERVAL                  2
#define MGLS_ATTR_RETRY_COUNT                     3
#define MGLS_ATTR_RECONNECT                       4
#define MGLS_ATTR_RECONNECT_DONE                  5
#define MGLS_ATTR_EXIT_CALL                       6
#define MGLS_ENV_CONN_TIMEOUT                     7
#define MGLS_ENV_LICENSE_FILE                     8
#define MGLS_ATTR_QUEUE                           9
#define MGLS_ENV_MGLS_HOME                       10
#define MGLS_ENV_PKGINFO_FILE                    11
#define MGLS_ENV_EXP_WARN_DAYS                   12
#define MGLS_ENV_LICENSE_TEST                    13
#define MGLS_ENV_LICENSE_SOURCE                  14
#define MGLS_ENV_NO_WARNING                      15
#define MGLS_ATTR_NO_ASYNCH                      16
#define MGLS_ATTR_SIGACTION                      17
#define MGLS_ATTR_DISPLAY                        18
#define MGLS_ATTR_FEATURE_TIMEOUT                19
#define MGLS_ATTR_MAX_TIMEOUT                    20
#define MGLS_ATTR_FFDATE                         21
#define MGLS_ATTR_MULTIPLE_COUNTS                22

/*
 *  MGLS_ATTR_MGLS_ASYNCH_CONTROL functionality was deprecated in v2012_1.
 *  You should no longer attempt to set this attribute.  If you have
 *  comments or questions, please send e-mail to lic_help@mentor.com.
 *
 */

#define MGLS_ATTR_MGLS_ASYNCH_CONTROL_DEPRECATED 23

#define MGLS_ATTR_LIC_PER_INVOKE                 24
#define MGLS_ATTR_VFORK                          25
#define MGLS_ATTR_LICENSE_INUSE                  26
#define MGLS_ATTR_ERR_MSG_PREFIX                 27
#define MGLS_ATTR_BLOCK_SIGALRM                  28 /* dts0100551072 */
#define MGLS_ATTR_EXCEPTION_STATUS               30 /* dts0100322288 */
#define MGLS_ATTR_LINGER_INTERVAL                31
#define MGLS_ATTR_LICENSE_FILE                   32
#define MGLS_ATTR_EXCEPTION_TYPE                 33 /* dts0100722319 */
#define MGLS_ENV_INCLUDE_LICENSES                34 /* dts0100740520 */
#define MGLS_ENV_EXCLUDE_LICENSES                35 /* dts0100740520 */
#define MGLS_ATTR_PKGINFO_FILE                   36
#define MGLS_ATTR_DEFAULT_DISPLAY                37
#define MGLS_ATTR_IGNORE_SIGINT                  38 /* dts0100844152 */
#define MGLS_ATTR_CHECKOUT_CONTROL               39 /* dts0100917074 */

#define MGLS_LAST_ATTR_NUMBER                    39

/* this is included for backward compatibility. This attribute may no
   longer be set from the end-user's environment. */
#define MGLS_ENV_NO_ASYNCH          MGLS_ATTR_NO_ASYNCH


/*
**  Following are the default and min/max values
**  for some of the above attributes
*/

/* for MGLS_ATTR_LINGER_INTERVAL  */
#define MGLS_LINGER_OFF            0
#define MGLS_LINGER_RECOMMENDED    3
#define MGLS_LINGER_MIN            1
#define MGLS_LINGER_MAX            MAXINT - 1

/* for MGLS_ATTR_CHECK_INTERVAL attribute */
#define MGLS_CHECK_INT_MIN      30	/* seconds */
#define MGLS_CHECK_INT_DEFAULT  180	/* seconds - 3 minutes */
#define MGLS_CHECK_INT_MAX      3600	/* seconds - 60 minutes - 1 hour */
/*
 * MGLS_CHECK_INT_DEFAULT is the interval for
 * mgls_asynch checking on the mgcld/lmgrd server
 * to notice whether it is still there.
 *
 * MGLS_CHECK_INT_DEFAULT of 3 minutes is intended to be
 * a low-frequency, low-cost, but non-ignorable check.
 */


/* for MGLS_ATTR_RETRY_INTERVAL attribute */
#define MGLS_RETRY_INT_MIN      30	/* seconds */
#define MGLS_RETRY_INT_DEFAULT  180	/* seconds - 3 minutes */
#define MGLS_RETRY_INT_MAX      3600	/* seconds - 60 minutes - 1 hour */
/*
 * MGLS_RETRY_INT_DEFAULT is the interval for
 * mgls_asynch to re-establish contact with a license server
 * and re-acquire the application's licenses when mgls_asynch
 * has noticed that the mgcld/lmgrd server has gone away
 * (or communication has been lost).
 *
 * MGLS_RETRY_INT_DEFAULT of 3 minutes is intended to allow
 * lots of time for manual recovery/restart of the lmgrd server
 * if it goes down, but with some attempt at licensing effectiveness.
 */


/* for MGLS_ATTR_RETRY_COUNT attribute */
#define MGLS_RETRY_CNT_MIN      0
#define MGLS_RETRY_CNT_DEFAULT  20
#define MGLS_RETRY_CNT_MAX      20
/*
 * The combined effect of MGLS_CHECK_INT_DEFAULT, MGLS_RETRY_INT_DEFAULT,
 * and MGLS_RETRY_CNT_DEFAULT allows by default about an hour between the
 * time a connection to a license server is lost, and the end user is
 * presented with a dialog box (for CUI-based applications) informing him/her
 * of that fact. During this one hour period, the MGLS child process tries 20
 * times (once every three minutes) to re-establish communication with the
 * license server.
 *
 * During this time, mgls_asynch transparently tries to re-acquire the lost
 * licenses. If completely successful, then MGLS_CHECK_INT_DEFAULT restarts.
 * If only partially successful, then the licenses that are re-acquired
 * through retries help maintain the effectiveness of licensing (they are
 * unavailable for other processes to [over-]use), which reduces the number
 * of processes that eventually will be forced to exit because licenses
 * have become over-subscribed due to the license server going down and
 * new licenses being granted before existing processes have re-acquired
 * their licenses.
 */

/* In the initial release of MGLS, the use of redundant license servers was
 * not recommended. The default values provided allowed for much more time
 * to re-establish communication with the license server. At the time MGC
 * began recommending the use of redundant servers, the default values were
 * changed to these values.
 */

/* for MGLS_ATTR_CONN_TIMEOUT attribute */
#define MGLS_CONN_TOUT_DEFAULT  10      /* seconds */
#define MGLS_CONN_TOUT_MAX	600	/* seconds = 10 minutes */


/* for MGLS_ATTR_QUEUE  attribute */
#define MGLS_QUEUE_DEFAULT    2
#define MGLS_QUEUE_NOT        0         /* Disable queuing       */
#define MGLS_STAT_MAXUSERS    42


/* used in conjunction with MGLS_ATTR_EXIT_CALL (set up client exit handler) */
#define MGLS_ENOSERVER        1   /* Possible status returned to client's   */
#define MGLS_ENOLICENSE       2   /* exit handler routine                   */
#define MGLS_ENOASYNCH        3   /* used to indicate the mgls_asynch process*/
                                  /* was abnormally terminated - possibly by */
                                  /* a user issuing the Unix kill command    */
#define MGLS_ESUSPEND         4

#define MGLS_CLIENT_RETRY    10   /* Possible values that the client's exit */
#define MGLS_CLIENT_IGNORE   20   /* handler may return to MGLS, specifying */
#define MGLS_CLIENT_EXIT     40   /* what actions MGLS to take              */

/* Used with MGLS_ATTR_MAX_TIMEOUT. */
#define MGLS_MAX_TIMEOUT_DEFAULT 900

/*
**  Following are the environment variable related defines
*/

/* MGLS_ATTR_LICENSE_FILE attribute & LM_LICENSE_FILE environment variable   */
#define MGLS_LICENSE_PATH   "etc/cust/mgls/mgc.licenses" /* at '$MGLS_HOME/' */


/* for MGLS_LICENSE_SOURCE environment variable */
#define MGLS_LICENSE_SOURCE_DEFAULT "ANY"        /* Any license source is OK */


/* for MGLS_EXP_WARN_DAYS environment variable */
#define MGLS_EXP_WARN_MIN           1           /* Day  */
#define MGLS_EXP_WARN_DEFAULT       15          /* Days */
#define MGLS_EXP_WARN_MAX           90          /* Days */

/* for MGLS_LICENSE_TEST environment variable */
#define MGLS_LICENSE_TEST_DEFAULT   0 /* 0->normal operation, -1->test only */

/* for MGLS_NO_WARNING environment variable */
#define MGLS_NO_WARNING_DEFAULT     0 /* 0->OK to output warnings, -1->don't */

typedef enum MGLS_LICTYPE
{
    MGLS_floating,
    MGLS_nodelocked_counted,
    MGLS_nodelocked_uncounted
} mgls_lictype;

enum mgls_licinfo_values {
    MGLS_MAX_FEATURE_LEN=60,   /*  this is twice the value for Flex (30) */
    MGLS_MAX_SERIAL_NO_LEN=50, /*  this length is arbitrarily chosen - data will be truncated to fit here if necessary  */
    MGLS_SERVER_HOSTID_LEN=(1024+9),
    MGLS_LOCATION_LEN=1034     /*  matches FLEX length for port@host: MAX_HOSTNAME + MAX_SHORT_LEN + 5  */
};

/* structure containing license information returned by the                 */
/* core_Mheap_licinfo function                                              */
typedef struct MGLS_LICINFO
{
   char         feature_name[MGLS_MAX_FEATURE_LEN];
   mgls_lictype license_type;
   int          next_count;
   int          expiration_info;
   char         serial_number[MGLS_MAX_SERIAL_NO_LEN];
   int          count;
   int          granted_application_id;
   char         server_hostid[MGLS_SERVER_HOSTID_LEN];
   int          exact_access_year;
   int          exact_access_month;
   char         granted_location[MGLS_LOCATION_LEN];
} mgls_licinfo;

#define MGLS_UNKNOWN_HOSTID "unknown"
#define MGLS_UNCOUNTED_HOSTID "uncounted"

/* for MGLS_ATTR_SIGACTION */
#define MGLS_AS_preexist	0	/* preserve original behavior */
#define MGLS_AS_restart		1	/* add SA_RESTART to sa_flags */
#define MGLS_AS_interrupt	2	/* add SA_INTERRUPT to sa_flags */

/* for MGLS_ATTR_BLOCK_SIGALRM */
#define MGLS_SIGALRM_ALLOWED    0
#define MGLS_SIGALRM_BLOCKED    1

/* for core_Mheap_ferrs */
typedef struct mgls_FLEX_err {
    int flex_stat;
    char* req_feat;
    char* act_feat;
    char* server;
    char* flex_err;
} MGLS_FLEX_ERR;

/* for MGLS_ATTR_LICENSE_FILE */
#define MGLS_TRUNCATED_ENV_SIZE 1024

/* for MGLS_ATTR_EXCEPTION_TYPE */
enum mgls_exception_type {
    MGLS_DEFAULT_EXCEPTION = 0,
    MGLS_POLLING_EXCEPTION = 1
};

/* for core_Mheap_lost */
#define MGLS_SERVER_LEN 1024

typedef struct mgls_lost_features {
    char granted_feature[MGLS_MAX_FEATURE_LEN];
    char server[MGLS_SERVER_LEN];
    int dongle_lost;
    int retry_count;
    long duration;
} MGLS_LOST_FEATURES;

/* for core_Mheap_count */
typedef enum MGLS_COUNT_FLAGS {
   MGLS_COUNT_DEFAULT = 0
} mgls_count_flags;

/* associated with core_Mheap_candidates functionality  */
typedef enum MGLS_EXACT_TYPES {
   MGLS_CHECKOUT_DEFAULT = 0,
   MGLS_EXPLICIT_CHECKOUT = 1
} mgls_exact_types;

#endif
