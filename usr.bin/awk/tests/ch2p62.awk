$1 == "#include" { gsub(/"/, "", $2); system("cat " $2); next }
                 { print }
