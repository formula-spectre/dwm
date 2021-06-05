PROG=$(rofi -dmenu -p "what do you need to kill?")

killall $PROG
$PROG

