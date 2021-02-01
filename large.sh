#!/bin/bash

ipcrm -a

export SO_HOLES=50

export SO_TOP_CELLS=40

export SO_SOURCES=10 # default = 190 (SO_WIDTH * SO_HEIGHT - SO_HOLES)

export SO_CAP_MIN=1

export SO_CAP_MAX=1

export SO_TAXI=5 # default = 95 (SO_SOURCES / 2)

export SO_TIMENSEC_MIN=100000000

export SO_TIMENSEC_MAX=300000000

export SO_TIMEOUT=1 # default = 1

export SO_DURATION=10 # default = 20

./obj/master.o
