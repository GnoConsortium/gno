# awk program to parse the output of "lseg -d <files>" and calculate
#    1. The total size of code in each object file
#    2. The name of any procedure that uses over "maxseg" bytes (default=100)

BEGIN {
	# If maxseg was not set on command line, use default
	if (maxseg <= 0)  maxseg = 100;
}

/ Code / {
	# Is this the beginning of a new file's statistics?
	if ($1 != filename)   {
		if (codesize > 0)   {
			sizes[filename] = codesize;
			}
		filename = $1;
		codesize = 0;
		}
	codesize += $3;
	# If there are > 4 fields, fourth one is the stack size
        if ($4 > maxseg) print $5," in file ", $1," uses ",$4," stack bytes"
}

END {
	# Print the report
        print "Code file         Code bytes"
	for (fname in sizes) {
        	printf "%s", fname;
                i = length(fname);
                while (i++ < 20) printf " ";
                printf " %6d\n", sizes[fname];
                }
}
