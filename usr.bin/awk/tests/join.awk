# join - join file1 file2 on first field
#   input:  two sorted files, tab-separated fields
#   output: natural join of lines with common first field

BEGIN {
    OFS = sep = "\t"
    file2 = ARGV[2]
    ARGV[2] = ""  # read file1 implicitly, file2 explicitly
    eofstat = 1   # end of file status for file2
    if ((ng = getgroup()) <= 0)
        exit      # file2 is empty
}

{   while (prefix($0) > prefix(gp[1]))
        if ((ng = getgroup()) <= 0)
            exit  # file2 exhausted
    if (prefix($0) == prefix(gp[1]))  # 1st attributes in file1
        for (i = 1; i <= ng; i++)     #     and file2 match
            print $0, suffix(gp[i])   # print joined line
}

function getgroup() { # put equal prefix group into gp[1..ng]
    if (getone(file2, gp, 1) <= 0)    # end of file
        return 0
    for (ng = 2; getone(file2, gp, ng) > 0; ng++)
        if (prefix(gp[ng]) != prefix(gp[1])) {
            unget(gp[ng])    # went too far
            return ng-1
        }
    return ng-1
}

function getone(f, gp, n) {  # get next line in gp[n]
    if (eofstat <= 0) # eof or error has occurred
        return 0
    if (ungot) {      # return lookahead line if it exists
        gp[n] = ungotline
        ungot = 0
        return 1
    }
    return eofstat = (getline gp[n] <f)
}

function unget(s)  { ungotline = s; ungot = 1 }
function prefix(s) { return substr(s, 1, index(s, sep) - 1) }
function suffix(s) { return substr(s, index(s, sep) + 1) }
