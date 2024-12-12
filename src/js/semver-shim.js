"use strict";

const semver = require('semver');

// Export just the lt function to maintain compatibility
module.exports = function lt(a, b) {
    return semver.lt(a, b);
};
