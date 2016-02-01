/*  -*- mode:C++; tab-width:4; c-basic-offset:4; indent-tabs-mode:nil; -*-   */
/******************************************************************************
 *                                                                            *
 * Copyright (c) Mentor Graphics Corporation, 1990-2014, All Rights Reserved. *
 *                      UNPUBLISHED, LICENSED SOFTWARE.                       *
 *           CONFIDENTIAL AND PROPRIETARY INFORMATION WHICH IS THE            *
 *         PROPERTY OF MENTOR GRAPHICS CORPORATION OR ITS LICENSORS.          *
 *                                                                            *
 *                                  NOTICE                                    *
 *                                                                            *
 * This source code belongs to Mentor Graphics Corporation.  It is considered *
 * trade secret and is not to be divulged or used by parties who have not     *
 * received written authorization from the owner.                             *
 *                                                                            *
 *****************************************************************************/

#include <stdio.h>
#include <dcs_license.h>
#include <mgls_client.h>
#include <time.h>

#include <map>
#include <vector>
#include <set>
#include <algorithm>

#ifdef _MSC_VER
#include <windows.h>
#include <process.h>
#include <tlhelp32.h>
#define sleep(secs) Sleep(secs*1000)
#else
#include <sys/utsname.h>
#include <unistd.h>
#include <stdarg.h>
#endif

//#define LICENSE_USE_CLIM

#ifdef  LICENSE_USE_CLIM
#include "cds_clip/CLIM.h"
#endif

/////////////////////////////////////////////////////////////////////////////

// Code must be compiled DEBUG to enable stubs
#if defined(DEBUG_LICENSE_STUB) // && !defined(DEBUG)  -- NOT SUPPORTED AT THIS TIME
# undef DEBUG_LICENSE_STUB
#endif

// #define DEBUG_LICENSE 1
#ifdef DEBUG_LICENSE
# include <iostream>
# include <iomanip>
using std::cout;
using std::endl;
using std::setw;
using std::setiosflags;
#endif

#if defined(DEBUG_LICENSE) && DEBUG_LICENSE > 1
# define DEBUG_LICENSE_print void print() const;
#else
# define DEBUG_LICENSE_print inline void print() const {}
#endif

#if defined(_MSC_VER) && !defined(DEBUG)
# pragma warning( disable : 4355 )  // Disable: 'this' : used in base member initializer list
#endif

//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////

// mgls headers only define MGLS_ATTR_VALUE under Linux as they do in the
// headers for Windows, but they do not, so we are forced to apply a hack here:
#ifdef _WIN64
typedef long long MGLS_ATTR_VALUE;
#else
typedef long MGLS_ATTR_VALUE;
#endif

static const unsigned int max_valid_secs = 120;
static char mgls_app_date[] = __DATE__;
static const char unknown_str[] = "unknown";
static const char null_str[] = "";

static void (*mgls_print_handler)(MGSG_LicenseMessages::Index, 
								  MGSG_LicenseMessages::Severity, const char *) = 0;

/*==========================================================================
- ============================ UTILITY FUNCTIONS ===========================
- ========================================================================== */

static bool MGSG_is_digits(const char *s) {
	if (!s) 
		return(false);
	while(*s >= '0' && *s <= '9')
		++s;
	return !*s;
}

static void MGSG_GetHostName(std::string &result) {
#ifdef _MSC_VER
	// Computer name (nodename)
	char buffer[512];
	DWORD len = sizeof (buffer) - 1;
	if (GetComputerNameA (buffer, &len))
		result = buffer;
#else
	struct utsname uname_info;
	uname( &uname_info );
	result = uname_info.nodename;
#endif
}

static int MGSG_GetPid(bool parent=false) {
#ifdef _MSC_VER
	DWORD curProcessID = GetCurrentProcessId();
	if (!parent)
		return curProcessID;
	DWORD parentProcessID = -1;
	HANDLE hProcessSnap = CreateToolhelp32Snapshot( TH32CS_SNAPPROCESS, 0 );
	if( hProcessSnap != INVALID_HANDLE_VALUE ) {
		PROCESSENTRY32 pe32;
		pe32.dwSize = sizeof(PROCESSENTRY32);
		for (BOOL pValid = Process32First( hProcessSnap, &pe32 );
			 pValid != FALSE; pValid = Process32Next( hProcessSnap, &pe32 )) {
			if (pe32.th32ProcessID == curProcessID) {
				parentProcessID = pe32.th32ParentProcessID;
				break;
			}
		}
		CloseHandle( hProcessSnap );
	}
	return parentProcessID;
#else
	// This is problematic on linux because it includes threads!
	return (int)(parent ? getppid() : getpid());
#endif
}


static const char *MGSG_GetDisplayEnv() {
	return getenv("DISPLAY");
}

static void MGSG_GetDisplay(std::string &result) {
	const char *displayEnv = MGSG_GetDisplayEnv();
	if (displayEnv && *displayEnv)
		result = displayEnv;
	else
		MGSG_GetHostName(result);
	if (! result.empty()) {
		char buffer[32];
		sprintf(buffer, "-%05d", MGSG_GetPid());
		result += buffer;
	}
}

static bool MGSG_PutEnv(const char varname[], const char value[])
{
    bool fail = true;
#ifdef _MSC_VER
    fail = !SetEnvironmentVariable(varname, value);
#else
    unsigned int varname_len = strlen(varname);
    char *nbuf = new char[varname_len + (value ? strlen(value) : 0) + 2];
	char *p = nbuf;
	strcpy(p, varname);	p += varname_len;
	*p++ = '=';
	if (value)
		strcpy(p, value);
	if (fail = !!putenv(nbuf))
        delete [] nbuf;
#endif
    return !fail;
}

static bool is_special_pid(int pid)
{
	return pid == 4099 || pid == 4098; // mentorall_s / mgc_s
}

static const char *get_type_str(MGSG_License_Type type)
{
    // MUST match enumerated type
	static const char *typeToStr[] = {
		"uninitialized",
		"disabled",
		"unused",
		"unknown",
        "unix",
		"nodelocked-uncounted",
		"nodelocked-counted",
		"floating"
	};
	return typeToStr[type];
}

#ifdef DEBUG_LICENSE_STUB
static unsigned int get_compile_time_secs(bool *success = 0) {
    static unsigned int compile_time_secs = 0;
	static bool parse_okay = false;
    if (compile_time_secs == 0) {
        struct tm compile_tm;
        const char date_str[] = __DATE__; // Mmm DD YYYY
        const char time_str[] = __TIME__; // hh:mm:ss

        bool parsed_date = false;

        const char month_str[][4] = { "Jan", "Feb", "Mar", "Apr",
                                      "May", "Jun", "Jul", "Aug",
                                      "Sep", "Oct", "Nov", "Dec" };

        for (compile_tm.tm_mon = 0; compile_tm.tm_mon < (int)(sizeof(month_str)/sizeof(month_str[0]));
             ++compile_tm.tm_mon) {
            if (!strncmp(month_str[compile_tm.tm_mon], date_str, sizeof(month_str[0])-1))
                break;
        }
        if (compile_tm.tm_mon < (int)(sizeof(month_str)/sizeof(month_str[0])) 
            && date_str[sizeof(month_str[0])-1] == ' ') {
            const char *ptr = date_str + sizeof(month_str[0]);
            if ((ptr[0] == ' ' || ptr[0] >= '0' && ptr[0] <= '9')
                && (ptr[1] >= '0' && ptr[1] <= '9')) {
                char buf[] = "00";
                if (ptr[0] != ' ')
                    buf[0] = ptr[0];
                buf[1] = ptr[1];
                compile_tm.tm_mday = atoi(buf);
                ptr += 2;
                if (*ptr == ' ') {
                    ++ptr;
                    if ((ptr[0] >= '0' && ptr[0] <= '9')
                        && (ptr[1] >= '0' && ptr[1] <= '9')
                        && (ptr[2] >= '0' && ptr[2] <= '9')
                        && (ptr[3] >= '0' && ptr[3] <= '9')
                        && ptr[4] == '\0') {
                        int ayear = atoi(ptr);
                        if (ayear > 1900) {
                            compile_tm.tm_year = ayear - 1900;
                            parsed_date = true;
                        }
                    }
                }
            }
        }

        bool parsed_time = false;
        if (parsed_date) {
            if ((time_str[0] == ' ' || time_str[0] >= '0' && time_str[0] <= '9')
                && (time_str[1] >= '0' && time_str[1] <= '9')
                && (time_str[2] == ':')) {
                char buf[] = "00";
                if (time_str[0] != ' ')
                    buf[0] = time_str[0];
                buf[1] = time_str[1];
                compile_tm.tm_hour = atoi(buf);
                if ((time_str[3] >= '0' && time_str[3] <= '9')
                    && (time_str[4] >= '0' && time_str[4] <= '9')
                    && (time_str[5] == ':')) {
                    buf[0] = time_str[3];
                    buf[1] = time_str[4];
                    compile_tm.tm_min = atoi(buf);
                    if ((time_str[6] >= '0' && time_str[6] <= '9')
                        && (time_str[7] >= '0' && time_str[7] <= '9')
                        && (time_str[8] == '\0')) {
                        buf[0] = time_str[6];
                        buf[1] = time_str[7];
                        compile_tm.tm_sec = atoi(buf);
                        parsed_time = true;
                    }
                }
            }
        }

        compile_tm.tm_wday = 0;
        compile_tm.tm_yday = 0;
        compile_tm.tm_isdst = -1;

        if (parsed_time && parsed_date) {
            time_t tsecs = mktime(&compile_tm);
            if (tsecs > time(0)) // make sure time is not in future
                tsecs = time(0);
			else
				parse_okay = true;
            compile_time_secs = (unsigned int)tsecs;
        } else {
            time_t lsecs = time(0); // unabled ot parse return current time
            compile_time_secs = (unsigned int)lsecs;
        }
    }

	if (success)
		*success = parse_okay;

    return compile_time_secs;
}
#endif

//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////

/*==========================================================================
- ============================== STUB CLIM API =============================
- ========================================================================== */

#ifndef  LICENSE_USE_CLIM
namespace CLIM
{
    using std::string;
    using std::list;

    typedef void (*CallbackWithIntArg)(int value);
    typedef void (*CallbackWithStringArg)(const string& msg);
    typedef void (*CallbackWithVarg)(const char* format, ...);

    struct Options
    {
        long pgid;
        bool heartbeat;
        string dup_code;
        string lm_license_file;
        string pkginfo_file;
        bool timestamp_in_liclog;
        bool call_reconn_cb_if_ok;
        bool do_not_close_forked_child_pipedes_in_parent;

        CallbackWithVarg      p_log_cb;
        CallbackWithStringArg p_abort_cb;
        CallbackWithIntArg    p_heartbeat_reconn_cb;
        CallbackWithIntArg    p_heartbeat_fail_cb;

        void                 *p_clientData;

        Options() { }
    };

    inline bool Enabled() { return false; }

    int Init(const string& clip_path, const Options& options = Options()) { return 1; }
    int Destroy() { return 1; }

    Options &GetOptions() { return *((Options *)0); }

    bool GetEnvVar(const string& name, string *val) { return false; }

    void Ping() { }

    int Checkout(const string& feature_name, int queue_timeout = -1) { return 1; }

    int GetServerName(string* p_server_name) { return 1; }

    int GetDaysLeft(const string& feature_name) { return 1; }

    int GetUserList(const string& feature_name, string* p_userlist) { return 1; }

    enum featureType { featureTypeError, featureTypeUncounted, featureTypeFloating, featureTypeNodelocked };
    featureType GetFeatureType(const string& feature_name) { return featureTypeError; }

    int GetFeatureList(string* p_featlist) { return 1; }

    int GetFeatureServerList(const string& feature_name, string* p_svrlist) { return 1; }

    int GetErrorString(string* p_string) { return 1; }

    typedef list<string> StringList;
    bool CheckoutBackground(const StringList& feature_names, unsigned int retry=0) { return false; }

    bool CheckoutBackgroundResult(StringList& obtained_features, StringList& other_features, unsigned int& num_tries) { return false; }

    class Synchronizer {
    public:
        template<typename VARTYPE>
        struct Value {
            Value() { }
            Value(VARTYPE ivalue) { }
            ~Value() { }
            struct Access {
                Access(Value& v) { }
                ~Access() { }
                VARTYPE& operator*() { return *((VARTYPE *)0); }
            };
        };
    };
}
#endif

//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////

/*==========================================================================
- ======================== CORE LICENSING API CALLS ========================
- ========================================================================== */

class MGSG_LicenseHandlerBase::Core 
{
  enum handlerType { hMGLS, hCLIM };

  handlerType d_errno_own;

  MGSG_LicenseHandlerBase &d_handler;
  Core(const Core &); 

  typedef std::pair<int, std::string *> PidToNamePair;
  struct PidToNamePair_ltstr { 
    bool operator()(const PidToNamePair &s1, const PidToNamePair &s2) const 
    { return s1.first < s2.first; }
  };
  typedef std::set<PidToNamePair, PidToNamePair_ltstr> PidToName;
  PidToName d_pid_to_name; // pid id queried as lookup

  typedef std::pair<const std::string *,int> NameToPidPair;
  struct NameToPidPair_ltstr { 
    bool operator()(const NameToPidPair &s1, const NameToPidPair &s2) const 
    { return s1.first->compare(*s2.first) < 0; }
  };
  typedef std::set<NameToPidPair, NameToPidPair_ltstr> NameToPid;
  NameToPid d_known_pid; // pidname queried as lookup

  struct ServerName {
    std::string name;
    bool defined;
    ServerName() : defined(false) { }
  };
  ServerName d_server;

  typedef NameToPid NameToDaysLeft;
  typedef NameToPidPair NameToDaysLeftPair;
  NameToDaysLeft d_days_left;

  typedef NameToPid NameToLicType;
  typedef NameToPidPair NameToLicTypePair;
  NameToLicType d_lic_type;

  typedef std::vector<int> PidNums;
  typedef std::vector<const std::string *> PidNames;

  class PidToMembers;
  PidToMembers *d_pid_members;

  class Satisfies;
  Satisfies *d_satisfies;

  class PidExists;
  PidExists *d_pid_exists;

  class TransactionIDs;
  TransactionIDs *d_trans_ids;

  CLIM::Options *d_clim_ops;
  enum CLIM_mode { CLIM_enabled, CLIM_disabled, CLIM_running, CLIM_exited_error };
  CLIM_mode d_clim_mode;

  enum MGLS_mode { MGLS_enabled, MGLS_disabled };
  MGLS_mode d_mgls_mode;

  struct Ids; 
  class ContainsInfo;
  typedef std::pair<int, ContainsInfo *> PidToContainsPair;
  struct PidToContains_ltstr { 
    bool operator()(const PidToContainsPair &s1, const PidToContainsPair &s2) const 
    { return s1.first < s2.first; }
  };
  typedef std::set<PidToContainsPair, PidToContains_ltstr> PidToContains;
  PidToContains *d_clim_to_pids; // pid id queried, returns "contains" data for CLIM registered

  bool HasTransaction(handlerType htype) const;

  void CLIM_use_start();
  inline bool CLIM_use() 
  {
      if (d_clim_mode == CLIM_running) {
          CLIM::Ping();
          if (*SyncClimStatus::Access(d_clim_cb_status) == climAborted) 
            d_clim_mode = CLIM_enabled;
      }
      if (d_clim_mode == CLIM_enabled)
          CLIM_use_start();
      return d_clim_mode == CLIM_running;
  }
  inline bool MGLS_use() const
  {
      return d_mgls_mode == MGLS_enabled;
  }

  ///////////////////////////////////////////////////////////////////////////////////////////////////////////
  typedef std::vector<std::string> Strings;

  static bool isSplitChar(const char c, const char *split_chars, unsigned int char_sz)
  {
    for (unsigned int i = 0; i < char_sz; ++i) {
      if (split_chars[i] == c)
        return true;
    }
    return false;
  }

  static void splitStr(Strings &s, const char *str, const char *split_chars, unsigned int split_char_sz)
  {
    const char *epos = str ? str : "";
    char c;
    while (*epos != '\0') {
      while (!isSplitChar((c = *epos), split_chars, split_char_sz) && c != '\0')
        ++epos;
      s.push_back(std::string(str,(epos-str)));
      if (c != '\0')
        str = ++epos;
    }
  }

  static void splitStr(Strings &s, const char *str)
  {
    const char split_chars[] = { ':', ';' };
    splitStr(s, str, split_chars, sizeof(split_chars));
  }

  ///////////////////////////////////////////////////////////////////////////////////////////////////////////
  void namesToKnownPidNums(const Strings &names, PidNums *res) const
  {
    for (Strings::const_iterator s = names.begin(); s !=  names.end(); ++s) {
      NameToPid::const_iterator p = d_known_pid.find(NameToPidPair(&(*s),0));
      if (p != d_known_pid.end()) {
        res->push_back((*p).second);
      }
    }
  }

  bool nameToPidNum(const std::string &pidname, int *pid) const
  {
      NameToPid::const_iterator p = d_known_pid.find(NameToPidPair(&pidname,0));
      if (p != d_known_pid.end()) {
        *pid = (*p).second;
        return true;
      }
      return false;
  }

  inline bool nameToPidNum(const std::string *pidname, int *pid) const
  { return pidname && nameToPidNum(*pidname, pid); }

  ///////////////////////////////////////////////////////////////////////////////////////////////////////////

  inline void init_Satisfies() { if (! d_satisfies) { init_PidExists(); init_Satisfies(&d_satisfies); } }
  void init_Satisfies(Satisfies **obj);

  inline void init_PidExists() { if (! d_pid_exists) { init_PidExists(&d_pid_exists); } }
  void init_PidExists(PidExists **obj);

  inline void init_PidToMembers() { if (! d_pid_members) init_PidToMembers(&d_pid_members); }
  void init_PidToMembers(PidToMembers **obj);

  inline void init_TransactionIDs() { if (! d_trans_ids) { init_Satisfies(); init_TransactionIDs(&d_trans_ids); } }
  void init_TransactionIDs(TransactionIDs **obj);

  ///////////////////////////////////////////////////////////////////////////////////////////////////////////

  bool d_exp_warn_days_unset;
  long d_exp_warn_days;

  inline long Mheap_ret(handlerType h, long ret) { d_errno_own = h; return ret; }
  inline long Mheap_set_errno(handlerType h, long errno_, long ret = MGLS_ERR) 
  { d_errno_own = h; mgls_errno = errno_; return ret; }

  enum climStatus { climOkay, climReconnecting, climAborted };
  typedef CLIM::Synchronizer::Value<climStatus> SyncClimStatus;
  SyncClimStatus d_clim_cb_status;
  int d_clim_num_tries;

  static void clim_log_cb(const char* format, ...);
  static void clim_abort_cb(const std::string& msg);
  static void clim_hrtbeat_reconn_cb(int num_retries);
  static void clim_hrtbeat_fail_cb(int max_retries);

  ///////////////////////////////////////////////////////////////////////////////////////////////////////////

  enum lostMode { 
      lostModeUndefined,
      flexDoingReaquire,      // just wait for d_clim_cb_status to equal climOkay
      climDoingReaquire,      // do background checkout, and monitor background result
      climDoingReaquireAgain, // retry background checkout, and monitor background result
      climRestarted           // clim was restarted, but the licenses have not been requested
  };
  lostMode d_lost_mode;
  char *d_lost_buf;

  enum { clim_appid_start = -50000 };
  int d_next_clim_appid;

  inline int next_clim_only_appid() { int r = d_next_clim_appid; --d_next_clim_appid; return r; }


  class BlockSigsB {
      struct N {
#   ifdef _MSC_VER
          N() { }
          ~N() { }
#   else
          sigset_t oldmask;
          N() {
              sigset_t newmask;
              sigemptyset(&newmask);
              sigaddset(&newmask, SIGSTOP);
              sigaddset(&newmask, SIGTSTP);
              pthread_sigmask(SIG_BLOCK, &newmask, &oldmask);
#           ifdef DEBUG_LICENSE
              cout << "// Debug: signals blocked for license thread\n";
#           endif
          }
          ~N() {
              pthread_sigmask(SIG_SETMASK, &oldmask, NULL);
#           ifdef DEBUG_LICENSE
              cout << "// Debug: signals restored for license thread\n";
#           endif
          }
#   endif
      };
      unsigned int *const d_cnt;
      N *d_n;
  public:
      BlockSigsB(BlockSigsB &obj) 
          : d_cnt(obj.d_cnt), d_n((*d_cnt) == 0 ? (obj.d_n=new N) : obj.d_n) { ++(*d_cnt); }
      BlockSigsB() : d_cnt(new unsigned int), d_n(0) { *d_cnt = 0; }
      ~BlockSigsB() 
      { 
          if (!d_n || *d_cnt == 0) // d_n never assigned, e.g. no stack, or no refs
              delete d_cnt;
          else if (--(*d_cnt) == 0)
              delete d_n;
      }
  };
  BlockSigsB d_blocksigs;  // used to block signals during license transactions (UNIX) 
    
public:
  Core(MGSG_LicenseHandlerBase &handler) 
    : d_errno_own(hMGLS)
    , d_handler(handler), d_pid_members(0), d_satisfies(0), d_pid_exists(0), d_trans_ids(0) 
    , d_clim_ops(0), d_clim_mode(CLIM_disabled)
    , d_mgls_mode(MGLS_enabled)
    , d_clim_to_pids(0)
    , d_exp_warn_days_unset(true), d_exp_warn_days(MGLS_EXP_WARN_DEFAULT)
    , d_clim_cb_status(climOkay), d_clim_num_tries(0), d_lost_mode(lostModeUndefined), d_lost_buf(0)
    , d_next_clim_appid(clim_appid_start)
  { }
  ~Core();
  long Mheap_get_errno();
  long Mheap_errs( char **error_string );
  long Mheap_allocv( int app_id, char *app_date );
  long Mheap_stat( long transaction_id );
  long Mheap_free( long transaction_id );
  long Mheap_clean( );
  long Mheap_set( int which, long new_value );
  long Mheap_get( int  which, MGLS_ATTR_VALUE *new_value );
  long Mheap_validv( int app_id, char *app_date );
  long Mheap_licinfo( struct MGLS_LICINFO *license_info );
  long Mheap_contains( int pid, int *num_of_ids, int **id_list );
  long Mheap_lookup( int  pid, char *fname );
  long Mheap_member( int pid, int *num_of_ids, const int **id_list );
  long Mheap_lost( int *num_features, MGLS_LOST_FEATURES *lost_features[] );
  long Mheap_exists( int app_id );

  struct BlockSigs : private BlockSigsB {
      BlockSigs(Core &core) : BlockSigsB(core.d_blocksigs) { }
      ~BlockSigs() { }
  };

  // initialize the feature table
  void init() { init_TransactionIDs(); }

  // extended -- return const reference to std::string of preallocated buffer
  long Mheap_lookup( int  pid, const std::string **fname );

  // extended -- checks appid for allocation by allocateCLIM
  inline bool is_clim_only_appid(int appid) const { return appid <= clim_appid_start && appid > d_next_clim_appid; }

  // extended -- add support to allocate a new "named" entry for allocation
  //             on CLIM only interface, cannot be checkedout/validv on MGLS
  int allocateCLIM(const char *MGSG_CRYPT_pidname);

  // extended -- add support to have "composite" like CLIM features
  bool RegisterCLIM_contains(MGSG_MGLS_pid &hpid
    , const MGSG_MGLS_pid *pids[], unsigned int pids_num_of
    , const MGSG_MGLS_pid *additional_co[], unsigned int additional_co_num_of);

  // extended -- configure daemon support
  bool enableDaemon(daemonType dtype, bool val)
  {
      if (dtype == MGCLD_daemon)
          d_mgls_mode = val ? MGLS_enabled : MGLS_disabled;
      else if (dtype == CALYPTOD_daemon) {
          if (d_clim_mode != CLIM_enabled && d_clim_mode != CLIM_disabled)
              return false;
          else if (val && !CLIM::Enabled())
              return false;
          d_clim_mode = val ? CLIM_enabled : CLIM_disabled;
      } else
          return false;
      return true;
  }
};

//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////

/*==========================================================================
- ========================== MESSAGING INTERFACE  ==========================
- ========================================================================== */

class MGSG_LicenseMessages::Core {
  MGSG_LicenseHandlerBase::Core &d_core;
public:
  Core(MGSG_LicenseHandlerBase::Core &core) : d_core(core) { }
  inline MGSG_LicenseHandlerBase::Core & operator()() const { return d_core; }
  inline operator MGSG_LicenseHandlerBase::Core &() const { return d_core; }
};

void MGSG_LicenseMessages::reset() {
	for (list_MGSG_LicenseMessage::iterator i = begin(), iend=end(); i != iend; ++i)
	{
		MGSG_LicenseMessage *m = *i; 
		delete m;
    }
    clear();
}

void MGSG_LicenseMessages::take(MGSG_LicenseMessages &messages) {
	while(messages.size()) {
		push_back(*messages.begin());
		messages.pop_front();
	}
}

void MGSG_LicenseMessages::append(const MGSG_LicenseMessages &messages) {
	for (list_MGSG_LicenseMessage::const_iterator i = messages.begin(); 
		 !(i == messages.end()); ++i ) {
		const MGSG_LicenseMessage *msg = (*i);
		push_back(new MGSG_LicenseMessage(*msg));
	}
}

void MGSG_LicenseMessages::print(MGSG_LicenseMessages::Severity severity_ge) const {
	if (!d_print)
		printf("%s", this->get(severity_ge));
	else for (list_MGSG_LicenseMessage::const_iterator i = begin(); !(i == end()); ++i ) {
		const MGSG_LicenseMessage *msg = (*i);
		if (msg->severity() >= severity_ge || NoSuppress(msg->index()))
			d_print(msg->index(), msg->severity(), msg->get());
	}
}

const char *MGSG_LicenseMessages::get(MGSG_LicenseMessages::Severity severity_ge,
									  bool noprefix) const {
	MGSG_LicenseMessages *cached_val = (MGSG_LicenseMessages *)this;
	cached_val->d_str.erase();
	for (list_MGSG_LicenseMessage::const_iterator i = begin(); !(i == end()); ++i ) {
		const MGSG_LicenseMessage *msg = (*i);
		if (msg->severity() >= severity_ge || NoSuppress(msg->index())) {
			if (!noprefix) {
				switch (msg->severity()) {
				case SeverityError:
				case SeverityUrgent:
					cached_val->d_str += "Error: ";
					break;
				case SeverityWarning:
					cached_val->d_str += "Warning: ";
					break;
				default:
					break;
				}
			}
			cached_val->d_str += msg->get();
		}
	}
	return d_str.c_str();
}

bool MGSG_LicenseMessages::NoSuppress(MGSG_LicenseMessages::Index index)
{
	// These messages will not be suppressed if the "quiet" flag is present
	if (index & SeverityUrgent)
		return true;
	switch(index) {
	case MSG_INF_ExpireSoon1:
	case MSG_INF_ExpireSoon2:
	case MSG_INF_BetaFeatureExpiration:
	case MSG_URG_ContactingLicenseServer:
		return true;
	default:
		break;
	}

	return false;
}

const char *MGSG_LicenseMessages::GetErrMsgFmt(MGSG_LicenseMessages::Index index)
{
	switch(index) {
	case MSG_INF_Default:
		return "%s";
	case MSG_INF_ExpireSoon1:
		return "Your %s license is about to expire";
	case MSG_INF_ExpireSoon2:
		return "Please contact your account representative for continued access to this feature.";
	case MSG_INF_BetaFeatureExpiration:
		return "%s software feature will expire in %d day%s.";
	case MSG_WARN_Default:
		return "%s";
	case MSG_WARN_LicenseCheckinFailed:
		return "License check-in failed, mgls_errno = %ld";
	case MSG_WARN_LicenseException:
		return "License exception from MGLS, mgls_errno = %ld";
	case MSG_WARN_QueuedLicense:
		return "No licenses are available, waiting for license...";
	case MSG_WARN_ReaquireLicense:
		return "Attempting to reacquire license, please wait...";
	case MSG_ERR_Default:
		return "%s";
	case MSG_ERR_LicenseRequestFailed:
		return "License request failed, pid = %s, mgls_errno = %ld";
	case MSG_ERR_UnexpectedLicenseReqResponse:
		return "License request failed, mgls_errno = %ld";
	case MSG_ERR_ContactCustomerSupport:
		return "Please contact customer service for assistance.";
	case MSG_ERR_DisabledLicenseRequestPID:
		return "License request failed, pid = %s, mgls_errno = 9999";
	case MSG_ERR_DisabledLicenseRequestName:
		return "License request for %s feature failed (feature disabled)";
	case MSG_ERR_license_is_not_valid:
		return "License verification failed, pid = %s, (feature unavailable)";
	case MSG_ERR_LicenseNotRegistered:
		return "Internal license registration problem encountered, unable to checkout license";
	case MSG_ERR_no_duplicates_allocated:
		return "License checkin failed, pid = %s, no duplicates allocated";
	case MSG_ERR_no_actuals_allocated:
		return "License checkin failed, pid = %s, no actuals allocated";
	case MSG_ERR_no_references_allocated:
		return "License unreference failed, pid = %s, no reference allocated";
	case MSG_URG_LicenseProblemsEncountered:
		return "Licensing problems encountered, exiting.";
	case MSG_URG_ContactingLicenseServer:
		return "Unable to contact license server, exiting.";
	case MSG_URG_BetaFeatureExpired:
		return "Access to feature %s has been blocked, software feature expired.";
	default:
		return "%s";
	}
	return 0;
}

MGSG_LicenseMessages::Severity MGSG_LicenseMessages::Index_To_Severity_enum(MGSG_LicenseMessages::Index index)
{
	if (index & SeverityError)
 		return SeverityError;
	if (index & SeveritySuccess)
		return SeveritySuccess;
	if (index & SeverityInformational)
		return SeverityInformational;
	if (index & SeverityWarning)
		return SeverityWarning;
	return SeverityUrgent;
}

MGSG_LicenseMessages::Index MGSG_LicenseMessages::MGLS_To_License_enum(unsigned int errno_)
{
	switch(errno_) {
	case MGLS_problem_starting_child:
		return MSG_URG_problem_starting_child;
	case MGLS_no_mgls_home:
		return MSG_URG_no_mgls_home;
	case MGLS_no_falconfw_license:
		return MSG_ERR_no_falconfw_license;
	case MGLS_pkginfofile_corrupt:
		return MSG_URG_pkginfofile_corrupt;
	case MGLS_svr_comm_failure:
		return MSG_URG_svr_comm_failure;
	case MGLS_license_file_problem:
		return MSG_URG_license_file_problem;
	case MGLS_unknown_attribute:
		return MSG_URG_unknown_attribute;
	case MGLS_error_in_set_lou:
		return MSG_URG_error_in_set_lou;
	case MGLS_malloc_failure:
		return MSG_ERR_malloc_failure;
	case MGLS_persistent_lic_problem:
		return MSG_ERR_persistent_lic_problem;
	case MGLS_license_request_failed:
		return MSG_ERR_license_request_failed;
	case MGLS_invalid_license_version:
		return MSG_ERR_invalid_license_version;
	case MGLS_subversion_detected:
		return MSG_ERR_subversion_detected;
	case MGLS_somesort_of_failure:
		return MSG_ERR_somesort_of_failure;
	case MGLS_comm_failure:
		return MSG_ERR_comm_failure;
	case MGLS_flexlm_failure:
		return MSG_ERR_flexlm_failure;
	case MGLS_no_license_source:
		return MSG_ERR_no_license_source;
	case MGLS_waiting_in_queue:
		return MSG_INF_waiting_in_queue;
	case MGLS_checking_queue_status:
		return MSG_INF_checking_queue_status;
	case MGLS_reconnect_in_progress:
		return MSG_INF_reconnect_in_progress;
	case MGLS_reconnect_done:
		return MSG_INF_reconnect_done;
	case MGLS_waiting_for_response:
		return MSG_INF_waiting_for_response;
	case MGLS_trans_id_not_found:
		return MSG_WARN_trans_id_not_found;
	case MGLS_exp_warning_msg:
		return MSG_WARN_exp_warning_msg;
	case MGLS_attr_out_of_range:
		return MSG_WARN_attr_out_of_range;
	case MGLS_queue_not_requested:
		return MSG_WARN_queue_not_requested;
	case MGLS_lost_license:
		return MSG_WARN_lost_license;
	case MGLS_lost_connection:
		return MSG_WARN_lost_connection;
	case MGLS_somesort_of_warning:
		return MSG_WARN_somesort_of_warning;
	case MGLS_wont_override_env:
		return MSG_WARN_wont_override_env;
	case MGLS_checkin_warning:
		return MSG_WARN_checkin_warning;
	case MGLS_flexlm_warning:
		return MSG_WARN_flexlm_warning;
	case MGLS_unknown_license_name:
		return MSG_URG_unknown_license_name;
	case MGLS_unknown_license_id:
		return MSG_URG_unknown_license_id;
	case MGLS_invalid_version_num:
		return MSG_URG_invalid_version_num;
#  ifdef MGLS_lost_asynch
	case MGLS_lost_asynch:
		return MSG_WARN_lost_asynch;
#  endif
#  ifdef MGLS_license_info_error
	case MGLS_license_info_error:
		return MSG_ERR_license_info_error;
#  endif
#  ifdef MGLS_flexlm_server_too_old
	case MGLS_flexlm_server_too_old:
		return MSG_URG_flexlm_server_too_old;
#  endif
	default:
		break;
	}
	return MSG_ERR_Default;
}

void MGSG_LicenseMessages::append(Index index, ...)
{
	const char *fmt = GetErrMsgFmt(index);
	Severity severity = Index_To_Severity_enum(index);
	if (fmt) {
		va_list args;
		va_start(args,index);
    
		static char buffer[7168];       /* assumes buffer is big enough - STATIC NTS*/
		int bufferLen = vsprintf(buffer, fmt, args);
    
		va_end(args);
    
		if (bufferLen) {
			if (buffer[bufferLen-1] != '\n') {
				buffer[bufferLen] = '\n';
				buffer[++bufferLen] = '\0';
			}
			push_back(new MGSG_LicenseMessage(index, severity, buffer));
#ifdef DEBUG_LICENSE
			cout << "// Debug: mgls message " << buffer;
#endif
		}
	}
}

void MGSG_LicenseMessages::appendMheapErrs(const Core &core)
{
    Index mgls_errorno_idx = MGLS_To_License_enum(core().Mheap_get_errno());
	char mgls_error_msg_buf[SIZE_OF_MGLS_ERROR_STRING];
	char *mgls_error_msg = mgls_error_msg_buf;
	if (core().Mheap_errs(&mgls_error_msg) == 0) {

		char *ptr = mgls_error_msg ;
		do {
			if (*ptr == 0 || *ptr == '\n') {
				// Advance by "//  "
				if (mgls_error_msg[0] == '/' && mgls_error_msg[1] == '/' &&
					mgls_error_msg[2] == ' ' && mgls_error_msg[3] == ' ')
					mgls_error_msg += 4;
				// "Strip off" the newline
				char b = *ptr;
				*ptr = 0;
				if (mgls_error_msg < ptr)
					this->append(mgls_errorno_idx, mgls_error_msg);
				if (b == '\n')
					*ptr++ = b;
				mgls_error_msg = ptr;
			}
		} while(*ptr++);
	}
}

/*==========================================================================
- ========================== HANDLER::SERIALNUMS ===========================
- ========================================================================== */

//////////////////////////////////////////////////////////////////////////////
// Class Definition : The SerialNums class maintains a counted list of 
//                    license serial nums associated with a transaction.
//////////////////////////////////////////////////////////////////////////////

struct MGSG_LicenseHandlerBase::SerialNums::Node {
	char *s;
	unsigned int slen;
	Node *next;
	Node(const char *_s, unsigned int _slen)
		: s(new char [_slen+1]), slen(_slen), next(0)
	{ memcpy(s,_s,_slen); s[slen] = '\0'; }
	~Node() { delete [] s; }
};

char *MGSG_LicenseHandlerBase::SerialNums::null_str() 
{ 
	return (char *)::null_str; 
}

void MGSG_LicenseHandlerBase::SerialNums::update() const 
{
	SerialNums *cache = (SerialNums *)this;
	cache->d_update = false;
	if (d_str != null_str())
		delete [] cache->d_str;
	if (!d_head)
		cache->d_str = null_str();
	else {
		char *wptr = new char [d_strl+1];
		cache->d_str = wptr;
		for (Node *i=d_head; i; i=i->next) {
			memcpy(wptr, i->s, i->slen);
			wptr += i->slen;
			*wptr++ = ' ';
		}
		*--wptr = '\0';
	}
}

void MGSG_LicenseHandlerBase::SerialNums::add(const char *s, const unsigned int slen)
{
	if (d_head) {
		Node *l=0;
		for (Node *i=d_head; i; l=i, i=i->next) {
			if (i->slen == slen && !strcmp(i->s, s)) {
				l=0;
				break;
			} 
		}
		if (l) {
			l->next = new Node(s, slen);
			d_strl += (slen+1);
			d_update = true;
		}
	} else {
		d_head = new Node(s, slen);
		d_strl = (slen+1);
		d_update = true;
	}
}

void MGSG_LicenseHandlerBase::SerialNums::clear() {
	while (d_head) {
		Node *c = d_head;
		d_head = d_head->next;
		delete c;
	}
	if (d_str != null_str())
		delete [] d_str;
	d_str = (char *)null_str();
	d_update = false;
}

void MGSG_LicenseHandlerBase::SerialNums::add(const char *s) 
{
	add(s, static_cast<unsigned int>(strlen(s))); 
}

void MGSG_LicenseHandlerBase::SerialNums::add(const SerialNums &s) {
	for (Node *i=s.d_head; i; i=i->next)
		add(i->s, i->slen);
}


static const MGSG_LicenseHandlerBase::SerialNums s_nullSerialNums;

/*==========================================================================
- =========================== HANDLER::PIDPOOL =============================
- ========================================================================== */

//////////////////////////////////////////////////////////////////////////////
// Class Definition : The PidPool class maintains a reference counted
//                    list of "known" int pids.  This includes composites
//                    and atomics.
//////////////////////////////////////////////////////////////////////////////

class MGSG_LicenseHandlerBase::PidPool
{
public:
	PidPool(MGSG_LicenseHandlerBase &handler) : d_handler(handler), d_pool(0) { }
	~PidPool();

	class Reference;

private:
    struct Node;

	CC____RelaxProtection(public)
    class Actions {
        struct Node {
            Node *next;
            int args[2];
            bool applied;
            Node(int arg0, int arg1) 
                : next(0), applied(false) { args[0] = arg0; args[1] = arg1; }
            ~Node() {}
        };
        Node *d_actions[actionType_cnt];
    public:
        Actions() { memset(d_actions,0,sizeof(d_actions)); }
        ~Actions() {
			for (unsigned int i=0; i<actionType_cnt; ++i) {
				Node *a=d_actions[i];
				while (a) {
					Node *d=a;
					a=a->next;
					delete d;
				}
			}
		}
        void add(actionType action, int arg0, int arg1) {
            if (Node *c = d_actions[action]) {
                while (c->next)
                    c = c->next;
                c->next = new Node(arg0, arg1);
            } else
                d_actions[action] = new Node(arg0, arg1);
        }
		class const_iterator;
        friend class const_iterator;
        class const_iterator {
            Node *d_cur;
        public:
            const_iterator(Actions *actions, actionType action)
                : d_cur(actions ? actions->d_actions[action] : 0) { }
    		const_iterator& operator++() { if (!is_end()) d_cur=d_cur->next; return *this; }
            bool is_end() const { return !d_cur; }
            int arg0() const { return d_cur->args[0]; }
            int arg1() const { return d_cur->args[1]; }
            bool applied() const { return d_cur->applied; }
            void set_applied(bool val=true) { d_cur->applied = val; }
        };
    };

    class Alternates {
        struct Node {
            Node *next;
            const int flags;
            const unsigned int pids_num_of;
            int *pids;
            const unsigned int grants_num_of;
            int *grants;
            Reference **d_pidrefs;
            Node(int _flags, int _pids_num_of, const int _pids[]
            , int _grants_num_of, const int _grants[])
              : next(0)
              , flags(_flags)
              , pids_num_of(_pids_num_of), pids(new int[pids_num_of])
              , grants_num_of(_grants_num_of), grants(new int[grants_num_of])
              , d_pidrefs(0)
            { 
              memcpy(pids,_pids,pids_num_of*sizeof(pids[0])); 
              memcpy(grants,_grants,grants_num_of*sizeof(grants[0])); 
            }
            ~Node();
            const Reference *const *pidrefs(PidPool &);
        };
        Node *d_node;

    public:
        Alternates() : d_node(0) { }
        ~Alternates() {
            while (d_node) {
                Node *c=d_node;
                d_node=d_node->next;
                delete c;
            }
        }
        void add(int flags
          , const unsigned int pids_num_of, const int pids[]
          , const unsigned int grants_num_of, const int grants[]
        ) {
            if (Node *c = d_node) {
                while (c->next)
                    c = c->next;
                c->next = new Node(flags, pids_num_of, pids, grants_num_of, grants);
            } else
                d_node = new Node(flags, pids_num_of, pids, grants_num_of, grants);
        }

        class const_iterator;
        friend class const_iterator;
        class const_iterator {
            Node *d_cur;
            PidPool &d_pool;
        public:
            const_iterator(const Alternates *alternates, PidPool &pidpool)
                : d_cur(alternates ? alternates->d_node : 0), d_pool(pidpool) { }
            const_iterator& operator++() { if (d_cur) d_cur = d_cur->next; return *this; }
            const unsigned int size() const { return d_cur ? d_cur->pids_num_of : 0; }
            const int *const operator*() const { return d_cur ? d_cur->pids : 0; }
            const Reference *const *pidrefs() const { return d_cur ? d_cur->pidrefs(d_pool) : 0; }
            const unsigned int grants_num_of() const { return d_cur ? d_cur->grants_num_of : 0; }
            const int *const grants() const { return d_cur ? d_cur->grants : 0; }
        };
    };
	CC____RestoreProtection(private)

	struct Node {
		Node *next;

		unsigned int refcnt;

		const int pidnum;
		MGSG_MGLS_pid *pid;

		const Reference *primary;

        Actions *actions;
        Alternates *alternates;

        Node(const int _pidnum, Node *_next)
			: next(_next), refcnt(0), 
			  pidnum(_pidnum), pid(0),
			  primary(0), actions(0), alternates(0) { }

        ~Node() { 
			delete actions; 
			delete alternates; 
			if (!primary)
				delete pid;
		}

        void add_action(actionType action, int arg0, int arg1) const
        {
            Node *t = (Node *)this;
            if (!actions)
                t->actions = new Actions;
            t->actions->add(action, arg0, arg1);
        }

        void add_alternate(int flags
          , const unsigned int pids_num_of, const int pids[]
          , const unsigned int grants_num_of, const int grants[]) const
        {
            Node *t = (Node *)this;
            if (!alternates)
                t->alternates = new Alternates;
            t->alternates->add(flags, pids_num_of, pids, grants_num_of, grants);
        }
    };

	MGSG_LicenseHandlerBase &d_handler;
	Node *d_pool;

	friend class Reference;

	Node *GetNode(const int pidnum) {
		Node *c=d_pool, *l=0;
		while (c && c->pidnum < pidnum) {
			l = c;
			c = c->next;
		}
		if (!c || c->pidnum != pidnum) {
    		if (!l) // insert node at the head
    			c = d_pool = new Node(pidnum, d_pool);
            else
    			c = l->next = new Node(pidnum, l->next);
		} // else: already have a node for this pidnum
		++c->refcnt;
		return c;
	}

	const Node *Alloc(const int pid, int flags) {
		Node *a=GetNode(pid);
		if (!a->pid) {
			// allocate and "auto register" the pid container
			a->pid = new MGSG_MGLS_pid();
            d_handler.Register_pid(*a->pid, pid, flags|flag_pidInternal, 0, 0, 0, 0); 
        }
		return a;
	}

	// PRIMARYLY USED IN PID Register ... ALL OTHERS CAN USE THE pidnum ABOVE
	const Node *Alloc(MGSG_MGLS_pid &pid, const Reference &reference) {
		Node *n=GetNode(pid.get_pid());
		if (!n->primary) { // if there is already a primary the pid was already registered ...
			n->primary = &reference;
            if (n->pid != &pid) {
    			delete n->pid; // this was an "auto register" pid container
	    		n->pid = &pid;
	    	}
		}
		return n;
	}

	// CALLER SHOULD HAVE UNREGISTED PRIMARY PID FROM THE HANDLER
	// PRIOR TO DELETING THE REFERENCE TO IT
	void Dealloc(const Node *node, const Reference *reference) {
		for (Node *n=d_pool, *l=0; n; l=n, n=n->next) {
			if (n == node) {
				bool delete_node = --n->refcnt == 0;
				if (!delete_node && n->refcnt == 1 && (n->pid->get_flags() & flag_pidInternal))
				   delete_node = true;
				else if (d_handler.d_destructing)
				   delete_node = true;
				if (delete_node) { // remove it from the pool
					if (!l)
						d_pool = n->next;
					else
						l->next = n->next;
				}
				if (n->primary == reference) {
					n->primary = 0;
					if (delete_node) {
						n->pid = 0; // ensure we do not delete the pid below
					} else {  // allocate an "auto registered" pid for this node
						n->pid = new MGSG_MGLS_pid();
                        d_handler.Register_pid(*n->pid, n->pidnum, n->pid->get_flags()|flag_pidInternal, 0, 0, 0, 0); 
                    }
				}
				if (delete_node)
					delete n;
				break;
			}
		}
	}

public:
	DEBUG_LICENSE_print

	class const_iterator;
	friend class const_iterator;
    class const_iterator {
        Node *d_cur;
    public:
        const_iterator(PidPool *pids)
            : d_cur(pids ? pids->d_pool : 0) { }
        const_iterator& operator++() { if (!is_end()) d_cur=d_cur->next; return *this; }
        bool is_end() const { return !d_cur; }
    	const MGSG_MGLS_pid &operator*() const { return *d_cur->pid; }
    };
};

#if defined(DEBUG_LICENSE) && DEBUG_LICENSE > 1
void MGSG_LicenseHandlerBase::PidPool::print() const
{
	for (const Node *c = d_pool; c; c=c->next) {
		cout << "// Debug: PidPool"
			 << " pidnum="          << setw(7)  << c->pidnum
			 << " pidname="         << setw(16) << ((c->pid && c->pid->get_name()) ? c->pid->get_name() : unknown_str)
			 << " refcnt="          << setw(2)  << c->refcnt 
			 << endl;
		if (c->pid) {
			for (MGSG_MGLS_pid::contains_iterator i(*c->pid); !i.is_end(); ++i) {
				if (const MGSG_MGLS_pid *p = *i) {
					cout << "// Debug:    Cont "
						 << " pidnum="          << setw(6)  << p->get_pid()
                         << " pidname="         << setw(16) << (p->get_name() ? p->get_name() : unknown_str)
						 << endl;
				}
			}
		}
		cout.flush();
	}
}
#endif

class MGSG_LicenseHandlerBase::PidPool::Reference {
	CC____RelaxProtection(public)
	PidPool &d_pool;
	const Node *d_node;
	CC____RestoreProtection(private)
public:
	Reference(PidPool &pool, const int pid, int flags=flag_undefined) 
        : d_pool(pool), d_node(d_pool.Alloc(pid, flags)) {}
	Reference(MGSG_MGLS_pid &pid)
		: d_pool(*get_pidpool(pid)),
		  d_node(NULL)
	{
		d_node = d_pool.Alloc(pid, *this);
	}
	Reference(const MGSG_MGLS_pid &pid) 
		: d_pool(*get_pidpool(pid)), d_node(d_pool.Alloc(pid.get_pid(), pid.get_flags())) {}
	~Reference() { d_pool.Dealloc(d_node, this); }
	operator const MGSG_MGLS_pid &() const { return *d_node->pid; }
	const MGSG_MGLS_pid &operator*() const { return *d_node->pid; }
	const char *get_name() const { return d_node->pid ? d_node->pid->get_name() : 0; }

    void add_action(MGSG_LicenseHandlerBase::actionType action, int arg0, int arg1) 
	{ d_node->add_action(action, arg0, arg1); }
    class actions_iterator : public Actions::const_iterator {
    public:
        actions_iterator(const Reference &reference, MGSG_LicenseHandlerBase::actionType action)
            : Actions::const_iterator(reference.d_node->actions, action) { }
    };
    
    void add_alternate(int flags
      , const unsigned int pids_num_of, const int pids[]
      , const unsigned int grants_num_of, const int grants[])
    { d_node->add_alternate(flags, pids_num_of, pids, grants_num_of, grants); }
    class alternates_iterator : public Alternates::const_iterator {
    public:
        alternates_iterator(const Reference &reference)
            : Alternates::const_iterator(reference.d_node->alternates,
                                         reference.d_pool) { }
    };

    bool CLIM_hash(STDCOLON string *feature_hash, unsigned int valid_secs) const
    {
      // comma separate enclosed CLIMs
      MGSG_LicenseHandlerBase::Core &core = d_pool.d_handler.core();
      std::vector<int> cpids;
      for (alternates_iterator i(*this); *i; ++i) {
        const int *ipids = *i;
        for (unsigned int ii=0, iiend=i.size(); ii<iiend; ++ii) {
          const int ipid = ipids[ii];
          if (core.is_clim_only_appid(ipid))
            cpids.push_back(ipid);
        }
      }
      bool res = false;
      if (!cpids.empty()) {
        std::sort( cpids.begin(), cpids.end() );
        cpids.erase( std::unique( cpids.begin(), cpids.end() ), cpids.end() );
        std::string features;
        for (unsigned int i=0, iend=cpids.size(); i<iend; ++i)
        {
          const std::string *fname;
          if (core.Mheap_lookup( cpids[i], &fname ) == MGLS_OK) {
            if (i)
              features += ',';
            features += *fname;
          } else {
            features.clear();
            break;
          }
        }
        if (! features.empty())
          res = MGSG_LicenseHandlerBase::GetDisplayHash(feature_hash, valid_secs, features.c_str());
      }
      return res;
    }
};

MGSG_LicenseHandlerBase::PidPool::~PidPool()
{
# ifdef DEBUG_LICENSE
	cout << "// Debug: Deleting PidPool, unregistering/deleting all remaining pids" << endl;
# endif
	print();
	while (d_pool) { // these should only have a ref count of one, as they are allocated when the real is deallocated
		MGSG_MGLS_pid *pid = d_pool->pid;
    d_pool->pid->UnRegister(&d_handler);
    delete pid; // nobody should be referencing the pid any longer
	}
	print();
}

class MGSG_LicenseHandlerBase::PidPool_Reference : public MGSG_LicenseHandlerBase::PidPool::Reference
{
public:
    PidPool_Reference(MGSG_MGLS_pid &pid)
        : PidPool::Reference(pid) { }
};

//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////

MGSG_LicenseHandlerBase::PidPool::Alternates::Node::~Node()
{
    delete [] pids;
    delete [] grants;
    if (d_pidrefs) {
        for (unsigned int i=0; i<pids_num_of; ++i)
            delete d_pidrefs[i];
        delete [] d_pidrefs;
    }
}

const MGSG_LicenseHandlerBase::PidPool::Reference *const *MGSG_LicenseHandlerBase::PidPool::Alternates::Node::pidrefs(PidPool &pool)
{
    if (!d_pidrefs) {
        d_pidrefs = new Reference *[pids_num_of];
        for (unsigned int i=0; i<pids_num_of; ++i)
            d_pidrefs[i] = new Reference(pool, pids[i], flags);
    }
    return d_pidrefs;
}

/*==========================================================================
- ============================= HANDLER::Core ==============================
- ========================================================================== */

struct MGSG_LicenseHandlerBase::Core::Ids 
{
  int num_of_ids;
  int *id_list;
  PidNums *ids;

  Ids(int num_of_ids_, int *id_list_)
    : num_of_ids(num_of_ids_), ids(new PidNums)
  {
    int *n = new int[num_of_ids+1];
    int i;
    for (i=0; i<num_of_ids; ++i)
      ids->push_back((n[i] = id_list_[i]));
    n[i] = 0;
    id_list = n;
  }

  Ids(const MGSG_MGLS_pid *pids[], unsigned int num_of_pids_)
    : num_of_ids(num_of_pids_), ids(new PidNums)
  {
    int *n = new int[num_of_ids+1];
    int i;
    for (i=0; i<num_of_ids; ++i)
      ids->push_back((n[i] = pids[i]->get_pid()));
    n[i] = 0;
    id_list = n;
  }

  ~Ids() { 
    if (id_list) 
      delete [] id_list; 
    delete ids; 
  }
};

long MGSG_LicenseHandlerBase::Core::Mheap_get_errno()
{ return mgls_errno; }

long MGSG_LicenseHandlerBase::Core::Mheap_set( int which, long new_value )
{ BlockSigs block_sigs(*this); return Mheap_ret(hMGLS, core_Mheap_set( which, new_value )); }

long MGSG_LicenseHandlerBase::Core::Mheap_get( int  which, MGLS_ATTR_VALUE *new_value )
{ BlockSigs block_sigs(*this); return Mheap_ret(hMGLS, core_Mheap_get( which, new_value )); }

long MGSG_LicenseHandlerBase::Core::Mheap_lookup( int  pid, char *fname )
{ 
  BlockSigs block_sigs(*this);
  const std::string *s_fname;
  long r = this->Mheap_lookup( pid, &s_fname );
  if (r == MGLS_OK)
    strcpy(fname, s_fname->c_str());
  return r;
}

long MGSG_LicenseHandlerBase::Core::Mheap_lookup( int  pid, const std::string **fname )
{
  BlockSigs block_sigs(*this);
  PidToName::iterator it = d_pid_to_name.find(PidToNamePair(pid,0)); // check cache first
  if (it == d_pid_to_name.end()) {
    char fname[MGLS_MAX_FEATURE_LEN];
    if (Mheap_ret(hMGLS, core_Mheap_lookup( pid, fname )) == MGLS_OK) {
      it = (d_pid_to_name.insert( PidToNamePair(pid,new std::string(fname)) )).first;
      d_known_pid.insert(NameToPidPair((*it).second,(*it).first));
    }
  }
  if (it == d_pid_to_name.end()) 
    return MGLS_ERR;
  *fname = it->second;
  return MGLS_OK;
}

static void MGSG_dstring(std::string *res, const char *ebuf)
{
    char tbuf[MGSG_CRYPT_buf_sz];

    memcpy(tbuf, ebuf, MGSG_CRYPT_buf_sz);

    tbuf[ 0] -= MGSG_CRYPT_offset_00;
    tbuf[ 1] -= MGSG_CRYPT_offset_01;
    tbuf[ 2] -= MGSG_CRYPT_offset_02;
    tbuf[ 3] -= MGSG_CRYPT_offset_03;
    tbuf[ 4] -= MGSG_CRYPT_offset_04;
    tbuf[ 5] -= MGSG_CRYPT_offset_05;
    tbuf[ 6] -= MGSG_CRYPT_offset_06;
    tbuf[ 7] -= MGSG_CRYPT_offset_07;
    tbuf[ 8] -= MGSG_CRYPT_offset_08;
    tbuf[ 9] -= MGSG_CRYPT_offset_09;
    tbuf[10] -= MGSG_CRYPT_offset_10;
    tbuf[11] -= MGSG_CRYPT_offset_11;
    tbuf[12] -= MGSG_CRYPT_offset_12;
    tbuf[13] -= MGSG_CRYPT_offset_13;
    tbuf[14] -= MGSG_CRYPT_offset_14;
    tbuf[15] -= MGSG_CRYPT_offset_15;
    tbuf[16] -= MGSG_CRYPT_offset_16;
    tbuf[17] -= MGSG_CRYPT_offset_17;
    tbuf[18] -= MGSG_CRYPT_offset_18;
    tbuf[19] -= MGSG_CRYPT_offset_19;
    tbuf[20] -= MGSG_CRYPT_offset_20;
    tbuf[21] -= MGSG_CRYPT_offset_21;
    tbuf[22] -= MGSG_CRYPT_offset_22;
    tbuf[23] -= MGSG_CRYPT_offset_23;
    tbuf[24] -= MGSG_CRYPT_offset_24;
    tbuf[25] -= MGSG_CRYPT_offset_25;
    tbuf[26] -= MGSG_CRYPT_offset_26;
    tbuf[27] -= MGSG_CRYPT_offset_27;
    tbuf[28] -= MGSG_CRYPT_offset_28;
    tbuf[29] -= MGSG_CRYPT_offset_29;
    tbuf[30] -= MGSG_CRYPT_offset_30;
    tbuf[31] -= MGSG_CRYPT_offset_31;
    tbuf[32] -= MGSG_CRYPT_offset_32;

    *res = tbuf;
}

int MGSG_LicenseHandlerBase::Core::allocateCLIM( const char *MGSG_CRYPT_pidname )
{
    int pid = next_clim_only_appid();

    std::string *tbuf = new std::string;
    MGSG_dstring(tbuf, MGSG_CRYPT_pidname);

    PidToName::const_iterator it = (d_pid_to_name.insert( PidToNamePair(pid, tbuf) )).first;
    d_known_pid.insert(NameToPidPair((*it).second,(*it).first));

    return pid;
}

class MGSG_LicenseHandlerBase::Core::ContainsInfo 
{
public:
  const Ids atomics;
  const Ids additional_checkout;
  ContainsInfo(const MGSG_MGLS_pid *pids[], unsigned int pids_num_of
    , const MGSG_MGLS_pid *additional_co[], unsigned int additional_co_num_of)
    : atomics(pids, pids_num_of), additional_checkout(additional_co, additional_co_num_of)
  { }
  ~ContainsInfo() { }
};

bool MGSG_LicenseHandlerBase::Core::RegisterCLIM_contains(MGSG_MGLS_pid &pid
                                                          , const MGSG_MGLS_pid *pids[], unsigned int pids_num_of
                                                          , const MGSG_MGLS_pid *additional_co[], unsigned int additional_co_num_of)
{
  if (!is_clim_only_appid(pid))
    return false;
  if (! d_clim_to_pids)
    d_clim_to_pids = new PidToContains;
  else {
    PidToContains::const_iterator p = d_clim_to_pids->find(PidToContainsPair(pid,0));
    if (p != d_clim_to_pids->end())
      return false;
  }
  d_clim_to_pids->insert(PidToContainsPair(pid.get_pid(),
    new ContainsInfo(pids, pids_num_of, additional_co, additional_co_num_of)));
  return true;
}

/*==========================================================================
- ======================= HANDLER::CORE::PIDTOMEMBERS ======================
- ========================================================================== */

class MGSG_LicenseHandlerBase::Core::PidToMembers
{
  MGSG_LicenseHandlerBase &d_handler;

  Core &core() { return *d_handler.d_core; }

  typedef std::vector<Ids *> DataCache;
  DataCache d_cache;

  typedef std::map<int, const Ids *> Data;
  Data d_data;

  long get(int pid, const Ids **ids)
  {
    int r = MGLS_ERR;
    Data::iterator it = d_data.find(pid); // check cache first
    if (it == d_data.end()) {
      struct delete_id_list {
        int num_of_ids;
        int *id_list;
        bool owner;
        delete_id_list() : num_of_ids(0), owner(false) { }
        ~delete_id_list() { if (owner) delete [] id_list; }
      };
      delete_id_list pids;
      if (!core().is_clim_only_appid(pid))
        r = core().Mheap_ret(hMGLS, core_Mheap_member(pid, &pids.num_of_ids, &pids.id_list));
      else
        r = MGLS_OK;
      if (r == MGLS_OK && core().d_clim_to_pids) {
        for (PidToContains::iterator p = core().d_clim_to_pids->begin(), pend=core().d_clim_to_pids->end(); p != pend; ++p) 
        {
          PidNums &pnums = *(*p).second->atomics.ids;
          if (std::find(pnums.begin(), pnums.end(), pid) != pnums.end()) {
            if (!pids.owner) { // convert to RW list and take ownership to add additional CLIM pids
              int *id_list_t = new int [core().d_clim_to_pids->size()+pids.num_of_ids+1];
              for (int i=0; i<pids.num_of_ids; ++i)
                id_list_t[i] = pids.id_list[i];
              pids.id_list = id_list_t;
              pids.owner = true;
            }
            pids.id_list[pids.num_of_ids++] = (*p).first;
          }
        }
        if (pids.owner)
          pids.id_list[pids.num_of_ids] = 0;
      }
      if (r == MGLS_OK) {
        const Ids *dcache = 0;
        for (DataCache::const_iterator p = d_cache.begin(); p != d_cache.end(); ++p) 
        {
          const Ids *i = (*p);
          bool j_eq = (i->num_of_ids == pids.num_of_ids);
          for (int j = 0; j_eq && j<pids.num_of_ids; ++j) {
            if (pids.id_list[j] != i->id_list[j])
              j_eq = false;
          }
          if (j_eq) {
            dcache = i;
            break;
          }
        }
        if (!dcache) {
          d_cache.push_back(new Ids(pids.num_of_ids, pids.id_list));
          dcache = d_cache.back();
        }
        d_data[pid] = dcache;
        *ids = dcache; 
      }
    } else { 
      *ids = (*it).second;
      r = MGLS_OK;
    }
    return r;
  }

public:
  PidToMembers(MGSG_LicenseHandlerBase &handler) : d_handler(handler) { }
  ~PidToMembers() { 
    for (DataCache::const_iterator p = d_cache.begin(); p != d_cache.end(); ++p) 
      delete *p;
  }
  
  long get(int pid, int *num_of_ids, const int **id_list) {
    const Ids *i = 0; /* gcc lint */
    long r = get(pid, &i);
    if (r == MGLS_OK) {
      *num_of_ids = i->num_of_ids;
      *id_list = i->id_list;
    }
    return r;
  }

  class const_iterator {
    friend class PidToMembers;
    Data::const_iterator d_itr;
    const_iterator(const Data::const_iterator &itr) : d_itr(itr) { }
  public:
    const_iterator() { }
    inline void operator=(const const_iterator &itr) { d_itr = itr.d_itr; }
    inline const_iterator &operator++() { ++d_itr; return *this; }
    inline bool operator==(const const_iterator &itr) const { return d_itr == itr.d_itr; }
    inline bool operator!=(const const_iterator &itr) const { return !operator==(itr); }
    int pid() const { return (*d_itr).first; }
    const PidNums &members() const { return *(*d_itr).second->ids; }
  };
  inline const_iterator begin() const { return d_data.begin(); }
  inline const_iterator end() const { return d_data.end(); }
};

void MGSG_LicenseHandlerBase::Core::init_PidToMembers(PidToMembers **obj)
{ *obj = new PidToMembers(d_handler); }

long MGSG_LicenseHandlerBase::Core::Mheap_member( int pid, int *num_of_ids, const int **id_list )
{
  BlockSigs block_sigs(*this);
  init_PidToMembers();
  return d_pid_members->get(pid, num_of_ids, id_list);
}

/*==========================================================================
- ======================== HANDLER::CORE::PIDEXISTS ========================
- ========================================================================== */

class MGSG_LicenseHandlerBase::Core::PidExists
{
  MGSG_LicenseHandlerBase &d_handler;
  Core &core() const { return *d_handler.d_core; }

  enum eStatus { eYes, eNo, eUndefined };
  class Entry {
    eStatus d_mgc;
    eStatus d_clip;
  public:
    Entry() : d_mgc(eUndefined), d_clip(eUndefined) { }
    bool is_set(handlerType htype) const
    { 
      if (htype == hMGLS) return d_mgc != eUndefined;
      if (htype == hCLIM) return d_clip != eUndefined;
      return false;
    }
    bool is(handlerType htype, eStatus val) const
    { 
      if (htype == hMGLS) return d_mgc == val;
      if (htype == hCLIM) return d_clip == val;
      return false;
    }
    bool get(handlerType htype, bool *is_defined) const 
    { 
      if (htype == hMGLS) {
        if (is_defined)
          *is_defined = (d_mgc != eUndefined);
        return d_mgc == eYes;
      }
      if (htype == hCLIM) {
        if (is_defined)
          *is_defined = (d_clip != eUndefined);
        return d_clip == eYes;
      }
      if (is_defined)
        *is_defined = false;
      return false;
    }
    void set(handlerType htype, bool val) {
      if (htype == hMGLS) d_mgc = val ? eYes : eNo;
      else if (htype == hCLIM) d_clip = val ? eYes : eNo;
    }
  };
  typedef std::map<int, Entry> Entries;
  Entries d_entries;
public:
  PidExists(MGSG_LicenseHandlerBase &handler) : d_handler(handler) { }
  ~PidExists() { }

  bool get(int pid, handlerType htype, bool *is_defined=0) const
  { 
    Entries::const_iterator i = d_entries.find(pid);
    if (i == d_entries.end()) {
      if (htype == hMGLS) {
        // if atomic not in composite || composite
        int num_of_ids;
        const int *id_list;
        int member_ret;
        if (core().is_clim_only_appid(pid)) {
          const bool val = false;
          const_cast<PidExists *>(this)->set(pid, htype, val);
          if (is_defined)
            *is_defined = true;
          return val;
        } else if (((member_ret = core().Mheap_member( pid, &num_of_ids, &id_list )) == MGLS_OK
          && (num_of_ids == 0 || (num_of_ids == 1 && id_list[0] == pid))) /* PCLS case see API doc */
          || (member_ret == MGLS_ERR /* MGLS case see API doc -- handle pid is composite */
          && mgls_errno != MGLS_pkginfofile_corrupt
          && mgls_errno != MGLS_unknown_license_id
          && mgls_errno != MGLS_malloc_failure)) {
            const bool val = (core().Mheap_ret(hMGLS, core_Mheap_exists( pid )) == MGLS_LICENSE_AVAILABLE);
            const_cast<PidExists *>(this)->set(pid, htype, val);
            if (is_defined)
              *is_defined = true;
            return val;
        } else {
          if (is_defined)
            *is_defined = false;
          return false;
        }
      }
      if (is_defined)
        *is_defined = false;
      return false;
    }
    return (*i).second.get(htype, is_defined);
  }

  void set(int pid, handlerType htype, bool val) 
  {
    Entries::iterator i = d_entries.find(pid);
    if (i == d_entries.end())
      i = (d_entries.insert( std::pair<int,Entry>(pid,Entry()) )).first;
    (*i).second.set(htype,val);
  }

  class const_iterator {
    friend class PidExists;
  protected:
    Entries::const_iterator d_itr;
    const_iterator(const Entries::const_iterator &itr) : d_itr(itr) { }
  public:
    const_iterator() { }
    inline void operator=(const const_iterator &itr) { d_itr = itr.d_itr; }
    inline const_iterator &operator++() { ++d_itr; return *this; }
    inline bool operator==(const const_iterator &itr) const { return d_itr == itr.d_itr; }
    inline bool operator!=(const const_iterator &itr) const { return !operator==(itr); }
    int pid() const { return (*d_itr).first; }
    bool exists(handlerType htype) const { return (*d_itr).second.get(htype,0); }
    bool defined(handlerType htype) const { bool d; (*d_itr).second.get(htype,&d); return d; }
  };
  inline const_iterator begin() const { return d_entries.begin(); }
  inline const_iterator end() const { return d_entries.end(); }

  class iterator : public const_iterator {
    friend class PidExists;
    iterator(const Entries::iterator &itr) : const_iterator(itr) { }
    inline Entry &itr_second() { return const_cast<Entry &>((*d_itr).second); }
  public:
    iterator() { }
    inline void operator=(const iterator &itr) { d_itr = itr.d_itr; }
    inline iterator &operator++() { ++d_itr; return *this; }
    inline bool operator==(const iterator &itr) const { return d_itr == itr.d_itr; }
    inline bool operator!=(const iterator &itr) const { return !operator==(itr); }
    void set_exists(handlerType htype, bool val) { itr_second().set(htype, val); }
  };
  inline iterator begin() { Entries::iterator i = d_entries.begin(); return i; }
  inline iterator end() { Entries::iterator i = d_entries.end(); return i; }
};

void MGSG_LicenseHandlerBase::Core::init_PidExists(PidExists **obj)
{ *obj = new PidExists(d_handler); }

long MGSG_LicenseHandlerBase::Core::Mheap_exists( int app_id )
{ 
  BlockSigs block_sigs(*this);
  long r;
  if (d_pid_exists->get(app_id, hCLIM) || d_pid_exists->get(app_id, hMGLS)) {
    r = MGLS_LICENSE_AVAILABLE;
  } else {
    r = MGLS_LICENSE_UNAVAILABLE;
  }
  return r;
}

/*==========================================================================
- ======================== HANDLER::CORE::SATISFIES ========================
- ========================================================================== */
//
// Satisfies
//   take a list of known features --
//      index mgls registered atomic / composite information
//
//      Say the server has the licenses a, b, c, d, e and all but d can satisfy feature foo.
//         MGLS_LICENSE_SOURCE=c:b:a
//         MGLS_INCLUDE_LICENSES=a:c
//         MGLS_EXCLUDE_LICENSES=a
//
//      The server says it has licenses "a, b, c, d, e".
//          The mgc.pkginfo file reduces this to "a, b, c, e" (since d can't satisfy foo).
//          This list is sorted per MGLS_LICENSE_SOURCE: "c, b, a, e"
//          The list is intersected with MGLS_INCLUDE_LICENSES: "c, a"
//          Items contained in MGLS_EXCLUDE_LICENSES are removed: "c"
//          So MSL would try to checkout 'c' to satisfy the request for foo.
//
//
class MGSG_LicenseHandlerBase::Core::Satisfies
{
  MGSG_LicenseHandlerBase &d_handler;
  bool d_err;

  inline Core &core() { return *d_handler.d_core; }
  inline const Core &core() const { return *d_handler.d_core; }

  // Optimized sorted entry data, shares common entries where possible
  class Data {
    struct Entry {
      PidNames *const mgc_has;
      PidNames *const clip_has;
      Entry(const PidNames *const mgc_has_, const PidNames *const clip_has_)
        : mgc_has(new PidNames(*mgc_has_)), clip_has(new PidNames(*clip_has_)) { }
      ~Entry() { delete mgc_has; delete clip_has; }

      struct Cache {
        const PidNames *k1;  // old key
        const PidNames *k2;  // old key
        Entry *e;
        Cache(const PidNames *k1_
          , const PidNames *k2_
          , Entry *e_) : k1(k1_), k2(k2_), e(e_) { }
        ~Cache() { delete e; }
      };
    };
    typedef std::vector<Entry::Cache *> CacheEntries;
    CacheEntries d_cache;

    typedef std::pair<int, const Entry *> PidToEntryPair;
    struct PidToEntryPair_ltstr { 
      bool operator()(const PidToEntryPair &s1, const PidToEntryPair &s2) const 
      { return s1.first < s2.first; }
    };
    typedef std::set<PidToEntryPair, PidToEntryPair_ltstr> PidToEntry;
    PidToEntry d_pid_to_data; // pid id queried as lookup 


  public:
    Data() { }
    ~Data() {
      for (CacheEntries::const_iterator c = d_cache.begin(); c != d_cache.end(); ++c)
        delete (*c);
    }

    bool get(int pid, const PidNames **CLIPnames, const PidNames **MGLSnames) const
    {
        PidToEntry::const_iterator e = d_pid_to_data.find(PidToEntryPair(pid,0));
        if (e == d_pid_to_data.end())
          return false;
        *CLIPnames = (*e).second->clip_has;
        *MGLSnames = (*e).second->mgc_has;
        return true;
    }

    void set(int pid, const PidNames *CLIPnames, const PidNames *MGLSnames)
    {
      const Entry *cache = 0;
      for (CacheEntries::const_iterator c = d_cache.begin(); c != d_cache.end(); ++c) {
        const Entry::Cache *e = *c;
        if (e->k1 == MGLSnames && e->k2 == CLIPnames) {
          cache = e->e;
          break;
        }
      }
      if (!cache) {
        Entry *e = new Entry(MGLSnames, CLIPnames);
        d_cache.push_back(new Entry::Cache(MGLSnames, CLIPnames, e));
        cache = e;
      }
      d_pid_to_data.insert(PidToEntryPair(pid, cache));
    }

    void finalize_cache() { // remove temporary pointer references
      for (CacheEntries::iterator c = d_cache.begin(); c != d_cache.end(); ++c) {
        Entry::Cache *e = *c;
        e->k1 = 0;
        e->k2 = 0;
      }
    }
  };
  Data d_data;

  // Helper class used during init
  class cacheSortPrune {
    MGSG_LicenseHandlerBase &d_handler;

    PidNums d_pid_source;

    PidNums d_pid_include;
    bool d_pid_include_set;

    PidNums d_pid_exclude;

    const PidExists &d_pid_exists;

    inline void StrToPidNums(const char *str, PidNums *n) const
    {
      Strings s;
      splitStr(s, str);
      d_handler.core().namesToKnownPidNums(s, n);
    }

    struct Entry
    {
      handlerType h;
      int a;
      const PidNums *m;
      const PidNums *result;
      const PidNames *result_names;
      Entry(const handlerType h_, int a_, const PidNums *const m_
        , const PidNums *const result_, const PidNames *result_names_)
        : h(h_), a(a_), m(m_), result(result_), result_names(result_names_) { }
      void operator=(const Entry &obj)
      { h = obj.h; a = obj.a; m = obj.m; result = obj.result; result_names = obj.result_names; }
    };
    typedef std::vector<Entry> Entries;

    Entries d_cache;

    void doSortPrunePids(PidNums &n,         // result
                       const int this_pid, // atomic itself
                       const PidNums &m,   // sorted and pruned members
                       const handlerType h,
                       const PidNums *h_order) // how pids are ordered on server (optional)
    {
      // first order m by those listed in license source 
      for (PidNums::const_iterator pp = d_pid_source.begin(); pp != d_pid_source.end(); ++pp) {
        const int pid = *pp;
        if (d_pid_exists.get(pid,h)) { // license exists
          if (pid == this_pid || std::find(m.begin(), m.end(), pid) != m.end()) { // will satisfy (member)
            if (std::find(n.begin(), n.end(), pid) == n.end()) { // not already in list
              n.push_back(pid);
            }
          }
        }
      }

      // push any other known licenses
      if (h_order) { // use license order from server information
        for (PidNums::const_iterator pp = h_order->begin(), ppend = h_order->end(); pp != ppend; ++pp) {
          const int pid = *pp;
          if (d_pid_exists.get(pid,h)) { // license exists
            if (pid == this_pid || std::find(m.begin(), m.end(), pid) != m.end()) { // will satisfy (member)
              if (std::find(n.begin(), n.end(), pid) == n.end()) { // not already in list
                n.push_back(pid);
              }
            }
          }
        }
      } else { // just use order from pid exists
        for (PidExists::const_iterator e = d_pid_exists.begin(); e != d_pid_exists.end(); ++e) {
          const int pid = e.pid();
          if (e.exists(h)) { // license exists
            if (pid == this_pid || std::find(m.begin(), m.end(), pid) != m.end()) { // will satisfy (member)
              if (std::find(n.begin(), n.end(), pid) == n.end()) { // not already in list
                n.push_back(pid);
              }
            }
          }
        }
      }

      // now remove any not present in includes, notice use orignal include which has 
      // any license, not just known licenses
      if (d_pid_include_set) {
        for (PidNums::iterator pp = n.begin(); pp != n.end(); ) {
          if (std::find(d_pid_include.begin(), d_pid_include.end(), *pp) == d_pid_include.end()) // erase_it
            pp = n.erase(pp);
          else
            ++pp;
        }
      }

      // now remove any specifically excluded licenses
      if (!d_pid_exclude.empty()) {
        for (PidNums::const_iterator pp = d_pid_exclude.begin(); pp != d_pid_exclude.end(); ++pp) {
          PidNums::iterator e = std::find(n.begin(), n.end(), *pp);
          if (e != n.end())
            n.erase(e);
        }
      }
    }

    typedef std::pair<PidNames *,PidNums *> PidNamesPidNumsPair;
    typedef std::vector<PidNamesPidNumsPair> Pids;
    Pids d_cache_pids;

  public:
    cacheSortPrune(MGSG_LicenseHandlerBase &handler
      , const char *e_source
      , const char *e_include
      , const char *e_exclude
      , const PidExists &pid_exists)
      : d_handler(handler), d_pid_exists(pid_exists)
    { 
      StrToPidNums(e_source, &d_pid_source);
      StrToPidNums(e_include, &d_pid_include);
      StrToPidNums(e_exclude, &d_pid_exclude);
      d_pid_include_set = *e_include != '\0';
    }

    ~cacheSortPrune() {
      for (Pids::const_iterator cn = d_cache_pids.begin(); cn != d_cache_pids.end(); ++cn) {
        delete (*cn).first;
        delete (*cn).second;
      }
    }

    const PidNames &SortPrunePids(const int pid, const PidNums &m, handlerType h, const PidNums *h_order=0)
    {
      const PidNums *cache_result = 0;
      const PidNames *names = 0;

      const bool has_pid = d_pid_exists.get(pid,h);

      for (Entries::const_iterator i = d_cache.begin(); i != d_cache.end(); ++i) {
        const Entry &e = *i;
        if (e.m == &m && e.h == h && (e.a == (has_pid ? pid : 0))) {
          cache_result = e.result;
          names = e.result_names;
          break;
        }
      }

      if (!cache_result) {
        PidNums *n = new PidNums;
        doSortPrunePids(*n, pid, m, h, h_order);

        cache_result = n;
        for (Pids::const_iterator cn = d_cache_pids.begin(); cn != d_cache_pids.end(); ++cn)
        {
          if (*((*cn).second) == *cache_result) {
            names = ((*cn).first);
            cache_result = ((*cn).second);
            delete n;
            n = 0;
            break;
          }
        }
        if (n) {
          PidNames *pnames = new PidNames;
          for (PidNums::const_iterator p = n->begin(); p != n->end(); ++p) {
            const std::string *pname = 0;
            d_handler.core().Mheap_lookup(*p, &pname);
            pnames->push_back(pname);
          }
          names = pnames;
          d_cache_pids.push_back(PidNamesPidNumsPair(pnames,n));
        }
        const bool use_pid = std::find(cache_result->begin(), cache_result->end(), pid) != cache_result->end();
        d_cache.push_back(Entry(h, use_pid ? pid : 0, &m, cache_result, names));
      }

      return *names;
    }
  };

  void init() {
    // go over all registered IDs may be composites or atomics
    // create a list of all possible requested id's, and the license choices
    // known to the server

    PidExists &pid_exists = *core().d_pid_exists;

    typedef std::vector<const int *> mCache;
    mCache mcache; // cache of all encountered pid lists, will be same pointer if same series

    for (PidPool::const_iterator p(d_handler.d_pidpool); !p.is_end(); ++p) {
      const MGSG_MGLS_pid &pid = *p;
      int num_of_ids;
      const int *id_list;
      if (core().Mheap_member(pid.get_pid(), &num_of_ids, &id_list) == MGLS_OK) {
        {
          const std::string *fname;
          if (core().Mheap_lookup( pid.get_pid(), &fname ) == MGLS_OK) {
            // name cached
          } else {
            d_err = true;
          }
        }
        if (std::find(mcache.begin(), mcache.end(), id_list) == mcache.end()) {
          for (int i=0; i<num_of_ids; ++i) {
            const int i_pid = id_list[i];
            const std::string *fname;
            if (core().Mheap_lookup( i_pid, &fname ) == MGLS_OK) {  // map all pid ids to names
              // ultamiate goal is a sorted list of names to check out from CLIP if exists
              // list of composites for MGLSLD -- do they exist
            } else {
              d_err = true;
            }
          }
          mcache.push_back(id_list);
          if (num_of_ids == 0) { // atomic, no composites contain it -- always do this?
            pid_exists.get( pid.get_pid(), hMGLS );  // get existance cached for known MGLS pids
          } else { // these are the composite ids containing this atomic
            for (int i=0; i<num_of_ids; ++i)
              pid_exists.get( id_list[i], hMGLS );   // get existance cached for known MGLS pids
          }
        } else { // already 
          // atomic, no composites contain it -- needs to be 
          // existance cached for known MGLS pid, since that
          // would not be handled in this cached case.
          if (num_of_ids == 0) 
             pid_exists.get( pid.get_pid(), hMGLS ); // get existance cached for known MGLS pids
        }
      } else
        d_err = true;
    }

    // Get the CLIP known PIDs loaded, and seed existance, MGLS taken care of above
	PidNums CLIM_lic_order;
    if (core().CLIM_use()) {
      std::string c_featlist;
      CLIM::GetFeatureList(&c_featlist);
      Strings c_feats;
      splitStr(c_feats, c_featlist.c_str(), "\n", 1);
      PidNums &c_pids = CLIM_lic_order;
      core().namesToKnownPidNums(c_feats, &c_pids);
      for (PidExists::iterator i = pid_exists.begin(); i != pid_exists.end(); ++i) {
        const bool exists = std::find(c_pids.begin(), c_pids.end(), i.pid()) != c_pids.end();
        i.set_exists(hCLIM, exists);
      }
    }

    // now we need come up with sorted lists
    std::string e_source;
    std::string e_include;
    std::string e_exclude;

    char attr_buf[MGLS_TRUNCATED_ENV_SIZE];

    if (core().Mheap_get( MGLS_ENV_LICENSE_SOURCE, reinterpret_cast<MGLS_ATTR_VALUE *>(attr_buf)) == MGLS_OK)
        e_source = attr_buf;
    else
        d_err = true;
    if (core().Mheap_get( MGLS_ENV_INCLUDE_LICENSES, reinterpret_cast<MGLS_ATTR_VALUE *>(attr_buf)) == MGLS_OK)
        e_include = attr_buf;
    else
        d_err = true;
    if (core().Mheap_get( MGLS_ENV_EXCLUDE_LICENSES, reinterpret_cast<MGLS_ATTR_VALUE *>(attr_buf)) == MGLS_OK)
        e_exclude = attr_buf;
    else
        d_err = true;

    if (!d_err) {
      cacheSortPrune cache(d_handler, e_source.c_str(), e_include.c_str(), e_exclude.c_str(), pid_exists);

      // The list is sorted per MGLS_LICENSE_SOURCE: "c, b, a, e"
      // all other known licenses
      // The list is intersected with MGLS_INCLUDE_LICENSES: "c, a"
      // Items contained in MGLS_EXCLUDE_LICENSES are removed: "c"
      PidToMembers &pid_to_members = *core().d_pid_members;
      for (PidToMembers::const_iterator p = pid_to_members.begin(); p != pid_to_members.end(); ++p)
      {
        const int pid = p.pid();
        const PidNums &m = p.members();
        const PidNames &nCLIPnames = cache.SortPrunePids(pid, m, hCLIM, &CLIM_lic_order);
        const PidNames &nMGLSnames = cache.SortPrunePids(pid, m, hMGLS);
        d_data.set(pid, &nCLIPnames, &nMGLSnames);
      }
      d_data.finalize_cache();
    }
  }

public:
  Satisfies(MGSG_LicenseHandlerBase &handler) : d_handler(handler), d_err(false) { init(); }
  ~Satisfies() { }
  inline bool get(int pid, const PidNames **CLIPnames, const PidNames **MGLSnames)
  { return d_data.get(pid, CLIPnames, MGLSnames); }
  inline bool mgls_err() const { return d_err; }
};

void MGSG_LicenseHandlerBase::Core::init_Satisfies(Satisfies **obj)
{
  unsigned int mretry=10;
  for (;;) {
    *obj = (Satisfies *)1; // avoid multiple attempts to initialize, although it cannot be used it in this state
    *obj = new Satisfies(d_handler);
    if (!(*obj)->mgls_err())
      break;
    if (!(--mretry))
      break;
    delete *obj;
    *obj = 0;
    sleep(2); // wait a moment and retry
  }
}

/*==========================================================================
- ====================== HANDLER::CORE::TRANSACTIONIDS =====================
- ========================================================================== */

class MGSG_LicenseHandlerBase::Core::TransactionIDs
{
  typedef int TransId;

  TransId d_cur_id;

  struct Entry {
    const TransId trans_id;
    const int pid;
    struct Result {
      const handlerType h;
      const std::string *pidname;
      std::string error_str;
      int code;
      int days_left;
      mgls_lictype lic_type;
      bool lost;
      std::string server;
      Result(const handlerType h_, const std::string *pidname_, int code_
          , int days_left_, mgls_lictype lic_type_, const std::string &error_str_
          , const std::string &server_) 
        : h(h_), pidname(pidname_), error_str(error_str_), code(code_)
        , days_left(days_left_)
        , lic_type(lic_type_), lost(false), server(server_) { }
    };
    typedef std::vector<Result *> Results;
    Results results;
    Entry(TransId trans_id_, int pid_) 
      : trans_id(trans_id_), pid(pid_) { }
    ~Entry() { 
      for (Results::const_iterator r = results.begin(); r != results.end(); ++r)
        delete (*r);
    }
  };

  typedef std::pair<TransId, Entry *> TransIdToEntryPair;
  struct TransIdToEntryPair_ltstr { 
    bool operator()(const TransIdToEntryPair &s1, const TransIdToEntryPair &s2) const 
    { return s1.first < s2.first; }
  };
  typedef std::set<TransIdToEntryPair, TransIdToEntryPair_ltstr> TransIdToEntry;
  TransIdToEntry d_data;
  

public:
  TransactionIDs() : d_cur_id(0) { }
  ~TransactionIDs()
  {
    for (TransIdToEntry::const_iterator t = d_data.begin(); t != d_data.end(); ++t)
      delete (*t).second;
  }

  class Transaction {
    Entry *const d_entry;
    Transaction(Entry *entry) : d_entry(entry) { }
    friend class TransactionIDs;
  public:
    Transaction(const Transaction &obj) : d_entry(obj.d_entry) { }
    ~Transaction() { }
    void add_clip_result(const std::string &pidname, int code, int days_left
        , mgls_lictype lic_type, const std::string &error_str, const std::string &server)
    { d_entry->results.push_back(new Entry::Result(hCLIM, &pidname, code, 
                                 days_left, lic_type, error_str, server)); }
    void add_clip_no_exists_result(const std::string &pidname)
    { add_clip_result(pidname, -18, MGLS_ERR, MGLS_floating, 
                      "License server does not support this feature", std::string()); }

    void add_mgc_result(int code)
    { d_entry->results.push_back(new Entry::Result(hMGLS, 0, code, 
                                 MGLS_ERR, MGLS_floating,std::string(),std::string())); }
    int id() const { return d_entry->trans_id; }
    operator bool() const { return !!d_entry; }

    class result_iterator {
      Entry::Results::const_iterator d_itr;
      result_iterator(const Entry::Results::const_iterator &itr) : d_itr(itr) { }
      friend class Transaction;
    public:
      result_iterator() { }
      inline void operator=(const result_iterator &itr) { d_itr = itr.d_itr; }
      inline result_iterator &operator++() { ++d_itr; return *this; }
      inline bool operator==(const result_iterator &itr) const { return d_itr == itr.d_itr; }
      inline bool operator!=(const result_iterator &itr) const { return !operator==(itr); }
      handlerType hType() const { return (*d_itr)->h; }
      const std::string *pidname() const { return (*d_itr)->pidname; }
      const std::string &error_str() const { return (*d_itr)->error_str; }
      int code() const { return (*d_itr)->code; }
      bool good() const { return (*d_itr)->code == 0 && !(*d_itr)->lost; }
      int days_left() const { return (*d_itr)->days_left; }
      mgls_lictype lic_type() const { return (*d_itr)->lic_type; }
      bool reaquire() const { return (*d_itr)->code == 0 && (*d_itr)->lost; }
      void set_lost(bool val) { (*d_itr)->lost = val; }
      bool lost() const { return (*d_itr)->lost; }
      const std::string &server() const { return (*d_itr)->server; }
      void set_server(const std::string &val) { (*d_itr)->server = val; }
    };
    inline result_iterator results_begin() const { return Entry::Results::const_iterator(d_entry->results.begin()); }
    inline result_iterator results_end() const { return Entry::Results::const_iterator(d_entry->results.end()); }
  };

  Transaction new_transaction(int pid)
  {
    Entry *e = new Entry(++d_cur_id, pid);
    d_data.insert(TransIdToEntryPair(e->trans_id, e));
    return e;
  }

  Transaction get_transaction(int transid)
  {
    TransIdToEntry::iterator it = d_data.find(TransIdToEntryPair(transid,0));
    Entry *e = (it == d_data.end()) ? 0 : (*it).second;
    return e;
  }

  inline bool free_transaction(int transid) { 
	  TransIdToEntry::iterator it = d_data.find(TransIdToEntryPair(transid,0));
	  const bool res = it != d_data.end();
	  if (res) {
		  delete (*it).second;
		  d_data.erase(it);
	  }
	  return res;
  }

  inline Transaction last_transaction() { return get_transaction(d_cur_id); }

  class transaction_iterator {
      TransIdToEntry::const_iterator d_itr;
      transaction_iterator(const TransIdToEntry::const_iterator &itr) : d_itr(itr) { }
      friend class TransactionIDs;
  public:
      transaction_iterator() { }
      inline void operator=(const transaction_iterator &itr) { d_itr = itr.d_itr; }
      inline transaction_iterator &operator++() { ++d_itr; return *this; }
      inline bool operator==(const transaction_iterator &itr) const { return d_itr == itr.d_itr; }
      inline bool operator!=(const transaction_iterator &itr) const { return !operator==(itr); }
      Transaction operator*() { return (*d_itr).second; }
  };
  inline transaction_iterator transaction_begin() const { return d_data.begin(); }
  inline transaction_iterator transaction_end() const { return d_data.end(); }

  void set_lost(handlerType h, bool val=true)
  {
      for (transaction_iterator t = transaction_begin(); t != transaction_end(); ++t) {
          for (Transaction::result_iterator r = (*t).results_begin(), rend = (*t).results_end(); r != rend; ++r) {
              if (r.good() && r.hType() == h)
                  r.set_lost(val);
          }
      }
  }

  static bool HasPidName(const PidNames &names, const std::string *pidname)
  {
      for (PidNames::const_iterator p = names.begin(), pend = names.end();
          p != pend; ++p)
      {
          if ((*p) == pidname)
              return true;
      }
      return false;
  }

  void reorder_based_on_checkedout(handlerType h, const PidNames &names, PidNames *cnames)
  {
      PidNames tnames;

      for (transaction_iterator t = transaction_begin(); t != transaction_end(); ++t) {
          for (Transaction::result_iterator r = (*t).results_begin(), rend = (*t).results_end(); r != rend; ++r) {
              if (r.good() && r.hType() == h) {
                  if (HasPidName(names, r.pidname()) && !HasPidName(tnames, r.pidname()))
                      tnames.push_back(r.pidname());
              }
          }
      }

      if (tnames.empty()) { // nothing checked out satisfies, just copy
          for (PidNames::const_iterator p = names.begin(), pend = names.end();
              p != pend; ++p)
          {
              cnames->push_back(*p);
          }
      } else {  // now push any names found to front
          for (PidNames::const_iterator p = names.begin(), pend = names.end();
              p != pend; ++p)
          {
              if (HasPidName(tnames, *p))
                  cnames->push_back(*p);
          }

          // remaining to back
          for (PidNames::const_iterator p = names.begin(), pend = names.end();
              p != pend; ++p)
          {
              if (!HasPidName(*cnames, *p))
                  cnames->push_back(*p);
          }
      }
  }

};

void MGSG_LicenseHandlerBase::Core::init_TransactionIDs(TransactionIDs **obj) 
{ *obj = new TransactionIDs; }

/////////////////////////////////////////////////////////////////////////////

long MGSG_LicenseHandlerBase::Core::Mheap_lost( int *num_features, MGLS_LOST_FEATURES *lost_features[] )
{
    BlockSigs block_sigs(*this);
    unsigned int clim_num_tries = 0;

    if (d_clim_mode != CLIM_disabled) {
        CLIM::Ping();

        const climStatus cstatus = *SyncClimStatus::Access(d_clim_cb_status);

        if (cstatus != climOkay || d_lost_mode != lostModeUndefined) {
            if (cstatus == climAborted) { // need to restart CLIM and request needed features
                CLIM::Destroy();
                delete d_clim_ops;
                d_clim_ops = 0;
                d_clim_mode = CLIM_enabled;
                if (CLIM_use()) // cstatus reset to climOkay, now that CLIM restarted
                    d_lost_mode = climDoingReaquireAgain; // schedule reaquire
            } else if (cstatus == climOkay && d_lost_mode == climRestarted) { // clim restarted, get licenses back
                d_lost_mode = climDoingReaquireAgain;
            } else if (cstatus == climReconnecting && d_lost_mode == lostModeUndefined) { // initial loss
                d_lost_mode = flexDoingReaquire;
                d_trans_ids->set_lost(hCLIM, true);
            } else if (cstatus == climOkay && d_lost_mode == flexDoingReaquire) { // recovered
                d_lost_mode = lostModeUndefined;
                d_trans_ids->set_lost(hCLIM, false);
            }
            if (d_lost_mode == climDoingReaquireAgain) { // initiate background checkout request
                d_lost_mode = climDoingReaquire;
                CLIM::StringList feature_names;
                for (TransactionIDs::transaction_iterator t = d_trans_ids->transaction_begin(); t != d_trans_ids->transaction_end(); ++t) {
                    for (TransactionIDs::Transaction::result_iterator r = (*t).results_begin(), rend = (*t).results_end(); r != rend; ++r) {
                        if (r.reaquire() && r.hType() == hCLIM)
                            feature_names.push_back(*r.pidname());
                    }
                }
                if (!feature_names.empty())
                    CLIM::CheckoutBackground(feature_names, 2048);
            }
        }

        if (d_lost_mode == climDoingReaquire) { // get status, mark update features
            CLIM::StringList obtained_features;
            CLIM::StringList other_features;
            const bool still_pending = !CLIM::CheckoutBackgroundResult(obtained_features, other_features, clim_num_tries);
            if (!still_pending && !other_features.empty()) // failed to get some features, still in lost mode
                d_lost_mode = climDoingReaquireAgain;
            else if (!still_pending)
                d_lost_mode = lostModeUndefined; // all outstanding CLIM requests are satisfied
            if (!obtained_features.empty()) { //update features
                bool refresh_server = false;
                if (!d_server.defined) {
                    if (!CLIM::GetServerName(&d_server.name)) {
                        d_server.defined = true;
                        refresh_server = true;
                    }
                }
                for (TransactionIDs::transaction_iterator t = d_trans_ids->transaction_begin(); t != d_trans_ids->transaction_end(); ++t) {
                    for (TransactionIDs::Transaction::result_iterator r = (*t).results_begin(), rend = (*t).results_end(); r != rend; ++r) {
                        if (r.hType() == hCLIM) {
                            if (r.lost()) {
                                for (CLIM::StringList::const_iterator o = obtained_features.begin(), oend = obtained_features.end();
                                    o != oend; ++o)
                                {
                                    if (*o == *r.pidname())
                                        r.set_lost(false); // should update license info, type, expire days, etc @@@
                                }
                            }
                            if (r.good() && refresh_server) // refresh the server name
                                r.set_server(d_server.name);
                        }
                    }
                }
            }
        } else if (d_lost_mode == flexDoingReaquire) 
            clim_num_tries = d_clim_num_tries;
    }

    int mgls_num_features;
    MGLS_LOST_FEATURES *mgls_lost_features;

    long res = Mheap_ret(hMGLS, core_Mheap_lost( &mgls_num_features, &mgls_lost_features ));

    if (res != MGLS_OK) // something bad happened, just return an error
        return res;

    if (d_lost_mode != lostModeUndefined) { // there are CLIM features lost
        typedef std::vector<TransactionIDs::Transaction::result_iterator> LostResults;
        LostResults lost_results;
        for (TransactionIDs::transaction_iterator t = d_trans_ids->transaction_begin(); t != d_trans_ids->transaction_end(); ++t) {
            for (TransactionIDs::Transaction::result_iterator r = (*t).results_begin(), rend = (*t).results_end(); r != rend; ++r) {
                if (r.lost() && r.hType() == hCLIM)
                    lost_results.push_back(r);
            }
        }
        // combine any MGC ones.
        enum { easz = sizeof(MGLS_LOST_FEATURES) };
        const unsigned int ecnt = (lost_results.size()+mgls_num_features);
        const unsigned int esz = easz*ecnt;
        if (d_lost_buf)
            delete [] d_lost_buf;
        char *ebuf = new char[esz];
        d_lost_buf = ebuf;
        memset((void *)ebuf, 0, esz);
        MGLS_LOST_FEATURES *efirst = (MGLS_LOST_FEATURES *)ebuf;
        MGLS_LOST_FEATURES *eptr = efirst;
        for (LostResults::const_iterator p = lost_results.begin(), pend = lost_results.end(); p != pend; ++p) {
            strcpy(eptr->granted_feature, (*p).pidname()->c_str());
            strcpy(eptr->server, (*p).server().c_str());
            eptr->retry_count = clim_num_tries;
            eptr++;
        }
        for (int p = 0; p < mgls_num_features; ++p) {
            memcpy(eptr, &mgls_lost_features[p], sizeof(MGLS_LOST_FEATURES));
            eptr++;
        }
        *num_features = ecnt;
        *lost_features = efirst;
    } else { // just return MGLS results
        *num_features = mgls_num_features;
        *lost_features = mgls_lost_features;
    }

    return MGLS_OK;
}

/////////////////////////////////////////////////////////////////////////////

long MGSG_LicenseHandlerBase::Core::Mheap_allocv( int app_id, char *app_date )
{ 
  BlockSigs block_sigs(*this);
  if (d_exp_warn_days_unset) { // @@@ maybe have a startup "init"
    d_exp_warn_days_unset = true;
    MGLS_ATTR_VALUE wdays;
    if (Mheap_get(MGLS_ENV_EXP_WARN_DAYS, &wdays) == MGLS_OK)
      d_exp_warn_days = wdays;
  }

  const PidNames *CLIPnames, *MGLSnames;
  if (!d_satisfies->get(app_id, &CLIPnames, &MGLSnames))
  { CLIPnames = 0; MGLSnames = 0; }

  TransactionIDs::Transaction t = d_trans_ids->new_transaction(app_id);

  bool gotit = false;
  if (CLIM_use() && CLIPnames) {
      if (!CLIPnames->empty()) {
          PidNames CLIPnames_sorted; // reprioritize checked out first
          d_trans_ids->reorder_based_on_checkedout(hCLIM, *CLIPnames, &CLIPnames_sorted);
          for (PidNames::const_iterator p = CLIPnames_sorted.begin(), pend = CLIPnames_sorted.end();
              !gotit && p != pend; ++p) 
          {
              const int code = CLIM::Checkout((*p)->c_str());
              int days_left = MGLS_ERR;
              mgls_lictype lic_type = MGLS_floating;
              std::string error_str;
              if (!d_server.defined) {
                  if (!CLIM::GetServerName(&d_server.name))
                      d_server.defined = true;
              }
              if (!code) {
                  gotit = true;
                  {
                      NameToDaysLeft::iterator d = d_days_left.find(NameToDaysLeftPair(*p,0));
                      if (d == d_days_left.end()) {
                          days_left = CLIM::GetDaysLeft(*(*p));
                          d_days_left.insert(NameToDaysLeftPair(*p,days_left));
                      } else
                          days_left = (*d).second;
                  }
                  {
                      NameToLicType::iterator d = d_lic_type.find(NameToLicTypePair(*p,0));
                      if (d == d_lic_type.end()) {
                          switch(CLIM::GetFeatureType(*(*p)))
                          {
                          case CLIM::featureTypeNodelocked:
                              lic_type = MGLS_nodelocked_counted; 
                              break;
                          case CLIM::featureTypeUncounted:
                              lic_type = MGLS_nodelocked_uncounted;
                              break;
                          case CLIM::featureTypeFloating:
                              lic_type = MGLS_floating; 
                              break;
                          case CLIM::featureTypeError:
                              break; // assume default of lic_type
                          }
                          d_lic_type.insert(NameToPidPair(*p,lic_type));
                      } else
                          lic_type = (mgls_lictype)(*d).second;
                  }
              } else {
                  CLIM::GetErrorString(&error_str);
              }
              t.add_clip_result(*(*p), code, days_left, lic_type, error_str, d_server.name);
          }
      } else {
          // no licenses available on server -- just report all possible -- filter special?
          int num_of_ids;
          const int *id_list;
          if (Mheap_member( app_id, &num_of_ids, &id_list ) == MGLS_OK) { 
              for (int i=0; i<num_of_ids; ++i) {
                  const std::string *fname;
                  if (Mheap_lookup( id_list[i], &fname ) == MGLS_OK)
                      t.add_clip_no_exists_result(*fname);
              }
          }
      }
  }

  if (!gotit && is_clim_only_appid(app_id)) {
      const std::string *fname;
      if (Mheap_lookup( app_id, &fname ) == MGLS_OK)
          t.add_clip_no_exists_result(*fname);
  } else if (!gotit && MGLS_use()) { // always try mgc even if not possible according to MGLSnames  ???
      int c = core_Mheap_allocv( app_id, app_date );
      t.add_mgc_result(c);
      Mheap_ret(hMGLS, c);
      if (c == MGLS_ERR) {
          d_trans_ids->free_transaction(t.id());
          return c;
      }
  }

  return t.id();
}

long MGSG_LicenseHandlerBase::Core::Mheap_stat( long transaction_id )
{ 
  BlockSigs block_sigs(*this);
  TransactionIDs::Transaction t =  d_trans_ids->get_transaction(transaction_id);
  if (!t)
    return Mheap_set_errno(hCLIM, MGLS_trans_id_not_found);

  for (TransactionIDs::Transaction::result_iterator itr = t.results_begin(); itr != t.results_end(); ++ itr) {
    if (itr.hType() == hMGLS)
      return Mheap_ret(hMGLS, core_Mheap_stat( itr.code() ));
    if (itr.hType() == hCLIM && itr.code() == 0) { // check out good
      if (itr.days_left() != MGLS_ERR && itr.days_left() <= d_exp_warn_days)
        return MGLS_EXP_WARNING;
      return MGLS_OK;
    }
  }

  return Mheap_set_errno(hCLIM, MGLS_license_request_failed);
}

long MGSG_LicenseHandlerBase::Core::Mheap_free( long transaction_id )
{ 
  BlockSigs block_sigs(*this);
  TransactionIDs::Transaction t =  d_trans_ids->get_transaction(transaction_id);
  if (!t)
    return Mheap_set_errno(hCLIM, MGLS_trans_id_not_found);

  long res = MGLS_OK;
  for (TransactionIDs::Transaction::result_iterator itr = t.results_begin(); itr != t.results_end(); ++ itr) {
    if (itr.hType() == hMGLS) {
        long r = Mheap_ret(hMGLS, core_Mheap_stat( itr.code() ));
        if ( r != MGLS_OK )
            res = r;
    } else if (itr.hType() == hCLIM && itr.code() == 0) { // check was good -- return to server
      // CLIM::CheckIn(itr.pidname()) -- no API call
    }
  }

  d_trans_ids->free_transaction( t.id() );

  return res;
}

long MGSG_LicenseHandlerBase::Core::Mheap_licinfo( struct MGLS_LICINFO *license_info )
{ 
  BlockSigs block_sigs(*this);
  TransactionIDs::Transaction t =  d_trans_ids->last_transaction();
  if (t) {
    for (TransactionIDs::Transaction::result_iterator itr = t.results_begin(); itr != t.results_end(); ++ itr) {
      if (itr.hType() == hMGLS)
        return Mheap_ret(hMGLS, core_Mheap_licinfo( license_info ));
      if (itr.hType() == hCLIM && itr.code() == 0) {
        int actual_id;
        if (!nameToPidNum(itr.pidname(), &actual_id))
          actual_id = 0;
        strcpy(license_info->feature_name, itr.pidname()->c_str());
        license_info->license_type = itr.lic_type();
        license_info->next_count = 0;
        license_info->expiration_info = itr.days_left();
        strcpy(license_info->serial_number, "CDS");
        license_info->count = 1;
        license_info->granted_application_id = actual_id;
        strcpy(license_info->server_hostid, MGLS_UNKNOWN_HOSTID); // @@@ could also be MGLS_UNCOUNTED_HOSTID
        license_info->exact_access_month = -1;
        license_info->exact_access_year = -1;
        return license_info->next_count;
      }
    }
  }
  return Mheap_set_errno(hCLIM, MGLS_license_info_error);
}

long MGSG_LicenseHandlerBase::Core::Mheap_validv( int app_id, char *app_date )
{ 
  BlockSigs block_sigs(*this);
  const PidNames *CLIPnames, *MGLSnames;
  if (!d_satisfies->get(app_id, &CLIPnames, &MGLSnames))
  { CLIPnames = 0; MGLSnames = 0; }

  if (CLIM_use() && CLIPnames && !CLIPnames->empty()) {
      for (PidNames::const_iterator p = CLIPnames->begin(); p != CLIPnames->end(); ++p) {
          std::string svrlist;
          CLIM::GetFeatureServerList(*(*p), &svrlist);
          if (!svrlist.empty())
              return MGLS_LICENSE_AVAILABLE;
    }
  }

  // always try mgc even if not possible according to MGLSnames  ???
  if (MGLS_use() && !is_clim_only_appid(app_id))
      return Mheap_ret(hMGLS, core_Mheap_validv( app_id, app_date )); 

  return Mheap_set_errno(hCLIM, MGLS_license_request_failed);
}

long MGSG_LicenseHandlerBase::Core::Mheap_errs( char **error_string )
{ 
    BlockSigs block_sigs(*this);
    if (d_errno_own == hCLIM) {
        *(*error_string) = '\0';
        if (d_trans_ids) {
          TransactionIDs::Transaction t =  d_trans_ids->last_transaction();
          if (t) {
            for (TransactionIDs::Transaction::result_iterator itr = t.results_begin(); itr != t.results_end(); ++ itr) {
              if (itr.hType() == hCLIM && itr.code() != 0) {
                strncpy(*error_string, itr.error_str().c_str(), SIZE_OF_MGLS_ERROR_STRING);
                (*error_string)[SIZE_OF_MGLS_ERROR_STRING-1] = '\0';
                break;
              }
            }
          }
        }
        return MGLS_OK;
    }
    return Mheap_ret(hMGLS, core_Mheap_errs( error_string )); 
}

long MGSG_LicenseHandlerBase::Core::Mheap_contains( int pid, int *num_of_ids, int **id_list )
{ 
  BlockSigs block_sigs(*this);
  if (is_clim_only_appid(pid)) {
    if (d_clim_to_pids) {
      PidToContains::const_iterator p = d_clim_to_pids->find(PidToContainsPair(pid,0));
      if (p != d_clim_to_pids->end()) {
        *num_of_ids = (*p).second->atomics.num_of_ids;
        *id_list = (*p).second->atomics.id_list;
        return MGLS_OK;
      }
    }
    return MGLS_ERR;
  }
  return Mheap_ret(hMGLS, core_Mheap_contains( pid, num_of_ids, id_list )); 
}

/////////////////////////////////////////////////////////////////////////////

void MGSG_LicenseHandlerBase::Core::clim_log_cb(const char* format, ...)
{
    // !!!CAUTION!!! MAY BE ON DIFFERENT THREAD !!!
    //Core *core = (Core *)CLIM::GetOptions().p_clientData;
    // format and store in last error?
}

void MGSG_LicenseHandlerBase::Core::clim_abort_cb(const std::string& msg)
{
    // !!!CAUTION!!! MAY BE ON DIFFERENT THREAD !!!
    Core *core = (Core *)CLIM::GetOptions().p_clientData;
    *SyncClimStatus::Access(core->d_clim_cb_status) = climAborted; // what if still in recovery mode?
}

void MGSG_LicenseHandlerBase::Core::clim_hrtbeat_reconn_cb(int num_retries)
{
    // !!!CAUTION!!! MAY BE ON DIFFERENT THREAD !!!
    Core *core = (Core *)CLIM::GetOptions().p_clientData;
    SyncClimStatus::Access amode(core->d_clim_cb_status);
    if (*amode != climAborted) {
        core->d_clim_num_tries = num_retries;
        if (num_retries == 0) { // recovered sucessful
            *amode = climOkay;  // what if still in recovery mode?
        } else { // still trying to recover
            *amode = climReconnecting;  // what if still in recovery mode?
        }
    }
}

void MGSG_LicenseHandlerBase::Core::clim_hrtbeat_fail_cb(int max_retries)
{
    // !!!CAUTION!!! MAY BE ON DIFFERENT THREAD !!!
    Core *core = (Core *)CLIM::GetOptions().p_clientData;
    SyncClimStatus::Access amode(core->d_clim_cb_status);
    if (*amode != climAborted)
        *amode = climReconnecting;  // what if still in recovery mode?
}

/////////////////////////////////////////////////////////////////////////////

bool MGSG_LicenseHandlerBase::Core::HasTransaction(handlerType htype) const
{
  if (d_trans_ids) {
    for (TransactionIDs::transaction_iterator t = d_trans_ids->transaction_begin(); t != d_trans_ids->transaction_end(); ++t) {
      for (TransactionIDs::Transaction::result_iterator r = (*t).results_begin(), rend = (*t).results_end(); r != rend; ++r) {
        if (r.hType() == htype)
          return true;
      }
    }
  }
  return false;
}

void MGSG_LicenseHandlerBase::Core::CLIM_use_start() 
{
   *SyncClimStatus::Access(d_clim_cb_status) = climOkay;
   if (HasTransaction(hCLIM)) {
        d_trans_ids->set_lost(hCLIM, true);
        d_lost_mode = climRestarted;
   } else
        d_lost_mode = lostModeUndefined;
    d_server.defined = false;
    d_clim_ops = new CLIM::Options;
#  ifndef _MSC_VER
    d_clim_ops->pgid = getpgrp();
#  endif
    d_clim_ops->heartbeat = true;

    char attr_buf[MGLS_TRUNCATED_ENV_SIZE];
    if (Mheap_get( MGLS_ATTR_DISPLAY, reinterpret_cast<MGLS_ATTR_VALUE *>(attr_buf)) == MGLS_OK)
        d_clim_ops->dup_code = attr_buf;
    d_clim_ops->do_not_close_forked_child_pipedes_in_parent = false;
    d_clim_ops->p_log_cb = clim_log_cb;
    d_clim_ops->p_abort_cb = clim_abort_cb;
    d_clim_ops->p_heartbeat_reconn_cb = clim_hrtbeat_reconn_cb;
    d_clim_ops->p_heartbeat_fail_cb = clim_hrtbeat_fail_cb;

    d_clim_ops->p_clientData = (void *)this;

    d_clim_ops->call_reconn_cb_if_ok = true;

    if (d_handler.d_lic_file_calyptod)
        d_clim_ops->lm_license_file = *d_handler.d_lic_file_calyptod;
        
    {
        static char e[] = MGSG_CRYPT("MGC_HOME");
        std::string ename;
        MGSG_dstring(&ename, e);
        std::string pkginfo_file;
        CLIM::GetEnvVar(ename, &pkginfo_file);
        if (!pkginfo_file.empty()) {
            static char e[] = MGSG_CRYPT("/lib/cds_clip.pkginfo");
            MGSG_dstring(&ename, e);
            pkginfo_file += ename;
            d_clim_ops->pkginfo_file = pkginfo_file;
        }
    }

    std::string clip_command;
    {
        static char e[] = MGSG_CRYPT("CALYPTOD_CLIP_COMMAND");
        std::string ename;
        MGSG_dstring(&ename, e);
        CLIM::GetEnvVar(ename, &clip_command);
    }
    if (clip_command.empty()) {
        static char e[] = MGSG_CRYPT("MGC_HOME");
        std::string ename;
        MGSG_dstring(&ename, e);
        CLIM::GetEnvVar(ename, &clip_command);
        if (!clip_command.empty()) {
            static char e[] = MGSG_CRYPT("/pkgs/cds_clip/.bin/CLIP");
            MGSG_dstring(&ename, e);
            clip_command += ename;
        }
    }

    int sres = CLIM::Init(clip_command, *d_clim_ops);
    if (sres != 0) {
        delete d_clim_ops;
        d_clim_ops = 0;
        d_clim_mode = CLIM_exited_error;
    } else {
        d_clim_mode = CLIM_running;
    }
}

long MGSG_LicenseHandlerBase::Core::Mheap_clean()
{
  long res;
  { 
	BlockSigs block_sigs(*this); 
	res = Mheap_ret(hMGLS, core_Mheap_clean()); 
  }
  if (d_clim_ops) {
    CLIM::Destroy();
    delete d_clim_ops;
    d_clim_ops = 0;
    d_clim_mode = CLIM_enabled;
  }
  delete d_trans_ids; d_trans_ids=0;
  delete d_satisfies; d_satisfies=0;
  delete d_pid_exists; d_pid_exists=0;
  delete d_pid_members; d_pid_members=0;
  if (d_lost_buf) {
      delete [] d_lost_buf; 
      d_lost_buf = 0;
  }
  d_server.defined = false;
  d_days_left.clear();
  d_lic_type.clear();
  return res;
}

/////////////////////////////////////////////////////////////////////////////

MGSG_LicenseHandlerBase::Core::~Core() 
{ 
    Mheap_clean();
    if (d_clim_to_pids) {
      for (PidToContains::iterator p = d_clim_to_pids->begin(), pend=d_clim_to_pids->end(); p != pend; ++p)
        delete (*p).second;
      delete d_clim_to_pids;
      d_clim_to_pids=0;
    }
    for (PidToName::const_iterator i = d_pid_to_name.begin(); i != d_pid_to_name.end(); ++i)
        delete (*i).second;
}

/*==========================================================================
- ======================= HANDLER::TRANSACTIONPOOL =========================
- ========================================================================== */

//////////////////////////////////////////////////////////////////////////////
// Class Definition : The TransactionPool class maintains a reference counted
//                    list of MGLS transactions.
//////////////////////////////////////////////////////////////////////////////
class MGSG_LicenseHandlerBase::TransactionPool
{
	CC____RelaxProtection(public)
  enum otherTransactionType
  { 
    licenseType = 0x00,
    serialNums
  };
  enum { transactionType_start = serialNums+1 };
	CC____RestoreProtection(private)

  struct Node;

public:
	TransactionPool(MGSG_LicenseHandlerBase &handler) 
		: d_handler(handler), d_pool(0) { }

	class Reference;
    class dynamicReference;
	class contains_iterator;

	enum transactionType {
		None /* Used as Reference Counter */  = transactionType_start,
		Contained,
		Shared,
		Checkedout,
		Duplicate,
		Actual
	};

  class TransactionTypeCounts {
    Node *const d_node;

    enum { transactionType_sz = Actual+1 };

    unsigned int d_transtypecnt[transactionType_sz];
    mutable bool d_transtypecnt_update[transactionType_sz];
    template<typename tType>
    inline void update_(tType transtype) const
    {
      if (d_transtypecnt_update[transtype]) {
        // recurse forever if request for value while update, update flag after
        d_node->update_transcount(transtype);
        d_transtypecnt_update[transtype] = false;
      }
    }
    template<typename tType>
    inline const unsigned int &get_(tType transtype) const
    { 
      update_(transtype); return d_transtypecnt[transtype]; 
    }

  public:
    template<typename tType>
    class Writable {
      TransactionTypeCounts &d_obj;
      const tType d_type;
      void operator=(Writable &); // keep gcc happy
    public:
      Writable(TransactionTypeCounts &obj, tType t) 
        : d_obj(obj), d_type(t) { }
      inline void invalidate() { d_obj.d_transtypecnt_update[d_type] = true; }
      inline void operator=(unsigned int value) { invalidate(); d_obj.d_transtypecnt[d_type] = value; }
      inline void operator--() { invalidate(); --d_obj.d_transtypecnt[d_type]; }
      inline void operator++() { invalidate(); ++d_obj.d_transtypecnt[d_type]; }
      inline operator const unsigned int &() const { return d_obj.get_(d_type); }
    };

    TransactionTypeCounts(Node *node)
      : d_node(node)
    {
      memset(d_transtypecnt,0,sizeof(d_transtypecnt));
      memset(d_transtypecnt_update,0,sizeof(d_transtypecnt_update));
      d_transtypecnt[licenseType] = MGSG_uninitialized;
    }

    template<typename tType>
    inline Writable<tType> operator[](tType transtype) { return Writable<tType>(*this, transtype); }

    template<typename tType>
    inline const unsigned int &operator[](tType transtype) const { return get_(transtype); }
  };

private:
	CC____RelaxProtection(public)
	struct Node {
        MGSG_LicenseHandlerBase &handler;

		Node *next;

		unsigned int refcnt;

		const int idx;

		PidPool::Reference *real;

        bool allocated;
        long transaction;  // Transaction id returned by Core::Mheap_allocv
		bool expiresoon;   // indicates license which satified will expire soon
        bool isactual;     // indicates that the license which satisfied request is same as requested

		SerialNums serialnums;  // maintains serial number information for transactions

    TransactionTypeCounts transtypecnt;

    class ActualReference;
    ActualReference *actual;
        
		class References {
        protected:
            References(Reference *_transaction) 
                : next(0), transaction(_transaction), pid_reference(false) { }
        public:
			References *next;
			Reference  *transaction;
            bool        pid_reference; 
            
			References(MGSG_LicenseMessages *messages, 
                       References **start, References *cur, Reference *_transaction,
                       bool alloc_pid_reference);
			~References();
		};
		References *contains;
		bool contains_set;

        Core &core() { return *handler.d_core; }

        static void set_contains(bool *pids_contained, const MGSG_MGLS_pid &pid);

        void set_contains(MGSG_LicenseMessages *messages, const int num_of_ids, const int *id_list);

        void set_contains(MGSG_LicenseMessages *messages);

        const References *get_contains() const {
            if (!contains_set)
                ((Node *)this)->set_contains(0);
            return contains;
        }

        class Alternate : public References {
            struct Node {
                Node *prev;
                Reference *transaction;
                Node(Reference *_transaction, Node *_prev)
                    : prev(_prev), transaction(_transaction) { }
            };
            Node *d_node;
        public:
            Alternate(MGSG_LicenseMessages &messages, 
                      Alternate **start, Alternate *cur, Reference *transaction);
            ~Alternate();
            bool alloc(MGSG_LicenseMessages &messages, const transactionType transactiontype);
            bool unalloc(MGSG_LicenseMessages *messages, const transactionType transactiontype);
        };
        Alternate *alternate;
    
        void apply_actions(const TransactionPool &pool, const MGSG_MGLS_pid &pid);

        void allocv(TransactionPool &pool, MGSG_LicenseMessages &messages,
                    const transactionType transactiontype);
        void sharedv(TransactionPool &pool, MGSG_LicenseMessages &messages);
        void realv(TransactionPool &pool, MGSG_LicenseMessages &messages);
        void alternatev(TransactionPool &pool, MGSG_LicenseMessages &messages,
                        const transactionType transactiontype);
        void setv(MGSG_LicenseMessages &messages, MGSG_License_Type lictype, 
				  const SerialNums &licsnums, int actualpid);
        void unallocv(MGSG_LicenseMessages *messages);

        bool should_invalidate_actual_transcount() const;
        TransactionTypeCounts &actual_transcount();

        void set_expiresoon(MGSG_LicenseMessages &messages);
        void update_transcount(otherTransactionType transtype) const;
        void update_transcount(transactionType transtype) const;

        inline unsigned int transcount(transactionType transtype) const {
          return transtypecnt[transtype];
        }

        inline const SerialNums &transserialnums() const {
          transtypecnt[serialNums]; // cause updated if need be
          return serialnums;
        }

        void set_lictype(MGSG_License_Type lictype) {
            if (transtypecnt[licenseType] < (unsigned int)lictype
                || (lictype == MGSG_disabled && transtypecnt[licenseType] == MGSG_unused)) {
                transtypecnt[licenseType]=lictype; 
            }
        }

        Node(MGSG_LicenseHandlerBase &handler_) 
          : handler(handler_), next(0), refcnt(0), idx(0),
          real(0), allocated(false),
          transaction(MGLS_ERR), expiresoon(false), isactual(false),
          transtypecnt(this),
          actual(0), contains(0), contains_set(false), alternate(0) 
        { }

        Node(MGSG_LicenseHandlerBase &handler_,
             const int _idx, const MGSG_MGLS_pid &pid, Node *_next) 
          : handler(handler_), next(_next), refcnt(0), idx(_idx),
            real(new PidPool::Reference(pid)), allocated(false),
            transaction(MGLS_ERR), expiresoon(false), isactual(false),
            transtypecnt(this),
            actual(0), contains(0), contains_set(false), alternate(0)
        { }

        ~Node();

		static const Node *invalid();
	};
	CC____RestoreProtection(private)

	MGSG_LicenseHandlerBase &d_handler;
	Node *d_pool;

	friend class Reference;
	friend class contains_iterator;
	friend class actuals_iterator;
	friend struct actuals_iterator::data;

  inline Core &core() { return *d_handler.d_core; }

	const Node *Alloc(MGSG_LicenseMessages &messages,
					  const MGSG_MGLS_pid &pid, 
					  const transactionType transactiontype);

	void Dealloc(MGSG_LicenseMessages *messages,
			     const Node *node,
				 const transactionType transactiontype);

public:
	DEBUG_LICENSE_print

	class Reference {
		TransactionPool *d_pool;
		const transactionType d_transactiontype;
        const Node *d_node;
		friend struct Node;
		friend class contains_iterator;
    public:
		Reference(MGSG_LicenseMessages &messages, const MGSG_MGLS_pid &pid,
				  const transactionType transactiontype)
			: d_pool(get_transpool(pid)),
			  d_transactiontype(transactiontype),
			  d_node(d_pool ? d_pool->Alloc(messages, pid, d_transactiontype) : 0)
		{ if (!d_node) d_node = Node::invalid(); }
        ~Reference() { if (valid()) d_pool->Dealloc(0, d_node, d_transactiontype); } //@@@ messages
        transactionType transactiontype() const { return d_transactiontype; }
		operator bool () const { return valid(); }
		bool valid() const { return d_node != Node::invalid(); }
		const MGSG_MGLS_pid *pid() const { return d_node->real ? &(*(*d_node->real)) : 0; }
		int pidnum() const { return d_node->real ? (*(*d_node->real)).get_pid() : -1; }
		const char *pidname() const { return d_node->real ? d_node->real->get_name() : unknown_str; }
		int actualpidnum() const;
        const char *actualpidname() const;
		bool expiresoon() const { return d_node->expiresoon; }
        bool allocated() const { return d_node->allocated; }
        MGSG_License_Type lictype() const { return (MGSG_License_Type)d_node->transtypecnt[licenseType]; }
        void set_lictype(MGSG_License_Type lictype) { ((Node *)d_node)->set_lictype(lictype); }
        const SerialNums &serialnums() const { return d_node->transserialnums(); }
        bool transallocated() const { return d_node->transaction != MGLS_ERR; }
        bool alternateallocated() const { return allocated() && !transallocated();  }
        const TransactionTypeCounts &transcount() const { return d_node->transtypecnt; }
		unsigned int checkedout() const { return d_node->transcount(Checkedout); }
		unsigned int actuals() const { return d_node->transcount(Actual); }
		unsigned int duplicates() const { return d_node->transcount(Duplicate); }
    };

    class contains_iterator {
		const Node::References *d_cur;
	public:
		contains_iterator(const Node &node) 
            : d_cur(node.get_contains()) { }
		contains_iterator(const Reference &transaction) 
            : d_cur(transaction.d_node->get_contains()) { }
		contains_iterator(const MGSG_MGLS_pid &pid); 
		contains_iterator& operator++() { if (!is_end()) d_cur = d_cur->next;; return *this; }
		const MGSG_MGLS_pid* operator*() const { return !is_end() ? d_cur->transaction->pid() : 0; }
        unsigned int transcount(transactionType transactiontype) const 
        { return !is_end() ? d_cur->transaction->transcount()[transactiontype] : 0; }
        bool transallocated() const 
        { return !is_end() ? d_cur->transaction->transallocated() : false; }
        bool alternateallocated() const 
        { return !is_end() ? d_cur->transaction->alternateallocated() : false; }
		bool is_end() const { return !d_cur; }
        static bool is_empty(const Reference &transaction) { return !transaction.d_node->get_contains(); }
	};
	
    const MGSG_MGLS_pid *contained_allocated(const MGSG_MGLS_pid &pid) const;

    void contained_remove(const MGSG_MGLS_pid &pid) const;
	void UnRegister_pid(MGSG_MGLS_pid &pid);
};

const MGSG_LicenseHandlerBase::TransactionPool::Node *MGSG_LicenseHandlerBase::TransactionPool::Node::invalid()
{
	static const Node s_invalid(*(MGSG_LicenseHandlerBase *)0);
	return &s_invalid;
}

const MGSG_MGLS_pid *MGSG_LicenseHandlerBase::TransactionPool::contained_allocated(const MGSG_MGLS_pid &pid) const
{
    const MGSG_MGLS_pid *spid=0;
    for (const Node *n=d_pool; n && !spid; n=n->next) {
        bool pidfound=false;
        for (contains_iterator c(*n); !c.is_end(); ++c) {
            if ((*c) == &pid) {
                pidfound=true;
                if (spid)
                    break; // already found checked out pid
            } else if (c.transallocated() && !spid) {
                spid = (*c);
                if (pidfound)
                    break; // matching pid was already found, no need to search more
            }
        }
        if (!pidfound)
            spid=0;
    }
    return spid;
}

/*==========================================================================
- ============================== TRANSACTIONS ==============================
- ========================================================================== */

//////////////////////////////////////////////////////////////////////////////
// Class Definition : The Transaction class is the basic class to interface
// to MGLS to checkout a license.  The transactions are maintained in the
// transaction pool to allow management of the MGLS transactions 
// independently.
//////////////////////////////////////////////////////////////////////////////


class MGSG_LicenseHandlerBase::Transactions : public MGSG_LicenseHandlerBase::TransactionPool::Reference {
	Transactions *prev;
	Transactions(MGSG_LicenseMessages &messages, MGSG_MGLS_pid &pid, 
				 TransactionPool::transactionType transtype)
		: TransactionPool::Reference(messages, pid, transtype)
	{
        if (valid()) {
            prev=pid.d_transactions;
            pid.d_transactions=this;
        } else
            prev=0;
    }

    static bool alloc(MGSG_LicenseMessages &messages,
                      TransactionPool::transactionType transtype,
					  MGSG_MGLS_pid &pid)
    {
        bool res=false;
        if (!pid.is_registered())
            messages.append(MGSG_LicenseMessages::MSG_ERR_LicenseNotRegistered);
        else {
            Transactions *t = new Transactions(messages, pid, transtype);
            if (t->valid())
                res = true;
            else
                delete t;
        }
        return res;
    }

    static bool unalloc(TransactionPool::transactionType transtype,
						MGSG_MGLS_pid &pid)
    {
		bool res=false;
    	Transactions *c=pid.d_transactions, *l=0;
		while (c) {
			if (c->transactiontype() == transtype) {
				if (!l)
					pid.d_transactions = c->prev;
				else
					l->prev = c->prev;
				c->prev = 0;
				delete c;  //@@@ NEED TO GET MESSAGES / STATUS FROM TRANS DELETE (i.e. checkin)
				res = true;
				break;
			}
			l=c; c=c->prev;
		}
        if (const MGSG_LicenseHandlerBase::TransactionPool *transpool = get_transpool(pid))
            transpool->print();
		return res;
    }


public:
	~Transactions() { while (prev) { Transactions *p=prev; prev=prev->prev; p->prev=0; delete p; } }

    static bool reference(MGSG_LicenseMessages &messages, const MGSG_MGLS_pid &pid)
    {
        return alloc(messages, MGSG_LicenseHandlerBase::TransactionPool::None,
                     (MGSG_MGLS_pid &)pid);
    }

    static bool unreference(MGSG_LicenseMessages *messages, const MGSG_MGLS_pid &pid)
    {
        const bool res=unalloc(MGSG_LicenseHandlerBase::TransactionPool::None,
                               (MGSG_MGLS_pid &)pid);
		if (!res && messages)
			messages->append(MGSG_LicenseMessages::MSG_ERR_no_references_allocated, pid.get_name());
		return res;
    }

    static bool checkout(MGSG_LicenseMessages &messages, const bool duplicate, 
						 MGSG_MGLS_pid &pid) 
    {
        const bool res = alloc(messages, 
							   duplicate ? MGSG_LicenseHandlerBase::TransactionPool::Duplicate 
							   : MGSG_LicenseHandlerBase::TransactionPool::Actual,
							   pid);
        if (const MGSG_LicenseHandlerBase::TransactionPool*transpool = get_transpool(pid))
            transpool->print();
		return res;
	}

	static bool checkin(MGSG_LicenseMessages &messages, const bool duplicate, 
						MGSG_MGLS_pid &pid)
    {
        const bool res=unalloc(duplicate ? MGSG_LicenseHandlerBase::TransactionPool::Duplicate 
			    : MGSG_LicenseHandlerBase::TransactionPool::Actual, pid);
		if (!res)
			messages.append(duplicate ? MGSG_LicenseMessages::MSG_ERR_no_duplicates_allocated
							: MGSG_LicenseMessages::MSG_ERR_no_actuals_allocated, pid.get_name());
		return res;
	}
};

MGSG_LicenseHandlerBase::TransactionPool::contains_iterator::contains_iterator(const MGSG_MGLS_pid &pid) 
    : d_cur(get_transactions(pid) ? get_transactions(pid)->d_node->get_contains() : 0)
{ }

/*==========================================================================
- =========================== CONTAINS ITERATOR  ===========================
- ========================================================================== */

class MGSG_LicenseHandlerBase::contains_iterator::iterator : public MGSG_LicenseHandlerBase::TransactionPool::contains_iterator
{
public:
    iterator(const MGSG_MGLS_pid &pid)
        : MGSG_LicenseHandlerBase::TransactionPool::contains_iterator(pid) { }
};

MGSG_LicenseHandlerBase::contains_iterator::contains_iterator(const MGSG_MGLS_pid &pid)
    : d_itr(new iterator(pid))
{ 
    _end();
}

MGSG_LicenseHandlerBase::contains_iterator::~contains_iterator()
{ 
    delete d_itr;
}

void MGSG_LicenseHandlerBase::contains_iterator::_end()
{
    if (d_itr->is_end()) {
        delete d_itr;
        d_itr = 0;
    }
}

void MGSG_LicenseHandlerBase::contains_iterator::_next()
{
    ++(*d_itr);
    _end();
}

const MGSG_MGLS_pid* MGSG_LicenseHandlerBase::contains_iterator::_get() const
{
    return *(*d_itr);
}

//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////

class MGSG_LicenseHandlerBase::TransactionPool::Node::ActualReference
    : public MGSG_LicenseHandlerBase::TransactionPool::Reference
{
public:
    ActualReference(MGSG_LicenseMessages &messages, const MGSG_MGLS_pid &pid,
				    const MGSG_LicenseHandlerBase::TransactionPool::transactionType transactiontype)
        : MGSG_LicenseHandlerBase::TransactionPool::Reference(messages, pid, transactiontype) 
    { MGSG_LicenseHandlerBase::Transactions::reference(messages, pid); }
    ~ActualReference() { // @@@ MESSAGES 
        if (const MGSG_MGLS_pid *p = pid())
            MGSG_LicenseHandlerBase::Transactions::unreference(0, *p);
        //else
        //	messages->append(MGSG_LicenseMessages::MSG_ERR_LicenseNotRegistered);
    }
};

MGSG_LicenseHandlerBase::TransactionPool::Node::~Node()
{
    delete contains;
    delete alternate;
    delete actual;
    delete real; 
}

int MGSG_LicenseHandlerBase::TransactionPool::Reference::actualpidnum() const
{
    return d_node->actual ? d_node->actual->pidnum() : -1; 
}

const char *MGSG_LicenseHandlerBase::TransactionPool::Reference::actualpidname() const
{
    return d_node->actual ? d_node->actual->pidname() : unknown_str;
}

MGSG_LicenseHandlerBase::TransactionPool::Node::References::References(MGSG_LicenseMessages *messages,
                                                                       References **start, 
                                                                       References *cur, 
                                                                       Reference *_transaction,
                                                                       bool alloc_pid_reference)
    : next(0), transaction(_transaction), pid_reference(alloc_pid_reference)
{
    if (start) {
        if (cur)
            cur->next = this;
        else if (!*start)
            *start = this;
        else {
            cur = *start;
            while (cur->next)
                cur = cur->next;
            cur->next = this;
        }
    }
    if (pid_reference) {
        bool delete_messages=false;
        if (!messages && !delete_messages)
                delete_messages = !!(messages = new MGSG_LicenseMessages);
        if (const MGSG_MGLS_pid *pid = transaction->pid())
            MGSG_LicenseHandlerBase::Transactions::reference(*messages, *pid);
        else {
            pid_reference=false;
			messages->append(MGSG_LicenseMessages::MSG_ERR_LicenseNotRegistered);
        }
        if (delete_messages)
            delete messages;
    }
}

MGSG_LicenseHandlerBase::TransactionPool::Node::References::~References()
{ 
    while (next) { 
        References *n=next->next;
        next->next=0;
        delete next;
        next=n; 
    }
    if (pid_reference) {
        if (const MGSG_MGLS_pid *pid = transaction->pid())
            MGSG_LicenseHandlerBase::Transactions::unreference(0, *pid); // @@@ messages
        // else should not happen, but add messages
    }
    delete transaction;
}

MGSG_LicenseHandlerBase::TransactionPool::Node::Alternate::Alternate(MGSG_LicenseMessages &messages, 
                                                                     Alternate **start, 
                                                                     Alternate *cur, 
                                                                     Reference *transaction)
    : References(new Reference(messages, *transaction->pid(), None)),
      d_node(new Node(transaction,0))
{
    if (start) {
        if (cur)
            cur->next = this;
        else if (!*start)
            *start = this;
        else {
            cur = *start;
            while (cur->next)
                cur = (Alternate *)cur->next;
            cur->next = this;
        }
    }
}

MGSG_LicenseHandlerBase::TransactionPool::Node::Alternate::~Alternate()
{
    while (d_node) {
        Node *n=d_node;
        d_node=n->prev;
        delete n->transaction;
        delete n;
    }
}

bool MGSG_LicenseHandlerBase::TransactionPool::Node::Alternate::alloc(MGSG_LicenseMessages &messages,
                                                                      const transactionType transactiontype) 
{
    bool res=true;
    struct nodeData { 
        nodeData *next;
        Reference *r;
        nodeData(Reference *_r) : next(0), r(_r) {}
    } *ndata=0,*ndata_c=0;
    for (Alternate *a=this; a; a=(Alternate *)a->next) {
        Reference *r=new Reference(messages, *a->d_node->transaction->pid(), transactiontype);
        if (r->valid()) {
            if (!ndata)
                ndata=ndata_c=new nodeData(r);
            else
                ndata_c=ndata_c->next=new nodeData(r);
        } else {
            delete r;
            while ((ndata_c=ndata)) {
                ndata=ndata_c->next;
                delete ndata_c->r;
                delete ndata_c;
            }
            res=false;
            break;
        }
    }
    for (Alternate *aa=this; ndata; aa=(Alternate *)aa->next) {
        aa->d_node = new Node(ndata->r, aa->d_node);
        ndata_c=ndata;
        ndata=ndata->next;
        delete ndata_c;
    }
    return res;
}

bool MGSG_LicenseHandlerBase::TransactionPool::Node::Alternate::unalloc(MGSG_LicenseMessages * /* messages */,
                                                                        const transactionType transactiontype) 
{
    bool res=!!this->d_node; // if there is atleast one entry start as success, otherwise it will be a failure
    for (Alternate *a=this; a; a=(Alternate *)a->next) {
        bool found=false;
        for (Node *n=a->d_node,*l=0; n; l=n,n=n->prev) {
            if (n->transaction->transactiontype()==transactiontype) {
                if (l)
                    l->prev=n->prev;
                else
                    a->d_node=n->prev;
                delete n->transaction;
                delete n;
                found=true;
                break;
            }
        }
        if (!found)
            res=false;  // mismatched transtype count ...
    }
    return res;
}

void MGSG_LicenseHandlerBase::TransactionPool::UnRegister_pid(MGSG_MGLS_pid &pid)
{
#if defined(DEBUG_LICENSE) && DEBUG_LICENSE > 1
	cout << "// Debug: TransactionPool::Unregister_pid"
		 << " pidname=" << pid.get_name() 
		 << endl;
#endif

	struct todo {
		struct todo *prev;
		Node::References *r;
        Node::ActualReference *actual;
		todo(struct todo *_prev, Node::References *_r)
			: prev(_prev), r(_r), actual(0) { }
		todo(struct todo *_prev, Node::ActualReference *_actual)
			: prev(_prev), r(0), actual(_actual) { }
	} *post_todo = 0;

	// need to remove the pid from the contains list and actual
	// post delete items since deleting may invalidate the d_pool pointer
    for (Node *n=d_pool; n; n=n->next) {
		for (Node::References *l=0,*r=n->contains; r; ) {
			if (r->transaction->pid() != &pid) {
				l=r, r=r->next;
			}  else {
				post_todo = new todo(post_todo, r);
				if (!l)
					r = n->contains = r->next;
				else
					r = l->next = r->next;
				post_todo->r->next = 0;
			}
		}
		if (n->actual && n->actual->pid() == &pid) {
			post_todo = new todo(post_todo, n->actual);
			n->actual = 0;
		}
    }

	while (todo *c = post_todo) {
		post_todo = c->prev;
		delete c->r;
		delete c->actual;
		delete c;
	}

	print();
}

//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////

#if defined(DEBUG_LICENSE) && DEBUG_LICENSE > 1
void MGSG_LicenseHandlerBase::TransactionPool::print() const 
{
    for (const Node *c = d_pool; c; c=c->next) {
		cout << "// Debug: Transaction" << setiosflags(std::ios::left)
			 << " refcnt="         << setw(4)  << c->refcnt
			 << " pidname="        << setw(16) << (c->real && c->real->get_name() ? c->real->get_name() : "(null)")
			 << " actual_pidname=" << setw(16) << (c->actual ? c->actual->pidname() : "(null)")
			 << " checkedout="     << setw(2)  << c->transcount(Checkedout)
			 << " actuals="        << setw(2)  << c->transcount(Actual)
			 << " duplicates="     << setw(2)  << c->transcount(Duplicate)
			 << " shared="         << setw(2)  << c->transcount(Shared)
			 << " contained="      << setw(2)  << c->transcount(Contained)
			 << " references="     << setw(2)  << c->transcount(None)
			 << endl;
    }
    for (MGSG_LicenseHandlerBase::actuals_iterator i(d_handler); !i.is_end(); ++i) {
		cout << "// Debug: Actual   " << setiosflags(std::ios::left)
			 << " pidnum="          << setw(6)  << (*i)->get_pid()
			 << " pidname="         << setw(16) << (*i)->get_name()
			 << " type="            << setw(20) << (*i)->get_type_str()
			 << " serialnums="      << setw(18) << (*i)->get_serialnums_str()
			 << "      "
			 << " checkedout="      << setw(2)  << (*i)->get_checkedout()
			 << " actuals="         << setw(2)  << (*i)->get_actuals()
			 << " duplicates="      << setw(2)  << (*i)->get_duplicates()
			 << " shared="          << setw(2)  << ((*i)->d_transactions ? (*i)->d_transactions->transcount()[Shared] : 0)
			 << " licensetype="     << setw(10) << (MGSG_MGLS_pid::contains_iterator(*(*i)).is_end() ? "atomic" : "composite")
			 << endl;
        for (MGSG_MGLS_pid::contains_iterator c(*(*i)); !c.is_end(); ++c) {
			cout << "// Debug:   Cont   " << setiosflags(std::ios::left)
				 << " pidnum="          << setw(6)  << (*c)->get_pid()
                 << " pidname="         << setw(16) << ((*c)->get_name() ? (*c)->get_name() : unknown_str)
				 << " type="            << setw(20) << (*c)->get_type_str()
				 << " serialnums="      << setw(18) << (*c)->get_serialnums_str()
				 << "      "
				 << " checkedout="      << setw(2)  << (*c)->get_checkedout()
				 << " actuals="         << setw(2)  << (*c)->get_actuals()
				 << " duplicates="      << setw(2)  << (*c)->get_duplicates()
				 << " shared="          << setw(2)  << ((*c)->d_transactions ? (*c)->d_transactions->transcount()[Shared] : 0)
				 << " allocated="       << setw(3)  << ((*c)->d_transactions && (*c)->d_transactions->transallocated() ? "yes" : "no")
				 << " alternate="       << setw(3)  << ((*c)->d_transactions && (*c)->d_transactions->alternateallocated() ? "yes" : "no")
				 << (MGSG_MGLS_pid::contains_iterator(*(*c)).is_end() ? "" : "licensetype=composite **")
				 << endl;
        }
    }
	cout.flush();
}
#endif

void MGSG_LicenseHandlerBase::TransactionPool::Node::set_contains(MGSG_LicenseMessages *messages,
                                                                  const int num_of_ids, const int *id_list)
{
  References *cur = 0;
  bool delete_messages = false;
  if (!messages && !delete_messages)
    delete_messages = !!(messages = new MGSG_LicenseMessages);
  PidPool *pidpool = get_pidpool(*(*real));
  PidPool::const_iterator p(pidpool);
  int *intArray = new int[num_of_ids];
  memcpy(intArray, id_list, num_of_ids*sizeof(int));
  std::sort(intArray, intArray + num_of_ids);
  id_list = intArray;
  for (int c = 0; c < num_of_ids && !p.is_end(); ++c) {
    int pids_pid = -1;
    while (!p.is_end() && (pids_pid = (*p).get_pid()) < id_list[c])
      ++p;
    if (pids_pid == id_list[c]) {
      // do not inline these intermediate variables MSVC6 has a code generation error
      PidPool::Reference pref(*pidpool, pids_pid);
      Reference *ref = new Reference(*messages, pref, Contained);
      cur = new References(messages, &contains, cur, ref, true);
      ++p;
    }
  }
  delete [] intArray;
  if (delete_messages)
    delete messages;
}

void MGSG_LicenseHandlerBase::TransactionPool::Node::set_contains(MGSG_LicenseMessages *messages)
{
  contains_set = true;
  const int actualpid = (*(*real)).get_pid();
  int num_of_ids, *id_list;
  if (core().Mheap_contains(actualpid, &num_of_ids, &id_list) == MGLS_OK
    && (num_of_ids > 1 || (num_of_ids == 1 && id_list[0] != actualpid)))
    set_contains(messages, num_of_ids, id_list); 
}

void MGSG_LicenseHandlerBase::TransactionPool::Node::set_contains(bool *pids_contained,
                                                                  const MGSG_MGLS_pid &pid)
{
    unsigned int pids_idx=0;
    MGSG_MGLS_pid::contains_iterator c(pid);
    for (MGSG_LicenseHandlerBase::pids_iterator pids_i(*get_handler(pid)); !pids_i.is_end();) {
        const int pids_pid = (*pids_i)->get_pid();
        const int c_pid = (c.is_end() ? pids_pid+1 : (*c)->get_pid());
        if (c_pid < pids_pid) {
            ++c;
        } else {
            if (c_pid == pids_pid)
                pids_contained[pids_idx] = true;
            ++pids_idx; ++pids_i;
        }
    }
}

void MGSG_LicenseHandlerBase::TransactionPool::Node::apply_actions(const TransactionPool &pool,
																   const MGSG_MGLS_pid &pid)
{
    struct pids_info_type {
        const unsigned int pids_sz;
        bool *pids_disabled;
        bool *pids_contained;
        pids_info_type(const MGSG_MGLS_pid &pid) 
            : pids_sz(MGSG_LicenseHandlerBase::pids_iterator::size(*MGSG_LicenseHandlerBase::get_handler(pid))),
            pids_disabled(new bool[pids_sz]),
            pids_contained(new bool[pids_sz])
        {
            memset(pids_disabled,0,sizeof(pids_disabled[0])*pids_sz);
            clear_contained();
        }
        ~pids_info_type() { delete [] pids_disabled; delete [] pids_contained; }
        void clear_contained() { memset(pids_contained,0,sizeof(pids_contained[0])*pids_sz); }
    } *pids_info=0;

    if (allocated) {
        // Gather "to-do" for action
        for (PidPool::Reference::actions_iterator a(*real,action_disable_flagged_uncontained_pids_on_checkout_success);
            !a.is_end(); ++a) 
        {
            if (!a.applied()) { 
                a.set_applied();
                if (!pids_info) {
                    pids_info = new pids_info_type(*real);
                    if (contains)
                        set_contains(pids_info->pids_contained, *real);
                    if (alternate) {
                        for (References *alt_c=alternate; alt_c; alt_c=alt_c->next)
                            if (const Reference *alt_actual = alt_c->transaction->d_node->actual)
                                set_contains(pids_info->pids_contained, *(alt_actual->pid()));
                    } else if (actual) {
                        set_contains(pids_info->pids_contained, *actual->pid());
                    }
                }
                const int pflags=a.arg0();
                unsigned int pids_idx=0;
                for (pids_iterator pids_i(*get_handler(pid)); !pids_i.is_end(); ++pids_i, ++pids_idx) {
                    if (!pids_info->pids_contained[pids_idx] && ((*pids_i)->get_flags() & pflags))
                        pids_info->pids_disabled[pids_idx] = true;
                }
            }
        }

        // Gather "to-do" for action
        for (PidPool::Reference::actions_iterator aa(*real,action_disable_flagged_uncontained_pids_in_composite_on_checkout_success);
            !aa.is_end(); ++aa)
        {
            if (!aa.applied()) { 
                aa.set_applied();
                if (!pids_info)
                    pids_info = new pids_info_type(*real);
                pids_info->clear_contained();

				PidPool::Reference composite_pid(*(get_pidpool(*(*real))), aa.arg1());
				MGSG_LicenseMessages messages;
				MGSG_LicenseHandlerBase::Transactions::reference(messages, composite_pid);
                set_contains(pids_info->pids_contained, composite_pid);
				MGSG_LicenseHandlerBase::Transactions::unreference(&messages, composite_pid);

                const int pflags=aa.arg0();
                unsigned int pids_idx=0;
                for (pids_iterator pids_i(*get_handler(pid)); !pids_i.is_end(); ++pids_i, ++pids_idx) {
                    if (!pids_info->pids_contained[pids_idx]) {
						if (((*pids_i)->get_flags() & pflags)) {
							pids_info->pids_disabled[pids_idx] = true;
						}
					}
                }
            }
        }
    } else {
        // Gather "to-do" for action
        for (PidPool::Reference::actions_iterator a(*real,action_disable_flagged_on_checkout_failure);
            !a.is_end(); ++a)
        {
            if (!a.applied()) { 
                a.set_applied();
                if (!pids_info)
                    pids_info = new pids_info_type(*real);
                const int pflags=a.arg0();
                unsigned int pids_idx=0;
                for (pids_iterator pids_i(*get_handler(pid)); !pids_i.is_end(); ++pids_i, ++pids_idx) {
                    if ((*pids_i)->get_flags() & pflags)
                        pids_info->pids_disabled[pids_idx] = true;
                }
            }
        }
    }

    // Apply disable from action info
    if (pids_info) {
        unsigned int pids_idx=0;
        for (pids_iterator pids_i(*get_handler(pid)); !pids_i.is_end(); ++pids_i, ++pids_idx) {
            if (pids_info->pids_disabled[pids_idx]) {
#             ifdef DEBUG_LICENSE
				cout << "// Debug: Action for"
					 << " pidname=" << real->get_name() 
					 << " pidnum=" << (*(*real)).get_pid()
					 << " DISABLE >>>"
					 << " pidname=" << (*pids_i)->get_name()
					 << " pidnum=" <<  (*pids_i)->get_pid()
					 << endl;
#             endif
                (*pids_i)->disable();
            }
        }
        delete pids_info;
        //DEBUG
		pool.print();
        //<<<<
    }
}

void MGSG_LicenseHandlerBase::TransactionPool::Node::unallocv(MGSG_LicenseMessages *messages)
{
    if (allocated) {
        if (transaction != MGLS_ERR) {
#         ifdef DEBUG_LICENSE
			cout << "// Debug: Core::Mheap_free(" << transaction << ") pidname=" << (*(*real)).get_name() << endl;
#         endif
			if (core().Mheap_free(transaction) == MGLS_ERR) {
				if (messages) {
                    messages->append(MGSG_LicenseMessages::MSG_WARN_LicenseCheckinFailed, core().Mheap_get_errno());
					messages->appendMheapErrs(core());
				}
			}
			transaction=MGLS_ERR;
        } else if (alternate) {
            delete alternate; // @@@ messages?
            alternate = false;
        }
        allocated=false;
        expiresoon=false;
        isactual=false;
        transtypecnt[licenseType] = MGSG_unused;
        serialnums.clear();
        if (actual) {
            if (should_invalidate_actual_transcount())
              actual_transcount()[serialNums].invalidate();
            delete actual;
            actual = 0;
        }

    }
}

void MGSG_LicenseHandlerBase::TransactionPool::Node::allocv(TransactionPool &pool,
                                                            MGSG_LicenseMessages &messages,
                                                            const transactionType transactiontype)
{
	const MGSG_MGLS_pid &pid = *(*real);
	if (!pid.is_disabled(messages) && !pid.is_expired(messages)) {
        if (!pid.is_disabled_primary(messages)) {
            sharedv(pool,messages);
            if (!allocated) // try to allocate real pid
                realv(pool, messages);
        }
		if (!allocated) // try alternate pids
			alternatev(pool, messages, transactiontype);
	}
    apply_actions(pool, *(*real));
}

void MGSG_LicenseHandlerBase::TransactionPool::Node::setv(MGSG_LicenseMessages &messages,
                                                          MGSG_License_Type lictype,
														  const SerialNums &licsums,
                                                          int actualpid)
{
    isactual = (*(*real)).get_pid() == actualpid;
    bool actual_contains = false;
    if (actualpid != -1) {
        PidPool *pidpool = get_pidpool(*(*real));
        PidPool::Reference apid(*pidpool, actualpid);
        actual = new ActualReference(messages, apid, None);
        ((Node *)actual->d_node)->isactual = true;
        if ( !actual->d_node->contains_set ) {
            if (!isactual)
                ((Node *)actual->d_node)->set_contains(&messages); // call to push any messages in return messages
            else
                ((Node *)actual->d_node)->contains_set=true; // no need to call as the actual is this atomic
        }
        actual_contains = !!actual->d_node->contains;

    }

    // store the license type
    if (transtypecnt[licenseType] < (unsigned int)lictype)
        transtypecnt[licenseType] = lictype;

  	// invalidate the serial numbers on the actual, it will be recalculated
  	serialnums.set(licsums);
    if (should_invalidate_actual_transcount())
      actual_transcount()[serialNums].invalidate();

    allocated=true;
}

void MGSG_LicenseHandlerBase::TransactionPool::Node::sharedv(TransactionPool &pool,
	                                                         MGSG_LicenseMessages &messages)
{
#if defined(_MSC_VER) || defined(LICENSE_USE_CLIM)
    // Check to see if an actual already satisfies this request. Work around the lack
    // of a cross child cache on Windows.  Without the cross child cache MGLS will not
    // consider licenses already consumed to satisfy the request.
    // 
    // CLIM needs this to share "granted" features from registered alternates
    //
    if (const MGSG_MGLS_pid *spid = pool.contained_allocated(*(*real))) {
#     ifdef DEBUG_LICENSE
		cout << "// Debug: Shared license checkout, pidname=" << spid->get_name()
			 << " can satisfy pidname=" << (*(*real)).get_name() << endl;
#     endif
        Reference *sref = new Reference(messages, *spid, Shared);
        if (sref->valid()) {
            alternate = new Alternate(messages, 0, 0, sref);
            setv(messages, spid->get_type(), 
				 spid->get_serialnums(), spid->get_actual_id());
        } else
            delete sref;
    }
#endif
}

void MGSG_LicenseHandlerBase::TransactionPool::Node::realv(TransactionPool &pool,
	                                                       MGSG_LicenseMessages &messages)
{
#ifdef DEBUG_LICENSE_STUB
	allocated = true;
	return;
#endif

	// Request the pid from the license server
	const int pidnum = (*(*real)).get_pid();
	transaction = pool.core().Mheap_allocv(pidnum, mgls_app_date);
	if (transaction == MGLS_ERR) {
		/* license request failed.  Find out why and return */
		const char *pidname = real->get_name();
        messages.append(MGSG_LicenseMessages::MSG_ERR_LicenseRequestFailed, pidname, core().Mheap_get_errno());
		messages.appendMheapErrs(core());
	} else {
		bool retry;
        bool licexpiresoon = false;
        bool licvalid = false;
		do {
			retry = false;
			// License request is pending.  Don't proceed until license server has responded.
			long stat = pool.core().Mheap_stat( transaction );
#ifdef DEBUG_LICENSE
			cout << "// Debug: Requested pidnum (" << pidnum 
				 << ") transaction-id (" << transaction << ")" 
				 << " status (" << stat << ")" << endl;
#endif
			switch( stat ) {
			case MGLS_EXP_WARNING:
				licexpiresoon = true;
			case MGLS_OK:
				licvalid = true;
				break;
			case MGLS_ERR: {
				const char *pidname = real->get_name();
				messages.append(MGSG_LicenseMessages::MSG_ERR_LicenseRequestFailed, pidname, core().Mheap_get_errno());
				messages.appendMheapErrs(core());
				break;
			}
			case MGLS_REQ_IN_QUEUE:
				messages.append(MGSG_LicenseMessages::MSG_WARN_QueuedLicense);
				sleep(10);
				retry = true;
				break;
			default:
				messages.append(MGSG_LicenseMessages::MSG_ERR_UnexpectedLicenseReqResponse, stat);
				break;
			}
		} while(retry);

		// If the license is invalid delete the transaction
		if (! licvalid) {
			if (core().Mheap_free(transaction) != 0) {
				messages.append(MGSG_LicenseMessages::MSG_WARN_LicenseCheckinFailed, core().Mheap_get_errno());
				messages.appendMheapErrs(core());
			}
			transaction = MGLS_ERR; // be sure to reset the transaction to "unallocated"
		} else {
			// Make successive calls to Core::Mheap_licinfo() to get license information
			// for the requested feature and any additional prerequisite features.
			int return_value;
			MGSG_License_Type lictype = MGSG_unknown;
			SerialNums licsnums;
			int actualpid = -1;
			do {
			    MGSG_License_Type ltype;
				MGLS_LICINFO license_info;
				return_value = core().Mheap_licinfo(&license_info);
				if (return_value == MGLS_ERR) {
					messages.appendMheapErrs(core());
					break;
				}
				switch (license_info.license_type) {
				case MGLS_nodelocked_uncounted:
					ltype = MGSG_nodelocked_uncounted;
					break;
				case MGLS_nodelocked_counted:
					ltype = MGSG_nodelocked_counted;
					break;
				case MGLS_floating:
					ltype = MGSG_floating;
					break;
				default:
					ltype = MGSG_unknown;
					break;
				}
				if (lictype < ltype)
				    lictype = ltype;
				licsnums.add(license_info.serial_number);
                if (actualpid == -1)
                  actualpid = license_info.granted_application_id;
			} while ( return_value > 0 );

            // store the license data and mark as a valid license
            setv(messages, lictype, licsnums, actualpid);

            // post and set the expire soon status
            if (licexpiresoon)
                set_expiresoon(messages);
        }
        //DEBUG
        if (MGSG_LicenseHandlerBase::PidPool *pidpool = get_pidpool(*(*real)))
            pidpool->print();
        pool.print();
        //<<<<
    }
}

void MGSG_LicenseHandlerBase::TransactionPool::Node::alternatev(TransactionPool &pool,
                                                                MGSG_LicenseMessages &messages,
                                                                const transactionType transactiontype)
{
    MGSG_LicenseMessages tmessages;
    for (PidPool::Reference::alternates_iterator i(*real); *i; ++i) {
        tmessages.clear();
        const PidPool::Reference *const *pidrefs = i.pidrefs();
        const unsigned int pids_size = pidrefs ? i.size() : 0;
        Alternate *transactions=0, *transactions_cur=0;
        for (unsigned int j=0; j < pids_size; ++j) {
            Reference *r = new Reference(tmessages, *(*pidrefs[j]), transactiontype);
            if (!r->valid()) { // license request failed
                delete transactions;
                transactions = 0;
                break;
            }
            transactions_cur = new Alternate(tmessages, &transactions, transactions_cur, r);
        }
        if (transactions) {
            messages.clear();
            messages.take(tmessages);

            std::vector<int> contains_pid;
            if (i.grants_num_of()) {
              for (unsigned int ii=0, iiend=i.grants_num_of(); ii < iiend; ++ii) {
                int id = i.grants()[ii], num_of_ids, *id_list;
                if (pool.core().Mheap_contains(id, &num_of_ids, &id_list) == MGLS_OK)
                  for (int iii = 0; iii < num_of_ids; ++iii)
                    contains_pid.push_back(id_list[iii]);
                else
                  contains_pid.push_back(id);
              }
            }
            if (i.size()) {
              for (unsigned int ii=0, iiend=i.size(); ii < iiend; ++ii)
                contains_pid.push_back((*i)[ii]);
            }
            if (contains_pid.size())
              set_contains(&messages, contains_pid.size(), &contains_pid[0]);

            alternate =  transactions;
            setv(messages, transactions->transaction->lictype(),
              transactions->transaction->serialnums(),
              transactions->transaction->actualpidnum());
            break;
        }
    }
}

bool MGSG_LicenseHandlerBase::TransactionPool::Node::should_invalidate_actual_transcount() const
{
  if (actual && actual->d_node->contains_set && !actual->d_node->contains)
    return false;
  return actual && actual->d_node != this;
}

MGSG_LicenseHandlerBase::TransactionPool::TransactionTypeCounts &
MGSG_LicenseHandlerBase::TransactionPool::Node::actual_transcount()
{
  return const_cast<Node *>(actual->d_node)->transtypecnt;
}

void MGSG_LicenseHandlerBase::TransactionPool::Node::set_expiresoon(MGSG_LicenseMessages &messages)
{
    const char *name=0;
    if (actual && actual->d_node != this) {
        if (!actual->expiresoon()) {
    	    ((Node *)actual->d_node)->expiresoon = true;
    	    name = actual->pidname();
    	}
    } else if (!expiresoon) {
        name = (*(*real)).get_name();
        expiresoon = true;
    }
    if (name) {
        messages.append(MGSG_LicenseMessages::MSG_INF_ExpireSoon1, name);
        messages.append(MGSG_LicenseMessages::MSG_INF_ExpireSoon2);
    }
}

void MGSG_LicenseHandlerBase::TransactionPool::Node::update_transcount(otherTransactionType transtype) const
{
	Node *t = (Node *)this;
	switch (transtype) {
	case serialNums:
    if (actual) {
      // push down
    } else if (const References *r=contains) {
      t->serialnums.clear();
      for (References *r=contains; r; r=r->next)
        t->serialnums.add(r->transaction->serialnums());
    }
    break;
  case licenseType:
    if (actual) {
      if (contains) {
        // @@@ this is it, push to any contains
      }
    } else if (const References *r=contains) {
      unsigned int tval = 0;
      tval = r->transaction->transcount()[transtype];
      for (r=r->next; r; r=r->next) {
        if (tval < r->transaction->transcount()[transtype])
          tval = r->transaction->transcount()[transtype];
      }
      t->transtypecnt[transtype] = tval;
    }
    break;
  }
}

void MGSG_LicenseHandlerBase::TransactionPool::Node::update_transcount(transactionType transtype) const
{
  Node *t = (Node *)this;
  if (actual) {
    // push down
  } else if (const References *r=contains) {
    unsigned int tval = 0;
    tval = r->transaction->transcount()[transtype];
    for (r=r->next; r; r=r->next) {
      if (tval < r->transaction->transcount()[transtype])
        tval = r->transaction->transcount()[transtype];
    }
  }
}

//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
  
const MGSG_LicenseHandlerBase::TransactionPool::Node *
MGSG_LicenseHandlerBase::TransactionPool::Alloc(MGSG_LicenseMessages &messages,
                                                const MGSG_MGLS_pid &pid, 
                                                const transactionType transactiontype)
{
  Node *c=d_pool, *l=0;
  const int idx = pid.get_pid(); // use the pidnum for the index (for now...)
  while (c && c->idx < idx) {
    l = c;
    c = c->next;
  }
  if (!c || c->idx != idx) {
    Node **nptr = (!l ? &d_pool : &l->next);
    *nptr = c = new Node(d_handler, idx, pid, *nptr);
    c->set_lictype(pid.is_disabled() ? MGSG_disabled : MGSG_unused);
  } // else: already have a node for this pidnum

  ++c->refcnt;

  ++c->transtypecnt[transactiontype];

  if (transactiontype == Actual 
    && c->transtypecnt[transactiontype] > c->transtypecnt[Checkedout]) {
      c->transtypecnt[Checkedout] = c->transtypecnt[transactiontype];
  } else if (transactiontype == Duplicate
    && !c->transtypecnt[Checkedout]) {
      c->transtypecnt[Checkedout] = 1;
  }

  if (!c->allocated && transactiontype != None && transactiontype != Contained) { // allocate a real license
    c->transtypecnt[serialNums].invalidate();
    c->allocv(*this, messages, transactiontype);
    if (!c->allocated) {
      Dealloc(&messages, c, transactiontype);
      c=0;
    }
  } else if (c->alternate && (transactiontype == Actual || transactiontype == Duplicate)) {
    c->alternate->alloc(messages, transactiontype);
  }

#ifdef DEBUG_LICENSE
  if (transactiontype == Actual || transactiontype == Duplicate) {
    cout << "// Debug: Checkout" << setiosflags(std::ios::left)
      << (transactiontype == Duplicate ? " Duplicate" : "")
      << " pidnum="         << setw(7)  << pid.get_pid()
      << " pidname="        << setw(16) << pid.get_name()
      << " checkedout="     << setw(2)  << pid.get_checkedout()
      << " actuals="        << setw(2)  << pid.get_actuals()
      << " duplicates="     << setw(2)  << pid.get_duplicates()
      << " serialnums="     << setw(10) << pid.get_serialnums_str()
      << " lictype="        << pid.get_type_str()
      << (!c ? " ***FAILED***" :"")
      << endl;
  }
#endif
  return c;
}

void MGSG_LicenseHandlerBase::TransactionPool::Dealloc(MGSG_LicenseMessages *messages,
                                                       const Node *node,
                                                       const transactionType transactiontype) 
{

  for (Node *c=d_pool, *l=0; c; l=c, c=c->next) {
    if (c == node) {
      bool last_invalid=false;
      --c->transtypecnt[transactiontype];

      if (transactiontype == Checkedout || transactiontype == Shared)
        c->transtypecnt[serialNums].invalidate();

      // Adjust the checkedout count, min actuals, or one if duplicates
      if (((*(*c->real)).get_flags() & MGSG_LicenseHandlerBase::flag_pidCheckin)) {
        if (transactiontype == Shared) {
          if (!c->transtypecnt[Shared] && !c->transtypecnt[Checkedout]) {
            if (c->should_invalidate_actual_transcount())
              c->actual_transcount()[Shared].invalidate();
            c->unallocv(messages);
            last_invalid=true; // unallocv may corrupt l
          }
        } else {
          if (c->alternate && (transactiontype == Actual || transactiontype == Duplicate))
            c->alternate->unalloc(messages, transactiontype);
          const unsigned int checkedout_ref = c->transtypecnt[Checkedout];
          if (transactiontype == Actual 
            && c->transtypecnt[transactiontype] < c->transtypecnt[Checkedout]) {
              if (!c->transtypecnt[transactiontype] && c->transtypecnt[Duplicate])
                c->transtypecnt[Checkedout] = 1;
              else
                c->transtypecnt[Checkedout] = c->transtypecnt[transactiontype];
          } else if (transactiontype == Duplicate
            && !c->transtypecnt[transactiontype] && !c->transtypecnt[Actual]) {
              c->transtypecnt[Checkedout] = 0;
          }
          if (checkedout_ref != c->transtypecnt[Checkedout]) {
            if (c->should_invalidate_actual_transcount())
              c->actual_transcount()[Checkedout].invalidate();
            if (!c->transtypecnt[Checkedout]) {
              c->transtypecnt[licenseType] = (*(*c->real)).is_disabled() ? MGSG_disabled : MGSG_unused;
            }
          }
          if (c->allocated && !c->transtypecnt[Checkedout] && !c->transtypecnt[Shared]) {
            c->unallocv(messages);
            last_invalid=true; // unallocv may corrupt l
          }
        }
      }
      if (--c->refcnt == 0) { // remove it from the pool
        if (last_invalid) { // find the last pointer again
          if (c==d_pool)
            l=0;
          else {
            l=d_pool;
            while(l->next && l->next != c)
              l=l->next;
          }
        }
        if (!l)
          d_pool = c->next;
        else
          l->next = c->next;
        delete c;
      }
      break;
    }
  }
}

//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////

/*==========================================================================
- ============================== PIDS ITERATOR =============================
- ========================================================================== */

struct MGSG_LicenseHandlerBase::pids_iterator::data {
    MGSG_MGLS_pids &d_pids;
    MGSG_MGLS_pids::const_iterator d_itr;
    data(MGSG_MGLS_pids &pids) : d_pids(pids), d_itr(d_pids.begin()) { }
};

void MGSG_LicenseHandlerBase::pids_iterator::_next()
{
    ++d_data->d_itr;
    if (d_data->d_itr == d_data->d_pids.end()) {
		delete d_data;
		d_data = 0;
	}
}

MGSG_MGLS_pid* MGSG_LicenseHandlerBase::pids_iterator::_get() const 
{
    return d_data ? *d_data->d_itr : 0;
}

MGSG_LicenseHandlerBase::pids_iterator::pids_iterator(const MGSG_LicenseHandlerBase &handler)
	: d_data(new data(*handler.d_pids))
{
    if (d_data->d_itr == d_data->d_pids.end()) {
		delete d_data;
		d_data = 0;
	}
}

MGSG_LicenseHandlerBase::pids_iterator::~pids_iterator()
{
	delete d_data;
}

unsigned int MGSG_LicenseHandlerBase::pids_iterator::size(MGSG_LicenseHandlerBase &handler)
{
    return static_cast<unsigned int>(handler.d_pids->size());
} 

/*==========================================================================
- ============================ ACTUALS ITERATOR ============================
- ========================================================================== */

struct MGSG_LicenseHandlerBase::actuals_iterator::data {
	const MGSG_LicenseHandlerBase::TransactionPool::Node *cur;
};

bool MGSG_LicenseHandlerBase::actuals_iterator::_is_actual() const
{
  return d_data->cur->contains 
    || (d_data->cur->isactual && !d_data->cur->transtypecnt[TransactionPool::Contained]); 
}

void MGSG_LicenseHandlerBase::actuals_iterator::_next()
{
	do {
		d_data->cur = d_data->cur->next;
	} while (d_data->cur && !_is_actual());
	if (!d_data->cur) {
		delete d_data;
		d_data = 0;
	}
}

const MGSG_MGLS_pid* MGSG_LicenseHandlerBase::actuals_iterator::_get() const 
{
	return &(*(*d_data->cur->real));
}

MGSG_LicenseHandlerBase::actuals_iterator::actuals_iterator(const MGSG_LicenseHandlerBase &handler)
	: d_data(new data)
{
	d_data->cur = handler.d_transpool->d_pool;
	if (!d_data->cur) {
	    delete d_data;
	    d_data = 0;
	} else if (!_is_actual())
		_next(); // not a toplevel trans, find the next toplevel trans
}

MGSG_LicenseHandlerBase::actuals_iterator::~actuals_iterator()
{
	delete d_data;
}

//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////

/*==========================================================================
- ================================== PID ===================================
- ========================================================================== */

//////////////////////////////////////////////////////////////////////////////
// Class Definition : This class manages the expiration of a feature via
//                    a hard encoded time value
//////////////////////////////////////////////////////////////////////////////

class MGSG_MGLS_pid::Texpire {
	const MGSG_MGLS_pid   *d_pid;
	unsigned int           d_expire_secs;       // time (in seconds) when feature expires
	unsigned int           d_expire_secs_;      // time (in seconds) when feature expires (real)
	const int              d_warn_days;         // number of days to display warning
	const MGSG_MGLS_pid   *d_unexpire_pid;      // pid unsed to unexpire a hard expire

	bool is_i(MGSG_LicenseMessages *messages) const
	{
		bool cache_result = !!messages;

		// Check for software expiration (hard expiration date for a feature)
		time_t currentTime = time(0);
		if (currentTime > (time_t)d_expire_secs) {
			MGSG_LicenseMessages messages_tmp, &alloc_messages = (messages ? *messages : messages_tmp);
			if (d_unexpire_pid && d_unexpire_pid->allocate_return(alloc_messages, true)) {
				// If we can checkout this alternative feature we will ignore this expiration
				if (cache_result) {
					Texpire *cached_val = (Texpire *)this;
					cached_val->d_expire_secs = 0;
				}
				return false;
			}
			if (messages)
				messages->append(MGSG_LicenseMessages::MSG_URG_BetaFeatureExpired, d_pid->get_name());
			if (cache_result) {
				Texpire *cached_val = (Texpire *)this;
				cached_val->d_expire_secs = 1;
			}
			return true;
		}


		if (messages) {
			int expire_in_days = (int)((((d_expire_secs - currentTime) / 60) / 60) / 24);
			if (d_warn_days < 0 || (expire_in_days < d_warn_days)) {
				messages->append(MGSG_LicenseMessages::MSG_INF_BetaFeatureExpiration, d_pid->get_name(), expire_in_days, 
								 ((expire_in_days == 1) ? "" : "s"));
			}
		}

		if (cache_result) {
			Texpire *cached_val = (Texpire *)this;
			cached_val->d_expire_secs = 0;
		}

		return false;
	}

public:
	Texpire(const MGSG_MGLS_pid *pid, unsigned int expire_secs, int warn_days,
			const MGSG_MGLS_pid *unexpire_pid)
		: d_pid(pid), d_expire_secs(expire_secs), 
		  d_warn_days(warn_days), d_unexpire_pid(unexpire_pid) {
		if (! d_expire_secs)
			d_expire_secs = 1;  // forced expiration
		d_expire_secs_ = d_expire_secs;
	}
	~Texpire() { }
	bool is(MGSG_LicenseMessages *messages=0) const
	{
		if (d_expire_secs == 0) return false;
		if (d_expire_secs == 1) return true;
		return is_i(messages);
	}
	unsigned int get_seconds() const { return d_expire_secs_; }
	const MGSG_MGLS_pid *get_unexpire_pid() const { return d_unexpire_pid; }
};

//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////

/*==========================================================================
- ================================== PID  ==================================
- ========================================================================== */

//////////////////////////////////////////////////////////////////////////////
// Class Definition : This class defines a license to be managed
//////////////////////////////////////////////////////////////////////////////

class MGSG_MGLS_pid::CheckoutFailedCache
{
    const time_t d_time_created;
    const MGSG_LicenseMessages d_messages;
public:
    CheckoutFailedCache(const MGSG_LicenseMessages &messages)
        : d_time_created(time(0)), d_messages(messages)
    { }
    bool valid() const { 
        const time_t cur_time = time(0);
        return d_time_created<=cur_time && d_time_created<(cur_time+(4*60)); 
    }
    const MGSG_LicenseMessages &messages() const { return d_messages; }
};

MGSG_MGLS_pid::~MGSG_MGLS_pid()
{ 
	if (d_handler) 
		d_handler->UnRegister_pid(*this); 
	delete d_transactions;
	delete d_valid_msgs;
	delete d_expire;
	delete d_registration;
	delete d_checkout_failed;
}

bool MGSG_MGLS_pid::Register(MGSG_LicenseHandlerBase *handler, 
							 int pid, int flags,
							 unsigned int expire_secs, int warn_days, const MGSG_MGLS_pid *unexpire,
							 int alternate_pid)
{
	if (d_handler || (unexpire && unexpire->d_handler != handler))
		return false;
	d_handler = handler;

	d_pidnum = pid;
    d_flags = flags;
			
	d_flags = flags;
	d_valid_set = false;

	bool enable_warn = handler->d_debug;

#   ifdef DEBUG_LICENSE_STUB
	enable_warn = true;
	bool get_cts_success = false;
	expire_secs = get_compile_time_secs(&get_cts_success);
	if (get_cts_success)
		expire_secs += (60 * 60 * 24 * 3); // Forced 3 day expiration for STUB builds
#   endif
	
	if (expire_secs)
		d_expire = new Texpire(this, expire_secs, (enable_warn ? -1 : warn_days), unexpire);

	d_registration = new MGSG_LicenseHandlerBase::PidPool_Reference(*this);

    if (alternate_pid > 0)
        d_handler->AddAlternate(*this, 1, &alternate_pid);

    return true;
}

bool MGSG_MGLS_pid::UnRegister(MGSG_LicenseHandlerBase *handler)
{
	if (d_handler != handler)
		return false;

  if (d_handler->d_pids) {
    MGSG_MGLS_pids &pids = *d_handler->d_pids;
    MGSG_MGLS_pids::iterator it = std::find(pids.begin(), pids.end(), this);
    if (it != pids.end())
      pids.erase(it);
  }

	delete d_transactions; d_transactions = 0;
	delete d_registration; d_registration = 0;
	delete d_expire;       d_expire = 0;
	d_pidnum = 0;
	d_pidname = 0;
	d_flags = 0;
	d_handler = 0;
	return true;
}

const char *MGSG_MGLS_pid::get_name() const
{ 
 	if (!d_pidname && d_handler)
 		((MGSG_MGLS_pid *)this)->d_pidname = d_handler->pvt_lookup(d_pidnum, 0, false);
	return d_pidname;
}

bool MGSG_MGLS_pid::is_library_id() const 
{ 
	return !!(d_flags & d_handler->flag_pidLibrary);
}

bool MGSG_MGLS_pid::is_disabled() const
{
	return !!(d_flags & d_handler->flag_pidDisable);
}

bool MGSG_MGLS_pid::is_disabled_primary() const
{
	return !!(d_flags & d_handler->flag_pidDisableP);
}

void MGSG_MGLS_pid::disable() 
{
	d_flags |= (d_handler->flag_pidDisable|d_handler->flag_pidCheckin);
    if (d_transactions)
        d_transactions->set_lictype(MGSG_disabled);
}

void MGSG_MGLS_pid::disable_primary() 
{
	d_flags |= (d_handler->flag_pidDisableP);
}

bool MGSG_MGLS_pid::hard_expired(MGSG_LicenseMessages *messages) const
{
	return d_expire ? d_expire->is(messages) : false;
}

bool MGSG_MGLS_pid::get_hard_expire(std::string *str) const
{
	str->erase();

	if (! d_expire)
		return false;

	time_t conv_time = (time_t)d_expire->get_seconds();

	const char *ascstr = asctime(localtime(&conv_time));
	str->assign(ascstr, strlen(ascstr)-1);

	return true;
}

bool MGSG_MGLS_pid::get_hard_unexpire(std::string *str) const
{
	str->erase();

	if (! d_expire)
		return false;

	const MGSG_MGLS_pid *pid = d_expire->get_unexpire_pid();

	if (!pid)
		return false;

	return pid->get_name(str);
}

bool MGSG_MGLS_pid::is_disabled(MGSG_LicenseMessages *messages, int disabled_flag) const
{
    const int mflag = d_handler ? d_handler->flag_pidDisable|d_handler->flag_pidDisableP : 0;
	if (messages) {
		if (!d_handler)
			messages->append(MGSG_LicenseMessages::MSG_ERR_LicenseNotRegistered);
		else if (is_disabled()) {
			messages->append(MGSG_LicenseMessages::MSG_ERR_DisabledLicenseRequestPID, get_name());
			messages->append(MGSG_LicenseMessages::MSG_ERR_DisabledLicenseRequestName, get_name());
		}
	}
	return !mflag || (d_flags & (mflag & disabled_flag));
}

bool MGSG_MGLS_pid::is_valid(MGSG_LicenseMessages *messages, bool recheck) const
{ 
	if (is_disabled(*messages))
		return false;

	if (d_transactions)
		return true;

	SetDefaults();  // ensure server is configured properly

	// cache value by contacting server or check hard expiration
	if (!d_valid_set || recheck) {
		MGSG_MGLS_pid *cached_val = (MGSG_MGLS_pid *)this;
		delete cached_val->d_valid_msgs;
		cached_val->d_valid_msgs = new MGSG_LicenseMessages;
		if (hard_expired(cached_val->d_valid_msgs))
			cached_val->d_valid = false;
		else if ( d_handler->d_transpool->contained_allocated(*this) ) {
			// use handler valid to check if an atomic is part of a 
			// checked out composite. Works around PCLS issue where
			// it does not know about the cross child cache, and
			// will ask the server, potentially finding a different
			// composite, instead of checking one that has already
			// been checked out by a different licensing call.
			cached_val->d_valid = true;
		} else {
#ifdef DEBUG_LICENSE_STUB
			cached_val->d_valid = true;
#else
			cached_val->d_valid = (core().Mheap_validv(get_pid(), mgls_app_date) != -1);
#endif

			if (!cached_val->d_valid) {
			    MGSG_LicenseHandlerBase::PidPool *ppool = MGSG_LicenseHandlerBase::get_pidpool(*this);
			    MGSG_LicenseHandlerBase::PidPool::Reference r(*ppool, get_pid());
			    for (MGSG_LicenseHandlerBase::PidPool::Reference::alternates_iterator i(r); *i; ++i) {
			        const MGSG_LicenseHandlerBase::PidPool::Reference *const *pidrefs = i.pidrefs();
			        const unsigned int pids_size = pidrefs ? i.size() : 0;
			        MGSG_LicenseMessages tmessages;
			        bool avalid = pids_size > 0;
			        for (unsigned int j=0; j < pids_size; ++j) {
			            const MGSG_LicenseHandlerBase::PidPool::Reference &jpid = *(*pidrefs[j]);
			            if (!(*jpid).is_valid(&tmessages, recheck)) {
			                avalid = false;
			                break;
			            }
			        }
			        tmessages.clear();
			        if (avalid)
			            cached_val->d_valid = true;
			    }
			}
			if (!cached_val->d_valid) {
				cached_val->d_valid_msgs->append(MGSG_LicenseMessages::MSG_ERR_license_is_not_valid, get_name());
				cached_val->d_valid_msgs->appendMheapErrs(core());
			}
		}
		cached_val->d_valid_set = true;
	}

	if (messages && d_valid_msgs)
		messages->append(*d_valid_msgs);

	return d_valid;
}

unsigned int MGSG_MGLS_pid::get_actuals() const 
{ 
 	return d_transactions ? d_transactions->actuals() : 0;
}
  
unsigned int MGSG_MGLS_pid::get_duplicates() const 
{
 	return d_transactions ? d_transactions->duplicates() : 0;
}

unsigned int MGSG_MGLS_pid::get_checkedout() const
{
 	return d_transactions ? d_transactions->checkedout() : 0;
}

MGSG_License_Type MGSG_MGLS_pid::get_type() const
{
	return d_transactions ? d_transactions->lictype() 
		: (is_disabled() ? MGSG_disabled : MGSG_unused);
}

const char *MGSG_MGLS_pid::get_type_str() const
{
	return ::get_type_str(get_type());
}

const MGSG_LicenseHandlerBase::SerialNums &MGSG_MGLS_pid::get_serialnums() const
{
	return d_transactions ? d_transactions->serialnums() : s_nullSerialNums;
}

const char *MGSG_MGLS_pid::get_serialnums_str() const
{
	return get_serialnums().c_str();
}

int MGSG_MGLS_pid::get_actual_id() const
{
 	return d_transactions ? d_transactions->actualpidnum() : -1;
}

bool MGSG_MGLS_pid::is_active() const
{
 	return d_transactions ? !!d_transactions->checkedout() : false;
}

bool MGSG_MGLS_pid::is_actual_special_id() const
{
 	return is_special_pid(get_actual_id());
}

bool  MGSG_MGLS_pid::is_actual_an_alternate() const
{
 	return d_transactions ? d_transactions->alternateallocated() : false;
}

const char *MGSG_MGLS_pid::get_actual_name() const
{
	return d_transactions ? d_transactions->actualpidname() : unknown_str;
}

bool MGSG_MGLS_pid::checkout(MGSG_LicenseMessages &messages, bool duplicate)
{
	SetDefaults();  // ensure server is configured properly
	bool res = false;
	if (d_checkout_failed) {
	    if (d_checkout_failed->valid()) {
	        messages.append(d_checkout_failed->messages());
	    } else {
	        delete d_checkout_failed; 
	        d_checkout_failed = 0;
	    }
	}
	if (! d_checkout_failed) {
	    res = MGSG_LicenseHandlerBase::Transactions::checkout(messages, duplicate, *this);
	    if (!res)
	        d_checkout_failed = new CheckoutFailedCache(messages);
	}
	return res;
	
}

bool MGSG_MGLS_pid::checkin(MGSG_LicenseMessages &messages, bool duplicate)
{
	return MGSG_LicenseHandlerBase::Transactions::checkin(messages, duplicate, *this);
}

bool MGSG_MGLS_pid::allocate_one(MGSG_LicenseMessages &messages) 
{
	if (!is_disabled() && get_actuals() > 0)
		return true;
	return this->checkout(messages, false);
}

bool MGSG_MGLS_pid::allocate_return(MGSG_LicenseMessages &messages, bool duplicate) const
{
	MGSG_MGLS_pid *cpid = (MGSG_MGLS_pid *)this; // used in a const manner, it undoes the change.
	if (! cpid->checkout(messages, duplicate))
		return false;
	return cpid->checkin(messages, duplicate);
}

bool MGSG_MGLS_pid::CLIM_hash(STDCOLON string *feature_hash, unsigned int valid_secs) const
{
  return d_registration && d_registration->CLIM_hash(feature_hash, valid_secs);
}

unsigned int MGSG_MGLS_pids::add(MGSG_MGLS_pid &pid)
{
	bool inserted = false; // insert sorted by pid
	unsigned int pos = 0;
	for (MGSG_MGLS_pids::iterator p = begin(); !(p == end()); ++p, ++pos ) {
		if ((*p) == &pid) {
			inserted = true;
			break; // already present
		} else if ((*p)->get_pid() > pid.get_pid()) {
			insert(p, &pid);
			inserted = true;
			break;
		}
	}
	if (!inserted)
		push_back(&pid); 
	return pos;
}

//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
// Class PID to Name mapping
//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////

class MGSG_LicenseHandlerBase::PidToName {
	struct Entry {
		Entry *next;
		const int pid;
		const STDCOLON string name;
		Entry(const int _pid, const char *_name)
			: next(0), pid(_pid), name(_name) { }
	};
	Entry *d_head;
public:
	PidToName() : d_head(0) { }
	~PidToName();
	const char *find(const int pid) const;
	const char *add(const int pid, const char *name);
};

MGSG_LicenseHandlerBase::PidToName::~PidToName() { 
	while(d_head) {
		Entry *i = d_head;
		d_head = i->next;
		delete i;
	}
}

const char *MGSG_LicenseHandlerBase::PidToName::find(const int pid) const 
{
	for (Entry *i = d_head; i; i = i->next) {
		if (i->pid == pid)
			return i->name.c_str();
	}
	return 0;
}

const char *MGSG_LicenseHandlerBase::PidToName::add(const int pid, const char *name)
{
	if (!d_head) {
		d_head = new Entry(pid, name);
		return d_head->name.c_str();
	}
	Entry *l = 0;
	for (Entry *i = d_head; i; i = i->next) {
		const bool name_eq = !i->name.compare(name);
		if (name_eq || i->pid == pid)
			return (name_eq && i->pid == pid) ?  i->name.c_str() : 0;
		l = i;
	}
	l->next = new Entry(pid, name);
	return l->next->name.c_str();
}

//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////////
// Class Definition : The purpose of this class is to be able to generate
//                    a scrambled hash string which can be passed to 
//                    immediate children.  The child can then use this hash
//                    to configure the exact display setting as the parent.
//                    This will allow the child to share the licenses held
//                    by the parent via duplicate grouping.
//////////////////////////////////////////////////////////////////////////////

class MGSG_LicenseHandlerBase::DisplayHash {
    // Scramble tables
    static const unsigned int s_tab_move[];
    static const char s_tab_fill[];
    static const unsigned int s_tab_sz;

    static void memcpy_incr(char **data_p, const void *data, unsigned int data_sz)
    {
        if (data_sz) {
            memcpy(*data_p, data, data_sz);
            *data_p += data_sz;
        }
    }

    static bool memcpy_incr(void *data, const char **data_p, const char *data_e, unsigned int data_sz)
    {
        if (!data_sz)
            return true;
        if ((*data_p + data_sz) > data_e)
            return false;
        memcpy(data, *data_p, data_sz);
        *data_p += data_sz;
        return true;
    }

    char         *d_data;
    unsigned int  d_checksum;

    char         *d_hostname;
    unsigned int  d_hostname_len;

    char         *d_str;
    unsigned int  d_str_len;

    int           d_pid;

    /* not fully typesafe, but good enough for now while time can be a 32-bit number 
       avoid 32bit -vs- 64bit platform issue */
    typedef unsigned int time32_t;
    time32_t      d_time_s;
    time32_t      d_time_e;
    inline void time32(time32_t *v) {
      union {
        time_t   tset;
        time32_t tret;
      } tval;
      if (time(&tval.tset) == -1)
        tval.tset = 0;
      *v = tval.tret;
    }

    void delete_vars() {
        d_checksum = 0;
        if (d_data) {
            delete [] d_data;
            d_data = 0;
        }
        if (d_hostname) {
            delete [] d_hostname;
            d_hostname = 0;
            d_hostname_len = 0;
        }
        if (d_str) {
            delete [] d_str;
            d_str = 0;
            d_str_len = 0;
        }
        d_pid = 0;
    }

public:
    DisplayHash() : d_data(0), d_checksum(0), d_hostname(0), d_str(0), d_str_len(0), d_pid(0) { }
    DisplayHash(const char *str) : d_data(0), d_checksum(0), d_hostname(0), d_str(0), d_str_len(0), d_pid(0) { Set(str); }
    ~DisplayHash() { delete_vars(); }

    const char *Get() const { return d_data; }
    const char *GetStr(const char *hostname, int cpid) const { 
		(void)cpid;
		return (d_data 
#if 0
				&& cpid == d_pid // @@@ ignore for now, need to check ancestors
#endif
				&& !strcmp(hostname, d_hostname)) ? d_str : 0;
	}
    bool Set(const char *str);

    const char *Generate(const char *hostname, int cpid, const char *str,
						 unsigned int valid_secs);

    bool IsValid() const { return !!d_data; }
    bool IsEqual(const DisplayHash &obj) const {
        return d_pid == obj.d_pid
            && d_checksum == obj.d_checksum
            && d_data && obj.d_data && !strcmp(d_data, obj.d_data)
            && d_hostname && obj.d_hostname && !strcmp(d_hostname, obj.d_hostname);
    }
};

const unsigned int MGSG_LicenseHandlerBase::DisplayHash::s_tab_move[] = 
	{ 21, 13, 2, 18, 15, 6, 24, 11, 8, 16, 28, 3, 31, 
	  1, 7, 27, 20, 29, 14, 4, 9, 12, 30, 19, 25, 23, 
	  5, 17, 22, 0, 10, 26 };

const char MGSG_LicenseHandlerBase::DisplayHash::s_tab_fill[] = 
	{ 'T', '%', 'G', 'O', 'W', 'L', 'Y', 'P', 'D', '=', 'S', 
	  'M', 'L', '%', '-', 'K', '~', 'X', '\'', '<', '|', '1', 
	  'V', 'I', 'W', 'J', '^', 'Q', '+', 'U', '*', 'C' };

const unsigned int MGSG_LicenseHandlerBase::DisplayHash::s_tab_sz = sizeof(s_tab_move) / sizeof(s_tab_move[0]);

const char *MGSG_LicenseHandlerBase::DisplayHash::Generate(const char *hostname, int cpid, const char *str,
														   unsigned int valid_secs)
{
    delete_vars();

    d_hostname_len = static_cast<unsigned int>(strlen(hostname));
    d_hostname = new char [d_hostname_len+1];
    strcpy(d_hostname, hostname);

    if (str) {
        d_str_len = static_cast<unsigned int>(strlen(str));
        d_str = new char [d_str_len+1];
        strcpy(d_str, str);
    }
        
    d_pid = cpid;

	if (valid_secs > max_valid_secs)
		valid_secs = max_valid_secs;

    time32(&d_time_s);
    d_time_e = d_time_s + valid_secs;

    // Format hostname_len(i)time_s(i)hostname(a$hostname_len)pid(i)time_e(i)str_len(i)str(a$str_len)checksum(i)
    const unsigned int key_sz = sizeof(d_hostname_len) + sizeof(d_time_s) + d_hostname_len 
        + sizeof(d_pid) + sizeof(d_time_e) + sizeof(d_str_len) + d_str_len + sizeof(d_checksum);
    
    unsigned int key_pad_sz = 3 - (key_sz % 3);
    if (key_pad_sz == 3)
        key_pad_sz = 0;

    const unsigned int r_sz = ((key_sz / 3) + !!key_pad_sz) * 4;
    const unsigned int rmtab_sz = (r_sz % s_tab_sz);
    const unsigned int pad_sz = rmtab_sz ? s_tab_sz - rmtab_sz : 0;

    const unsigned int total_sz  = r_sz + pad_sz + 1;

    d_data = new char [total_sz];

    char *data_s = d_data;
    char *data_w = data_s + total_sz - key_sz - key_pad_sz;
    char *data_p = data_w;

    memcpy_incr(&data_p, (void *)&d_hostname_len, sizeof(d_hostname_len));
    memcpy_incr(&data_p, (void *)&d_time_s, sizeof(d_time_s));
    memcpy_incr(&data_p, (void *)d_hostname, d_hostname_len);
    memcpy_incr(&data_p, (void *)&d_pid, sizeof(d_pid));
    memcpy_incr(&data_p, (void *)&d_time_e, sizeof(d_time_e));
    memcpy_incr(&data_p, (void *)&d_str_len, sizeof(d_str_len));
    memcpy_incr(&data_p, (void *)d_str, d_str_len);

    for (char *data_i = data_w; data_i < data_p; ++data_i)
        d_checksum += (*data_i);

    memcpy_incr(&data_p, (void *)&d_checksum, sizeof(d_checksum));

    memcpy_incr(&data_p, (void *)"\0\0", key_pad_sz); // null pad

    char *data_wp = d_data;
    for (const char *data_e = data_w; data_e < data_p; ++data_wp, ++data_e) {
        const char c1 = *data_e;
        const char c2 = *++data_e;
        const char c3 = *++data_e;

        char c = c1 >> 2;
        *data_wp = (c != 0) ? ((c & 0x3f) + 0x20) : 0x60;
        
        c = ((c1 << 4) & 060) | ((c2 >> 4) & 017);
        *++data_wp = (c != 0) ? ((c & 0x3f) + 0x20) : 0x60;

        c = ((c2 << 2) & 074) | ((c3 >> 6) & 003);
        *++data_wp = (c != 0) ? ((c & 0x3f) + 0x20) : 0x60;

        c = (c3 & 077);
        *++data_wp = (c != 0) ? ((c & 0x3f) + 0x20) : 0x60;
    }

    // Pad until full size
    if (pad_sz) {
        for (unsigned int pad_i = pad_sz; pad_i > 0; --pad_i) {
            memcpy_incr(&data_wp, (void *)&s_tab_fill[pad_i], 1);
        }
    }

    *data_wp = '\0';

    // Move data, scramble encoding
    for (unsigned int r_pos = 0; r_pos < (total_sz-1); r_pos += s_tab_sz) {
        for (unsigned int m = 1; m < s_tab_sz; m += 2) {
            const unsigned int p1 = s_tab_move[m-1] + r_pos;
            const unsigned int p2 = s_tab_move[m] + r_pos;
            const char c1 = d_data[p1];
            const char c2 = d_data[p2];
            d_data[p1] = c2;
            d_data[p2] = c1;
        }
    }

    // Make sure we can decode the hash
    if (!DisplayHash(d_data).IsEqual(*this))
        delete_vars();

    return d_data;
}

bool MGSG_LicenseHandlerBase::DisplayHash::Set(const char *str)
{
    delete_vars();

    // Check for invalid string length
    unsigned int s_sz = static_cast<unsigned int>(strlen(str));
    if (!s_sz || (s_sz % s_tab_sz))
        return false;

    d_data = new char[s_sz+1];
    strcpy(d_data, str);

    // Move data, unscramble encoding
    for (int r_pos = s_sz - s_tab_sz; r_pos >= 0; r_pos -= s_tab_sz) {
        for (int m = s_tab_sz - 2; m >= 0; m -= 2) {
            const unsigned int p1 = s_tab_move[m + 1] + r_pos;
            const unsigned int p2 = s_tab_move[m] + r_pos;
            const char c1 = d_data[p1];
            const char c2 = d_data[p2];
            d_data[p1] = c2;
            d_data[p2] = c1;
        }
    }

    // Unencode data
    const char *data_p = d_data + s_sz;
    char *data_wp = d_data;
    for (const char *data_e = d_data; data_e < data_p; ++data_wp, ++data_e) {
        const char c0 = *data_e;
        const char c1 = *++data_e;
        const char c2 = *++data_e;
        const char c3 = *++data_e;

        *data_wp = ((((c0-0x20)&0x3F) << 2) & 0xFF) | ((((c1-0x20)&0x3F) >> 4) & 0xFF);
        *++data_wp = ((((c1-0x20)&0x3F) << 4) & 0xFF) | ((((c2-0x20)&0x3F) >> 2) & 0xFF);
        *++data_wp = ((((c2-0x20)&0x3F) << 6) & 0xFF) | (((c3-0x20)&0x3F) & 0xFF);
    }

    // Need to parse checksum, hostname_len and then we should know how to read the rest
    bool valid = false;
    data_p = d_data;
    if (memcpy_incr((void *)&d_hostname_len, &data_p, data_wp, sizeof(d_hostname_len))
        && memcpy_incr((void *)&d_time_s, &data_p, data_wp, sizeof(d_time_s))) {

        d_hostname = new char [d_hostname_len+1];

        if (memcpy_incr((void *)d_hostname, &data_p, data_wp, d_hostname_len)
            && memcpy_incr((void *)&d_pid, &data_p, data_wp, sizeof(d_pid))
            && memcpy_incr((void *)&d_time_e, &data_p, data_wp, sizeof(d_time_e))
            && memcpy_incr((void *)&d_str_len, &data_p, data_wp, sizeof(d_str_len))) {
            
            if (d_str_len)
                d_str = new char [d_str_len+1];

            if (memcpy_incr((void *)d_str, &data_p, data_wp, d_str_len)) {
                unsigned int checksum = 0;
                for (char *data_i = d_data; data_i < data_p; ++data_i)
                    checksum += (*data_i);

                if (memcpy_incr((void *)&d_checksum, &data_p, data_wp, sizeof(d_checksum))) {
                    if (checksum == d_checksum) {
                        time32_t time_s, time_e;
                        time32(&time_s);
                        time_e = time_s + max_valid_secs; // maximum key duration
                        
                        if (d_time_s < d_time_e
                            && d_time_s <= time_s
                            && d_time_e >= time_s
                            && d_time_e <= time_e) {
                            d_hostname[d_hostname_len] = '\0';
							if (d_str)
								d_str[d_str_len] = '\0';
                            valid = true;
                        }
					}
                }
			}
        }
    }

    if (valid)
        strcpy(d_data, str);
    else
        delete_vars();

    return !!d_data;
}

//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////////
// Class Definition : This class registers PIDs and manages overall license
//                    querys
//////////////////////////////////////////////////////////////////////////////

/*==========================================================================
- ============================ LicenseHandler ==============================
- ========================================================================== */

//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////

MGSG_LicenseHandlerBase::Tmessage::Tmessage(MGSG_LicenseHandlerBase *handler, bool quiet)
	: d_handler(handler), d_quiet(quiet)
{
	handler->resetMessage();
}

MGSG_LicenseHandlerBase::Tmessage::~Tmessage()
{ 
	if (d_handler)
		d_handler->printMessage(d_quiet ? MGSG_LicenseMessages::SeverityUrgent :
								MGSG_LicenseMessages::SeveritySuccess);
}

//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////

void MGSG_LicenseHandlerBase::LostExceptionHandler::validateProcess(exceptionHandlerMode m)
{
  LostExceptionHandler &ehandler = *this;

  //
  // Exception Handler implemented with polling "lost" check
  //
  const bool disable_poll = (m == deactivateExceptionHandler);

  if (!disable_poll && d_exception == exception_init) {
    // initial setup for license handler to configure polling exceptions
    if (d_exception == exception_init) {
      static bool mgls_does_not_allow_change_even_after_clean = false;
      const int status = core().Mheap_set(MGLS_ATTR_EXCEPTION_TYPE, MGLS_POLLING_EXCEPTION);
      if (!mgls_does_not_allow_change_even_after_clean && MGLS_ERR == status) {
        ehandler.cannotConfigure(1);
        d_exception = exception_exit;
      } else
        mgls_does_not_allow_change_even_after_clean = true; // so ignore any errors if we are called again.
    }

    if (d_exception == exception_init) {
        enum { my_min = 30 }; // 60
        d_poll_interval_check = my_min;
        if (core().Mheap_set(MGLS_ATTR_CHECK_INTERVAL, d_poll_interval_check) == MGLS_ERR) {
          ehandler.cannotConfigure(2);
          d_exception = exception_exit;
        }
        if (d_exception == exception_init)
          d_poll_interval_check *= 1000; // callback needs ms
    }

    if (d_exception == exception_init) {
      enum { my_min = 30 }; // 60
      if (my_min < d_poll_interval_retry) {
        d_poll_interval_retry = my_min;
        if (core().Mheap_set(MGLS_ATTR_RETRY_INTERVAL, d_poll_interval_retry) == MGLS_ERR) {
          ehandler.cannotConfigure(3);
          d_exception = exception_exit;
        }
      }
      if (d_exception == exception_init)
        d_poll_interval_retry *= 1000; // callback needs ms
    }

    if (d_exception == exception_init)
      d_exception = exception_none;
  }

  ehandler.unsheduleValidate(); // remove any existing scheduled callback

  if (!disable_poll) {
    if (d_exception != exception_exit) {
      while(1) {
#ifdef DEBUG_LICENSE
        struct tm * timeinfo;
        char buffer [80];
        time_t rawtime;
        time ( &rawtime );
        timeinfo = localtime ( &rawtime );
        strftime (buffer,sizeof(buffer),"%c",timeinfo);
        printf("// Debug: Check license poll, current time %s\n", buffer);
#endif
        int num_features_lost = 0;
        MGLS_LOST_FEATURES *lost_feats = NULL;
        if (core().Mheap_lost(&num_features_lost, &lost_feats) == MGLS_ERR) { // triggers async creation
          ehandler.communicationFailed();
          d_exception = exception_exit;
          break;
        }
        int mretry = 0;
        int cb_num_features_lost = ehandler.featuresLost(num_features_lost, &mretry, &d_poll_interval_retry_client);
        if (0 < cb_num_features_lost) {
#ifdef DEBUG_LICENSE
          printf("// Debug: License poll reported **LOST** licenses\n");
#endif
          // some license feature has ben lost, enter exception mode handling.
          std::vector<std::string> posted;
          for (int i = 0; i<num_features_lost;++i) {
            if (lost_feats[i].retry_count > mretry) {
              mretry = lost_feats[i].retry_count;
            }
            if (lost_feats[i].retry_count == 1) {
              const std::string k = std::string(lost_feats[i].granted_feature) 
                + (*lost_feats[i].server != '\0' ? "|" : "") 
                + lost_feats[i].server;
              if ( std::find(posted.begin(), posted.end(), k) == posted.end()) {
                posted.push_back(k);
                ehandler.reaquireAttempt(lost_feats[i].server, lost_feats[i].granted_feature);
              }
            }
          }
          if (mretry >= d_max_retry_count) {
            ehandler.reaquireFailed();
            d_exception = exception_exit;
            break;
          } else
            ehandler.reaquireAttempt();
          if (d_exception != exception_retry) {
            // Might want to poll more often when license is lost.
            d_exception = exception_retry;
            d_poll_interval = &d_poll_interval_retry;
          }
          ehandler.waitFor(d_poll_interval == &d_poll_interval_retry
            ? (d_poll_interval_retry_client < d_poll_interval_retry ? d_poll_interval_retry_client : d_poll_interval_retry)
            : *d_poll_interval);
        } else {
#ifdef DEBUG_LICENSE
          printf("// Debug: License poll reported licenses okay\n");
#endif
          if (d_exception != exception_none) {
            if (d_exception == exception_retry)
              ehandler.reaquireSuccessful();
            d_exception = exception_none;
            d_poll_interval = &d_poll_interval_check;
          }
          break;
        }
      }
    }
    ehandler.scheduleValidate(*d_poll_interval);  // schedule callback in future
  } else if (d_exception != exception_exit) {
    d_exception = exception_init;
  }

  if (!disable_poll && d_exception == exception_exit) {
    ehandler.reaquireFatal();
  }

  if (disable_poll)
    d_handler->d_exception = 0;
}

//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////

class MGSG_LicenseHandlerBase::DefaultExceptionHandler {
	/* Description: Client exception handler routines. Sends a request for a
	 *              license checkout retry
	 *
	 * status = mgls_error_handler( status, license_name, call_count )
	 *      long status             MGLS_ENOSERVER - total reconnect failure,
	 *                                               license_name is invalid
	 *                              MGLS_ENOLICENSE - server reconnect successful,
	 *                                                but no licenses available
	 *      (char *)license_name    license name - only valid with MGLS_ENOLICENSE
	 *      long call_count         number of retry times already tried
	 *
	 *      long status             MGLS_CLIENT_RETRY - if MGLS_ENOSERVER, then try
	 *                              to reconnect or recall this routine
	 *                              if MGLS_ENOLICENSE, then invoke
	 *                              Core::Mheap_alloc as in the original call.
	 */
	static void set_retry_count(long &count) {
		MGLS_ATTR_VALUE rval;
		if ( core().Mheap_get( MGLS_ATTR_RETRY_COUNT, &rval ) != MGLS_ERR )
			count = rval;
		// else use defaults
#     ifdef DEBUG_LICENSE
		cout << "// Debug: Core::Mheap_get retry_count " << count << endl;
#     endif
	}
	static long retry_count(bool doset=false) {
		static long count = 5;
		if (doset) set_retry_count(count);
		return count;
	}
	static long exception_handler( long status, char * /* license_name */, long call_count ) 
	{
#     ifdef DEBUG_LICENSE
		cout << "// mgls_error_handler status: " << status << " Call_count: " << call_count << endl;
#     endif
		if ( call_count <= retry_count() ) {
			// Only display once, when called at-least twice
			if (call_count == 2) {
				MGSG_LicenseMessages messages(mgls_print_handler);
				messages.append(MGSG_LicenseMessages::MSG_WARN_ReaquireLicense);
				messages.print();
			}
			return( (long)MGLS_CLIENT_RETRY );
		}
		
		MGSG_LicenseMessages messages(mgls_print_handler);
		if (status == MGLS_ENOSERVER) {
			messages.append(MGSG_LicenseMessages::MSG_URG_ContactingLicenseServer);
			messages.print();
		} else {
			messages.append(MGSG_LicenseMessages::MSG_URG_LicenseProblemsEncountered);
			messages.print();
		}
		return( (long)MGLS_CLIENT_EXIT );
	}

    static Core &core(Core *set_core=0) {
      static Core *s_core = 0;
      if (set_core)
        s_core = set_core;
      return *s_core;
    }
	
public:

  static void install(MGSG_LicenseHandlerBase::Core &core_) {
		core(&core_).Mheap_set(MGLS_ATTR_EXIT_CALL, (long)exception_handler);
		static bool retry_count_undef = true;
		if (retry_count_undef) {
			retry_count_undef = false;
			retry_count(true); // cause default to be filled, will start async
		}
	}
};

//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////

MGSG_LicenseHandlerBase::MGSG_LicenseHandlerBase(void (*print)(MGSG_LicenseMessages::Index, 
															   MGSG_LicenseMessages::Severity,
															   const char *),
												 const char *envvar_prefix)
	: d_pids(new MGSG_MGLS_pids),
      d_envvar_pfx(envvar_prefix), 
	  d_lic_count_mode(Default),
      d_lic_file_calyptod(0),
      d_lic_file_mgcld(0),
	  d_lic_src(0),
	  d_lic_include(0),
	  d_lic_exclude(0),
	  d_messages(print),
	  d_lookup(new PidToName),
	  d_defaults(false),
	  d_destructing(false),
	  d_flags(flag_pidInternal|flag_pidLibrary|flag_pidDisable|flag_pidDisableP|flag_pidCheckin),
	  d_pidpool(NULL),
	  d_transpool(NULL),
	  d_exception(0)
{
	d_core = new Core(*this);
	d_pidpool = new PidPool(*this);
	d_transpool = new TransactionPool(*this);
	mgls_print_handler = print; // global handler on exception used to print

    std::string envVar = d_envvar_pfx;
	envVar += "_DEBUG";
	d_debug = !! getenv(envVar.c_str());
}

MGSG_LicenseHandlerBase::~MGSG_LicenseHandlerBase()
{ 
# ifdef DEBUG_LICENSE
	cout << "// Debug: Deleting base license handler class and performing cleanup" << endl;
# endif
    d_destructing = true;
	if (d_exception)
		d_exception->unregisterHandler(*this);
	if (d_transpool)
		d_transpool->print();
	delete d_pidpool; // no need to delete d_transpool, deleting d_pidpool will remove all transactions
	while(d_pids->size()) {
		(*d_pids->rbegin())->UnRegister(this);
		d_pids->pop_back(); 
	}
	delete d_pids; d_pids = 0;
	delete d_lic_file_calyptod;
	delete d_lic_file_mgcld;
	delete d_lic_src;
	delete d_lic_include;
	delete d_lic_exclude;
	delete d_lookup;
	Clean();
	delete d_core;
}

bool MGSG_LicenseHandlerBase::SetDefaults() const
{
# ifdef DEBUG_LICENSE
	cout << "// Debug: Configuring defaults for license subsystem" << endl;
# endif

	((MGSG_LicenseHandlerBase *)this)->d_defaults = true;

	Core::BlockSigs block_sigs(core());

	if (d_lic_file_mgcld) {
#     if defined(MGLS_ATTR_LICENSE_FILE) // added 2010_2
		core().Mheap_set(MGLS_ATTR_LICENSE_FILE, (long)d_lic_file_mgcld->c_str());
#else
		MGSG_PutEnv("MGLS_LICENSE_FILE", d_lic_file_mgcld->c_str());
		// set does not allow override of user variables, thus we do a putenv
        // core().Mheap_set(MGLS_ENV_LICENSE_FILE, (long)d_lic_file_mgcld->c_str());
#endif
	}

	if (d_lic_src) {
#   if !defined(_MSC_VER) || defined(MGLS_ENV_INCLUDE_LICENSES)  // not sure when this was implemented on windows, just use on newer releases 2011_1
		core().Mheap_set(MGLS_ENV_LICENSE_SOURCE, (long)d_lic_src->c_str());
#     else
		MGSG_PutEnv("MGLS_LICENSE_SOURCE", d_lic_src->c_str()); // attr not impl on Windows <= 2007_2
#     endif
	}

	if (d_lic_include) {
#   if defined(MGLS_ENV_INCLUDE_LICENSES)  // 2011_1
		core().Mheap_set(MGLS_ENV_INCLUDE_LICENSES, (long)d_lic_include->c_str());
#   endif
	}

	if (d_lic_exclude) {
#   if defined(MGLS_ENV_EXCLUDE_LICENSES)  // 2011_1
		core().Mheap_set(MGLS_ENV_EXCLUDE_LICENSES, (long)d_lic_exclude->c_str());
#   endif
	}

#ifdef DEBUG_LICENSE_STUB
	static bool stubs_caution_printed = false;
	if (! stubs_caution_printed) {
		stubs_caution_printed = true;
		const char sep[] = "****************************************************************";
		const char pfx[] = "// ";
		printf("%s%s\n%s*** CAUTION: Licensing stubs enabled, NOT for redistribution ***\n%s%s\n",
			   pfx, sep, pfx, pfx, sep);
		return true;
	}
#endif

	// Disable license Queueing
	core().Mheap_set(MGLS_ATTR_QUEUE, 0);
	
#ifdef _MSC_VER
	/* Disable PCLS error messages.  We handle all messages */
	core().Mheap_set(MGLS_ATTR_APP_TYPE, MGC_LIC_APP_NO_ERR_MSG);
#endif

#ifdef MGLS_ATTR_LICENSE_INUSE
	core().Mheap_set(MGLS_ATTR_LICENSE_INUSE, 1);
#endif

	// Setup the license per invoke
	switch (d_lic_count_mode) {
	case Default:
		break;
	case DisplayStr:
	case DisplayHashStr:
		if (!d_display.empty()) {
#ifndef DEBUG_LICENSE_STUB
			core().Mheap_set(MGLS_ATTR_DISPLAY, (long)d_display.c_str());
#endif
			break;
		} // else fall through and enable per invoke
	case LicensePerInvoke:
#if !defined(DEBUG_LICENSE_STUB) && !defined(MGLS_ATTR_LIC_PER_INVOKE)
		core().Mheap_set(MGLS_ATTR_LIC_PER_INVOKE, 1);
#endif
		break;
	};

	// Configure the exception handler last as it calls "user" code which may start an async
 	if (LostExceptionHandler *h = ((MGSG_LicenseHandlerBase *)this)->GetLostExceptionHandler()) {
		MGSG_LicenseHandlerBase *tthis = const_cast<MGSG_LicenseHandlerBase *>(this);
		h->d_handler = tthis;
		tthis->d_exception = h;
		h->validateProcess(LostExceptionHandler::activateExceptionHandler); 
	} else
		DefaultExceptionHandler::install(core());

	// Get feature data mapping loaded
	core().init();

# ifdef DEBUG_LICENSE
	cout << "// Debug: Done configuring defaults for license subsystem" << endl;
# endif

	return true;
}

MGSG_LicenseHandlerBase *MGSG_LicenseHandlerBase::get_handler(const MGSG_MGLS_pid &pid)
{
    return pid.d_handler;
}

MGSG_LicenseHandlerBase::PidPool *MGSG_LicenseHandlerBase::get_pidpool(const MGSG_MGLS_pid &pid)
{
    return pid.d_handler ? pid.d_handler->d_pidpool : 0;
}

MGSG_LicenseHandlerBase::TransactionPool *MGSG_LicenseHandlerBase::get_transpool(const MGSG_MGLS_pid &pid)
{
    return pid.d_handler ? pid.d_handler->d_transpool : 0;
}

MGSG_LicenseHandlerBase::Transactions *MGSG_LicenseHandlerBase::get_transactions(const MGSG_MGLS_pid &pid)
{
    return pid.d_handler ? pid.d_transactions : 0;
}

bool MGSG_LicenseHandlerBase::Register_pid(MGSG_MGLS_pid &hpid, int pid,
										   int flags,
										   unsigned int expire_secs, int warn_days,
										   const MGSG_MGLS_pid *unexpire,
										   int alternate_pid)

{ 
 	if (get_pid(pid) || !hpid.Register(this, pid, flags, // @@ | flag_pidCheckin
									   expire_secs, warn_days, unexpire, alternate_pid))
 		return false;
	if (!(flags & flag_pidInternal))
		d_pids->add(hpid);
	return true;
}

bool MGSG_LicenseHandlerBase::UnRegister_pid(MGSG_MGLS_pid &pid) 
{ 
	d_transpool->UnRegister_pid(pid);
	if (d_pids) {
		for (MGSG_MGLS_pids::iterator i = d_pids->begin(); !(i == d_pids->end()); ++i ) {
			if (&pid == (*i)) {
				if (pid.UnRegister(this)) {
//					d_pids->erase(i);
					return true;
				}
				break;
			}
		}
	}
	return false;
}

int MGSG_LicenseHandlerBase::allocateCLIM(const char *clim_pidname)
{ 
    return core().allocateCLIM(clim_pidname);
}

bool MGSG_LicenseHandlerBase::RegisterCLIM_contains(MGSG_MGLS_pid &pid
                                                    , const MGSG_MGLS_pid *pids[], unsigned int pids_num_of
                                                    , const MGSG_MGLS_pid *additional_co[], unsigned int additional_co_num_of)
{
    return core().RegisterCLIM_contains(pid, pids, pids_num_of, additional_co, additional_co_num_of);
}

void MGSG_LicenseHandlerBase::AddAction(MGSG_MGLS_pid &pid, actionType action, int arg)
{
    PidPool::Reference(*d_pidpool, pid.get_pid()).add_action(action, arg, 0);
}

void MGSG_LicenseHandlerBase::AddAction(MGSG_MGLS_pid &pid, actionType action, int arg0, int arg1)
{
    PidPool::Reference(*d_pidpool, pid.get_pid()).add_action(action, arg0, arg1);
}

void MGSG_LicenseHandlerBase::AddAlternate(MGSG_MGLS_pid &pid,
                                           const unsigned int pids_num_of, const int pids[],
                                           const unsigned int grants_num_of, const int grants[])
{
    PidPool::Reference(*d_pidpool, pid.get_pid()).add_alternate(flag_undefined
      , pids_num_of, pids, grants_num_of, grants);
}

MGSG_LicenseHandlerBase::Flag MGSG_LicenseHandlerBase::Allocate_flag()
{
	int ii = 0x01;
	for (unsigned int i = 0; i < sizeof(i)*8; ++i, ii = (ii << 1)) {
		if (!(d_flags & ii)) {
			d_flags |= ii;
			return (Flag)ii;
		}
	}
	return flag_undefined;
}

bool MGSG_LicenseHandlerBase::ActiveLicense() const
{
	for (MGSG_MGLS_pids::const_iterator pid = d_pids->begin();
		 !(pid == d_pids->end()); ++pid ) {
		if ((*pid)->is_active())
			return true;
	}
	return false;
}

bool MGSG_LicenseHandlerBase::Clean()
{
	if (d_exception)
		d_exception->unregisterHandler(*this);
	core().Mheap_clean();
	d_defaults = false;
	return true;
}

bool MGSG_LicenseHandlerBase::Reset()
{
	bool res = !ActiveLicense();

	if (res) {
		Clean();
		SetDefaults(); // call last may cause async to start
	}

#ifdef DEBUG_LICENSE
	cout << "// Debug: Reset License Handler and applied defaults" << (res ? "PASSED" : "FAILED") << endl;
#endif

	return res;
}

bool MGSG_LicenseHandlerBase::EnableLicensePerInvoke()
{
	if (d_lic_count_mode == LicensePerInvoke || d_lic_count_mode == DisplayStr)
		return true;

	if (d_lic_count_mode != Default)
		return false;

	// See if there are any active features, once features are active
	// we must use the ATTR_LIC_PER_INVOKE ... Cannot modify the DISPLAY
	d_lic_count_mode = ActiveLicense() ? LicensePerInvoke : DisplayStr;

#ifdef DEBUG_LICENSE
	cout << "// Debug: Enabling license per invocation" << endl;
#endif

	Tmessage prt(this, false);
		
#ifdef DEBUG_LICENSE
	cout << "// Debug: Enabling license per invocation with "
		 << (d_lic_count_mode == LicensePerInvoke ? "ATTR_LIC_PER_INVOKE" : "ATTR_DISPLAY") << endl;
#endif

	bool res = false;

	if (d_lic_count_mode == LicensePerInvoke) {
#ifdef MGLS_ATTR_LIC_PER_INVOKE
#ifndef DEBUG_LICENSE_STUB
		if (core().Mheap_set(MGLS_ATTR_LIC_PER_INVOKE, 1) == MGLS_OK)
			res = true;
		else
			d_messages.appendMheapErrs(core());
#else
		res = true;
#endif
#else
		d_messages.append(MGSG_LicenseMessages::MSG_URG_LicenseProblemsEncountered);
#endif
	} else {
		if (d_defaults) // cleanup existing handler
			Clean();
		// Set the DISPLAY to include the PID to "emulate" license per invoke, 
		// When the attribute is not supported we turn it on at the application
		// start.  Otherwise the user can call the EnableLicensePerInvoke to
		// enable when desired.
		MGSG_GetDisplay(d_display);
		res = !d_display.empty();
	}

#ifdef DEBUG_LICENSE
	cout << "// Debug: Enabling license per invocation, " 
		 << (res ? "PASSED" : "FAILED") << endl;
#endif

	return res;
}

bool MGSG_LicenseHandlerBase::SetDaemon(bool enabled, daemonType dtype)
{
	bool res = !ActiveLicense();
	if (res) {
        res = core().enableDaemon(dtype, enabled);
		if (d_defaults && res)
			res = Reset();
    }
    return res;
}

bool MGSG_LicenseHandlerBase::SetLicenseFile(const char *location, daemonType dtype)
{
    std::string *&d_lic_file = (dtype == MGCLD_daemon ? d_lic_file_mgcld : d_lic_file_calyptod);
	bool res = location && *location && !ActiveLicense();
	if (res) {
		if (! d_lic_file)
			d_lic_file = new STDCOLON string;
		(*d_lic_file) = location;
		if (d_defaults)
			res = Reset();
	}
#ifdef DEBUG_LICENSE
	cout << "// Debug: SetLicenseFile " << location << " " << (res ? "PASSED" : "FAILED") << endl;
#endif
	return res;
}

bool MGSG_LicenseHandlerBase::SetLicenseSource(const char *source)
{
	bool res = source && *source && !ActiveLicense();
	if (res) {
		if (! d_lic_src)
			d_lic_src = new STDCOLON string;
		(*d_lic_src) = source;
		if (d_defaults)
			res = Reset();
	}
#ifdef DEBUG_LICENSE
	cout << "// Debug: SetLicenseSource " << source << " " << (res ? "PASSED" : "FAILED") << endl;
#endif
	return res;
}

bool MGSG_LicenseHandlerBase::SetLicenseInclude(const char *source)
{
	bool res = source && *source && !ActiveLicense();
	if (res) {
		if (! d_lic_include)
			d_lic_include = new STDCOLON string;
		(*d_lic_include) = source;
		if (d_defaults)
			res = Reset();
	}
#ifdef DEBUG_LICENSE
	cout << "// Debug: SetLicenseInclude " << source << " " << (res ? "PASSED" : "FAILED") << endl;
#endif
	return res;
}

bool MGSG_LicenseHandlerBase::SetLicenseExclude(const char *source)
{
	bool res = source && *source && !ActiveLicense();
	if (res) {
		if (! d_lic_exclude)
			d_lic_exclude = new STDCOLON string;
		(*d_lic_exclude) = source;
		if (d_defaults)
			res = Reset();
	}
#ifdef DEBUG_LICENSE
	cout << "// Debug: SetLicenseExclude " << source << " " << (res ? "PASSED" : "FAILED") << endl;
#endif
	return res;
}

bool MGSG_LicenseHandlerBase::SetDisplayHash(const char *display_hash)
{
	// Set the DISPLAY based on a valid hash
	bool res = false;
	if (d_lic_count_mode == Default && display_hash && !ActiveLicense()) {
        std::string display_str;
		if (SetDisplayHash(display_hash, &display_str)) {
			// the hash is good set the display attribute.
			d_display = display_str;
			d_lic_count_mode = DisplayHashStr;

			// ensure the mgls job is cleaned up, we cannot modify ATTR_DISPLAY once
			// the job has been started.  CAUTION, this will return all licenses,
			// however above we ensured there are no features active, e.g. transactions.
			if (d_defaults) {
				Clean();
				res = SetDefaults(); // call last may cause async to start
			} else
				res = true;
		}
	}

#ifdef DEBUG_LICENSE
	cout << "// Debug: Set DISPLAY token from hash " << (res ? "PASSED" : "FAILED") << endl;
#endif
	return res;
}

bool MGSG_LicenseHandlerBase::SetDisplayHash(const char *display_hash, std::string *display)
{
	// Set the DISPLAY based on a valid hash
	bool res = false;
    std::string hostname;
    MGSG_GetHostName(hostname);
    DisplayHash dhash(display_hash);
    if (const char *display_str = dhash.GetStr(hostname.c_str(), MGSG_GetPid(true))) {
      // the hash is good set the display attribute.
      if (display)
        *display = display_str;
      res = true;
    }
    return res;
}

bool CDS_LicenseHandler::GetDisplayHash(std::string *display_hash, unsigned int valid_secs, const char *display)
{
  if (display_hash) {
    std::string hostname;
	MGSG_GetHostName(hostname);
    std::string d_display;
    if (display)
      d_display = display;
    else {
      const char s[] = {'N', 'u', 'L', 'l'};
      d_display.append(s, sizeof(s));
    }
    DisplayHash dhash;
    if (const char *str = dhash.Generate(hostname.c_str(), MGSG_GetPid(), d_display.c_str(),
      valid_secs)) {
        *display_hash = str;
        return true;
    }
  }
  return false;
}

bool MGSG_LicenseHandlerBase::checkout_feature(MGSG_MGLS_pid &id, 
                                               bool duplicate,
                                               bool quiet)
{ 
    Tmessage prt(this, quiet);
    return id.checkout(d_messages, duplicate);
}

bool MGSG_LicenseHandlerBase::checkin_feature(MGSG_MGLS_pid &id, 
                                              bool duplicate,
                                              bool quiet)
{
    Tmessage prt(this, quiet);
    return id.checkin(d_messages, duplicate);
}

bool MGSG_LicenseHandlerBase::allocate_one_feature(MGSG_MGLS_pid &id, 
                                                   bool quiet)
{
    Tmessage prt(this, quiet);
    return id.allocate_one(d_messages);
}

bool MGSG_LicenseHandlerBase::allocate_return_feature(MGSG_MGLS_pid &id, 
                                                      bool duplicate,
                                                      bool quiet)
{
    Tmessage prt(this, quiet);
    return id.allocate_return(d_messages, duplicate);
}

unsigned int MGSG_LicenseHandlerBase::get_actuals(const MGSG_MGLS_pid &id)
{ 
    return id.get_actuals();
}

unsigned int MGSG_LicenseHandlerBase::get_duplicates(const MGSG_MGLS_pid &id)
{ 
    return id.get_duplicates(); 
}

MGSG_License_Type MGSG_LicenseHandlerBase::get_type(const MGSG_MGLS_pid &id)
{ 
    return id.get_type();
}

MGSG_MGLS_pid *MGSG_LicenseHandlerBase::get_pid(int app_id, bool library_pid) const
{
	for (MGSG_MGLS_pids::const_iterator pid = d_pids->begin();
		 !(pid == d_pids->end()); ++pid ) {
		if (app_id == (*pid)->get_pid()) {
			if (!library_pid || (*pid)->is_library_id())
				return(*pid);
			else
				break;
		}
	}
	return(0);
}

MGSG_MGLS_pid *MGSG_LicenseHandlerBase::get_pid(const char *app_id_name, bool library_pid) const
{
    std::string pid_name;
	for (MGSG_MGLS_pids::const_iterator pid = d_pids->begin();
		 !(pid == d_pids->end()); ++pid ) {
		if ((*pid)->get_name(&pid_name) && pid_name.compare(app_id_name) == 0) {
			if (!library_pid || (*pid)->is_library_id())
				return(*pid);
			else
				break;
		}
	}
	return(0);
}

const char *MGSG_LicenseHandlerBase::pvt_lookup(const int pid, const char *default_str, bool add_unique) const
{
	const char *res = d_lookup->find(pid);
	if (res) {
		if (default_str && add_unique && strcmp(res, default_str))
			res = 0;
		return res;
	}

	if (!d_defaults)
		SetDefaults();

#if !defined(DEBUG_LICENSE_STUB)
	char lookup_buf[MGLS_MAX_FEATURE_LEN+1];
	if (core().Mheap_lookup(pid, lookup_buf) == MGLS_OK) {
		res = d_lookup->add(pid, lookup_buf);
		if (res)
			return res;
	}
#endif
	
	if (default_str) {
		res = d_lookup->add(pid, default_str);
		if (res)
			return res;
	}

	char sbuf[64];
	sprintf(sbuf, "%d", pid);
	res = d_lookup->add(pid, sbuf);

	return res;
}

#if defined(__linux__) && defined(__GNUC__)
   // Include cstddef for ptrdiff_t:
#  include <cstddef>
#endif

void MGSG_LicenseHandlerBase::EnvVar_setup() {
	// Disables licenses via environmental variable. 
	//
	// The environmental variable will contain a colon delimited list 
	// of pids.
	// 
	// If the first entry is ENABLE, then only the list of pids
	// will be abled.  e.g. all features are disable except for
	// the listed pids.
	//
	// If the first entry is DISABLE, or not ENABLE, then the list
	// of pids will be disabled.
	//
    std::string envVar = d_envvar_pfx;
	envVar += "_LIMIT_LICENSE_PID";
	const char *pids = getenv(envVar.c_str());
	if (pids) {
		char buf[64];
		const char *ptr = pids, *sptr = pids;
		ptrdiff_t slen;
        std::map<int, int> pidm;
		bool enablePids = true;

		// Build list of pids to enable
		while(1) {
			if (*ptr == ':' || *ptr == '\0') {
				if ((slen = ptr - sptr) > 0 && (slen < static_cast<int>(sizeof(buf)))) {
					strncpy(buf, sptr, slen); buf[slen] = '\0';
					int app_id = 0;
					if (MGSG_is_digits(buf)) {
						app_id = atoi(buf);
					} else if (sptr == pids && strcmp(buf,"DISABLE") == 0) {
						enablePids = false;
					} else if (sptr == pids && strcmp(buf,"ENABLE") == 0) {
						enablePids = true;
					} else {
						const MGSG_MGLS_pid *pid = get_pid(buf);
						if (pid)
							app_id = pid->get_pid();
					}
					if (app_id > 0)
						pidm[app_id] = 1;
				}
				if (*ptr == ':') {
					ptr++;
					sptr = ptr;
				} else
					break;
			} else 
				ptr++;
		}

		if (enablePids) {
			// Disable pids NOT found in map, e.g. enable specified pids only
			for (MGSG_MGLS_pids::iterator pid = d_pids->begin();
				 !(pid == d_pids->end()); ++pid ) {
				if (!pidm[(*pid)->get_pid()])
					(*pid)->disable();
			}
		} else {
			// Disable pids found in map
			for (MGSG_MGLS_pids::iterator pid = d_pids->begin();
				 !(pid == d_pids->end()); ++pid ) {
				if (pidm[(*pid)->get_pid()])
					(*pid)->disable();
			}
		}

		if (d_debug) {
            std::string pid_name;
			for (MGSG_MGLS_pids::iterator pid = d_pids->begin();
				 !(pid == d_pids->end()); ++pid ) {
				if (! (*pid)->get_name(&pid_name))
					pid_name = "";

                std::string pid_expire;
				(*pid)->get_hard_expire(&pid_expire);

                std::string pid_unexpire;
				(*pid)->get_hard_unexpire(&pid_unexpire);

				printf("// Debug: %-16s product_id=%-5d status=%-8s%s%s%s%s\n",
					   pid_name.c_str(), (*pid)->get_pid(),
					   ((*pid)->is_disabled() ? "disabled" : "enabled"),
					   (pid_expire.empty() ? "" : " expires="), pid_expire.c_str(),
					   (pid_unexpire.empty() ? "" : " unexpire="), pid_unexpire.c_str());
			}
		}
	}
}

/////////////////////////////////////////////////////////////////////////////

/*==========================================================================
- ============================ CLIM API RUNTIME ============================
- ========================================================================== */

#ifdef LICENSE_USE_CLIM
#ifndef CRYPTOPP_NOPATH
#define CRYPTOPP_NOPATH
#endif
#include "cds_clip/CLIM.cpp"
#endif

/////////////////////////////////////////////////////////////////////////////

