# awk program to emulate the wc command
{
	# Has input file changed?
	if (FILENAME != file) {
		if (startup == 0) {
                        #  This is just the first time through
			startup = 1;
		}
		else {
			printf " %7d %7d %7d %s\n", nl, nw, nc, file;
			tnl += nl;  tnw += nw;  tnc += nc;
			nl = 0;  nw = 0;  nc = 0;
		}
		file = FILENAME;
		numfiles++
	}
	nl++;  nw += NF;  nc += length($0)+1;
}

END   {
	printf " %7d %7d %7d %s\n", nl, nw, nc, file;
	if (numfiles > 1) {
		tnl += nl;  tnw += nw;  tnc += nc;
		printf " %7d %7d %7d total\n", tnl, tnw, tnc;
	}
}                        
