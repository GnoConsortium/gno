BEGIN   { x = 0; y = 1 }

$1 > x  { if (x == y+1) {
              x = 1
              y = x * 2
          } else
              print x, z[x]
        }

NR > 1  { print $1 }

END     { print NR }

