
    { x[int($1/10)]++ }
END { max = MAXSTARS = 25
      for (i = 0; i <= 10; i++)
          if (x[i] > max)
              max = x[i]
      for (i = 0; i <= 10; i++)
          y[i] = x[i]/max * MAXSTARS
      for (i = 0; i < 10; i++)
          printf(" %2d - %2d: %3d %s\n",
              10*i, 10*i+9, x[i], rep(y[i],"*"))
      printf("100:      %3d %s\n", x[10], rep(y[10],"*"))
    }

function rep(n,s,   t) {  # return string of n s's
    while (n-- > 0)
        t = t s
    return t
}
