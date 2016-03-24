# See http://wiringpi.com/pins/ for pin numbers
#!/bin/sh

/usr/local/bin/gpio write 0 0
/usr/local/bin/gpio write 2 0
/usr/local/bin/gpio write 3 0
/usr/local/bin/gpio write 6 0
/usr/local/bin/gpio write 5 0
/usr/local/bin/gpio write 4 0
/usr/local/bin/gpio write 1 0
/usr/local/bin/gpio write 16 0
/usr/local/bin/gpio write 15 0

/usr/local/bin/gpio mode 0 output
/usr/local/bin/gpio mode 2 output
/usr/local/bin/gpio mode 3 output
/usr/local/bin/gpio mode 6 output
/usr/local/bin/gpio mode 5 output
/usr/local/bin/gpio mode 4 output
/usr/local/bin/gpio mode 1 output
/usr/local/bin/gpio mode 16 output
/usr/local/bin/gpio mode 15 output

echo "GPIO ports setup"