#!/bin/bash

purge() {
    if [ -d "$path" ]; then
        echo "Deleting $path..."
        rm -rf "$path"
    else
        echo "Deleting $(basename "path")..."
        rm "$path"
    fi
    echo "Purge complete."
}

listStopped() {
    grep "Monitoring stopped for" "$path" | awk -F'for: ' '{print $2}' | while read -r dir; do
        stoppedDir=$(grep "Added directory: $dir" "$path" | tail -1 | awk -F'directory: ' '{print $2}')
        echo "$stoppedDir"
    done
}


while getopts p:c: option
do
    case "${option}" in
        p) path=$OPTARG ;;
        c) command=$OPTARG ;;
    esac
done

if [ "$command" = "purge" ]; then
    purge
else [ "$command" = "listStopped" ];
    listStopped

fi
