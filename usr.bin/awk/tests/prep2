# prep2 - prepare countries by continent, inverse pop. den.

BEGIN { FS = "\t"}
      { den = 1000*$3/$2
        printf("%-15s:%12.8f:%s:%d:%d:%.1f\n",
            $4, 1/den, $1, $3, $2, den) | "sort"
      }
