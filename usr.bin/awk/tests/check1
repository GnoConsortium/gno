# check1 - print total deposits and checks

/^check/   { ck = 1; next }
/^deposit/ { dep = 1; next }
/^amount/  { amt = $2; next }
/^$/       { addup() }

END        { addup()
             printf("deposits $%.2f, checks $%.2f\n",
                 deposits, checks)
           }

function addup() {
    if (ck)
        checks += amt
    else if (dep)
        deposits += amt
    ck = dep = amt = 0
}
