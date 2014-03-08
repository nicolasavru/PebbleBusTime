var key = "TEST"

function fetchNextBus(stopID, lineRef){
  var response;
  var req = new XMLHttpRequest();
  req.open('GET', "http://bustime.mta.info/api/siri/stop-monitoring.json?" +
           "key=" + key + "&MonitoringRef=" + stopID + "&LineRef=" + lineRef, true);
  req.onload = function(e){
    if (req.readyState == 4){
      if(req.status == 200){
        console.log(req.responseText);
        response = JSON.parse(req.responseText);
        var stopName, lineName, presentableDistance;
        if (response &&
            response.Siri.ServiceDelivery.StopMonitoringDelivery
            && response.Siri.ServiceDelivery.StopMonitoringDelivery.length > 0){
          var vehicleResult = response.Siri.ServiceDelivery.StopMonitoringDelivery[0].MonitoredStopVisit[0].MonitoredVehicleJourney;
          stopName = vehicleResult.MonitoredCall.StopPointName;
          lineName = vehicleResult.PublishedLineName;
          presentableDistance = vehicleResult.MonitoredCall.Extensions.Distances.PresentableDistance;
          console.log(stopName);
          console.log(lineName);
          console.log(presentableDistance);
          Pebble.sendAppMessage({
            "stopName":stopName,
            "lineName":lineName,
            "distance":presentableDistance});
        }

      } else {
        console.log("Error");
      }
    }
  }
  req.send(null);
}

function findNearestStop(lat, lon){
  var response;
  var req = new XMLHttpRequest();
  // 30 seems to be enough to find the closest stop
  req.open('GET', "http://bustime.mta.info/api/where/stops-for-location.json?"
           + "key=" + key + "&lat=" + lat + "&lon=" + lon + "&maxCount=30", true);
  req.onload = function(e){
    if(req.readyState == 4){
      if(req.status == 200){
        console.log(req.responseText);
        response = JSON.parse(req.responseText);
        var minIdx, minDist, stop, dist;
        if(response &&
           response.data.stops &&
           response.data.stops.length > 0){
          minIdx = 0;
          minDist = 1;
          for(var i = 0; i < response.data.stops.length; i++){
            stop = response.data.stops[i];
            // console.log(stop.code);
            dist = Math.sqrt(Math.pow(lat - stop.lat, 2) +
                             Math.pow(lon - stop.lon, 2));
            // console.log(dist);
            if(dist < minDist){
              minDist = dist;
              minIdx = i;
            }
          }
          // console.log(minIdx);
          // console.log(minDist);
          stop = response.data.stops[minIdx];
          var stopObj = new Object();
          stopObj.stopID = stop.code;
          stopObj.routeIDs = new Array();
          for(var i = 0; i < stop.routes.length; i++){
            stopObj.routeIDs[i] = stop.routes[i].id;
          }
          console.log(JSON.stringify(stopObj));
          fetchNextBus(stopObj.stopID, stopObj.routeIDs[0]);
        }
      }
      else{
        console.log("Error");
      }
    }
  }
  req.send(null);
}

function locationSuccess(pos){
  var coordinates = pos.coords;
  // console.log(coordinates.latitude);
  // console.log(coordinates.longitude)
  findNearestStop(coordinates.latitude, coordinates.longitude);
}

function locationError(err) {
  console.warn('location error (' + err.code + '): ' + err.message);
  
  Pebble.sendAppMessage({
    "stopName":"Loc Unavailable",
    "lineName":"N/A",
    "distance":"N/A"});
}

var locationOptions = {"timeout": 15000, "maximumAge": 60000};

Pebble.addEventListener("ready",
                        function(e){
                          console.log("connect!" + e.ready);
                          locationWatcher = window.navigator.geolocation.watchPosition(locationSuccess, locationError, locationOptions);
                          console.log(e.type);
                        });

Pebble.addEventListener("appmessage",
                        function(e){
                          window.navigator.geolocation.getCurrentPosition(locationSuccess, locationError, locationOptions);
                          console.log(e.type);
                          console.log(e.payload.temperature);
                          console.log("message!");
                        });

Pebble.addEventListener("webviewclosed",
                        function(e){
                          console.log("webview closed");
                          console.log(e.type);
                          console.log(e.response);
                        });
