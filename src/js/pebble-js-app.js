var mConfig = {};

Pebble.addEventListener("ready", function(e) {
  loadLocalData();
  returnConfigToPebble();
});

Pebble.addEventListener("showConfiguration", function(e) {
	Pebble.openURL(mConfig.configureUrl);
});

Pebble.addEventListener("webviewclosed",
  function(e) {
    if (e.response) {
      var config = JSON.parse(e.response);
      saveLocalData(config);
      returnConfigToPebble();
    }
  }
);

function saveLocalData(config) {
  console.log("loadLocalData() " + JSON.stringify(config));
  
  localStorage.setItem("seconds", parseInt(config.seconds));  
  localStorage.setItem("invert", parseInt(config.invert)); 
  localStorage.setItem("bluetoothvibe", parseInt(config.bluetoothvibe)); 
  localStorage.setItem("vibeminutes", parseInt(config.vibeminutes)); 
  localStorage.setItem("hands", parseInt(config.hands)); 
  localStorage.setItem("style", parseInt(config.style)); 
  localStorage.setItem("background", parseInt(config.background)); 
  
  loadLocalData();
}
function loadLocalData() {
  mConfig.seconds = parseInt(localStorage.getItem("seconds"));
  mConfig.invert = parseInt(localStorage.getItem("invert"));
  mConfig.bluetoothvibe = parseInt(localStorage.getItem("bluetoothvibe"));
  mConfig.vibeminutes = parseInt(localStorage.getItem("vibeminutes"));
  mConfig.hands = parseInt(localStorage.getItem("hands"));
  mConfig.style = parseInt(localStorage.getItem("style"));
  mConfig.background = parseInt(localStorage.getItem("background"));
  mConfig.timezoneOffset = TimezoneOffsetMinutes();
  mConfig.configureUrl = "http://www.mirz.com/Aviatorv2/index.html";
}
function returnConfigToPebble() {
  console.log("Configuration window returned: " + JSON.stringify(mConfig));
  Pebble.sendAppMessage({
    "seconds":parseInt(mConfig.seconds), 
    "invert":parseInt(mConfig.invert), 
    "bluetoothvibe":parseInt(mConfig.bluetoothvibe), 
    "vibeminutes":parseInt(mConfig.vibeminutes),
    "hands":parseInt(mConfig.hands),
    "style":parseInt(mConfig.style),
    "background":parseInt(mConfig.background),
    "timezoneOffset":TimezoneOffsetMinutes()
  });    
}
function TimezoneOffsetMinutes() {
  // Get the number of seconds to add to convert localtime to utc
  console.log("Timezone: " + new Date().getTimezoneOffset() * 60);
  return new Date().getTimezoneOffset() * 60;
}