#!/bin/bash

# Clean and create build directory
rm -rf build/http
mkdir -p build/http/js

# Install dependencies
npm install vue socket.io-client jquery three browserify pug-cli

# Build the Pug templates
./node_modules/.bin/pug -o build/http src/pug

# Copy Vue and other dependencies
cp node_modules/vue/dist/vue.js build/http/js/
cp node_modules/socket.io-client/dist/socket.io.js build/http/js/
cp node_modules/jquery/dist/jquery.min.js build/http/js/jquery-1.11.3.min.js
cp node_modules/three/build/three.module.js build/http/js/three.min.js

# Bundle the JavaScript
./node_modules/.bin/browserify \
  src/js/app.js \
  -r ./src/js/api.js:api \
  -r ./src/js/cookie.js:cookie \
  -r ./src/js/sock.js:sock \
  -r semver/functions/lt:semver/functions/lt \
  -o build/http/js/app.js

# Copy static files
cp -r src/static/* build/http/
