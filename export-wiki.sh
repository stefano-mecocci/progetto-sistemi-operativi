#!/bin/bash

#required packages: pandoc, texlive-xetex


#assuming cloned wiki repo in the parent folder
PROJECT=$(pwd)
REPO=$(basename $PROJECT)
WIKIREPO="$REPO.wiki"
FILES="Home.md;Richieste-di-corse.md;Taxi.md;Generazione-Report.md"
IFS=';' read -r -a array <<< "$FILES"
PANDOCINPUT="./README.md"
for element in "${array[@]}"
do
    PANDOCINPUT="$PANDOCINPUT ../$WIKIREPO/$element"
done
pandoc $PANDOCINPUT --pdf-engine=xelatex -s -o "./out/The taxicab game.pdf"

