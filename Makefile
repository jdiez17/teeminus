build/teeminus.pbw: src/*.c src/js/*.js appinfo.json
	pebble build

debug: build/teeminus.pbw
	pebble install --logs
