import clientPromise from '../../lib/mongodb';

export default async function handler(req, res) {
    const client = await clientPromise;
    const db = client.db("TrackerReadings");
    await db.collection("gatewayRequests").insertOne(req.body);
    res.json({interval: "posted"});
}