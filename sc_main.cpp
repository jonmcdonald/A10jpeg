//
// sc_main.cpp:  Top level of the Vista design.
//               Instantiate schematic
//               Initiate Connection to FabricServer (if used)
//

#if (USE_VISTA_PREDICTOR == 1) && (USE_VISTA_STIMULUS == 0) 
    // For in UVMF, Vista predictor only
    #define TOP_MODULE top_predictor
    #include "top_predictor.h"
    #warning "sc_main: Vista compiled for predictor in UVMF simulation"
#elif (USE_VISTA_PREDICTOR == 0) && (USE_VISTA_STIMULUS == 1) 
    // For in UVMF, Vista stimulus only
    #define TOP_MODULE top_stimulus
    #include "top_stimulus.h"
    #warning "sc_main: Vista compiled for stimulus in UVMF simulation"
#elif (USE_VISTA_PREDICTOR == 1) && (USE_VISTA_STIMULUS == 1) 
    // For in UVMF, Vista stimulus and predictor 
    #define TOP_MODULE top_stimulus_predictor
    #include "top_stimulus_predictor.h"
    #warning "sc_main: Vista compiled for stimulus and predictor in UVMF simulation"
#else
    // Run Vista Standalone
    #define TOP_MODULE top
    #include "top.h"
    #warning "sc_main: Vista compiled for standalone simulation"
#endif

// Note: XL_TLM_REMOTE_CLIENT is set when Vista is NOT in Questa
//       It is required for UVM Connect dual process source code 
#ifdef XL_TLM_REMOTE_CLIENT
  #include <arpa/inet.h>
  #include "XlRemoteTlmConduitPkg.h"
  //using namespace XlTlm;
#elif (defined UVMC_FABRICSERVER_SRC)
  // bring in fabric server code in case UVMC conduit proxies are there (e.g. nbMemoryDriver)
  //#include ".../project_benches/*/tb/testbench/FabricServer.h"
  #include STRINGIFY(UVMC_FABRICSERVER_SRC)
#endif



static int readArgs(
     int argc, char *argv[]
    ,bool   &isSet_quantumSyncIntervalNs
    ,int    &quantumSyncIntervalNs
    ,bool   &isSet_quantumSyncRatio
    ,int    &quantumSyncRatio
    ,string &serverUrl
    ,bool   &isShutdown
    ,bool   &keepServerAlive 
    )
{   
    int i, usage = 0;
    char argName[BUFSIZ];
    char argVal[BUFSIZ];

    // Set up default values.
    quantumSyncIntervalNs       = 0xdeadbeef;  
    quantumSyncRatio            = 0xdeadbeef;
    serverUrl                   = "localhost";
    isShutdown                  = false;
    keepServerAlive             = false;
    isSet_quantumSyncIntervalNs = false;
    isSet_quantumSyncRatio      = false;

    printf("sc_main args: ");
    for( i=1; i<argc; i++ )
    {
        printf("%s ",argv[i]);
        sscanf( argv[i], "--%[^=]=%s", argName, argVal );
        string arg = argName;

        if( arg == "quantum-sync-interval" ) {
            sscanf( argVal, " %d", &quantumSyncIntervalNs );
            isSet_quantumSyncIntervalNs = true;

        } else if( arg == "quantum-sync-ratio" ) {
            sscanf( argVal, " %d", &quantumSyncRatio );
            isSet_quantumSyncRatio = true;

        } else if( arg == "server-url" ) {
            serverUrl = argVal; 

        } else if( arg == "shutdown" ) {
            if( string(argVal) == "yes" ) 
                isShutdown = true;
            else if( string(argVal) == "no" ) 
                isShutdown = false;

        } else if( arg == "keep-server-alive" ) {
            if( string(argVal) == "yes" ) 
                keepServerAlive = true;
            else if( string(argVal) == "no" ) 
                keepServerAlive = false;

        } else usage = 1;
    }
    printf("\n");

    if( usage ) {
        printf( "usage: run.x\n" );
        printf( "    --quantum-sync-interval=<# ns> (default: -1) |\n" );
        printf( "    --quantum-sync-ratio=<ratio>   (default:  1) |\n" );
        printf( "    --server-url=<server IP addr>  (default: localhost) |\n" );
        printf( "    --shutdown=yes|no              (default: no)\n" );
        printf( "    --keep-server-alive=yes|no     (default: no)\n" );
        printf( "\n  NOTE: quantum sync modes below:\n" );
        printf( "    quantum-sync-interval    quantum-sync-ratio\n"       );
        printf( "    (default=0)              (default=1)\n"              );
        printf( "    ---------------------    ------------------\n"       );
        printf( "     0                        X       PERF_EVENT_MODE\n" );
        printf( "    -1                       >0       HIFI_TIMED_MODE\n" );
        printf( "    >0                       >0       QUANT_SYNC_MODE\n" );
        printf( "    >0                        0       QUANT_SYNC_MODE - Adaptive\n" );

    }
    return( usage );
}

int sc_main( int argc, char *argv[] ) 
{

  #ifdef XL_TLM_REMOTE_CLIENT
    // If we're running this as a remote TLM fabric client, first connect to the remote server.

    int quantumSyncIntervalNs;
    int quantumSyncRatio;
    bool isSet_quantumSyncIntervalNs =false;
    bool isSet_quantumSyncRatio = false;
    string serverUrl;
    bool isShutdown, keepServerAlive; 
    unsigned vivy_disable = sdGetParameterAsUInt("vivy_disable");

    // set synchronization values
    // Currently command line options are only read if paramater does not exist in parameters.txt

    if (::mb::sysc::sdParameterExists("quantum-sync-interval")
        && ::mb::sysc::sdParameterExists("quantum-sync-ratio")) 
    {
        cout <<"Vista sc_main:  setting quantum-sync-interval and quantum-sync-ratio from parameters.txt\n";
        quantumSyncIntervalNs = ::mb::sysc::sdGetParameterAsInt("quantum-sync-interval");
        isSet_quantumSyncIntervalNs = true;
    
        quantumSyncRatio = ::mb::sysc::sdGetParameterAsInt("quantum-sync-ratio");
        isSet_quantumSyncRatio = true;
    }
    else
    {   // if not in parameters.txt, use command line arguments.
        if( readArgs( argc, argv
                ,isSet_quantumSyncIntervalNs, quantumSyncIntervalNs 
                ,isSet_quantumSyncRatio      ,quantumSyncRatio
                ,serverUrl, isShutdown, keepServerAlive ) )
            return -1;
    }


    if ( ! vivy_disable)
    {
        cout <<"sc_main:  Running VISTA_HYBRID configuration"<<endl;
        // Note: As long as it is not necessary to go over the network
        // to the remote client, you'll find AF_UNIX can be significantly
        // better performing than AF_INET with 'localhost'.
        //XlTlm::XlRemoteTlmConduit::setDomain( AF_UNIX );
    
        // Choose a hopefully unused port in the "ephemeral" port range, 49152-65535.
        // Note: not required for AF_UNIX (if used must match uvmc/src/FabricServer.h)
        XlTlm::XlRemoteTlmConduit::setDomain( AF_INET );
        XlTlm::XlRemoteTlmConduit::setPortNum( 50102 );
    
        // First stablish connection remote TLM fabric server ...
        cout <<"sc_main: Attempt connection to server\n";
        XlTlm::XlRemoteTlmConduit::connectToServer( "remoteTestbench", serverUrl.c_str() );
    
        if( isShutdown == true ) {
             // Shut down server and leave. 
            cout <<"sc_main:  No simulation.  Disconnecting from Questa/Server\n";
            XlTlm::XlRemoteTlmConduit::disconnectFromServer( /*isKeepServerAlive=*/false );
            return 0;
        }
  
        // set synchronization
        printf( "Vista sc_main: Connected to TLM fabric server!  QuantumSyncIntervalNs= ");
        if (isSet_quantumSyncIntervalNs) 
            printf("%d",quantumSyncIntervalNs);
        else 
            printf(" default");
        
        printf(" quantumSyncRatio= ");
        if (isSet_quantumSyncRatio) 
            printf("%d\n",quantumSyncRatio);
        else 
            printf(" default\n");

        if (isSet_quantumSyncIntervalNs) 
            XlTlm::XlRemoteTlmConduit::setQuantumSyncIntervalNs( quantumSyncIntervalNs );
        
        if (isSet_quantumSyncRatio) 
            XlTlm::XlRemoteTlmConduit::setQuantumSyncRatio( quantumSyncRatio );
    }
  #endif  // XL_TLM_REMOTE_CLIENT

    TOP_MODULE*     top_i          = new TOP_MODULE("top");
    #if (!defined XL_TLM_REMOTE_CLIENT && defined UVMC_FABRICSERVER_SRC)
      FabricServer* FabricServer_i = new FabricServer("FabricServer");
    #endif  
   
    // to avoid Questa warning: Warning: (vsim-6667) The declared name and constructor argument for instance 'sc_main.top' must be different
    // make pointer to top and constructor arg the same.  But, this means top level module cannot also have the same name.
    // (In Vista the top module is the same as the filename; so "top.h" makes TOP_MODULE="top" which won't compile.
    //TOP_MODULE *top = new TOP_MODULE("top");

  #ifdef XL_TLM_REMOTE_CLIENT
    if ( ! vivy_disable)
    {
        // if( we're running this as a remote TLM fabric client )
        //     We replace sc_start() with the special TLM fabric sim control
        //     loop for coordinated time sync between client and server,
        XlTlm::XlRemoteTlmConduit::scStart();

        // Disconnect from the TLM fabric server at this point.
        if (keepServerAlive)
            cout <<"sc_main:  simulation complete.  Stay connected to Questa/Server\n";
        else
            cout <<"sc_main:  Simulation complete.  Disconnecting from Questa/Server\n";

        XlTlm::XlRemoteTlmConduit::disconnectFromServer(keepServerAlive);
    }

    else 
  #endif  // XL_TLM_REMOTE_CLIENT
    {
        cout <<" sc_main(): sc_start() started \n";
        sc_start();
        cout <<" sc_main(): sc_start() completed \n";
        if (sc_is_running())
            sc_stop();
    }

    delete top_i;

    return 0;
}
