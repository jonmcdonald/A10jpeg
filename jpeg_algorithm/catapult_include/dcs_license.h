/*  -*- mode:C++; tab-width:4; c-basic-offset:4; indent-tabs-mode:t; -*-     */
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

#ifndef INCLUDED_DCS_LICENSE_H
#define INCLUDED_DCS_LICENSE_H

// Use to cipher feature name for RegisterCLIM_pid (32 chars max)
#define MGSG_CRYPT(str) { MGSG_CRYPT_(str \
                                      "\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0" \
                                      "\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0" ) }

// match c_str / enum cross reference table
enum MGSG_License_Type {
	MGSG_uninitialized,
	MGSG_disabled,
    MGSG_unused,
	MGSG_unknown,
    MGSG_unix,
	MGSG_nodelocked_uncounted,
	MGSG_nodelocked_counted,
	MGSG_floating
};

/*==========================================================================
- ========================  C++  I N T E R F A C E  ========================
- ==========================================================================*/

#ifdef __cplusplus

#if defined(_MSC_VER) && !defined(DEBUG)
# pragma warning( push )
# pragma warning( disable : 4275 )  // Disable this used in base member initializer warning
# pragma warning( disable : 4355 )  // Disable non dll-interface class warning
#endif

#include <string>
#include <list>

// work around subclass protection issues with MSVC60 / SUNWspro
#if (defined(_MSC_VER) && _MSC_VER < 1300) || defined(__SUNPRO_CC)
# define CC____RelaxProtection(to) to :
# define CC____RestoreProtection(to) to :
#else
# define CC____RelaxProtection(to)
# define CC____RestoreProtection(to)
#endif

//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////

class MGSG_PKGS_API MGSG_LicenseMessage;
class MGSG_PKGS_API MGSG_LicenseHandlerBase;
class MGSG_PKGS_API MGSG_MGLS_pid;
class MGSG_PKGS_API MGSG_MGLS_pids;

typedef MGSG_PKGS_API STDCOLON list<MGSG_LicenseMessage *> list_MGSG_LicenseMessage;
class MGSG_PKGS_API MGSG_LicenseMessages : public list_MGSG_LicenseMessage {
public:
    class Core;

    enum Severity {
		SeveritySuccess       = 0x01000,
		SeverityInformational = 0x02000,
		SeverityWarning       = 0x04000,
		SeverityError         = 0x08000,
		SeverityUrgent        = 0x10000
	};

	enum Index {
		MSG_INF_Default = SeverityInformational,
		MSG_INF_ExpireSoon1,
		MSG_INF_ExpireSoon2,
		MSG_INF_BetaFeatureExpiration,
		MSG_INF_waiting_in_queue,
		MSG_INF_checking_queue_status,
		MSG_INF_reconnect_in_progress,
		MSG_INF_reconnect_done,
		MSG_INF_waiting_for_response,

		MSG_WARN_Default = SeverityWarning,
		MSG_WARN_LicenseCheckinFailed,
		MSG_WARN_LicenseException,
		MSG_WARN_QueuedLicense,
		MSG_WARN_ReaquireLicense,
		MSG_WARN_trans_id_not_found,
		MSG_WARN_exp_warning_msg,
		MSG_WARN_attr_out_of_range,
		MSG_WARN_queue_not_requested,
		MSG_WARN_lost_license,
		MSG_WARN_lost_connection,
		MSG_WARN_somesort_of_warning,
		MSG_WARN_wont_override_env,
		MSG_WARN_checkin_warning,
		MSG_WARN_flexlm_warning,
		MSG_WARN_lost_asynch,

		MSG_ERR_Default = SeverityError,
		MSG_ERR_LicenseRequestFailed,
		MSG_ERR_UnexpectedLicenseReqResponse,
		MSG_ERR_ContactCustomerSupport,
		MSG_ERR_DisabledLicenseRequestPID,
		MSG_ERR_DisabledLicenseRequestName,
		MSG_ERR_LicenseNotRegistered,
		MSG_ERR_no_falconfw_license,
		MSG_ERR_malloc_failure,
		MSG_ERR_persistent_lic_problem,
		MSG_ERR_license_request_failed,
		MSG_ERR_invalid_license_version,
		MSG_ERR_subversion_detected,
		MSG_ERR_somesort_of_failure,
		MSG_ERR_comm_failure,
		MSG_ERR_flexlm_failure,
		MSG_ERR_no_license_source,
		MSG_ERR_license_info_error,
		MSG_ERR_no_duplicates_allocated,
		MSG_ERR_no_actuals_allocated,
		MSG_ERR_no_references_allocated,
		MSG_ERR_license_is_not_valid,

		MSG_URG_BetaFeatureExpired = SeverityUrgent,
		MSG_URG_LicenseProblemsEncountered,
		MSG_URG_ContactingLicenseServer,
		MSG_URG_problem_starting_child,
		MSG_URG_no_mgls_home,
		MSG_URG_pkginfofile_corrupt,
		MSG_URG_svr_comm_failure,
		MSG_URG_license_file_problem,
		MSG_URG_unknown_attribute,
		MSG_URG_error_in_set_lou,
		MSG_URG_license_info_error,
		MSG_URG_flexlm_server_too_old,
		MSG_URG_unknown_license_name,
		MSG_URG_unknown_license_id,
		MSG_URG_invalid_version_num
	};

	MGSG_LicenseMessages(void (*print)(Index, Severity, const char *) = 0)
		: list_MGSG_LicenseMessage(), d_print(print) { }
	~MGSG_LicenseMessages() { reset(); }
	MGSG_LicenseMessages(const MGSG_LicenseMessages &messages) { append(messages); }

	void append(Index index, ...);
    void appendMheapErrs(const Core &core);

	void reset();
	void take(MGSG_LicenseMessages &messages);
	void append(const MGSG_LicenseMessages &messages);
	void print(Severity severity_ge = SeveritySuccess) const;
	const char *get(Severity severity_ge = SeveritySuccess,
					bool noprefix = false) const;

private:
	static const char *GetErrMsgFmt(Index index);
	static Index MGLS_To_License_enum(unsigned int mgls_errno);
	static Severity Index_To_Severity_enum(Index index);
	static bool NoSuppress(Index index);

	void (*d_print)(Index, Severity, const char *);
	STDCOLON string          d_str;
};

class MGSG_PKGS_API MGSG_LicenseMessage {
	STDCOLON string                 d_str;
	MGSG_LicenseMessages::Severity  d_severity;
	MGSG_LicenseMessages::Index     d_index;
  
	friend class MGSG_LicenseMessages;
	friend class MGSG_MGLS_pid;
	MGSG_LicenseMessage(MGSG_LicenseMessages::Index index, 
						MGSG_LicenseMessages::Severity severity, 
						const char *str) 
		: d_str(str), d_severity(severity), d_index(index) { }
	~MGSG_LicenseMessage() { }
  
public:
	const char *get() const { return d_str.c_str(); }
	MGSG_LicenseMessages::Severity severity() const { return d_severity; }
	MGSG_LicenseMessages::Index index() const { return d_index; }
};

//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////

class MGSG_PKGS_API MGSG_LicenseHandlerBase {
public:
	class Core;
    Core &core() const { return *d_core; }

private:
    Core                 *d_core;

	MGSG_MGLS_pids       *d_pids;
	STDCOLON string       d_envvar_pfx;
	bool                  d_debug;
	bool                  d_destructing;

	enum usageMode {
		Default,
		LicensePerInvoke,
		DisplayStr,
		DisplayHashStr
	};

	usageMode             d_lic_count_mode;

	STDCOLON string       d_display;

    STDCOLON string      *d_lic_file_calyptod;
	STDCOLON string      *d_lic_file_mgcld;
	STDCOLON string      *d_lic_src;
	STDCOLON string      *d_lic_include;
	STDCOLON string      *d_lic_exclude;

	MGSG_LicenseMessages  d_messages;

	class MGSG_PKGS_API Tmessage {
		MGSG_LicenseHandlerBase *d_handler;
		bool d_quiet;
	public:
		Tmessage(MGSG_LicenseHandlerBase *handler, bool quiet);
		~Tmessage();
	};

	bool Clean();
	bool Reset();
	bool SetDefaults() const;
	bool ActiveLicense() const;

	class DefaultExceptionHandler;
	class DisplayHash;

	class PidToName;
	mutable PidToName *d_lookup;
	const char *pvt_lookup(const int pid, const char *default_str, bool add_unique) const;

    bool                  d_defaults;
	int                   d_flags;

    class PidPool_Reference;

	CC____RelaxProtection(public)
    class PidPool;
	CC____RestoreProtection(private)
	PidPool              *d_pidpool;

	class Transaction;
	CC____RelaxProtection(public)
    class Transactions;
	class TransactionPool;
	CC____RestoreProtection(private)
	TransactionPool      *d_transpool;

    friend class MGSG_MGLS_pid;                // Allowed access private handler classes
	friend class MGSG_LicenseMessages;         // Allowed to access Print
    friend class PidPool;                      // Allowed to access Register_pid

	CC____RelaxProtection(public)
    static MGSG_LicenseHandlerBase *get_handler(const MGSG_MGLS_pid &pid);
    static PidPool *get_pidpool(const MGSG_MGLS_pid &pid);
    static TransactionPool *get_transpool(const MGSG_MGLS_pid &pid);
    static Transactions *get_transactions(const MGSG_MGLS_pid &pid);
	CC____RestoreProtection(private)

	enum { flag_pidInternal  = (0x01 << 0) };
								  
    int allocateCLIM(const char *MGSG_CRYPT16_pidname); 

protected:
	CC____RelaxProtection(public)
	enum Flag {
		flag_undefined   = 0,
		flag_pidLibrary  = (0x01 << 1),
		flag_pidDisable  = (0x01 << 2),
		flag_pidDisableP = (0x01 << 3),
		flag_pidCheckin  = (0x01 << 4)
	};
	CC____RestoreProtection(protected)

	Flag Allocate_flag();
	void Deallocate_flag(Flag flag) { d_flags &= (~flag); }

	bool Register_pid(MGSG_MGLS_pid &hpid, int pid,
					  int flags = flag_undefined,
					  unsigned int expire_secs = 0, int warn_days = 0,
					  const MGSG_MGLS_pid *unexpire = 0,
					  int alternate_pid = -1);
	bool UnRegister_pid(MGSG_MGLS_pid &pid);

    // this registers a CLIM only feature
  bool RegisterCLIM_pid(MGSG_MGLS_pid &hpid, const char *MGSG_CRYPT16_pidname
    , int flags = flag_undefined
    , unsigned int expire_secs = 0, int warn_days = 0
    , const MGSG_MGLS_pid *unexpire = 0
    , int alternate_pid = -1)
  {
    return Register_pid(hpid, allocateCLIM(MGSG_CRYPT16_pidname), flags, expire_secs, 
      warn_days, unexpire, alternate_pid);
  }
  bool RegisterCLIM_contains(MGSG_MGLS_pid &hpid
    , const MGSG_MGLS_pid *pids[], unsigned int pids_num_of
    , const MGSG_MGLS_pid *additional_co[], unsigned int additional_co_num_of);

    CC____RelaxProtection(public)
	enum actionType {
		action_disable_flagged_on_checkout_failure = 0x00,                        // arg(int)=flags
		action_disable_flagged_uncontained_pids_on_checkout_success,              // arg(int)=flags
	    action_disable_flagged_uncontained_pids_in_composite_on_checkout_success  // arg(int)=flags,(int)=composite_pid
	};
    enum { actionType_cnt = action_disable_flagged_uncontained_pids_in_composite_on_checkout_success+1 };
	CC____RestoreProtection(protected)

    void AddAction(MGSG_MGLS_pid &pid, actionType action, int arg);
  	void AddAction(MGSG_MGLS_pid &pid, actionType action, int arg0, int arg1);

    void AddAlternate(MGSG_MGLS_pid &pid
      , const unsigned int pids_num_of, const int pids[]
      , const unsigned int grants_num_of, const int grants[]);
    void AddAlternate(MGSG_MGLS_pid &pid, const unsigned int pids_num_of, const int pids[]) { AddAlternate(pid, pids_num_of, pids, 0, 0); }
    void AddAlternate(MGSG_MGLS_pid &pid, const int pidnum) { AddAlternate(pid, 1, &pidnum); }

    // OBSOLETE -- update code to use new Register_pid
	bool Register_pid(MGSG_MGLS_pid &hpid, int pid, const char *name,  
					  bool library_id = false, bool disable = false,
					  unsigned int expire_secs = 0, int warn_days = 0,
					  const MGSG_MGLS_pid *unexpire = 0,
					  int alternate_pid = -1, const char *alternate_name = 0)
	{ 
		return Register_pid(hpid, pid, 
							((library_id?flag_pidLibrary:0) | (disable?flag_pidDisable:0)),
							expire_secs, warn_days, unexpire, alternate_pid); 
	}

protected:
        class LostExceptionHandler;
        virtual LostExceptionHandler *GetLostExceptionHandler() { return 0; }

private:
        LostExceptionHandler *d_exception;
        
public:
	MGSG_LicenseHandlerBase(void (*print)(MGSG_LicenseMessages::Index, 
										  MGSG_LicenseMessages::Severity,
										  const char *),
							const char *envvar_prefix);
	virtual ~MGSG_LicenseHandlerBase();

	// Must be called once license handler has been configured
	void EnvVar_setup();

    enum daemonType { MGCLD_daemon, CALYPTOD_daemon };

    // Configuration options
	bool EnableLicensePerInvoke();
    bool SetDaemon(bool enabled=true, daemonType dtype=MGCLD_daemon);
    bool SetLicenseFile(const char *location, daemonType dtype=MGCLD_daemon);
	bool SetLicenseSource(const char *source);
	bool SetLicenseInclude(const char *source);
	bool SetLicenseExclude(const char *source);

	// Returns a string which can be used to initialize a child task to have the 
	// same DISPLAY (AKA duplicate grouping) as the parent.  This allows the 
	// child task to share the same licenses.  The Hash has a limited time 
	// window in which it can be used.  The hash should be passed to the child
	// process which should then use the string to initialized the base license
	// handler, the valid_secs must be <= 120
	static bool GetDisplayHash(STDCOLON string *display_hash, unsigned int valid_secs, const char *display);
	// Returns true if the display_hash is valid, and decodes display 
	static bool SetDisplayHash(const char *display_hash, STDCOLON string *display);

	// Use class datamembers to fill in defaults for static functions
	bool GetDisplayHash(STDCOLON string *display_hash, unsigned int valid_secs=30) const
	{ return GetDisplayHash(display_hash, valid_secs, d_display.c_str()); }
	bool SetDisplayHash(const char *display_hash);

	// Messaging interface
	void resetMessage() { d_messages.reset(); }
    void takeMessage(MGSG_LicenseMessages &messages) { d_messages.take(messages); }
	void printMessage(MGSG_LicenseMessages::Severity severity_ge = MGSG_LicenseMessages::SeveritySuccess) 
		const { d_messages.print(severity_ge); }
	const char *getMessage(MGSG_LicenseMessages::Severity severity_ge = MGSG_LicenseMessages::SeveritySuccess) 
		const { return d_messages.get(severity_ge); }

	// Alternate interface to access license interface, includes auto printing
	bool checkout_feature(MGSG_MGLS_pid &id, bool duplicate=false, bool quiet=false);
	bool checkin_feature(MGSG_MGLS_pid &id, bool duplicate=false, bool quiet=false); 
	bool allocate_one_feature(MGSG_MGLS_pid &id, bool quiet=false);
	bool allocate_return_feature(MGSG_MGLS_pid &id, bool duplicate=false, bool quiet=false);

	unsigned int get_actuals(const MGSG_MGLS_pid &id);
	unsigned int get_duplicates(const MGSG_MGLS_pid &id);
	MGSG_License_Type get_type(const MGSG_MGLS_pid &id);

	// Lookup pid's in our handler list
	MGSG_MGLS_pid *get_pid(int app_id, bool library_pid = false) const;
	MGSG_MGLS_pid *get_pid(const char *app_id_name, bool library_pid = false) const;
	MGSG_MGLS_pid *get_library_pid(int app_id) const { return get_pid(app_id, true); }
	MGSG_MGLS_pid *get_library_pid(const char *app_id_name) const { return get_pid(app_id_name, true); }

	const MGSG_MGLS_pids *get_pids() const { return d_pids; } // OBSOLETE -- use pids_iterator
 
	class pids_iterator {
		struct data;
		data *d_data;
		void _next();
		MGSG_MGLS_pid* _get() const;
		friend class TransactionPool;
	public:
		pids_iterator(const MGSG_LicenseHandlerBase &handler);
		~pids_iterator();
		pids_iterator& operator++() { if (!is_end()) _next(); return *this; }
		MGSG_MGLS_pid* operator*() const { return !is_end() ? _get() : 0; }
		bool is_end() const { return !d_data; }
        static unsigned int size(MGSG_LicenseHandlerBase &handler);
    };
	friend class pids_iterator;

	const char *lookup(const int pid) const { return pvt_lookup(pid, 0, false); }

	// Class maintains the serial numbers associated with a transaction
	class SerialNums {
		struct Node;

		Node *d_head;
		char *d_str;
		unsigned int d_strl;
		bool d_update;

		static char * null_str();

		void update() const;
		void add(const char *s, const unsigned int slen);

	public:
		SerialNums() : d_head(0), 
					   d_str(null_str()), d_update(false) { }
		~SerialNums() { clear(); }

		void clear();

		void add(const char *s);
		void add(const SerialNums &s);

		inline void set(const SerialNums &s) { clear(); add(s); }

		const char *c_str() const { if (d_update) update(); return d_str; }
		operator const char *() const { return c_str(); }
	};

	// Class used to automatically "return" checked out licenses when class is destroyed
	class AutoReturn {
		MGSG_LicenseHandlerBase &d_handler;
		MGSG_MGLS_pid *d_pid;
		unsigned int d_actuals;
		unsigned int d_duplicates;
	public:
		AutoReturn(MGSG_LicenseHandlerBase &handler, MGSG_MGLS_pid *pid )
			: d_handler(handler), d_pid(pid), d_actuals(0), d_duplicates(0) { }
		~AutoReturn() {
			while (d_duplicates != 0) {
				d_handler.checkin_feature(*d_pid, true);
				--d_duplicates;
			}
			while (d_actuals != 0) {
				d_handler.checkin_feature(*d_pid, false);
				--d_actuals;
			}
		}
		bool checkout(bool duplicate = false,
					  bool quiet = false)
		{ 
			bool res = false;
			if (d_pid) {
				if (duplicate) {
					if ((res = d_handler.checkout_feature(*d_pid, true, quiet)))
						++d_duplicates;
				} else {
					if ((res = d_handler.checkout_feature(*d_pid, false, quiet)))
						++d_actuals;
				}
			}
			return res;
		}
	};

	// Iterator to iterate over actual licenses (i.e. composites or atomics)
	class actuals_iterator {
		struct data;
		data *d_data;
		void _next();
		const MGSG_MGLS_pid* _get() const;
		bool _is_actual() const;
		friend class TransactionPool;
	public:
		actuals_iterator(const MGSG_LicenseHandlerBase &handler);
		~actuals_iterator();
		actuals_iterator& operator++() { if (!is_end()) _next(); return *this; }
		const MGSG_MGLS_pid* operator*() const { return !is_end() ? _get() : 0; }
		bool is_end() const { return !d_data; }
	};
	friend class actuals_iterator;

	// Iterator to iterate over actual licenses (i.e. composites or atomics)
	class contains_iterator {
		class iterator;
		iterator *d_itr;
		void _next();
        void _end();
        const MGSG_MGLS_pid* _get() const;
	public:
		contains_iterator(const MGSG_MGLS_pid &pid);
		~contains_iterator();
		contains_iterator& operator++() { if (!is_end()) _next(); return *this; }
		const MGSG_MGLS_pid* operator*() const { return !is_end() ? _get() : 0; }
		bool is_end() const { return !d_itr; }
	};

	class SetHandler {
		friend class MGSG_MGLS_pid;
		inline static bool Defaults(const MGSG_LicenseHandlerBase *handler) {
			return handler && (handler->d_defaults || handler->SetDefaults());
		}
	};
};

typedef MGSG_LicenseHandlerBase CDS_LicenseHandler;

//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////

class MGSG_PKGS_API MGSG_LicenseHandlerBase::LostExceptionHandler {
  MGSG_LicenseHandlerBase *d_handler;
  Core &core() const { return d_handler->core(); }

  enum exception_mode { 
    exception_init=0x01, exception_retry, exception_exit, 
    exception_none, exception_disable 
  };
  exception_mode d_exception;

  long d_poll_interval_check;
  long d_poll_interval_retry;
  const int d_max_retry_count;
  const long *d_poll_interval;
  long d_poll_interval_retry_client;

  friend class MGSG_LicenseHandlerBase;
  enum exceptionHandlerMode { activateExceptionHandler, deactivateExceptionHandler };

  void validateProcess(exceptionHandlerMode m); // callback

protected:
  static void validateProcess(void *clientData)
  { ((LostExceptionHandler *)clientData)->validateProcess(activateExceptionHandler); }

public:
  LostExceptionHandler() 
    : d_handler(0)
    , d_exception(exception_init)
    , d_poll_interval_check(30*1000)
    , d_poll_interval_retry(d_poll_interval_check)
    , d_max_retry_count(5)
    , d_poll_interval(&d_poll_interval_retry)
    , d_poll_interval_retry_client(d_poll_interval_retry)
  { }
  virtual ~LostExceptionHandler() { }

  void unregisterHandler(MGSG_LicenseHandlerBase &handler)
  {
    if (d_handler) {
      validateProcess(deactivateExceptionHandler);
      d_handler->d_exception = 0;
    }
  }

  virtual void unsheduleValidate() = 0; // schedule call back to validateProcess, "this" is passed as param 
  virtual void scheduleValidate(int msecs) = 0; // remove any future scheduled events

  virtual int featuresLost(int num_features_lost, int *cb_retry_count, long int *cb_retry_interval) = 0;

  virtual void waitFor(int msecs) = 0; // in exception state -- unlicensed

  virtual void cannotConfigure(int id) = 0; // notification -- fatal type
  virtual void reaquireSuccessful() = 0;  // notification
  virtual void reaquireAttempt(const char *server, const char *granted_feature) = 0; // notification
  virtual void reaquireAttempt() = 0; // notification
  virtual void reaquireFailed() = 0; // notification
  virtual void reaquireFatal() = 0; // notification
  virtual void communicationFailed() = 0; // notification
};

//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////

class MGSG_PKGS_API MGSG_MGLS_pid {
	int                      d_pidnum;
	const char              *d_pidname;

	int                      d_flags;

	bool                     d_valid;             // used to cache valid check.
	bool                     d_valid_set;         // indicates valid has been checked.
	MGSG_LicenseMessages    *d_valid_msgs;        // any messages from check

	class Texpire;
	Texpire                 *d_expire;            // software feature expiration time

	MGSG_LicenseHandlerBase *d_handler;           // used to unregister if destructed
    MGSG_LicenseHandlerBase::Transactions *d_transactions;
    MGSG_LicenseHandlerBase::PidPool_Reference *d_registration;

	class CheckoutFailedCache;
	CheckoutFailedCache *d_checkout_failed;

	friend class MGSG_LicenseHandlerBase;
	friend class MGSG_LicenseHandlerBase::Transactions;
	friend class MGSG_LicenseHandlerBase::TransactionPool;
	friend class MGSG_LicenseHandlerBase::PidPool;

    ////////////////////////////////////////////////////////////////////////////
	// MGSG_LicenseHandlerBase accesses these functions to register the pid 
	// with the handler
	////////////////////////////////////////////////////////////////////////////
	bool Register(MGSG_LicenseHandlerBase *handler, int pid, int flags,
				  unsigned int expire_secs, int warn_days, const MGSG_MGLS_pid *unexpire,
				  int alternate_pid);
	bool UnRegister(MGSG_LicenseHandlerBase *handler);

	bool hard_expired(MGSG_LicenseMessages *messages=0) const;
	bool get_hard_expire(STDCOLON string *str) const;
	bool get_hard_unexpire(STDCOLON string *str) const;

	bool is_disabled(MGSG_LicenseMessages *messages, int flags) const;
	bool is_valid(MGSG_LicenseMessages *messages, bool recheck) const;

	bool SetDefaults() const { return MGSG_LicenseHandlerBase::SetHandler::Defaults(d_handler); }

    MGSG_LicenseHandlerBase::Core &core() const { return *(d_handler ? d_handler->d_core : 0); }

public:
    MGSG_MGLS_pid() : d_pidnum(0), d_pidname(0), d_flags(0), 
                      d_valid_set(false), d_valid_msgs(0), d_expire(0),
                      d_handler(0), d_transactions(0), d_registration(0), d_checkout_failed(0)
                      { }
	virtual ~MGSG_MGLS_pid();

	int get_pid() const { return d_pidnum; }
	int get_feature() const { return get_pid(); }
	const char *get_name() const;
	bool get_name(STDCOLON string *name) const // OBSOLETE use get_name()
    { *name = get_name(); return true; }
    int get_flags() const { return d_flags; }
	bool is_library_id() const;
	bool is_valid(bool recheck=false) const { return is_valid(0, recheck); }
	bool is_valid(MGSG_LicenseMessages &messages, bool recheck=false) const 
	{ return is_valid(&messages, recheck); }
	bool is_disabled() const; // disabled primary and alterates
    bool is_disabled_primary() const; // only primary is disabled, alternates still can be checked
    bool is_disabled(MGSG_LicenseMessages &messages) const 
    { return is_disabled(&messages, d_handler ? d_handler->flag_pidDisable : 0); }
    bool is_disabled_primary(MGSG_LicenseMessages &messages) const 
    { return is_disabled(&messages, d_handler ? d_handler->flag_pidDisableP : 0); }
	bool is_expired() const { return hard_expired(); }
	bool is_expired(MGSG_LicenseMessages &messages) const { return hard_expired(&messages); }
	void disable();
    void disable_primary(); // disable able access to primary feature, alternates okay

    operator int () const { return get_pid(); }

	bool is_registered() const { return !! d_handler; }
	bool is_active() const;
	bool is_handled(const MGSG_LicenseHandlerBase *handler) const 
    { return d_handler ? (d_handler == handler) : false; }

	// license which last satisfied request
	int get_actual_id() const; 
	const char *get_actual_name() const;
	bool is_actual_special_id() const;
	bool is_actual_an_alternate() const;

	unsigned int get_actuals() const;
	unsigned int get_duplicates() const;
	unsigned int get_checkedout() const;
	MGSG_License_Type get_type() const;
	const char *get_type_str() const;
	const MGSG_LicenseHandlerBase::SerialNums &get_serialnums() const;
	const char *get_serialnums_str() const;
	bool checkout(MGSG_LicenseMessages &messages, bool duplicate = false);
	bool checkin(MGSG_LicenseMessages &messages, bool duplicate = false);
	bool allocate_one(MGSG_LicenseMessages &messages);
	bool allocate_return(MGSG_LicenseMessages &messages, bool duplicate = false) const;
	bool CLIM_hash(STDCOLON string *feature_hash, unsigned int valid_secs=30) const;

	// Iterator to iterate over all of the contained licenses (i.e. atomics in a composite)
    typedef MGSG_LicenseHandlerBase::contains_iterator contains_iterator;
};

// OBSOLETE class use MGSG_LicenseHandlerBase::pids_iterator
typedef MGSG_PKGS_API STDCOLON list<MGSG_MGLS_pid *> list_MGSG_MGLS_pid;
class MGSG_PKGS_API MGSG_MGLS_pids : public list_MGSG_MGLS_pid {
	bool d_owner;
public:
	MGSG_MGLS_pids(bool owner = true) : list_MGSG_MGLS_pid(), d_owner(owner) { }
	virtual ~MGSG_MGLS_pids() { if (d_owner) while(size()) { delete *rbegin(); pop_back(); } }
	unsigned int add(MGSG_MGLS_pid &pid);
};

#if defined(_MSC_VER) && !defined(DEBUG)
# pragma warning( pop )  // Restore warning state
#endif

#endif /* cplusplus */

/* compile time cipher implementation for feature name */
enum { MGSG_CRYPT_offset_00 = 5,
       MGSG_CRYPT_offset_01 = 12,
       MGSG_CRYPT_offset_02 = 7,
       MGSG_CRYPT_offset_03 = 14,
       MGSG_CRYPT_offset_04 = 22,
       MGSG_CRYPT_offset_05 = 10,
       MGSG_CRYPT_offset_06 = 28,
       MGSG_CRYPT_offset_07 = 4,
       MGSG_CRYPT_offset_08 = 19,
       MGSG_CRYPT_offset_09 = 12,
       MGSG_CRYPT_offset_10 = 22,
       MGSG_CRYPT_offset_11 = 29,
       MGSG_CRYPT_offset_12 = 10,
       MGSG_CRYPT_offset_13 = 18,
       MGSG_CRYPT_offset_14 = 13,
       MGSG_CRYPT_offset_15 = 31,
       MGSG_CRYPT_offset_16 = 2,
       MGSG_CRYPT_offset_17 = 15,
       MGSG_CRYPT_offset_18 = 23,
       MGSG_CRYPT_offset_19 = 26,
       MGSG_CRYPT_offset_20 = 17,
       MGSG_CRYPT_offset_21 = 6,
       MGSG_CRYPT_offset_22 = 18,
       MGSG_CRYPT_offset_23 = 24,
       MGSG_CRYPT_offset_24 = 11,
       MGSG_CRYPT_offset_25 = 25,
       MGSG_CRYPT_offset_26 = 23,
       MGSG_CRYPT_offset_27 = 12,
       MGSG_CRYPT_offset_28 = 8,
       MGSG_CRYPT_offset_29 = 10,
       MGSG_CRYPT_offset_30 = 14,
       MGSG_CRYPT_offset_31 = 28,
       MGSG_CRYPT_offset_32 = 52,
       MGSG_CRYPT_buf_sz = 33 };

#define MGSG_CRYPT_(str) \
    (str)[ 0] + MGSG_CRYPT_offset_00, \
    (str)[ 1] + MGSG_CRYPT_offset_01, \
    (str)[ 2] + MGSG_CRYPT_offset_02, \
    (str)[ 3] + MGSG_CRYPT_offset_03, \
    (str)[ 4] + MGSG_CRYPT_offset_04, \
    (str)[ 5] + MGSG_CRYPT_offset_05, \
    (str)[ 6] + MGSG_CRYPT_offset_06, \
    (str)[ 7] + MGSG_CRYPT_offset_07, \
    (str)[ 8] + MGSG_CRYPT_offset_08, \
    (str)[ 9] + MGSG_CRYPT_offset_09, \
    (str)[10] + MGSG_CRYPT_offset_10, \
    (str)[11] + MGSG_CRYPT_offset_11, \
    (str)[12] + MGSG_CRYPT_offset_12, \
    (str)[13] + MGSG_CRYPT_offset_13, \
    (str)[14] + MGSG_CRYPT_offset_14, \
    (str)[15] + MGSG_CRYPT_offset_15, \
    (str)[16] + MGSG_CRYPT_offset_16, \
    (str)[17] + MGSG_CRYPT_offset_17, \
    (str)[18] + MGSG_CRYPT_offset_18, \
    (str)[19] + MGSG_CRYPT_offset_19, \
    (str)[20] + MGSG_CRYPT_offset_20, \
    (str)[21] + MGSG_CRYPT_offset_21, \
    (str)[22] + MGSG_CRYPT_offset_22, \
    (str)[23] + MGSG_CRYPT_offset_23, \
    (str)[24] + MGSG_CRYPT_offset_24, \
    (str)[25] + MGSG_CRYPT_offset_25, \
    (str)[26] + MGSG_CRYPT_offset_26, \
    (str)[27] + MGSG_CRYPT_offset_27, \
    (str)[28] + MGSG_CRYPT_offset_28, \
    (str)[29] + MGSG_CRYPT_offset_29, \
    (str)[30] + MGSG_CRYPT_offset_30, \
    (str)[31] + MGSG_CRYPT_offset_31, \
                MGSG_CRYPT_offset_32

#endif
