#!/bin/bash

# Install dependencies
npm install

# Build the Pug templates
./node_modules/.bin/pug -o build/http src/pug

# Create build directory structure
mkdir -p build/http/js

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

# Set permissions
sudo chown -R www-data:www-data build/http
sudo systemctl restart nginx
