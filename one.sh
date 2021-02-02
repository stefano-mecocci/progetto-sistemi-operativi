#!/bin/bash

ipcrm -a

export SO_HOLES=9

export SO_TOP_CELLS=4

export SO_SOURCES=1

export SO_CAP_MIN=1

export SO_CAP_MAX=1

export SO_TAXI=1

export SO_TIMENSEC_MIN=100000000

export SO_TIMENSEC_MAX=300000000

export SO_TIMEOUT=1

export SO_DURATION=10

./obj/master.o