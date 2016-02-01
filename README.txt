
    This is the base Vista platform which has two purposes:
        1) Show features of Vista platforms such as Ethernet virtualization and booting Linux
        2) Show usage of Vista models/schematics in RTL Verification, mixed simulation

    In this directory, you can run Vista standalone, or the SystemC compiled for Questa.
     
-------------------------------------
=== To compile and run the design ===
-------------------------------------

    First source setup file, then compile:

        % . ../../setup   # "bash" shell only !
        % make

    To Simulate in the Vista GUI:

        Select the Cortex-A9_Dual-Core project, and build it.  It will compile sc_main.cpp, and link
        the result with the .a's from the above make.  Open the project, Simulation->sc_main.cpp, right
        click and select simulate (or use simulate button, or simulate menu, after sc_main.cpp is selected 
        in the project).
        
    To Simulate from the Linux command line:

        % ./run.x   # generated from above make

    To Simulate in Questa by creating a linkable .a file with g++ and linking with "vista_sccom":

        % make archive_questa_link   # or "make aql" for short

        "run" options:

        run -v -w work_aql     # Questa GUI with Vista GUI
        run -i -w work_aql     # Questa GUI only
        run -c -w work_aql     # Questa batch/command line mode

    To Simulate in Questa by compiling all code with "vista_sccom"

        % make sccom   

        "run" options:

        run -i -w work_sccom   # Questa GUI only
        run -c -w work_sccom   # Questa batch/command line mode
        # Note: cannot run with Vista GUI when sccom is used

-------------------------
=== File descriptions ===
-------------------------

        Makefile               # Makefile to compile Vista models and schematics
        run.x                  # executable for Vista command line run

        objdir/                # objects for Vista simulation
        objdir_questa/         # objects for archive_questa_link simulation
        work_aql/              # Questa work directory for archive_questa_link
        work_sccom/            # Questa work directory vista_sccom compilation
        run*                   # run simulation script for Questa

        parameters.txt         # Vista model parameters
        generated_fractal.jpg  # JPEG file generated from image_data/*.bmp file in simulation

        Questa do files:
            sim.do
        Vista generated files/directories:
            vista.ini
            vista_modelsim.ini
            vista_power_transcript
            vista_work/
            vista_wave.wave
            papoulis_project/
            sim/


------------------------------------
===  ARM BareMetal demo software ===
------------------------------------

    Currently, only the "baremetal software" is shipped to excersize the hardware.  
    Linux software is quite large and in transition, contact MGC for info.

    Existing "baremetal software" will present the user with a menu on the virtual console.
    Note that characters typed into the console are not echo'd to the screen (a defect in the 
    baremetal UART driver).  

    For baremetal menu choices 2 and 3, the generated JPEG file is placed in the "image_data" directory
    A new menu is output to the screen when the JPEG is complete, but this may not be obvious if 
    several JPEG runs are done, as the console output will settle to a steady state.




--------------------------------
=== Linux RTOS demo software ===
--------------------------------

Scripts are available to automate the manual steps outlined below:

    telnet_target      # telnet to Vista Virtual Platform
    tftp_jpeg_program  # tftp new jpeg program to Vista Virtual Platform


transfer files:

1) start "inetd" on target.
    % inetd

2) run tftp on host
    % tftp localhost 2169
    binary
    verbose
    put jpeg_program


telnet:

1) start "telnetd" on target.
2) telnet localhost 2123

Note: vista window, messages are given for the port numbers:

    VLAN: setting host: 10.0.2.2
    VLAN: setting hostname: host
    VLAN: setting dhcpstart: 10.0.2.15
    VLAN: setting dns: 10.0.2.3
    VLAN: setting net: 10.0.2.0
    VLAN: udp_napt forward: :2169 => :69 ... Listening.
    VLAN: tcp_napt forward: :2122 => :22 ... Listening.
    VLAN: tcp_napt forward: :2123 => :23 ... Listening.
    VLAN: eth0 setting MAC address: 52:54:00:12:34:56

jpeg program:

1) start drivers in target:
    % ./jpeg_load  # jpeg
    % ./guc_load   # MPG
    % ./fbinit     # LCD driver and window

2) Convert bitmaps to jpeg
    % ./jpeg_program fractal_800x600.bmp  f.jpg
    % ./jpeg_program ShanaTova800x600.bmp a.jpg

3) show results in viewer
    % show.sh 1   # arg is a count

*) Or use "run" script to do above

Debug Software:

1) start gdbserver
    % gdbserver 0.0.0.0:23  ./jpeg_program fractal_800x600.bmp f.jpg

2) start codebench for arm linux
    * sourcerygxx-ide -beta &    # (cbal alias)
    * select workspace, etc. ("workspace" should already be here), if not:
    * create project, files from "Building_Linux/Linux_Build_resources/sysroot/root/jpeg_program"
      (use Import to get existing files into project)

    * Debugger:
        “Debugger” tab - “Debug Interface” select: “Sourcery CodeBench External Server”
        Type: TCP, IP Address: "localhost" , Port Number: 2123 (as defined in the vlan parameter.txt)





