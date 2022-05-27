"use strict";

async function hello(param) {
    let start = Date.now();
    return { ...param, ...{
        hello: "world",
        step1_start: start,
        step1_end: start,
        step2_start: start,
        step2_end: start,
        step3_start: start,
        step3_end: start,
    }};
}
exports.hello = hello;
