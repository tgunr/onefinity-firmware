"use strict";

const api = require("./api");
const cookie = require("./cookie")("bbctrl-");
const Sock = require("./sock");
const semverLt = require("./semver-shim");

function parse_version(v) {
    const pattern = /^(\d+)\.(\d+)\.(\d+)(?:[-.]?(.*))?$/;
    const [ version, major, minor, patch, pre ] = v.trim().match(pattern) || [];
    return { version, major, minor, patch, pre };
}

function fixup_version_number(version) {
    const v = parse_version(version);
    version = `${v.major}.${v.minor}.${v.patch}`;
    if (v.pre) {
        const [ , prefix, num ] = v.pre.match(/([a-zA-Z])(\d+)/);
        const suffix = prefix === "b" ? `beta.${num}` : v.pre;
        version = `${version}-${suffix}`;
    }
    return version;
}

function is_object(o) {
    return o !== null && typeof o == "object";
}

function is_array(o) {
    return Array.isArray(o);
}

function update_array(dst, src) {
    while (dst.length) dst.pop();
    for (let i = 0; i < src.length; i++) Vue.set(dst, i, src[i]);
}

function hasOwnProperty(obj, key) {
    return Object.prototype.hasOwnProperty.call(obj, key);
}

function update_object(dst, src, remove) {
    let props, index, key, value;

    if (remove) {
        props = Object.getOwnPropertyNames(dst);
        for (index in props) {
            key = props[index];
            if (!hasOwnProperty(src, key)) Vue.delete(dst, key);
        }
    }

    props = Object.getOwnPropertyNames(src);
    for (index in props) {
        key = props[index];
        value = src[key];

        if (is_array(value) && hasOwnProperty(dst, key) && is_array(dst[key]))
            update_array(dst[key], value);
        else if (is_object(value) && hasOwnProperty(dst, key) && is_object(dst[key]))
            update_object(dst[key], value, remove);
        else Vue.set(dst, key, value);
    }
}

module.exports = new Vue({
    el: "body",

    components: {
        estop: { template: "#estop-template" },
        "loading-view": { template: "<h1>Loading...</h1>" },
        "control-view": require("./control-view"),
        "settings-view": require("./settings-view"),
        "motor-view": require("./motor-view"),
        "tool-view": require("./tool-view"),
        "io-view": require("./io-view"),
        "admin-general-view": require("./admin-general-view"),
        "admin-network-view": require("./admin-network-view"),
        "macros-view": require('./macros'),
        "help-view": require("./help-view"),
        "cheat-sheet-view": { template: "#cheat-sheet-view-template" },
    },

    data: function() {
        return {
            status: "connecting",
            currentView: "loading",
            display_units: localStorage.getItem("display_units") || "METRIC",
            index: -1,
            modified: false,
            template: require("../resources/config-template.json"),
            config: {
                settings: { units: "METRIC" },
                motors: [{}, {}, {}, {}],
                version: "<loading>",
                full_version: "<loading>",
                ip: "<>",
                wifiName: "not connected",
                macros:[{},{},{},{},{},{},{},{}],
                macros_list:[],
                non_macros_list:[]
            },
            state: {
                messages: [],
            },
            video_size: cookie.get("video-size", "small"),
            crosshair: cookie.get("crosshair", "false") != "false",
            errorTimeout: 30,
            errorTimeoutStart: 0,
            errorShow: false,
            errorMessage: "",
            confirmUpgrade: false,
            confirmUpload: false,
            firmwareUpgrading: false,
            checkedUpgrade: false,
            firmwareName: "",
            latestVersion: "",
        };
    },

    watch: {
        display_units: function(value) {
            localStorage.setItem("display_units", value);
        }
    },

    events: {
        "config-changed": function() {
            this.modified = true;
            return false;
        },

        send: function(msg) {
            if (this._sock) this._sock.send(msg);
            return false;
        },

        connected: function() {
            return this._sock && this._sock.connected;
        },

        update: function() {
            this.update();
            return false;
        },

        check: function() {
            if (this.state.messages.length) {
                this.state.messages = [];
                return true;
            }
            return false;
        },

        upgrade: function() {
            this.confirmUpgrade = true;
            return false;
        },

        upload: function(firmware) {
            this.firmwareName = firmware;
            this.confirmUpload = true;
            return false;
        },

        error: function(msg) {
            if (msg.message) msg = msg.message;
            this.errorMessage = msg;
            this.errorTimeoutStart = new Date().getTime() / 1000;
            this.errorShow = true;

            if (this.errorTimeout) {
                setTimeout(() => {
                    const now = new Date().getTime() / 1000;
                    if (this.errorTimeoutStart + this.errorTimeout <= now)
                        this.errorShow = false;
                }, this.errorTimeout * 1000);
            }

            return false;
        }
    },

    computed: {
        popupMessages: function() {
            const messages = [];

            this.state.messages.forEach(function(message) {
                if (message.popup) messages.push(message.text);
            });

            return messages;
        }
    },

    ready: function() {
        window.onhashchange = () => this.parse_hash();
        this.parse_hash();
        this.connect();
        this.update();
    },

    methods: {
        block_error_dialog: function() {
            this.errorTimeout = 0;
            this.errorShow = true;
        },

        toggle_video: function() {
            const sizes = ["small", "medium", "large", "xlarge"];
            let i = sizes.indexOf(this.video_size) + 1;
            if (sizes.length <= i) i = 0;
            this.video_size = sizes[i];
            cookie.set("video-size", this.video_size);
        },

        toggle_crosshair: function(e) {
            this.crosshair = !this.crosshair;
            cookie.set("crosshair", this.crosshair);
            e.preventDefault();
        },

        estop: function() {
            this.$broadcast("estop");
            this._sock.send({"estop": true});
            this.errorMessage = "EStop";
            this.errorShow = true;
            this.errorTimeout = 0;
        },

        upgrade_confirmed: function() {
            this.confirmUpgrade = false;
            this.firmwareUpgrading = true;
            this.errorShow = false;

            api.put("upgrade").done(() => {
                this.firmwareUpgrading = false;
                this.errorMessage = "Firmware upgraded successfully";
                this.errorShow = true;
                this.errorTimeout = 3;
            });
        },

        upload_confirmed: function() {
            this.confirmUpload = false;
            this.firmwareUpgrading = true;
            this.errorShow = false;

            api.put("firmware/" + this.firmwareName).done(() => {
                this.firmwareUpgrading = false;
                this.errorMessage = "Firmware uploaded successfully";
                this.errorShow = true;
                this.errorTimeout = 3;
            });
        },

        show_upgrade: function() {
            if (this.config.full_version == "<loading>") return false;
            if (!this.checkedUpgrade) return false;
            if (!this.latestVersion) return false;
            return semverLt(this.config.full_version, this.latestVersion);
        },

        showShutdownDialog: function() {
            this.$broadcast("showShutdownDialog");
        },

        update: function() {
            api.get("version").done((version) => {
                this.latestVersion = version;
                this.checkedUpgrade = true;
            });

            api.get("config/load").done((config) => {
                if (!config.length) return;

                config = JSON.parse(config);

                if (typeof config.full_version == "undefined")
                    config.full_version = config.version;

                if (config.version) config.version = fixup_version_number(config.version);
                if (config.full_version)
                    config.full_version = fixup_version_number(config.full_version);

                update_object(this.config, config, true);
            });
        },

        connect: function() {
            const reconnect = () => {
                this.status = "disconnected";
                setTimeout(() => this.connect(), 2000);
            };

            this._sock = new Sock(this);

            this._sock.onmessage = (msg) => {
                if (msg.log) {
                    this.$broadcast("log", msg.log);
                    return;
                }

                if (msg.error) {
                    this.$broadcast("error", msg.error);
                    return;
                }

                if (msg.upgrade) {
                    this.firmwareUpgrading = false;
                    this.errorMessage = "Firmware upgraded successfully";
                    this.errorShow = true;
                    this.errorTimeout = 3;
                    return;
                }

                if (msg.firmware) {
                    this.firmwareUpgrading = false;
                    this.errorMessage = "Firmware uploaded successfully";
                    this.errorShow = true;
                    this.errorTimeout = 3;
                    return;
                }

                if (msg.message) {
                    this.state.messages.push(msg.message);
                    return;
                }

                update_object(this.state, msg, false);
            };

            this._sock.onopen = () => {
                this.status = "connected";
                this.$broadcast("connected");
            };

            this._sock.onclose = () => {
                this.$broadcast("disconnected");
                reconnect();
            };

            this._sock.onerror = () => reconnect();

            this.status = "connecting";
        },

        parse_hash: function() {
            let hash = document.location.hash.substr(1);
            if (!hash.length) hash = "control";

            const parts = hash.split(":");
            const view = parts[0];
            const index = parts.length == 2 ? parseInt(parts[1]) : -1;

            this.currentView = view + "-view";
            this.index = index;
        },

        save: function() {
            const config = JSON.parse(JSON.stringify(this.config));

            // Don't save the following
            delete config.version;
            delete config.full_version;
            delete config.ip;
            delete config.wifiName;

            api.put("config/save/" + JSON.stringify(config)).done(() => {
                this.modified = false;
                this.update();

                this.errorMessage = "Configuration saved";
                this.errorShow = true;
                this.errorTimeout = 3;
            });
        },

        close_messages: function(action) {
            const messages = [];

            this.state.messages.forEach(function(message) {
                if (!message.popup) messages.push(message);
            });

            this.state.messages = messages;

            if (typeof action == "function") action();
        }
    }
});
