# print k distinct random integers between 1 and n

{ random($1, $2) }

function random(k, n,    A, i, r) {
    for (i = n-k+1; i <= n; i++)
        ((r = randint(i)) in A) ? A[i] : A[r]
    for (i in A)
        print i
}

function randint(n) { return int(n*rand())+1 }
