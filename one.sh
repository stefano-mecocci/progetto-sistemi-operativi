ipcrm -a

export SO_HOLES=9

export SO_TOP_CELLS=4

export SO_SOURCES=1 # default = 190 (SO_WIDTH * SO_HEIGHT - SO_HOLES)

export SO_CAP_MIN=1

export SO_CAP_MAX=1

export SO_TAXI=1 # default = 95 (SO_SOURCES / 2)

export SO_TIMENSEC_MIN=100000000

export SO_TIMENSEC_MAX=300000000

export SO_TIMEOUT=1 # default = 1

export SO_DURATION=6 # default = 20

./obj/master.o