# Read values into an array

$1 == "REMOVE" { delete value[$2]; next }

$1 == "RESET" {delete value; next}

$1 == "SKIP" {nextfile}

$1 == "PRINT" {printarray(value); next}

{ value[$1] = $2 }

END { printarray(value) }

function printarray(A,  key) {
  print "Values in array";
  for (key in A) print "  ",key,A[key];
  print " ";
}
