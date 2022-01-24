const cassandra = require("cassandra-driver");

function hrtime(tp) {
    return tp[0] * 1000000000 + tp[1];
}


function sample(items) {
    return items[items.length * Math.random() | 0];
}

const contactPoints = ["192.168.2.25", "192.168.2.26", "192.168.2.27"];
const cassandraclient = new cassandra.Client({
    contactPoints: contactPoints,
    keyspace:"functionkv",
    localDataCenter: "datacenter1",
});

function cassandraexecute(query, params) {
    return cassandraclient.execute(query, params);
}

async function get_cassandra(key) {
    const query = "SELECT value FROM functionkv.store WHERE key=?";
    const result = await cassandraexecute(query, [key]);
    return result["rows"][0]["value"];
}

async function put_cassandra(key, value) {
    const query = "INSERT INTO functionkv.store (key, value) VALUES (?, ?);";
    await cassandraexecute(query, [key, value]);
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
    console.log("put_cassandra", await test(put_cassandra, +process.argv[2]));
    console.log("get_cassandra", await test(get_cassandra, +process.argv[2]));
    process.exit(0);
}

if (require.main === module) {
    main();
}
