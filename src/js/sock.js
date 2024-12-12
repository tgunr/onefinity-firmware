"use strict";

const Sock = function(url, retry, timeout) {
    if (!(this instanceof Sock)) {
        return new Sock(url, retry);
    }

    if (typeof retry == "undefined") {
        retry = 2000;
    }
    if (typeof timeout == "undefined") {
        timeout = 16000;
    }

    this.url = url;
    this.retry = retry;
    this.timeout = timeout;
    this.divisions = 4;
    this.count = 0;

    // Mock connection for development
    setTimeout(() => {
        console.debug("Mock connection established");
        this.onopen();
        
        // Send mock initial state
        this.onmessage({
            data: {
                sid: "mock-session",
                state: {
                    cycle: "idle",
                    line: 0,
                    tool: 0,
                    feed: 0,
                    speed: 0,
                    program: "",
                    position: {x: 0, y: 0, z: 0, a: 0},
                    bbox: {min: {x: 0, y: 0, z: 0}, max: {x: 0, y: 0, z: 0}},
                    messages: []
                }
            }
        });
    }, 1000);
};

Sock.prototype.onmessage = function() {
    // Ignore
};

Sock.prototype.onopen = function() {
    // Ignore
};

Sock.prototype.onclose = function() {
    // Ignore
};

Sock.prototype.connect = function() {
    console.debug("Mock connect called");
    // Do nothing in mock mode
};

Sock.prototype.close = function() {
    console.debug("Mock close called");
    // Do nothing in mock mode
};

Sock.prototype.send = function(msg) {
    console.debug("Mock send:", msg);
    // Simulate responses based on message type
    if (typeof msg === 'string') {
        try {
            const data = JSON.parse(msg);
            if (data.type === 'get') {
                // Simulate get response
                this.onmessage({
                    data: {
                        type: 'get-response',
                        value: {}
                    }
                });
            }
        } catch (e) {
            console.debug("Error parsing message:", e);
        }
    }
};

module.exports = Sock;
