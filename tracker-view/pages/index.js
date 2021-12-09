import Head from 'next/head'
import useSWR from 'swr';
import Tracker from "../components/Tracker.js"
import Map from '../components/Map';

const fetcher = (...args) => fetch(...args).then((response) => response.json());

const devURL = "http://localhost:3000";

const prodURL = "https://tracker-view.vercel.app";

const build = process.env.NODE_ENV === 'development' ? devURL : prodURL;

export default function Home() {

  const {data, error} = useSWR(`${build}/api/trackerreadings`, fetcher, { refreshInterval: 1000, refreshWhenHidden: true, refreshWhenOffline: true });

  if (error) { return ( <h1>Error</h1> ) }
  if (!data) { return ( <h1>Loading</h1> ) }

  return (
    <div className="main" style={{display: 'flex'}}>

      <Head>
        <title>Tracker View</title>
        <meta name="description" content="Tracker Viewing Web Application" />
        <link rel="icon" href="/favicon.ico" />
      </Head>

      <div className="sidebar" style={{flex: .2}}>
        { data.map(tracker => (<Tracker key={tracker[0]} info={tracker[1]} />)) }
      </div>

      <div className="mapContainer" style={{flex: .8}}>
        <Map trackers={data} />
      </div>  

    </div>
  )
}