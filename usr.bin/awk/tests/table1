# table1 - single column formatter
#   input:  one column of strings and decimal numbers
#   output: aligned column

BEGIN {
    blanks = sprintf("%100s", " ")
    number = "^[+-]?([0-9]+[.]?[0-9]*|[.][0-9]+)$"
    left = "^[+-]?[0-9]*"
    right = "[.][0-9]*"
}

{   row[NR] = $1
    if ($1 ~ number) {
        match($1, left) # matches the empty string, so RLENGTH>=0
        lwid = max(lwid, RLENGTH)
        if (!match($1, right))
            RLENGTH = 0
        rwid = max(rwid, RLENGTH)
        wid = max(wid, lwid + rwid)
    } else
        wid = max(wid, length($1))
}

END {
    for (r = 1; r <= NR; r++) {
        if (row[r] ~ number)
            printf("%" wid "s\n", numjust(row[r]))
        else
            printf("%-" wid "s\n", row[r])
    }
}

function max(x, y) { return (x > y) ? x : y }

function numjust(s) {   # position s
    if (!match(s, right))
        RLENGTH = 0
    return s substr(blanks, 1, int(rwid-RLENGTH+(wid-(lwid+rwid))/2))
}
