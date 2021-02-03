#!/bin/bash

# for debug
ipcrm -a

export SO_HOLES=50

export SO_TOP_CELLS=40

export SO_SOURCES=10 # default = 190 (SO_WIDTH * SO_HEIGHT - SO_HOLES)

export SO_CAP_MIN=3

export SO_CAP_MAX=5

export SO_TAXI=1000 # default = 95 (SO_SOURCES / 2)

export SO_TIMENSEC_MIN=10000000

export SO_TIMENSEC_MAX=100000000

export SO_TIMEOUT=3 # default = 1

export SO_DURATION=20 # default = 20

./obj/master
