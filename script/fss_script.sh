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

while getopts p:c: option
do
    case "${option}" in
        p) path=$OPTARG ;;
        c) command=$OPTARG ;;
    esac
done

if [ "$command" = "purge" ]; then
    purge
fi
