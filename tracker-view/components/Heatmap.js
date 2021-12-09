  /* eslint-disable */
  import React, { memo } from 'react'
  import PropTypes from 'prop-types'
  import {
    GoogleMap,
    HeatmapLayer
  } from '@react-google-maps/api'
  
  const ExampleHeatmapPropTypes = {
    styles: PropTypes.shape({
      container: PropTypes.object.isRequired
    }).isRequired
  }
  
  const center = {
    lat: 33.7749, 
    lng: -84.3964
  }
  
  const onClick = (...args) => {
    console.log('onClick args: ', args)
  }
  
  function Heatmap({ styles, readings }) {
    const data = readings.reduce(function(arr, obj) {
      if ('error' in obj) {
          return arr
      } else {
          if ('location' in obj) {
              arr.push(new google.maps.LatLng(obj.location.lat, obj.location.lng));
          }
      }
      return arr
    }, []);
  
    return (
      <div className='map'>
        <h2>HeatMap</h2>
        <div className='map-container'>
          <GoogleMap
            id='heatmap-example'
            mapContainerStyle={styles.container}
            zoom={16}
            center={center}
            onClick={onClick}
          >
            <HeatmapLayer data={data}/>
          </GoogleMap>
        </div>
      </div>
    )
  }
  
  Heatmap.propTypes = ExampleHeatmapPropTypes
  
  export default memo(Heatmap)