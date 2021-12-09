import clientPromise from '../../lib/mongodb';

export default async function handler(req, res) {
    const client = await clientPromise;
    const db = client.db("TrackerReadings");
    const data = await db.collection("readings").find({}).sort({$natural: -1}).toArray();
    const err = [];
    const clean = data.reduce(function(map, obj) {
        if (!map.has(obj.trackerID)) {
          if ('error' in obj) {
            err.push(obj.trackerID);
          }
          else if (err.indexOf(obj.trackerID) == -1) {
            map.set(obj.trackerID, obj)
          }
          else if (err.indexOf(obj.trackerID) > -1) {
            const tData = {
              trackerID: obj.trackerID,
              location: obj.location,
              gpsDate: obj.gpsDate,
              gpsTime: obj.gpsTime,
              error: "Cannot get fix"
            }
            map.set(obj.trackerID, tData)
          }
          
        }
        return map
      }, new Map());
    const ans = await Promise.all(clean)
    res.json(ans)
}