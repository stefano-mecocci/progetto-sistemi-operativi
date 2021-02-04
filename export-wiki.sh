#!/bin/bash

#required packages: pandoc, texlive-xetex


#assuming cloned wiki repo in the parent folder
PROJECT=$(pwd)
REPO=$(basename $PROJECT)
WIKIREPO="$REPO.wiki"
FILES="Home.md;Richieste-di-corse.md;Taxi.md;Generazione-Report.md"
IFS=';' read -r -a array <<< "$FILES"
PANDOCINPUT="../$REPO/README.md"
for element in "${array[@]}"
do
    PANDOCINPUT="$PANDOCINPUT ./$element"
done
cd ../$WIKIREPO
pandoc $PANDOCINPUT --pdf-engine=xelatex -s -o "../$REPO/out/The taxicab game.pdf"
cd ../$REPO

