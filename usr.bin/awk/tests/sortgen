# sortgen - generate sort command
#   input:  sequence of lines describing sorting options
#   output: Unix sort command with appropriate arguments

BEGIN { key = 0 }

/no |not |n't / { print "error: can't do negatives:", $0; ok = 1 }

# rules for global options
        { ok = 0 }
/uniq|discard.*(iden|dupl)/  { uniq = " -u"; ok = 1 }
/separ.*tab|tab.*sep/        { sep = "t'\t'"; ok = 1 }
/separ/ { for (i = 1; i <= NF; i++)
              if (length($i) == 1)
                  sep = "t'" $i "'"
          ok = 1
        }
/key/   { key++; dokey(); ok = 1 } # new key; must come in order

# rules for each key

/dict/                            { dict[key] = "d"; ok = 1 }
/ignore.*(space|blank)/           { blank[key] = "b"; ok = 1 }
/fold|case/                       { fold[key] = "f"; ok = 1 }
/num/                             { num[key] = "n"; ok = 1 }
/rev|descend|decreas|down|oppos/  { rev[key] = "r"; ok = 1 }
/forward|ascend|increas|up|alpha/ { next }    # this is sort's default
!ok   { print "error: can't understand:", $0 }

END {                        # print flags for each key
    cmd = "sort" uniq
    flag = dict[0] blank[0] fold[0] rev[0] num[0] sep
    if (flag) cmd = cmd " -" flag
    for (i = 1; i <= key; i++)
        if (pos[i] != "") {
            flag = pos[i] dict[i] blank[i] fold[i] rev[i] num[i]
            if (flag) cmd = cmd " +" flag
            if (pos2[i]) cmd = cmd " -" pos2[i]
        }
    print cmd
}

function dokey(    i) {      # determine position of key
    for (i = 1; i <= NF; i++)
        if ($i ~ /^[0-9]+$/) {
            pos[key] = $i - 1    # sort uses 0-origin
            break
        }
    for (i++; i <= NF; i++)
        if ($i ~ /^[0-9]+$/) {
            pos2[key] = $i
            break
        }
    if (pos[key] == "")
        printf("error: invalid key specification: %s\n", $0)
    if (pos2[key] == "")
        pos2[key] = pos[key] + 1
}
