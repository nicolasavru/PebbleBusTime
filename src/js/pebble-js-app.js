var key = "TEST";
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
      console.log(JSON.stringify(this.nextMessage()));
      Pebble.sendAppMessage(this.nextMessage(), ack, nack);
    }
  }
};

// http://stackoverflow.com/a/10915724
function splitSlice(str, len){
  var ret = [];
  for(var offset = 0, strLen = str.length; offset < strLen; offset += len){
    ret.push(str.slice(offset, len + offset));
  }
  return ret;
}

function transmitStringArr(key, arr){
  // console.log(JSON.parse(arr));
  console.log(arr);
  var outstr = "";
  for(var i = 0; i < arr.length; i++){
    outstr+=String.fromCharCode(arr[i].length);
    outstr+=(arr[i]);
  }
  // console.log(outstr);
  // console.log(JSON.parse(outstr));
  var outarr = splitSlice(outstr, 112);
  // console.log(outarr);
  // console.log(outarr.length);
  var buf;
  for(var i = 0; i < outarr.length; i++){
    buf = [];
    buf.push(outarr.length);
    buf.push(i);
    buf.push(outarr[i]);
    if(i == outarr.length-1){
      buf.push(255);
    }
    // console.log(buf);
    // console.log({key:buf});
    obj = {};
    obj[key] = buf;
    console.log(JSON.stringify(obj));
    appMessageQueue.add(obj);
  }
  appMessageQueue.send();
}

function fetchBusByStop(stopObj){
  var response;
  routeIdx %= stopObj.routes.length;
  console.log(routeIdx);
  var req = new XMLHttpRequest();
  req.open('GET', "http://bustime.mta.info/api/siri/stop-monitoring.json?" +
           "key=" + key + "&OperatorRef=MTA" +
           "&MonitoringRef=" + stopObj.id +
           "&LineRef=" + stopObj.routes[routeIdx].id +
           "&StopMonitoringDetailLevel=calls" +
           "&MaximumNumberOfCallsOnwards=1", true);
  req.onload = function(e){
    if (req.readyState == 4){
      if(req.status == 200){
        console.log(req.responseText);
        response = JSON.parse(req.responseText);
        var vehicleStopName, presentableDistance;
        // TODO: display lineName and stopName even if no buses are running
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
            "numBuses":[numBuses, 0]
          });
          // TODO: possibly hide buses with a scheduled layover
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

function fetchBusesByRoute(routeID){
  var response;
  var req = new XMLHttpRequest();
  // TODO: add support for MTABC routes and operators MTA%20NYCT/MTABC
  // TODO: handle SBS route id
  var fullRouteID = "MTA%20NYCT_" + routeID;
  req.open('GET', "http://bustime.mta.info/api/siri/vehicle-monitoring.json?" +
           "key=" + key + "&OperatorRef=MTA" + 
           "&LineRef=" + fullRouteID +
           "&StopMonitoringDetailLevel=calls" +
           "&MaximumNumberOfCallsOnwards=1", true);
  req.onload = function(e){
    console.log("before readystate");
    if (req.readyState == 4){
      console.log("before status");
      if(req.status == 200){
        console.log(req.responseText);
        response = JSON.parse(req.responseText);
        var vehicleStopName, presentableDistance;
        // TODO: display lineName and stopName even if no buses are running
        if(response &&
           response.Siri.ServiceDelivery.VehicleMonitoringDelivery &&
           response.Siri.ServiceDelivery.VehicleMonitoringDelivery.length > 0 &&
           response.Siri.ServiceDelivery.VehicleMonitoringDelivery[0].VehicleActivity &&
           response.Siri.ServiceDelivery.VehicleMonitoringDelivery[0].VehicleActivity.length > 0){
          var buses = response.Siri.ServiceDelivery.VehicleMonitoringDelivery[0].VehicleActivity;
          var dir0Buses = buses.filter(
            function(e){
              return e.MonitoredVehicleJourney.DirectionRef == 0;
            });
          var dir1Buses = buses.filter(
            function(e){
              return e.MonitoredVehicleJourney.DirectionRef == 1;
            });
          var numBuses = [dir0Buses.length, dir1Buses.length];
          console.log("numBuses");
          console.log(numBuses);

          appMessageQueue.add({
            "lineName":routeID,
            "numBuses":numBuses
          });
          // LEFT OFF HERE
          for(var i = 0; i < numBuses[0]; i++){
            var vehicleResult = dir0Buses[i].MonitoredVehicleJourney;
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
          fetchBusByStop(stopObj);
        }
      }
      else{
        console.log("Error");
      }
    }
  }
  req.send(null);
}

if(typeof String.prototype.startsWith != 'function'){
  String.prototype.startsWith = function(str){
    return this.slice(0, str.length) == str;
  };
}

// http://stackoverflow.com/questions/8107226/how-to-sort-strings-in-javascript-numerically
function sortArray(arr) {
    var tempArr = [], n;
    for (var i in arr) {
        tempArr[i] = arr[i].match(/([^0-9]+)|([0-9]+)/g);
        for (var j in tempArr[i]) {
            if( ! isNaN(n = parseInt(tempArr[i][j])) ){
                tempArr[i][j] = n;
            }
        }
    }
    tempArr.sort(function (x, y) {
        for (var i in x) {
            if (y.length < i || x[i] < y[i]) {
                return -1; // x is longer
            }
            if (x[i] > y[i]) {
                return 1;
            }
        }
        return 0;
    });
    for (var i in tempArr) {
        arr[i] = tempArr[i].join('');
    }
    return arr;
}

// borough should be one of the bus route prefixes:
// Bx, B, M, Q, S, X
function getRoutesByBorough(borough){
  var response;
  var req = new XMLHttpRequest();
  // 30 seems to be enough to find the closest stop
  req.open('GET', "http://bustime.mta.info/api/where/routes-for-agency/MTA%20NYCT.json?"
           + "key=" + key, true);
  req.onload = function(e){
    if(req.readyState == 4){
      if(req.status == 200){
        // console.log(req.responseText);
        response = JSON.parse(req.responseText);
        var routes, shortNames, boroughShortNames;
        if(response &&
           response.data.list &&
           response.data.list.length > 0){
          routes = response.data.list;
          shortNames = routes.map(function(e){return e.shortName;});
          boroughShortNames = shortNames.filter(
            function(e){
              if(borough === "B"){
                return (e.startsWith("B") && !e.startsWith("Bx"));
              }
              else{
                return e.startsWith(borough);
              }});
          // TODO: improve or replace sortArray()
          sortArray(boroughShortNames);
          console.log(boroughShortNames);
          transmitStringArr("routes", boroughShortNames);
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
                          console.log(e.type);
                          console.log(JSON.stringify(e.payload));
                          if(e.payload.hasOwnProperty("lineName")){
                            window.navigator.geolocation.getCurrentPosition(locationSuccess,
                                                                            locationError,
                                                                            locationOptions);
                            routeIdx += e.payload.lineName;
                            console.log(routeIdx);
                            locationWatcher = window.navigator.geolocation.getCurrentPosition(locationSuccess,
                                                                                              locationError,
                                                                                              locationOptions);
                          }
                          else if(e.payload.hasOwnProperty("routes")){
                            getRoutesByBorough(e.payload.routes);
                          }
                          else if(e.payload.hasOwnProperty("bus")){
                            console.log("before");
                            fetchBusesByRoute(e.payload.bus);
                            console.log("after");
                          }
                        });

Pebble.addEventListener("webviewclosed",
                        function(e){
                          console.log("webview closed");
                          console.log(e.type);
                          console.log(e.response);
                        });
