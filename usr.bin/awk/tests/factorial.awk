# Calculate factorial of command-line argument

BEGIN {
   # Get the command-line argument
   if (ARGC < 2)   {
      print "ERROR: please provide number as command-line parameter"
      exit 1
      }
   n = ARGV[1]; ARGV[1] = "";
   # Print heading
   print "Calculating",n,"factorial"
   # Start the recursion
   nf = factorial(n);
   # All done; print the result
   print "Result:",nf;
}

# Recursive function
function factorial(n) {
   if (n <= 1)   return 1;
   else          return n*factorial(n-1);
   }
