BROWSER=$(which qutebrowser)


$BROWSER "https://duckduckgo.com/?q=$(rofi -lines 0 -location 2 -dmenu -p "search the web")"
