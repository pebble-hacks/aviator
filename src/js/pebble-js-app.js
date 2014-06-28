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
  localStorage.setItem("timezone", parseInt(config.timezone)); 
  localStorage.setItem("bigmode", parseInt(config.bigmode)); 
  localStorage.setItem("dayname", parseInt(config.dayname)); 
  
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
	mConfig.timezone = parseInt(localStorage.getItem("timezone"));
	mConfig.bigmode = parseInt(localStorage.getItem("bigmode"));
	mConfig.dayname = parseInt(localStorage.getItem("dayname"));
	mConfig.configureUrl = "http://www.mirz.com/Aviatorv2/index3.html";
	
	if(isNaN(mConfig.seconds)) {
		mConfig.seconds = 1;
	}
	if(isNaN(mConfig.invert)) {
		mConfig.invert = 0;
	}
	if(isNaN(mConfig.bluetoothvibe)) {
		mConfig.bluetoothvibe = 1;
	}
	if(isNaN(mConfig.vibeminutes)) {
		mConfig.vibeminutes = 0;
	}   
	if(isNaN(mConfig.hands)) {
		mConfig.hands = 1;
	} 
	if(isNaN(mConfig.style)) {
		mConfig.style = 0;
	} 
	if(isNaN(mConfig.background)) {
		mConfig.background = 0;
	} 
	if(isNaN(mConfig.timezone)) {
		mConfig.timezone = 0;
	} 
	if(isNaN(mConfig.bigmode)) {
		mConfig.bigmode = 0;
	} 
	if(isNaN(mConfig.dayname)) {
		mConfig.dayname = 0;
	} 
}
function returnConfigToPebble() {
  //console.log("Configuration window returned: " + JSON.stringify(mConfig));
  Pebble.sendAppMessage({
    "seconds":parseInt(mConfig.seconds), 
    "invert":parseInt(mConfig.invert), 
    "bluetoothvibe":parseInt(mConfig.bluetoothvibe), 
    "vibeminutes":parseInt(mConfig.vibeminutes),
    "hands":parseInt(mConfig.hands),
    "style":parseInt(mConfig.style),
    "background":parseInt(mConfig.background),
	"timezone":parseInt(mConfig.timezone),
    "timezoneOffset":parseInt(TimezoneOffsetSeconds()),
	"timezoneLabel":getTimezoneNameAndOffset().label,
	"bigmode":parseInt(mConfig.bigmode),
	"dayname":parseInt(mConfig.dayname)
  });    
}
function TimezoneOffsetSeconds() {
	// Get the number of seconds to add to convert localtime to utc
	//console.log("Timezone: " + new Date().getTimezoneOffset() * 60);
	//return new Date().getTimezoneOffset() * 60;
	var x = getTimezoneNameAndOffset();
	
	//console.log("orig UTC: " + new Date().getTimezoneOffset() * 60);
	//console.log("TimezoneOffsetSeconds: " + (new Date().getTimezoneOffset() + (x.hours * 60) * 60));
	
    var now = new Date();
    var nowUtc = new Date(now.getTime() + (now.getTimezoneOffset() * 60000));	
	
	//console.log("UTC: " + nowUtc);
	//console.log("TimezoneOffsetMiliSeconds: " + (nowUtc.getTimezoneOffset() + (x.hours * 60) * 3600));
	
	var mil = x.hours * 3600000;
	
	var secondTimezone = new Date(nowUtc.getTime() + mil);
	//console.log("secondTimezone: " + secondTimezone);
	
	//console.log("is DST: " + secondTimezone.dst());
	var secOff = 0;
	//if(secondTimezone.dst()) {
	//  secOff = 1 * 3600000;
    //}
	
    var totalSeconds = ((secondTimezone.getTime() + secOff) - now.getTime()) / 1000;
	//console.log("totalSeconds: " + totalSeconds);
	return totalSeconds;
}

function getTimezoneNameAndOffset() {
	var hours = 0;
	var label = '';
	
	switch(parseInt(mConfig.timezone)) {
		case 0:
			hours = 0; label='GMT';
			break;
		case 1:
			hours = 0; label='GMT';
			break;
		case 2:
			hours = 0; label='GMT';
			break;
		case 3:
			hours = 1; label='GMT +01:00';
			break;
		case 4:
			hours = 1; label='GMT +01:00';
			break;
		case 5:
			hours = 1; label='GMT +01:00';
			break;
		case 6:
			hours = 1; label='GMT +01:00';
			break;
		case 7:
			hours = 1; label='GMT +01:00';
			break;
		case 8:
			hours = 2; label='GMT +02:00';
			break;
		case 9:
			hours = 2; label='GMT +02:00';
			break;
		case 10:
			hours = 2; label='GMT +02:00';
			break;
		case 11:
			hours = 2; label='GMT +02:00';
			break;
		case 12:
			hours = 2; label='GMT +02:00';
			break;
		case 13:
			hours = 2; label='GMT +02:00';
			break;
		case 14:
			hours = 2; label='GMT +02:00';
			break;
		case 15:
			hours = 2; label='GMT +02:00';
			break;
		case 16:
			hours = 2; label='GMT +02:00';
			break;
		case 17:
			hours = 3; label='GMT +03:00';
			break;
		case 18:
			hours = 3; label='GMT +03:00';
			break;
		case 19:
			hours = 3; label='GMT +03:00';
			break;
		case 20:
			hours = 3; label='GMT +03:00';
			break;
		case 21:
			hours = 3; label='GMT +03:00';
			break;
		case 22:
			hours = 3.5; label='GMT +03:30';
			break;
		case 23:
			hours = 4; label='GMT +04:00';
			break;
		case 24:
			hours = 4; label='GMT +04:00';
			break;
		case 25:
			hours = 4; label='GMT +04:00';
			break;
		case 26:
			hours = 4; label='GMT +04:00';
			break;
		case 27:
			hours = 4.5; label='GMT +04:30';
			break;
		case 28:
			hours = 5; label='GMT +05:00';
			break;
		case 29:
			hours = 5; label='GMT +05:00';
			break;
		case 30:
			hours = 5; label='GMT +05:00';
			break;
		case 31:
			hours = 5.5; label='GMT +05:30';
			break;
		case 32:
			hours = 5.5; label='GMT +05:30';
			break;
		case 33:
			hours = 5.75; label='GMT +05:45';
			break;
		case 34:
			hours = 6; label='GMT +06:00';
			break;
		case 35:
			hours = 6; label='GMT +06:00';
			break;
		case 36:
			hours = 6.5; label='GMT +06:30';
			break;
		case 37:
			hours = 7; label='GMT +07:00';
			break;
		case 38:
			hours = 7; label='GMT +07:00';
			break;
		case 39:
			hours = 8; label='GMT +08:00';
			break;
		case 40:
			hours = 8; label='GMT +08:00';
			break;
		case 41:
			hours = 8; label='GMT +08:00';
			break;
		case 42:
			hours = 8; label='GMT +08:00';
			break;
		case 43:
			hours = 8; label='GMT +08:00';
			break;
		case 44:
			hours = 9; label='GMT +09:00';
			break;
		case 45:
			hours = 9; label='GMT +09:00';
			break;
		case 46:
			hours = 9; label='GMT +09:00';
			break;
		case 47:
			hours = 9.5; label='GMT +09:30';
			break;
		case 48:
			hours = 9.5; label='GMT +09:30';
			break;
		case 49:
			hours = 10; label='GMT +10:00';
			break;
		case 50:
			hours = 10; label='GMT +10:00';
			break;
		case 51:
			hours = 10; label='GMT +10:00';
			break;
		case 52:
			hours = 10; label='GMT +10:00';
			break;
		case 53:
			hours = 10; label='GMT +10:00';
			break;
		case 54:
			hours = 11; label='GMT +11:00';
			break;
		case 55:
			hours = 12; label='GMT +12:00';
			break;
		case 56:
			hours = 12; label='GMT +12:00';
			break;
		case 57:
			hours = 13; label='GMT +13:00';
			break;
		case 58:
			hours = -1; label='GMT -01:00';
			break;
		case 59:
			hours = -1; label='GMT -01:00';
			break;
		case 60:
			hours = -2; label='GMT -02:00';
			break;
		case 61:
			hours = -3; label='GMT -03:00';
			break;
		case 62:
			hours = -3; label='GMT -03:00';
			break;
		case 63:
			hours = -3; label='GMT -03:00';
			break;
		case 64:
			hours = -3; label='GMT -03:00';
			break;
		case 65:
			hours = -3; label='GMT -03:00';
			break;
		case 66:
			hours = -3.5; label='GMT -03:30';
			break;
		case 67:
			hours = -4; label='GMT -04:00';
			break;
		case 68:
			hours = -4; label='GMT -04:00';
			break;
		case 69:
			hours = -4; label='GMT -04:00';
			break;
		case 70:
			hours = -4; label='GMT -04:00';
			break;
		case 71:
			hours = -4.5; label='GMT -04:30';
			break;
		case 72:
			hours = -5; label='GMT -05:00';
			break;
		case 73:
			hours = -5; label='GMT -05:00';
			break;
		case 74:
			hours = -5; label='GMT -05:00';
			break;
		case 75:
			hours = -6; label='GMT -06:00';
			break;
		case 76:
			hours = -6; label='GMT -06:00';
			break;
		case 77:
			hours = -6; label='GMT -06:00';
			break;
		case 78:
			hours = -6; label='GMT -06:00';
			break;
		case 79:
			hours = -7; label='GMT -07:00';
			break;
		case 80:
			hours = -7; label='GMT -07:00';
			break;
		case 81:
			hours = -7; label='GMT -07:00';
			break;
		case 82:
			hours = -8; label='GMT -08:00';
			break;
		case 83:
			hours = -8; label='GMT -08:00';
			break;
		case 84:
			hours = -9; label='GMT -09:00';
			break;
		case 85:
			hours = -10; label='GMT -10:00';
			break;
		case 86:
			hours = -11; label='GMT -11:00';
			break;
		case 87:
			hours = -12; label='GMT -12:00';
			break;		
	}
	return {
		hours: hours,
		label: label
	};

}

Date.prototype.stdTimezoneOffset = function() {
    var jan = new Date(this.getFullYear(), 0, 1);
    var jul = new Date(this.getFullYear(), 6, 1);
    return Math.max(jan.getTimezoneOffset(), jul.getTimezoneOffset());
};

Date.prototype.dst = function() {
    return this.getTimezoneOffset() < this.stdTimezoneOffset();
};

