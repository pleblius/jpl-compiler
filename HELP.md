./jplc [flags] [filename]
Compiles the provided jpl source code into an executable file.

    Flags:
        -h  Print help options.
        -l  Performs lexical analysis. Prints all identified tokens.
        -p  Performs parse analysis. Prints all s-expressions.
        -t  Performs type-checking analysis. Prints s-expressions with associated types.
        -c  Transcribes jpl file to C code. Prints all created C code.
        -r  Compiles and runs jpl file, printing standard output.

        --no-print  Disables printing output.
        --pp-print  Pretty prints s-expressions. [NOT IMPLEMENTED]
        --tab-print Prints s-expressions with appropriate tabs and newlines. [NOT IMPLEMENTED]
        --xml-print Prints s-expressions as xml nodes. [NOT IMPLEMENTED]

