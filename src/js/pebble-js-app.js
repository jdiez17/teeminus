function utcOffset() { 
    d = new Date()
    n = d.getTimezoneOffset();

    return n * 60;
}

Pebble.addEventListener("ready",
    function(e) {
        // Send UTC offset
        Pebble.sendAppMessage({
            "utcOffset": utcOffset(),
            "nextLaunchTS": Math.round((new Date()).getTime() / 1000) + 40 
        }, function(e) {
            console.log("Sent tz offset");
        }, function(e) {
            console.log("Did not send tz offset");
        });

        setTimeout(function() {
            Pebble.sendAppMessage({
                "flash": "ABORT"
            })
        }, 5000);
    }
);
