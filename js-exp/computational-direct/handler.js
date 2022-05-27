"use strict";

const composer = require("openwhisk-composer");
const { Etcd3 } = require("etcd3");
const request = require("sync-request");
const cassandra = require("cassandra-driver");

async function hello(param) {

    let start = Date.now();
    let param1 = await step1(param);
    param1 = {...param1, ...{step1_start: start, step1_end: Date.now()}};

    start = Date.now();
    let param2 = await step2(param1);
    param2 = {...param2, ...{step2_start: start, step2_end: Date.now()}};

    start = Date.now();
    let param3 = await step3(param2);
    param3 = {...param3, ...{step3_start: start, step3_end: Date.now()}};

    return param3;
}
exports.hello = hello;

// step functions

async function step1(param) {
    let value = param.value * param.value;
    return { value: value };
}

async function step2(param) {
    let value = param.value * param.value;
    return { value: value };
}

async function step3(param) {
    let value = param.value * param.value;
    return { status: "step3 done",
             value:  value };
}
