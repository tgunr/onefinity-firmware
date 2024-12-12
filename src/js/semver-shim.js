"use strict";

const semver = require('semver');

// Export just the lt function to maintain compatibility
module.exports = semver.lt;
module.exports.semver = semver;
