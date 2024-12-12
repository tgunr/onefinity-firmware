#!/bin/bash

# Clean and create build directory
rm -rf build/http
mkdir -p build/http/js

# Install dependencies (using specific versions that work with Node.js 23)
npm install vue@2.6.12 socket.io-client@2.3.0 jquery@3.5.1 three@0.125.0 browserify@17.0.0 pug-cli@1.0.0-alpha6

# Build the Pug templates
./node_modules/.bin/pug -o build/http src/pug

# Copy Vue and other dependencies
cp node_modules/vue/dist/vue.js build/http/js/
cp node_modules/socket.io-client/dist/socket.io.js build/http/js/
cp node_modules/jquery/dist/jquery.min.js build/http/js/jquery-1.11.3.min.js
cp node_modules/three/build/three.min.js build/http/js/three.min.js

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
