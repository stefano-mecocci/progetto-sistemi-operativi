#!/bin/bash

#assuming cloned wiki repo in the parent folder
PROJECT=$(pwd)
REPO=$(basename $PROJECT)
WIKIREPO="$REPO.wiki"
FILES="Home.md;Risorse-IPC-usate.md;Taxi.md;Generazione-Report.md"
IFS=';' read -r -a array <<< "$FILES"
PANDOCINPUT="./README.md"
#pandoc README.md --pdf-engine=xelatex -s -o ./out/README.pdf
#pandoc ../$WIKIREPO/*.md --pdf-engine=xelatex -s -o "./out/The taxicab game.pdf"
for element in "${array[@]}"
do
    PANDOCINPUT="$PANDOCINPUT ../$WIKIREPO/$element"
done
pandoc $PANDOCINPUT --pdf-engine=xelatex -s -o "./out/The taxicab game.pdf"

