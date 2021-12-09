import React, { memo } from 'react'
import { shapeExampleStyles } from "../components/styles"
import Heatmap from '../components/Heatmap'
import { LoadScript } from '@react-google-maps/api'
import clientPromise from '../lib/mongodb';

const googleMapsLibraries = ['drawing', 'visualization', 'places']

const onError = (err) => console.log('onError: ', err)

function analytics( {readings} ) {

    if (!readings) { return ( <div>Fetching Data</div> ) }

    return (
        <LoadScript
        googleMapsApiKey={''}
        onError={onError}
        libraries={googleMapsLibraries}
        preventGoogleFontsLoading
        >
        <div className="heatmap">
            <Heatmap styles={shapeExampleStyles} readings={readings} />
        </div>

        </LoadScript>
    )
    
}

export default memo(analytics)

export async function getServerSideProps(context) {
    const client = await clientPromise;
    const db = client.db("TrackerReadings");
    const data = await db.collection("readings").find({}).toArray();
    const readings = JSON.parse(JSON.stringify(data));

    return {
      props: { readings }
    }
}
