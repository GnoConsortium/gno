$3 > maxpop  { maxpop = $3; country = $1 }
END          { print "country with largest population:",
                   country, maxpop
             }
