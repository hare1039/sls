

const { Etcd3 } = require("etcd3");

function hrtime(tp) {
    return tp[0] * 1000000000 + tp[1];
}


function sample(items) {
    return items[items.length * Math.random() | 0];
}

const etcd_client = new Etcd3({
    hosts: ["http://192.168.2.27:23792", "http://192.168.2.26:23792", "http://192.168.2.25:23792"]
});

async function put_etcd3(key, value) {
    await etcd_client.put(key).value(value);
}

async function get_etcd3(key) {
    let value = await etcd_client.get(key).string();
    return value;
}


async function test(f, times) {
    let start = process.hrtime();
    for (let i = 0; i < times; i++) {
        await f("stepkey", "bbbbbbbbbb");
    }
    let duration = process.hrtime(start);

    return hrtime(duration) / times;
}

async function main() {
    console.log("put_etcd3", await test(put_etcd3, +process.argv[2]));
    console.log("get_etcd3", await test(get_etcd3, +process.argv[2]));
    process.exit(0);
}

if (require.main === module) {
    main();
}
