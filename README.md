# stcsp
Stream Constraint Satisfaction Problem solver. 

To make the executable, simply "make" in the directory, resulting in ./stcsp as the executable. Compilation requires working versions of lex and yacc.

Syntax:

stcsp [options] input_filename

Option flags:

[-s] Outputs the solution automaton to "solution.dot" in the same directory.
[-m"s"] Imposes a limit of "s" seconds to the solver.
[-t] Runs the test mode, sampling the solve time until convergence. Outputs log file to stdout. For very small cases (for example run time < 0.01s) it is quite in order for the solver to be run for hundreds of thousands of times, that it looks like it has crashed.
[-a] Runs the adversarial checking, by default the adversarial variable is the 6th variable in the code (will be changed later).
Example:

./stcsp -a -m3600 -s ./test.csp

imposes a 1 hour time limit of the test.csp test case, and outputs the solution automaton to "./solution.dot".

====

Solution automata are output in the DOT graph description language. There are plenty of software freely available to plot the automata from a .dot file. For example, Graphviz is available for Macs.