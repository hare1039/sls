//const request = require("sync-request");

const axios = require("axios");


function sample(items) {
    return items[items.length * Math.random() | 0];
}

function hrtime(tp) {
    return tp[0] * 1000000000 + tp[1];
}

async function put_hdfs(filename, value) {
    let access_coord = process.hrtime();
    let res1 = {};
    try {
        res1 = await axios({
            url: "http://192.168.2.24:9870/webhdfs/v1" + filename + "?op=CREATE&overwrite=true",
            method: "put",
            maxRedirects: 0
        });
    } catch(e) {
        res1 = e.response;
    }

    let access_coord_duration = process.hrtime(access_coord);
    let datanode = sample(["192.168.2.27", "192.168.2.25", "192.168.2.26"]);
    const dataloc = res1.headers["location"].replace("localhost", datanode)

    let put_file = process.hrtime();

    try {
        await axios({
            url: dataloc,
            method: "put",
            data: value,
            headers: {
                "Content-type": "text/plain; charset=utf-8"
            }
        });
    } catch(e) {}
    return [access_coord_duration, process.hrtime(put_file)];
}

async function get_hdfs(filename) {
    let get_file = process.hrtime();
    let res1 = {};
    try {
        res1 = await axios({
            url: "http://192.168.2.24:9870/webhdfs/v1" + filename + "?op=OPEN",
            method: "get",
            maxRedirects: 0
        });
    } catch(e) {
        res1 = e.response;
    }

    let access_coord_duration = process.hrtime(get_file);

    let datanode = sample(["192.168.2.27", "192.168.2.25", "192.168.2.26"]);
    const dataloc = res1.headers["location"].replace("localhost", datanode)

    let get_record = process.hrtime();
    try {
        let res2 = await axios({
            url: dataloc,
            method: "get"
        });
//        console.log(res2.data);
    } catch(e) {}

    return [access_coord_duration, process.hrtime(get_record)];
}

async function test(f, times) {
    let result_arr = [];
    let start = process.hrtime();
    for (let i = 0; i < times; i++) {
        result_arr.push(await f("/tmp/key", "bbbbbbbbbb"));
    }
    let duration = process.hrtime(start);

    let name = 0, data = 0;
    for (let i = 0; i < result_arr.length; i++) {
        name += hrtime(result_arr[i][0]);
        data += hrtime(result_arr[i][1]);
    }
    const access_namenode = (name/result_arr.length).toFixed(2);
    const access_datanode = (data/result_arr.length).toFixed(2);

    return [hrtime(duration) / times, +access_namenode, +access_datanode];
}

async function main() {
    const p = await test(put_hdfs, +process.argv[2]);
    console.log("put_hdfs", p[0], p[1], p[2]);

    const g = await test(get_hdfs, +process.argv[2]);
    console.log("get_hdfs", g[0], g[1], g[2]);
    process.exit(0);
}

if (require.main === module) {
    main();
}
