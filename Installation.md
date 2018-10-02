## Installation and Execution

1. Run make. Ensure it creates an executable "mctool"
2. Run ./mctool <input_file>.txt



## Input File Format

Should contain a Kripke Structure defined by the following tags and some appending information. 
N.B: An example.txt is provided in repository.

* STATES #num_states

* ARCS followed by list of arcs separated by semi-colon

* LABELS on each state to signify the properties that hold on that state

* CTL followed by list of CTL properties you want to check on the structure and their correspoding notation to print the list of states if the property holds true. 

![An example Kripke Structure](https://en.wikipedia.org/wiki/Kripke_structure_(model_checking)#/media/File:KripkeStructureExample.svg)

## Output Format

List of states where each proposition listed under CTL holds true.
