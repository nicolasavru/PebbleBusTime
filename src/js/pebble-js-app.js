var key = "TEST"
var routeIdx = 0;

// https://gist.github.com/Neal/7688920
var appMessageQueue = {
  queue: [],
  numTries: 0,
  maxTries: 5,
  add: function(obj){
    this.queue.push(obj);
  },
  clear: function(){
    this.queue = [];
  },
  isEmpty: function(){
    return this.queue.length === 0;
  },
  nextMessage: function(){
    return this.isEmpty() ? {} : this.queue[0];
  },
  send: function(){
    if(this.queue.length > 0){
      var ack = function(){
        appMessageQueue.numTries = 0;
        appMessageQueue.queue.shift();
        appMessageQueue.send();
      };
      var nack = function(){
        appMessageQueue.numTries++;
        console.log("Got a nack, retrying.")
        appMessageQueue.send();
      };
      if(this.numTries >= this.maxTries){
        console.log('Failed sending AppMessage: ' + JSON.stringify(this.nextMessage()));
        ack();
      }
      // console.log(JSON.stringify(this.nextMessage()));
      Pebble.sendAppMessage(this.nextMessage(), ack, nack);
    }
  }
};

function fetchNextBus(stopObj){
  var response;
  routeIdx %= stopObj.routes.length;
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
          var numBuses = response.Siri.ServiceDelivery.StopMonitoringDelivery[0].MonitoredStopVisit.length;
          // console.log("numBuses");
          // console.log(numBuses);
          appMessageQueue.add({
            "lineName":stopObj.routes[routeIdx].name,
            "stopName":stopObj.name,
            "numBuses":numBuses
          });
          for(var i = 0; i < numBuses; i++){
            var vehicleResult = response.Siri.ServiceDelivery.StopMonitoringDelivery[0].MonitoredStopVisit[i].MonitoredVehicleJourney;
            vehicleStopName = vehicleResult.OnwardCalls.OnwardCall[0].StopPointName;
            presentableDistance = vehicleResult.OnwardCalls.OnwardCall[0].Extensions.Distances.PresentableDistance;
            appMessageQueue.add({
              "bus":[i,
                      vehicleStopName.length, vehicleStopName,
                      presentableDistance.length, presentableDistance
                     ]
              });
          }

          appMessageQueue.send();
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
    "lineName":"N/A"
  });
}

var locationOptions = {"enableHighAccuracy":true, "timeout":15000, "maximumAge":300000};

Pebble.addEventListener("ready",
                        function(e){
                          console.log("connect!" + e.ready);
                          locationWatcher = window.navigator.geolocation.getCurrentPosition(locationSuccess,
                                                                                            locationError,
                                                                                            locationOptions);

                          // locationWatcher = window.navigator.geolocation.watchPosition(locationSuccess,
                          //                                                              locationError,
                          //                                                              locationOptions);
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
