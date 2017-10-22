# CS457 - Project 2: Anonymous Web Get
## Jeremy Green

### Setup
run the make file in the code directory (containing makefile, ss.cpp, awget.cpp, awget.h).
  - $make clean 
    - removes all .o and executable files (<b>as well as any lingering index.html files!</b>)
  - $make all
    - compiles all executables (<b>awget</b> and <b>ss</b>)
  - $make <{ss} or {awget}>
    - compiles either <b>awget</b> or <b>ss</b> as specified
    
### Running the programs
- for a steppingStone run the ss executable with an optional port number.
  - $./ss [port number]
  - Note: if port number is not specified, the program default is port 9001
  - The output in the console will include ip and port, which should be used to construct the chainfile.
  
- for running the awget:
  - $./awget [requested url] [-c {chainfile}]
  - the name of the requested webpage must be included
  - the name of the chainfile is optional
    -default is "chaingaing.txt"
  
- chainfile format:
  - 4	
  - 129.82.45.59	20000
  - 129.82.47.209	25000
  - 129.82.47.223	30000	
  - 129.82.47.243	35000
  - ....
  
## Outstanding Issues
- Concurrency
  - The stepping stones can handle concurrent requests, however if two awgets are running which produce the same file name
  then there are some race condition issues. 
  - For example: 
    - if machine 1 requests: www.google.com and machine 2 requests: www.reddit.com
    - the program defaults the requested page output file to "index.html" for both of these sites, which causes issues.
    
## References and Documentation
- Large portions of the actual socket connection functions were based heavily on the examples provided at: http://beej.us/guide/bgnet/
- readFile() was based heavily on the example given at:  http://www.cplusplus.com/doc/tutorial/files/
- the option parsing code was based on the example given in the class resources

The source files for this program, as well as documentation, can be found at: https://github.com/jeremyverde/project2
