$4 == "Asia" { pop = pop + $3; n = n + 1 }
END          { print "Total population of the", n,
                   "Asian countries is", pop, "million."
             }
