"use strict";

const composer = require("openwhisk-composer");
const { Etcd3 } = require("etcd3");
const request = require("sync-request");
const cassandra = require("cassandra-driver");

// step functions

async function step1(param) {
    let start = Date.now();
    let value = param.value * param.value;
    return {...param, ...{value: value, step1_start: start, step1_end: Date.now()}};
}
exports.step1 = step1;

async function step2(param) {
    let start = Date.now();
    let value = param.value * param.value;
    return {...param, ...{value: value, step2_start: start, step2_end: Date.now()}};
}
exports.step2 = step2;

async function step3(param) {
    let start = Date.now();
    let value = param.value * param.value;
    return {...param, ...{value: value, step2_start: start, step2_end: Date.now()}};
}
exports.step3 = step3;
