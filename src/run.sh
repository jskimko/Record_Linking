#!/usr/bin/env bash

###########################
#       READ CONFIG       #
###########################

echo "Reading configuration file... "
data1=`grep data1 config | awk -F'"' '{print $2}'`
data2=`grep data2 config | awk -F'"' '{print $2}'`
names1=`grep names1 config | awk -F'"' '{print $2}'`
names2=`grep names2 config | awk -F'"' '{print $2}'`
std_names=`grep std_names config | awk -F'"' '{print $2}'`
output=`grep output config | awk -F'"' '{print $2}'`
year1=`grep year1 config | awk -F'"' '{print $2}'`
year2=`grep year2 config | awk -F'"' '{print $2}'`
sex=`grep sex config | awk -F'"' '{print $2}'`
min_age1=`grep min_age1 config | awk -F'"' '{print $2}'`
max_age1=`grep max_age1 config | awk -F'"' '{print $2}'`
min_age2=`grep min_age2 config | awk -F'"' '{print $2}'`
max_age2=`grep max_age2 config | awk -F'"' '{print $2}'`

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
not_empty output    "$output"

#echo $data1; echo $data2; echo $names1; echo $names2; echo $std_names;
#echo $year1; echo $year2; echo $sex;
#echo $min_age1; echo $max_age1; echo $min_age2; echo $max_age2;


#####################
#       COMBINE     #
#####################

echo "Checking file paths..."
for f in "$data1" "$data2" "$names1" "$names2" "$std_names"; do
    if [ ! -f "$f" ]; then
        echo "run.sh:error: invalid path '$f'" >&2
        exit
    fi
done

echo "Reformatting files if necessary... "
function reformat {
    # Compress files
    echo -n "  Compressing $1... "
    timing=`{ time awk -F'\t' '{printf("%s;%s;%s;%s\n",$2,$42,$44,$69)}' \
                               "$1" > "$1.awk"; } 2>&1 | grep real`
    echo `echo $timing | awk '{print $2}'`

    echo -n "  Compressing $2... "
    timing=`{ time awk -F'\t' '{printf("%s;%s;%s\n"),$2,$4,$6}' \
                               "$2" > "$2.awk"; } 2>&1 | grep real`
    echo `echo $timing | awk '{print $2}'`

    # Sort data and names
    echo -n "  Sorting $1... "
    timing=`{ time tail -n+2 "$1.awk" | sort -t';' -sn > "$1.sort"; } 2>&1 | grep real`
    echo `echo $timing | awk '{print $2}'`
    rm -f "$1.awk"

    echo -n "  Sorting $2... "
    timing=`{ time tail -n+2 "$2.awk" | sort -t';' -sn > "$2.sort"; } 2>&1 | grep real`
    echo `echo $timing | awk '{print $2}'`
    rm -f "$2.awk"


    # Combine files
    echo -n "  Combining files... "
    timing=`{ time awk -F';' '{printf("%s;%s\n",$2,$3)}' "$2.sort" > "$2.tmp" &&
        paste -d';' "$1.sort" "$2.tmp" > "$1.paste"; } 2>&1 | grep real`
    echo `echo $timing | awk '{print $2}'`
    rm -f "$1.sort" "$2.sort" "$2.tmp"

    # Sorting by parish
    echo -n "  Sorting by parish... "
    timing=`{ time sort -t';' -sk4,4 "$1.paste" > "$1.par"; } 2>&1 | grep real`
    echo `echo $timing | awk '{print $2}'`
    rm -f "$1.paste"

    # Trim spaces
    echo -n "  Trimming spaces... "
    timing=`{ time tr -d ' ' < "$1.par" > "$1.tr"; } 2>&1 | grep real`
    echo `echo $timing | awk '{print $2}'`
    rm -f "$1.par"

    # Fill empty fields
    echo -n "  Filling empty fields... "
    timing=`{ time sed 's/;;/;.;/g' "$1.tr" > "$1.sed"; } 2>&1 | grep real`
    echo `echo $timing | awk '{print $2}'`
    rm -f "$1.tr"

    mv "$1.sed" "$1.in"
    echo "Generated $1.in"
}
if [ ! -f "$data1.in" ]; then
    reformat "$data1" "$names1"
fi
if [ ! -f "$data2.in" ]; then
    reformat "$data2" "$names2"
fi

# Sort std_names
if [ ! -f "$std_names.sort" ]; then
    echo -n "  Sorting $std_names... "
    timing=`{ time sort -T $HOME -sk1.1,1.1 "$std_names" > "$std_names.sort"; } 2>&1 | grep real`
    echo `echo $timing | awk '{printf $2}'`
fi

###############################
#       COMPILE AND RUN       #
###############################
args="$data1.in $data2.in"
args="$args $std_names.sort $year1 $year2 $sex"
args="$args $min_age1 $max_age1 $min_age2 $max_age2"
args="$args $output"
echo "Compiling..."
make
if [ ! $? -eq 0 ]; then
    echo "run.sh:error: could not compile" >&2
    exit
fi
echo "Running..."
echo "./record_linker $args"
echo "Start time is `date`"
echo "------------------------------------------"
./record_linker $args
echo "------------------------------------------"
if [ ! $? -eq 0 ]; then
    echo "run.sh:error: could not run" >&2
    exit
fi
echo "End time is `date`"
echo "Done."
