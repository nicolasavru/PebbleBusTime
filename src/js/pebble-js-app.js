var key = "TEST"
var routeIdx = 0;

function fetchNextBus(stopObj){
  var response;
  if(routeIdx >= stopObj.routes.length){
    routeIdx = stopObj.routes.length -1;
  }
  else if(routeIdx < 0){
    routeIdx = 0;
  }
  console.log(routeIdx);
  var req = new XMLHttpRequest();
  req.open('GET', "http://bustime.mta.info/api/siri/stop-monitoring.json?" +
           "key=" + key + "&MonitoringRef=" + stopObj.id +
           "&LineRef=" + stopObj.routes[routeIdx].id +
           "&StopMonitoringDetailLevel=calls" +
           "&MaximumNumberOfCallsOnwards=1", true);
  req.onload = function(e){
    if (req.readyState == 4){
      if(req.status == 200){
        console.log(req.responseText);
        response = JSON.parse(req.responseText);
        var vehicleStopName, lineName, presentableDistance;
        if(response &&
           response.Siri.ServiceDelivery.StopMonitoringDelivery &&
           response.Siri.ServiceDelivery.StopMonitoringDelivery.length > 0 &&
           response.Siri.ServiceDelivery.StopMonitoringDelivery[0].MonitoredStopVisit &&
           response.Siri.ServiceDelivery.StopMonitoringDelivery[0].MonitoredStopVisit.length > 0){
          var vehicleResult = response.Siri.ServiceDelivery.StopMonitoringDelivery[0].MonitoredStopVisit[0].MonitoredVehicleJourney;
          vehicleStopName = vehicleResult.OnwardCalls.OnwardCall[0].StopPointName;
          presentableDistance = vehicleResult.OnwardCalls.OnwardCall[0].Extensions.Distances.PresentableDistance;
          console.log(stopObj.routes[routeIdx].name);
          console.log(vehicleStopName);
          console.log(presentableDistance);
          Pebble.sendAppMessage({
            "lineName":stopObj.routes[routeIdx].name,
            "stopName":stopObj.name,
            "vehicleStopName":vehicleStopName,
            "distance":presentableDistance});
        }
      }
      else{
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
        // console.log(req.responseText);
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
          stopObj.id = stop.code;
          stopObj.name = stop.name;
          stopObj.routes = new Array();
          for(var i = 0; i < stop.routes.length; i++){
            stopObj.routes[i] = {
              id:stop.routes[i].id.replace("+", "%2B"),
              name:stop.routes[i].shortName + " " + stop.direction
            };
          }
          console.log(JSON.stringify(stopObj));
          fetchNextBus(stopObj);
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

var locationOptions = {"enableHighAccuracy":true, "timeout":15000, "maximumAge":300000};

Pebble.addEventListener("ready",
                        function(e){
                          console.log("connect!" + e.ready);
                          locationWatcher = window.navigator.geolocation.watchPosition(locationSuccess,
                                                                                       locationError,
                                                                                       locationOptions);
                          console.log(e.type);
                        });

Pebble.addEventListener("appmessage",
                        function(e){
                          window.navigator.geolocation.getCurrentPosition(locationSuccess, locationError, locationOptions);
                          console.log(e.type);
                          console.log(JSON.stringify(e.payload));
                          routeIdx += e.payload.lineName;
                          console.log(routeIdx);
                          locationWatcher = window.navigator.geolocation.getCurrentPosition(locationSuccess,
                                                                                            locationError,
                                                                                            locationOptions);
                        });

Pebble.addEventListener("webviewclosed",
                        function(e){
                          console.log("webview closed");
                          console.log(e.type);
                          console.log(e.response);
                        });
