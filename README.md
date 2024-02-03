A compiler for a custom language defined in a compiler class.

At the time of writing, only the lexer has been implemented.

Navigate to the build directory with "cd build" and run "cmake .." to generate the makefile (alternatively you could "run cmake--build ." in the build folder if there is already a makefile present). Then run "make" while in the build folder to build the project. The executable will be in the build directory. To run the executable, type "./compiler inputFileName.src". The program will generate inputFileName.outlextokens and inputFileName.outlexerrors. The first file will contain the tokens generated by the lexer and the second file will contain the errors generated by the lexer.
Note, this set of instructions will also create an executable, compiler_lexer_test, which can be run to run the associated tests with the lexer.
