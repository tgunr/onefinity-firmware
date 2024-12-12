#!/bin/bash

# Install dependencies
npm install

# Build the Pug templates
./node_modules/.bin/pug -o build/http src/pug

# Bundle the JavaScript
./node_modules/.bin/browserify \
  -x jquery \
  -x vue \
  -x socket.io-client \
  -x clusterize.js \
  -x three \
  src/js/app.js -o build/http/js/app.js

# Set permissions
sudo chown -R www-data:www-data build/http
sudo systemctl restart nginx
