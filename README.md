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
  - $./awget [requested url] [-c {chainfile}] [-d]
  - the name of the requested webpage must be included
  - the name of the chainfile is optional
    -default is "chaingaing.txt"
  - the d option is used to display the page after successfully receiving it. Not including this will save the page, but it will not be displayed in the browser.  
  
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
- Memory
  - I haven't had time to ensure that all memory blocks are adequetly freed after use, therefore I would imagine that mutiple 
  runs (greater than 10) would begin to create issues. Therefore, I would recommend restarting the ss nodes every 10 runs or 
  so to ensure proper performance.
- Imperfect relays
  - Adding more nodes creates minor, yet noticeable defects in the webpages. Characters and elements are seemingly lost in translation, and therefore do not display perfectly when rendered on the awget node. The issue is compounded as more nodes are added, therefore I would assume that greater than 10 nodes would cause noticeable degradation in quality. It's a scalability issue that I just didn't have time to adress properly. 
    
## References and Documentation
- Large portions of the actual socket connection functions were based heavily on the examples provided at: http://beej.us/guide/bgnet/
- readFile() was based heavily on the example given at:  http://www.cplusplus.com/doc/tutorial/files/
- the option parsing code was based on the example given in the class resources

The source files for this program, as well as documentation, can be found at: https://github.com/jeremyverde/project2
