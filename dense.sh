ipcrm -a

export SO_HOLES=10

export SO_TOP_CELLS=40

export SO_SOURCES=190 # default = 190 (SO_WIDTH * SO_HEIGHT - SO_HOLES)

export SO_CAP_MIN=1

export SO_CAP_MAX=1

export SO_TAXI=95 # default = 95 (SO_SOURCES / 2)

export SO_TIMENSEC_MIN=100000000

export SO_TIMENSEC_MAX=300000000

export SO_TIMEOUT=1 # default = 1

export SO_DURATION=20 # default = 20

./obj/master.o
