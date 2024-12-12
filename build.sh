#!/bin/bash

# Clean and create build directory with correct permissions
sudo rm -rf build/http
sudo mkdir -p build/http/js
sudo chown -R davec:staff build

# Install dependencies
npm install

# Build the Pug templates
./node_modules/.bin/pug -o build/http src/pug

# Bundle the JavaScript with Vue global
echo "window.Vue = require('vue');" > build/http/js/app.js
./node_modules/.bin/browserify \
  -r vue \
  -r ./src/js/api.js:api \
  -r ./src/js/cookie.js:cookie \
  -r ./src/js/sock.js:sock \
  -r semver/functions/lt:semver/functions/lt \
  src/js/app.js >> build/http/js/app.js

# Copy static files
cp -r src/static/* build/http/

# Set final permissions for nginx
sudo chown -R www-data:www-data build/http
sudo systemctl restart nginx
