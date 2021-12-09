import React from 'react'
import styles from "../styles/Tracker.module.css"

const devURL = "http://localhost:3000";

const prodURL = "https://tracker-view.vercel.app";

const build = process.env.NODE_ENV === 'development' ? devURL : prodURL;

async function postInterval( info ) {
    const reqFields = {
        method: "POST",
        headers: { "Content-Type": "application/json" },
        body: JSON.stringify(info)
    }
    const response = await fetch(`${build}/api/trackerinterval`, reqFields);
    await response.json();
}

function Tracker({ info }) {

    const changeInterval = async (event) => {
        event.preventDefault();
        const postData = {
            trackerID: info.trackerID,
            requestType: "setInterval",
            newInterval: event.target.interval.value
        }
        await postInterval(postData);
    };

    return (
        <div className={styles.trackerMain}>
            <h2 style={{margin: 0}}>{`Tracker ${info.trackerID}`}</h2>
            <h3>Last Seen Information: </h3>
            <h4>Location: {info.location.lat}, {info.location.lng}</h4>
            <h4>Date: {info.gpsDate} Time: {info.gpsTime}</h4>
            {('error' in info) &&
                <h4>Current Status: {info.error}</h4>
            }
            {!('error' in info) &&
                <h4>Current Status: Good</h4>
            }
            <form onSubmit={changeInterval}>
                <label htmlFor="interval">Change Pinging Interval (ms)</label>
                <br/>
                <div className={styles.formElem}>
                    <input className={styles.input} id="interval" name="interval" type="number" min="1000" required />
                    <button className={styles.btn} type="submit">Change Interval</button>
                </div>
            </form>
        </div>
    )
}

export default Tracker