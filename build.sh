#!/bin/bash

# Install dependencies
npm install

# Build the Pug templates
./node_modules/.bin/pug -o build/http src/pug

# Bundle the JavaScript
./node_modules/.bin/browserify \
  -r ./src/js/api.js:api \
  -r ./src/js/cookie.js:cookie \
  -r ./src/js/sock.js:sock \
  -r semver/functions/lt:semver/functions/lt \
  src/js/app.js -o build/http/js/app.js

# Set permissions
sudo chown -R www-data:www-data build/http
sudo systemctl restart nginx
