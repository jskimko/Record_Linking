#!/usr/bin/env bash

if [ $# -ne 1 ]; then
    echo "usage: $0 config_file"
    exit
fi

###########################
#       READ CONFIG       #
###########################

echo -n "Reading configuration file... "
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
function not_empty {
    if [ -z "$2" ]; then
        echo "run.sh:error: invalid config entry '$1'" >&2
        exit
    fi
}
not_empty data1     "$data1"     ; not_empty data2    "$data2";
not_empty names1    "$names1"    ; not_empty names2   "$names2";
not_empty std_names "$std_names" ; not_empty sex      "$sex";
not_empty year1     "$year1"     ; not_empty year2    "$year2";
not_empty min_age1  "$min_age1"  ; not_empty max_age1 "$max_age1";
not_empty min_age2  "$min_age2"  ; not_empty max_age2 "$max_age2";
echo "Done."

#echo $data1; echo $data2; echo $names1; echo $names2; echo $std_names;
#echo $year1; echo $year2; echo $sex;
#echo $min_age1; echo $max_age1; echo $min_age2; echo $max_age2;


#####################
#       SORT        #
#####################

# Sort data and name files
echo -n "Sorting files if necessary... "
for f in "$data1" "$data2" "$names1" "$names2"; do
    if [ ! -f "$f" ]; then
        echo "run.sh:error: invalid path '$f'" >&2
        exit
    fi

    if [ ! -f "$f.sort" ]; then
        path=`readlink -f "$f" | sed 's:\(.*\)/.*:\1:'`
        file=`readlink -f "$f" | sed 's:.*/\(.*\):\1:'`
        echo "Sorting $file..."
        time sort -T $HOME -snk2 "$f" > $path/$file.sort
    fi
done

# Sort std_names
if [ ! -f "$std_names" ]; then
    echo "run.sh:error: invalid path '$f'" >&2
    exit
fi

if [ ! -f "$f.sort" ]; then
    path=`readlink -f "$std_names" | sed 's:\(.*\)/.*:\1:'`
    file=`readlink -f "$std_names" | sed 's:.*/\(.*\):\1:'`
    echo "Sorting $file..."
    time sort -T $HOME -sk1.1,1.1 "$std_names" > $path/$file.sort
fi
echo "Done."

