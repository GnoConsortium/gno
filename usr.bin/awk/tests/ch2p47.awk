/Asia/   { pop["Asia"] += $3 }
/Europe/ { pop["Europe"] += $3 }
END      { print "Asian population is",
               pop["Asia"], "million."
           print "European population is",
               pop["Europe"], "million."
         }
