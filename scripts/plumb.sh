#!/bin/bash
# Dependencies are xclip and xorg-xprop.
# qrencode required for qrcode generation.
# groff/zathura required for man pages.
BROWSER=$(which qutebrowser)
prim="$(xclip -o)"; [ -z "$prim" ] && exit

PID=$(xprop -id "$(xprop -root | awk '/_NET_ACTIVE_WINDOW\(WINDOW\)/{print $NF}')" | grep -m 1 PID | cut -d " " -f 3)
PID=$(echo "$(pstree -lpA "$PID" | tail -n 1)" | awk  -F'---' '{print $NF}' | sed -re 's/[^0-9]//g')
cd "$(readlink /proc/"$PID"/cwd)"
#[ -f "$prim" ] && xdg-open "$prim" && exit
[ -f "$prim" ] && mimeopen "$prim" && exit
[ -d "$prim" ] && "$TERMINAL" "$prim" && exit
websearch() { "$BROWSER" "https://duckduckgo.com/?q=$@" ;}
wikipedia() { "$BROWSER" "https://en.wikipedia.org/wiki/$@" ;}
wiktionary() { "$BROWSER" "https://en.wiktionary.org/wiki/$@" ;}
ytplayer() {  youtube-dl --rm-cache-dir -o - -q $@ | mpv --quiet --really-quiet - & disown ;}

[ -n "$prim" ] && echo $prim | grep "^.*youtu\+.*" >/dev/null && ytplayer $prim && exit 
echo "$prim" | grep "^.*\.[A-Za-z]\+.*" >/dev/null && gotourl() { "$BROWSER" "$@" ;}
echo "$prim" | grep "^.*@.*\.[A-Za-z]\+$" >/dev/null && email() { xdg-email "$@" ;}

#command -v qrencode >/dev/null && qrcode() { qrencode "$@" -s 10 -o /tmp/qr.png && xdg-open /tmp/qr.png ;}
man -k "^$prim$" >/dev/null && manual() { man -Tpdf "$prim" | zathura - ;}

func="$(declare -F | awk '{print $3}' | dmenu -c -p "Plumb $prim to?" -i -l 15)"
#func="$(declare -F | awk '{print $3}' | rofi -dmenu -e "Plumb $prim to?" -i -l 15)"

[ -z "$func" ] || "$func" "$prim"
