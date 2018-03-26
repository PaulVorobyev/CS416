#!/usr/bin/bash

#This script runs tests and packages asst2 into a tgz

asst2_proj_files=( "asst1/" "asst2/" "bin/" "common/" "readme.pdf" "Makefile" )

asst2 () {
    make clean
    for file in "${asst2_proj_files[@]}"
    do
        if [[ ! -e "$file" ]]; then
            echo -e "\e[31mWarning: $file not found before packaging\e[33m"
        fi
    done
    tar -cvzf Asst2.tgz $(ls -d ${asst2_proj_files[@]} 2>/dev/null)
}

"$@"
