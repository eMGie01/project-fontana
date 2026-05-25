# Module Cli

i need to update it as fast as possible, no logs for now, but getting info from command-line and check, wheter it works with tasks meas api...
also i need to close the architecture,
i wanted it to look smth like this:
	- module cli has as only one access to uart0,
	- it is responsible of command-line interface and logging information,
	- loggs has its own api, that can be globally known for every module,
	- i dont know how to connect it to modules api's, i belive it should be by handlers... the question is: should the handlers be inside cli code, or rather outside, and only shot into the cli?

module-measurement:
	- measurement.cpp		// class code
	- measurement.hpp		// class header
	- meas_task_api.h		// api with enum for commands, and cmd struct
	- meas_task.cpp			// runtime application of measurement process and class variables manipulator
	- meas_task.hpp			// meas_task header
	
module-cli:
	- cli.cpp				// class code
	- cli.hpp				// class header
	- iostream.hpp			// class for cli input/output device interface (read/write)
	- uart_stream.hpp		// specific inherited from iostream class (uart source)
	- cli_meas_handles.cpp	// contains handles for meas_task api
	- cli_meas_handles.hpp	// header for handler for meas_task api