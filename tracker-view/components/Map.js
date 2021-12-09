/* eslint-disable no-undef */
import React, { useState } from "react";
import { GoogleMap, InfoWindow, LoadScript, Marker } from "@react-google-maps/api";

function Map({ trackers }) {
  const [activeMarker, setActiveMarker] = useState(null);

  const handleActiveMarker = (marker) => {
    if (marker === activeMarker) {
      return;
    }
    setActiveMarker(marker);
  };

//   const handleOnLoad = (map) => {
//     const bounds = new google.maps.LatLngBounds();
//     trackers.forEach((tracker) => bounds.extend(tracker[1].location));
//     map.fitBounds(bounds);
//   };

  const mapStyles = {        
    height: "86.8vh",
    width: "100%"
  };

  const defaultCenter = {
    lat: 33.78, 
    lng: -84.39
  }

  return (
    <>
    <LoadScript googleMapsApiKey=''>
        <GoogleMap
        // onLoad={handleOnLoad}
        onClick={() => setActiveMarker(null)}
        mapContainerStyle={mapStyles}
        zoom={14}
        center={defaultCenter}
        >
        {trackers.map(tracker => (
            <Marker
                key={`marker${tracker[0]}`}
                position={tracker[1].location}
                onClick={() => handleActiveMarker(tracker[0])}
            >
            {activeMarker === tracker[0] ? (
                <InfoWindow onCloseClick={() => setActiveMarker(null)}>
                    <div>Tracker {tracker[0]}</div>
                </InfoWindow>
            ) : null}
            </Marker>
        ))}
        </GoogleMap>
    </LoadScript>
    </>
  );
}

export default Map;