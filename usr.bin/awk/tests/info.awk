BEGIN { FS = "\t"; pat = ARGV[1]; ARGV[1] = "-" }
$1 ~ pat {
    printf("%s:\n", $1)
    printf("\t%d million people\n", $3)
    printf("\t%.3f million sq. mi.\n", $2/1000)
    printf("\t%.1f people per sq. mi.\n", 1000*$3/$2)
}
