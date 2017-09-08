#!/usr/bin/env bash

if [ $# -ne 1 ]; then
    echo "usage: $0 config_file"
    exit
fi

# Read configuration file
data1=`grep data1 $1 | awk -F'"' '{print $2}'`
data2=`grep data2 $1 | awk -F'"' '{print $2}'`
names1=`grep names1 $1 | awk -F'"' '{print $2}'`
names2=`grep names2 $1 | awk -F'"' '{print $2}'`
std_names=`grep std_names $1 | awk -F'"' '{print $2}'`
year1=`grep year1 $1 | awk -F'"' '{print $2}'`
year2=`grep year2 $1 | awk -F'"' '{print $2}'`
sex=`grep sex $1 | awk -F'"' '{print $2}'`
min_age1=`grep min_age1 $1 | awk -F'"' '{print $2}'`
max_age1=`grep max_age1 $1 | awk -F'"' '{print $2}'`
min_age2=`grep min_age2 $1 | awk -F'"' '{print $2}'`
max_age2=`grep max_age2 $1 | awk -F'"' '{print $2}'`

# Check if we have everything
function check_valid {
    if [ -z "$2" ]; then
        echo "run.sh:error: invalid config entry '$1'" >&2
        exit
    fi
}
check_valid data1     $data1     ; check_valid data2    $data2;
check_valid names1    $names1    ; check_valid names2   $names2;
check_valid std_names $std_names ; check_valid sex      $sex;
check_valid year1     $year1     ; check_valid year2    $year2;
check_valid min_age1  $min_age1  ; check_valid max_age1 $max_age1;
check_valid min_age2  $min_age2  ; check_valid max_age2 $max_age2;

#echo $data1; echo $data2; echo $names1; echo $names2; echo $std_names;
#echo $year1; echo $year2; echo $sex;
#echo $min_age1; echo $max_age1; echo $min_age2; echo $max_age2;
