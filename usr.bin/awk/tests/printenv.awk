
# Print environment variables' values; names passed on command line

BEGIN {
   if (ARGC < 2) {
      print "Error: provide environment variable name on command line"
      exit 1;
   }
   i = 1;
   do {
      rawname = ARGV[i];
      upname  = toupper(rawname);
      loname  = tolower(rawname);
      printf "%s = '%s'", rawname,ENVIRON[rawname];
      if (rawname != upname)
         printf "; %s = '%s'", upname,ENVIRON[upname];
      if (rawname != loname)
         printf "; %s = '%s'", loname,ENVIRON[loname];
      printf "\n"
      i++;
   } while (i < ARGC)

}
