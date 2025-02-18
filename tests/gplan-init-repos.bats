#!/usr/bin/env bats

@test "hello world!" {
    run echo "Hello, World!"
    [ "$status" -eq 0 ]
    [ "$output" = "Hello, World!" ]
}

load 'test_helper/bats-support/load'
load 'test_helper/bats-assert/load'

setup() {
    # Create temporary test directory
    TEST_DIR="$(mktemp -d)"
    export TEST_DIR
}

teardown() {
    # Clean up test directory
    rm -rf "$TEST_DIR"
}

@test "sources.list is created with correct content" {
    run ./gplan-init-repos.sh
    assert_success
    assert [ -f "/etc/apt/sources.list" ]
    run cat "/etc/apt/sources.list"
    assert_output --partial "deb [arch=armhf"
}

@test "apt configuration is created" {
    run ./gplan-init-repos.sh
    assert_success
    assert [ -f "/etc/apt/apt.conf.d/99custom-settings" ]
}

@test "architecture configuration is created" {
    run ./gplan-init-repos.sh
    assert_success
    assert [ -f "/etc/apt/apt.conf.d/01architecture" ]
}

@test "repository keys are installed" {
    run ./gplan-init-repos.sh
    assert_success
    assert [ -f "/usr/share/keyrings/raspberrypi-archive-keyring.gpg" ]
    assert [ -f "/usr/share/keyrings/raspbian-archive-keyring.gpg" ]
}