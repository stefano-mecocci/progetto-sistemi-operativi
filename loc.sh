make clean

echo -n "Linee di codice MASTER: "
wc -l master* | tail -n 1 | cut -d" " -f3

echo -n "Linee di codice TAXI: "
wc -l taxi_main.c taxi.* taxi_timer.c | tail -n 1 | cut -d" " -f2

echo -n "Linee di codice TAXIGEN: "
wc -l taxigen* | tail -n 1 | cut -d" " -f2

echo -n "Linee di codice SOURCE: "
wc -l source* | tail -n 1 | cut -d" " -f2

echo ""
echo -n "TOTALE: "
wc -l *.c *.h | tail -n 1 | cut -d" " -f2