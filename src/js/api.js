"use strict";

async function callApi(method, url, data) {
    // Mock API responses
    console.debug(`Mock API call: ${method} ${url}`, data);
    
    // Return mock data based on the endpoint
    switch (url) {
        case 'config':
            return {
                motors: [{}, {}, {}, {}],  // 4 mock motors
                axes: {
                    x: { length: 1000 },
                    y: { length: 1000 },
                    z: { length: 1000 },
                    a: { length: 360 }
                },
                tool: { length: 100 },
                units: 'METRIC'
            };
            
        case 'state':
            return {
                cycle: 'idle',
                line: 0,
                tool: 0,
                feed: 0,
                speed: 0,
                position: {x: 0, y: 0, z: 0, a: 0},
                bbox: {min: {x: 0, y: 0, z: 0}, max: {x: 0, y: 0, z: 0}}
            };
            
        default:
            return {};
    }
}

module.exports = {
    get: function(url) {
        return callApi("GET", url);
    },

    put: function(url, body = undefined) {
        return callApi("PUT", url, body);
    },

    delete: function(url) {
        return callApi("DELETE", url);
    }
};
