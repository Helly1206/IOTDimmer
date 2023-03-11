/* 
 * IOTDimmer - Webpage
 * Webpage content
 * Hardware: Lolin S2 Mini
 * Version 0.80
 * 14-5-2021
 * Copyright: Ivo Helwegen
 */

#ifndef INDEX_h
#define INDEX_h

const char webStart[] = R"literal(
<!DOCTYPE html>
<html>
  <head>
    <meta content="text/html; charset=UTF-8" http-equiv="content-type">
    <title>Dimmer Control</title>
    <meta content="helly" name="author">
    <script type="text/javascript">
      function menuIndex() {
        var xhttp = new XMLHttpRequest();
        xhttp.onreadystatechange = function() {
          if (this.readyState == 4 && this.status == 200) {
            var result = JSON.parse(this.responseText);
            if ("index" in result) {
              document.getElementById(result.index).classList.remove("navunselected");
              document.getElementById(result.index).classList.add("navselected");
            }
            if ("ap" in result) {
              if (result.ap) {
                document.getElementById("ap").innerHTML = "Access Point mode";
              } else {
                document.getElementById("ap").innerHTML = "";
              }
            }
          }
        };
        xhttp.open("GET", "menuload", true);
        xhttp.send();
      }  
    </script>
  </head>)literal";

const char webStyle[] = R"literal(
<style type="text/css">
body {
  font-family: Helvetica,Arial,sans-serif;
}
.button {
  width: 75px;
  padding: 1px 1px;
  text-align: center;
  text-decoration: none;
  display: inline-block;
  font-size: 14px;
}
.headertable table { 
  height: 100px; 
  width: 100%;
  border-collapse: collapse; 
  border-spacing: 0px; 
  border: none;
}
.headertable tr {
  border: none;
  padding: 0px;
}
.headertable td {
  background-color: #404040;
  color: white;
  padding: 0px;
  border: none;
}
.headertable a {
  text-decoration: none;
  color: white;
}
.headertable h1 {
  font-size: x-large;
  text-align: center;
}
.headertable span {
  color: white;
}
.headertable div {
  color: white;
  text-align: center;
}
.headertable b {
  color: white;
}
.bannertable table { 
  width: 100%;
  border-collapse: collapse; 
  border-spacing: 0px; 
}
.bannertable tr {
  border: none;
  padding: 0px;
}
.bannertable td {
  background-color: #404040;
  color: white;
  padding: 0px;
  border: none;
}
.bannertable td:hover {
  background-color: #BFBFBF;
  color: black;
}
.bannertable a {
  text-decoration: none;
  color: white;
}
.navselected {
  background-color: #1A1A1A !important; 
  width: 16.667%; 
  text-align: center; 
  vertical-align: middle;
}
.navunselected {
  width: 16.667%; 
  text-align: center; 
  vertical-align: middle;
}
.navtab {
  background-color: #404040 !important; 
  width: 16.667%; 
  text-align: center; 
  vertical-align: middle;
}
.logging textarea {
  font-size: 12pt;
  width: 100%;
  height: 400px;
}
input[type="file"] {
  display: none;
}
div.settings {
    display: inline-grid;
    grid-template-columns: auto auto auto;
    grid-gap: 5px;
    margin-top: 20px;
    margin-bottom: 10px;
    margin-left: 20px;
}
div.settings label {
  text-align: right;
  margin: 2px;
}
div.settings label:after {
  content: ":"; 
}
div.settings span {
  margin: 2px;
}
div.settings b {
  margin: 2px;
}
div.settings input {
  margin: 2px;
}
</style>)literal";

const char webBody[] = R"literal(
<body>)literal";

const char webEnd[] = R"literal(
</body>
</html>)literal";

const char webHead[] = R"literal(
<div class="headertable">
  <table>
    <tbody>
      <tr>
        <td>
          <b id="ap"></b>
          <h1><a href="/"><span>Dimmer Control</span></a></h1>
          <div>Dimmer Control software with IOT</div>
        </td>
      </tr>
    </tbody>
  </table>
</div>
<div class="bannertable">
  <table>
    <tbody>
      <tr>
        <!--selected-->
        <td id="1" class="navunselected"><a href="/"><span>Home</span></a></td>
        <td id="2" class="navunselected"><a href="/wifi"><span>WiFi</span></a></td>
        <td id="3" class="navunselected"><a href="/dimmer"><span>Dimmer</span></a></td>
        <td id="4" class="navunselected"><a href="/mqtt"><span>MQTT</span></a></td>
        <td id="5" class="navunselected"><a href="/log"><span>Log</span></a></td>
        <td id="6" class="navunselected"><a href="/reboot"><span>Reboot</span></a></td>
      </tr>
    </tbody>
  </table>
</div>)literal";
 
const char webHome[] = R"literal(
  <div class="settings">
    <label>Time</label>
    <span id="time">---</span>
    <span id="timestatus"></span>
    <label>MQTT connection status</label>
    <span id="mqttstatus">---</span>
    <span></span>
    <label>Mains frequency</label>
    <span id="mainsfreq">---</span>
    <span>Hz</span>
    <b>Dimmer control</b><span></span><span></span>
    <label>Level</label>
    <div>
    <span>Off</span>
    <progress id="progresslevel" value="0" max="100"></progress>
    <span>On</span>
    </div>
    <span id="percentlevel">0 %</span>
    <span></span>
    <div>
    <button class="button" onclick="send(-1)">Off</button>
    <button class="button" onclick="send(101)">On</button>
    <button class="button" onclick="send(110)">Lounge</button>
    </div>
    <span></span>
    <label>Percentage</label>
    <input type="number" min=0 max=100 step=1 onchange="checkLimits(this);" name="percentage"></input>
    <span>%</span>
    <span></span>
    <button class="button" onclick="send(-10)">Set level</button>
    <span></span>
    <b>Mode and effect control</b><span></span><span></span>
    <label>waveform mode</label>
      <select name="tempwaveformmode" onchange="sendCtrl(this, 0);">
        <option value="0">Instant</option>
        <option value="1">Linear</option>
        <option value="2">Sine</option>
        <option value="3">Qsine</option>      
      </select>
      <span></span>
      <label>Effect</label>
      <select name="tempeffect" onchange="sendCtrl(this, 1);">
        <option value="0">None</option>
        <option value="1">Ramp</option>
        <option value="2">Sine</option>
        <option value="3">Random</option>
        <option value="4">Input</option>      
      </select>
      <span></span>
      <label>Input</label>
      <input type="number" min=-1000000 max=1000000 step=1 onchange="checkLimits(this); sendCtrl(this, 2);" name="effectinput"></input>
      <span>%</span>
  </div>
  <script type="text/javascript">
    onload = function() {menuIndex(); homeUpdate(1);}
    setInterval(function() { homeUpdate(0); }, 1000); 
    function checkLimits(t) {
      if (t.hasAttribute("min")) {
        if (Number(t.value) < Number(t.getAttribute("min"))) {
          t.value = t.getAttribute("min");
        }
      }
      if (t.hasAttribute("max")) {
        if (Number(t.value) > Number(t.getAttribute("max"))) {
          t.value = t.getAttribute("max");
        }
      }
      if (t.hasAttribute("step")) {
        if (Number.isInteger(Number(t.getAttribute("step")))) {
          t.value = Math.round(Number(t.value));
        }
      }
    }
    function homeUpdate(loading) {
      var xhttp = new XMLHttpRequest();
      xhttp.onreadystatechange = function() {
        if (this.readyState == 4 && this.status == 200) {
          var result = JSON.parse(this.responseText);
          if ("time" in result) {
            document.getElementById("time").innerHTML = result.time;
          }
          if ("timestatus" in result) {
            document.getElementById("timestatus").innerHTML = result.timestatus;
          }
          if ("mqttstatus" in result) {
            document.getElementById("mqttstatus").innerHTML = result.mqttstatus;
          }
          if ("mainsfreq" in result) {
            document.getElementById("mainsfreq").innerHTML = result.mainsfreq;
          }
          if ("level" in result) {
            document.getElementById("progresslevel").value = result.level;
            document.getElementById("percentlevel").innerHTML = result.level + " %";
            if (loading) {
              document.getElementsByName("percentage")[0].value = result.level;
            }
          }
          if (("waveformmode" in result) && (document.getElementsByName("tempwaveformmode")[0] !== document.activeElement)) {
            document.getElementsByName("tempwaveformmode")[0].value = result.waveformmode.toString();
          }
          if (("effect" in result) && (document.getElementsByName("tempeffect")[0] !== document.activeElement)) {
            document.getElementsByName("tempeffect")[0].value = result.effect.toString();
          }
          if (("effectinput" in result) && (document.getElementsByName("effectinput")[0] !== document.activeElement)) {
            document.getElementsByName("effectinput")[0].value = result.effectinput;
          }
        }
      };
      xhttp.open("GET", "homeupdate", true);
      xhttp.send();
    }
    function send(cmdin) {
      var xhttp = new XMLHttpRequest();
      var cmd = 0;
      xhttp.onreadystatechange = function() {
        if (this.readyState == 4 && this.status == 200) {
          homeUpdate();
        }
      };
      if (cmdin < -1) {
        cmd = Number(document.getElementsByName("percentage")[0].value);
      } else {
        cmd = cmdin;
      }
      xhttp.open("GET", "dimmercommand?cmd="+cmd, true);
      xhttp.send();
    }
    function sendCtrl(t, type) {
      var xhttp = new XMLHttpRequest();
      xhttp.onreadystatechange = function() {
        if (this.readyState == 4 && this.status == 200) {
          homeUpdate();
        }
      };
      xhttp.open("GET", "dimmerctrl?type="+type+"&ctrl="+t.value, true);
      xhttp.send();
    }
  </script>)literal";

const char webWifi[] = R"literal(
  <form method='POST' action='wifisave'>
    <div class="settings">
      <b>Current connection</b><span></span><span></span>
      <label>Network</label>
      <span id="network">---</span>
      <span></span>
      <label>SSID</label>
      <span id="ssid">---</span>
      <span></span>
      <label>Hostname</label>
      <span id="whostname">---</span>
      <span></span>
      <label>IP address</label>
      <span id="ip">---</span>
      <span></span>
      <label>MAC address</label>
      <span id="mac">---</span>
      <span></span>
      <b>Access Point config</b><span></span><span></span>
      <label>SSID</label>
      <span id="apssid">---</span>
      <span></span>
      <b>WLAN config</b><span></span><span></span>
      <label>SSID</label>
      <span id="wlanssid">---</span>
      <span></span>
      <label>RSSI</label>
      <span id="wlanrssi">---</span>
      <span>dBm</span>
      <b>Connect to network</b><span></span><span></span>
      <label>Network</label>
      <select id="snetwork" name="inetwork">
      </select>
      <button type="button" onclick="wifiList()">refresh</button>
      <label>Password</label>
      <input type="password" placeholder="password" autocomplete="off" name="ipassword"/>
      <span></span>
      <span></span>
      <input type='submit' value='Store WiFi settings'/>
      <span>WiFi settings will be applied immediately</span>
      <span></span>
      <span></span>
      <span>Connection may be lost</span>
    </div>
  </form>
  <form method='POST' action='wifimiscsave'>
    <div class="settings">
      <b>Host and NTP settings</b><span></span><span></span>
      <label>Hostname</label>
      <input type="text" name="hostname"></input>
      <span>.local</span>
      <label>NTP time server</label>
      <input type="text" name="ntpserver"></input>
      <span></span>
      <label>time zone</label>
      <input type="number" min=-12 max=12 step=1 onchange="checkLimits(this);" name="timezone"></input>
      <span>h UTC offset</span>
      <label>Use daylight saving time</label>
      <input type="checkbox" name="usedst"></input>
      <span></span>
      <span></span>
      <input type='submit' value='Store settings'/>
      <span>Reboot required</span>
    </div>
  </form>
  <form method='POST' action='logsave'>
    <div class="settings">
      <b>UDP logging</b><span></span><span></span>
      <label>Port</label>
      <input type="number" min=1024 max=65535 step=1 onchange="checkLimits(this);" name="udpport"></input>
      <span>Reboot required</span>
      <label>Logging enabled</label>
      <input type="checkbox" name="udpenable" onclick="logEnable();"></input>
      <span></span>
      <span>Debug logging</span><span></span><span></span>
      <label>Debug log 1</label>
      <input type="checkbox" name="debug[]" id="debug_0" onclick="updateLevels();"></input>
      <span name="debugtext[]" id="debugtext_0"></span>
      <label>Debug log 2</label>
      <input type="checkbox" name="debug[]" id="debug_1" onclick="updateLevels();"></input>
      <span name="debugtext[]" id="debugtext_1"></span>
      <label>Debug log 3</label>
      <input type="checkbox" name="debug[]" id="debug_2" onclick="updateLevels();"></input>
      <span name="debugtext[]" id="debugtext_2"></span>
      <label>Debug log 4</label>
      <input type="checkbox" name="debug[]" id="debug_3" onclick="updateLevels();"></input>
      <span name="debugtext[]" id="debugtext_3"></span>
      <label>Debug log 5</label>
      <input type="checkbox" name="debug[]" id="debug_4" onclick="updateLevels();"></input>
      <span name="debugtext[]" id="debugtext_4"></span>
      <label>Debug log 6</label>
      <input type="checkbox" name="debug[]" id="debug_5" onclick="updateLevels();"></input>
      <span name="debugtext[]" id="debugtext_5"></span>
      <label>Debug log 7</label>
      <input type="checkbox" name="debug[]" id="debug_6" onclick="updateLevels();"></input>
      <span name="debugtext[]" id="debugtext_6"></span>
      <label>Debug log 8</label>
      <input type="checkbox" name="debug[]" id="debug_7" onclick="updateLevels();"></input>
      <span name="debugtext[]" id="debugtext_7"></span>
      <label>Debug log 9</label>
      <input type="checkbox" name="debug[]" id="debug_8" onclick="updateLevels();"></input>
      <span name="debugtext[]" id="debugtext_8"></span>
      <label>Debug log 10</label>
      <input type="checkbox" name="debug[]" id="debug_9" onclick="updateLevels();"></input>
      <span name="debugtext[]" id="debugtext_9"></span>
      <label>Debug log 11</label>
      <input type="checkbox" name="debug[]" id="debug_10" onclick="updateLevels();"></input>
      <span name="debugtext[]" id="debugtext_10"></span>
      <label>Debug log 12</label>
      <input type="checkbox" name="debug[]" id="debug_11" onclick="updateLevels();"></input>
      <span name="debugtext[]" id="debugtext_11"></span>
      <label>Debug log 13</label>
      <input type="checkbox" name="debug[]" id="debug_12" onclick="updateLevels();"></input>
      <span name="debugtext[]" id="debugtext_12"></span>
      <label>Debug log 14</label>
      <input type="checkbox" name="debug[]" id="debug_13" onclick="updateLevels();"></input>
      <span name="debugtext[]" id="debugtext_13"></span>
      <label>Debug log 15</label>
      <input type="checkbox" name="debug[]" id="debug_14" onclick="updateLevels();"></input>
      <span name="debugtext[]" id="debugtext_14"></span>
      <label>Debug log 16</label>
      <input type="checkbox" name="debug[]" id="debug_15" onclick="updateLevels();"></input>
      <span name="debugtext[]" id="debugtext_15"></span>
      <span></span>
      <input type='submit' value='Store settings'/>
      <span>Store as default settings</span>
    </div>
  </form>
  <form method='POST' enctype='multipart/form-data'>
    <div class="settings">
      <b>Software update</b><span></span><span></span>
      <label>Current version</label>
      <span id="appversion">---</span>
      <span></span>
      <label>File</label>
      <input type="text" readonly id="filetext" value="No file selected">
      <div>
        <button type="button" id="browsebutton" onclick="document.getElementById('updatefile').click()">Browse</button>
        <input type="file" id="updatefile" onchange="fileChanged(this)" accept=".bin">
      </div>
      <label>Progress</label>
      <center>
        <progress class="progress" id="progresslevel" value="0" max="100"></progress>
      </center>
      <span id="percentlevel">0 %</span>
      <span></span>
      <input type='submit' id="submitButton" value='Update' disabled>
      <span>System will reboot after updating</span>
      <label>Status</label>
      <span id="statusText">&nbsp;</span>
      <span></span>
    </div>
  </form>
  <div class="settings">
    <b>Diagnostics</b><span></span><span></span>
    <label>Reboot time</label>
    <span id="reboottime">---</span>
    <span></span>
    <label>Reboot reason CPU 0</label>
    <span id="rebootreason0">---</span>
    <span></span>
    <label>Reboot reason CPU 1</label>
    <span id="rebootreason1">---</span>
    <span></span>
    <label>Heap memory usage</label>
    <span id="heapmem">---</span>
    <span></span>
    <label>Program memory usage</label>
    <span id="progmem">---</span>
    <span></span>
    <label>SDK version</label>
    <span id="sdkversion">---</span>
    <span></span>
    <label>CPU/ xtal frequency</label>
    <span id="cpufreq">---</span>
    <span></span>
  <div>
  <script type="text/javascript">
    onload = function() {menuIndex(); wifiLoad(); wifiList(); udpTexts();}
    setInterval(function() { wifiUpdate(); }, 5000); 
    document.getElementById('submitButton').addEventListener('click', function(event){
      event.preventDefault();
      uploadOTA();
    });
    function checkLimits(t) {
      if (t.hasAttribute("min")) {
        if (Number(t.value) < Number(t.getAttribute("min"))) {
          t.value = t.getAttribute("min");
        }
      }
      if (t.hasAttribute("max")) {
        if (Number(t.value) > Number(t.getAttribute("max"))) {
          t.value = t.getAttribute("max");
        }
      }
      if (t.hasAttribute("step")) {
        if (Number.isInteger(Number(t.getAttribute("step")))) {
          t.value = Math.round(Number(t.value));
        }
      }
    }
    function wifiLoad() {
      var xhttp = new XMLHttpRequest();
      xhttp.onreadystatechange = function() {
        if (this.readyState == 4 && this.status == 200) {
          var result = JSON.parse(this.responseText);
          if ("network" in result) {
            document.getElementById("network").innerHTML = result.network;
          }
          if ("ssid" in result) {
            document.getElementById("ssid").innerHTML = result.ssid;
          }
          if ("ip" in result) {
            document.getElementById("ip").innerHTML = result.ip;
          }
          if ("mac" in result) {
            document.getElementById("mac").innerHTML = result.mac;
          }
          if ("whostname" in result) {
            document.getElementById("whostname").innerHTML = result.whostname;
          }
          if ("apssid" in result) {
            document.getElementById("apssid").innerHTML = result.apssid;
          }
          if ("wlanssid" in result) {
            document.getElementById("wlanssid").innerHTML = result.wlanssid;
          }
          if ("wlanrssi" in result) {
            document.getElementById("wlanrssi").innerHTML = result.wlanrssi;
          }
          if ("hostname" in result) {
            document.getElementsByName("hostname")[0].value = result.hostname;
          }
          if ("ntpserver" in result) {
            document.getElementsByName("ntpserver")[0].value = result.ntpserver;
          }
          if ("timezone" in result) {
            document.getElementsByName("timezone")[0].value = result.timezone;
          }
          if ("usedst" in result) {
            document.getElementsByName("usedst")[0].checked = result.usedst;
          }
          if ("appversion" in result) {
            document.getElementById("appversion").innerHTML = result.appversion;
          }
          if ("reboottime" in result) {
            document.getElementById("reboottime").innerHTML = result.reboottime;
          }
          if ("rebootreason0" in result) {
            document.getElementById("rebootreason0").innerHTML = result.rebootreason0;
          }
          if ("rebootreason1" in result) {
            document.getElementById("rebootreason1").innerHTML = result.rebootreason1;
          }
          if ("heapmem" in result) {
            document.getElementById("heapmem").innerHTML = result.heapmem;
          }
          if ("progmem" in result) {
            document.getElementById("progmem").innerHTML = result.progmem;
          }
          if ("sdkversion" in result) {
            document.getElementById("sdkversion").innerHTML = result.sdkversion;
          }
          if ("cpufreq" in result) {
            document.getElementById("cpufreq").innerHTML = result.cpufreq;
          }
          if ("udpport" in result) {
            document.getElementsByName("udpport")[0].value = result.udpport;
          }
          if ("udpenable" in result) {
            document.getElementsByName("udpenable")[0].checked = result.udpenable;
          }
          if ("udpdebug" in result) {
            setLevels(result.udpdebug);
          }
        }
      };
      xhttp.open("GET", "wifiload", true);
      xhttp.send();
    }
    function logEnable() {
      var xhttp = new XMLHttpRequest();
      var ena = document.getElementsByName("udpenable")[0].checked ? 1:0;
      xhttp.onreadystatechange = function() {};
      xhttp.open("GET", "logenable?ena="+ena, true);
      xhttp.send();
    }
    function setLevels(level) {
      var es = document.getElementsByName("debug[]");
      es.forEach((e, i) => {
        var mask = 1 << i;
        if ((level & mask) != 0) {
          e.checked = true;
        } else {
          e.checked = false;
        }
      });
    }
    function updateLevels() {
      var xhttp = new XMLHttpRequest();
      var lvl = 0;
      var es = document.getElementsByName("debug[]");
      es.forEach((e, i) => {
        var mask = 1 << i;
        if (e.checked) {
          lvl |= mask;          
        } else {
          lvl &= ~mask;
        }
      });
      xhttp.onreadystatechange = function() {};
      xhttp.open("GET", "loglevel?lvl="+lvl, true);
      xhttp.send();
    }
    function udpTexts() {
      var xhttp = new XMLHttpRequest();
      xhttp.onreadystatechange = function() {
        if (this.readyState == 4 && this.status == 200) {
          var es = document.getElementsByName("debugtext[]");
          var result = JSON.parse(this.responseText);
          result.forEach((logtext,i) => {
            es[i].innerHTML = logtext;
          });      
        }
      };
      xhttp.open("GET", "logtexts", true);
      xhttp.send();
    }
    function wifiList() {
      var xhttp = new XMLHttpRequest();
      xhttp.onreadystatechange = function() {
        if (this.readyState == 4 && this.status == 200) {
          var el = document.getElementById("snetwork");
          var result = JSON.parse(this.responseText);
          result.forEach(wlanitem => {
            let optData = document.createElement("OPTION");
            optData.value = wlanitem.ssid;
            optData.innerHTML = wlanitem.content;
            optData.selected = wlanitem.select;
            el.appendChild(optData);
          });        
        }
      };
      if (true) {
        var el = document.getElementById("snetwork");
        var i, L = el.options.length - 1;
        for(i = L; i >= 0; i--) {
          el.remove(i);
        }
      }
      xhttp.open("GET", "wifilist", true);
      xhttp.send();
    }
    function wifiUpdate() {
      var xhttp = new XMLHttpRequest();
      xhttp.onreadystatechange = function() {
        if (this.readyState == 4 && this.status == 200) {
          var result = JSON.parse(this.responseText);
          if ("wlanrssi" in result) {
            document.getElementById("wlanrssi").innerHTML = result.wlanrssi;
          }
        }
      };
      xhttp.open("GET", "wifiupdate", true);
      xhttp.send();
    }
    function fileChanged(t) {
      if (t.files.length > 0) {
        document.getElementById("filetext").value = t.files[0].name;
        if (t.files[0].name.split('.').pop().toLowerCase() == "bin") {
          document.getElementById("submitButton").disabled = false;
          document.getElementById("statusText").innerHTML = "Press Update to start";
        } else {
          document.getElementById("submitButton").disabled = true;
          document.getElementById("statusText").innerHTML = "Selected file is not a binary file";
        }
      } else {
        document.getElementById("filetext").value = "No file selected"
        document.getElementById("submitButton").disabled = true;
        document.getElementById("statusText").innerHTML = "Select file first";
      }
    }
    function uploadOTA() {
      var xhttp = new XMLHttpRequest();
      xhttp.onreadystatechange = function() {
        if (this.readyState == 4) {
          switch (this.status) {
            case 200:
              window.statusText.innerHTML = 'Updated, (Refresh!) ...';
              break;
            case 400:
              window.statusText.innerHTML = this.responseText;
              break;
            default: 
              window.statusText.innerHTML = 'Unknown error: ' + this.status;
          } 
        }
      }
      if (document.getElementById("updatefile").files.length <= 0) {
        document.getElementById("statusText").innerHTML = "No file selected";
        document.getElementById("updatefile").disabled = false;
        document.getElementById("browsebutton").disabled = false;
        return;
      }
      document.getElementById("updatefile").disabled = false;
      document.getElementById("browsebutton").disabled = true;
      document.getElementById("submitButton").disabled = true;
      xhttp.upload.addEventListener('progress', function(event) {
        var percent = Math.round((event.loaded / event.total) * 100);
        document.getElementById("progresslevel").value = percent;
        document.getElementById("percentlevel").innerHTML = percent + " %";
      }); 
      xhttp.open("POST", "wifiupdateota", true);
      var file = document.getElementById("updatefile").files[0];
      xhttp.setRequestHeader('FileSize', file.size);
      var data = new FormData();
      data.append('file', file);
      xhttp.send(data);
      document.getElementById("statusText").innerHTML = "Updating ...";
    }
  </script>)literal";

const char webDimmer[] = R"literal(
  <form method='POST' action='dimmersave'>
    <div class="settings">
      <b>Waveform settings</b><span></span><span></span>
      <label>waveform mode</label>
      <select name="waveformmode">
        <option value="0">Instant</option>
        <option value="1">Linear</option>
        <option value="2">Sine</option>
        <option value="3">Qsine</option>      
      </select>
      <span></span>
      <label>Mode 100% time</label>
      <input type="number" min=1 max=65535 step=1 onchange="checkLimits(this);" name="mode100"></input>
      <span>ms</span>
      <label>Effect</label>
      <select name="effect">
        <option value="0">None</option>
        <option value="1">Ramp</option>
        <option value="2">Sine</option>
        <option value="3">Random</option>
        <option value="4">Input</option>      
      </select>
      <span></span>
      <label>Effect magnitude</label>
      <input type="number" min=0 max=100 step=1 onchange="checkLimits(this);" name="effectmagnitude"></input>
      <span>%</span>
      <label>Effect gain</label>
      <input type="number" min=0 max=1023 step=0.1 onchange="checkLimits(this);" name="effectgain"></input>
      <span>%/int</span>
      <label>Effect time</label>
      <input type="number" min=1 max=65535 step=1 onchange="checkLimits(this);" name="effecttime"></input>
      <span>ms</span>
      <b>Triac settings</b><span></span><span></span>
      <label>Triac mode</label>
      <select name="triacmode">
        <option value="0">Timed</option>
        <option value="1">Power</option>
      </select>
      <span></span>
      <label>Level off</label>
      <input type="number" min=0 max=100 step=1 onchange="checkLimits(this);" name="leveloff"></input>
      <span>%</span>
      <label>Level on</label>
      <input type="number" min=0 max=100 step=1 onchange="checkLimits(this);" name="levelon"></input>
      <span>%</span>
      <label>Level lounge</label>
      <input type="number" min=0 max=100 step=1 onchange="checkLimits(this);" name="levellounge"></input>
      <span>%</span>
      <span></span>
      <input type='submit' value='Store settings'/>
      <span></span>
    </div>
  </form>
  <script type="text/javascript">
    onload = function() {menuIndex(); dimmerLoad();} 
    function checkLimits(t) {
      if (t.hasAttribute("min")) {
        if (Number(t.value) < Number(t.getAttribute("min"))) {
          t.value = t.getAttribute("min");
        }
      }
      if (t.hasAttribute("max")) {
        if (Number(t.value) > Number(t.getAttribute("max"))) {
          t.value = t.getAttribute("max");
        }
      }
      if (t.hasAttribute("step")) {
        if (Number.isInteger(Number(t.getAttribute("step")))) {
          t.value = Math.round(Number(t.value));
        }
      }
    }
    function dimmerLoad() {
      var xhttp = new XMLHttpRequest();
      xhttp.onreadystatechange = function() {
        if (this.readyState == 4 && this.status == 200) {
          var result = JSON.parse(this.responseText);
          if ("waveformmode" in result) {
            document.getElementsByName("waveformmode")[0].value = result.waveformmode.toString();
          }
          if ("mode100" in result) {
            document.getElementsByName("mode100")[0].value = result.mode100;
          }
          if ("effect" in result) {
            document.getElementsByName("effect")[0].value = result.effect.toString();
          }
          if ("effectmagnitude" in result) {
            document.getElementsByName("effectmagnitude")[0].value = result.effectmagnitude;
          }
          if ("effectgain" in result) {
            document.getElementsByName("effectgain")[0].value = result.effectgain;
          }
          if ("effecttime" in result) {
            document.getElementsByName("effecttime")[0].value = result.effecttime;
          }
          if ("triacmode" in result) {
            document.getElementsByName("triacmode")[0].value = result.triacmode.toString();
          }
          if ("leveloff" in result) {
            document.getElementsByName("leveloff")[0].value = result.leveloff;
          }
          if ("levelon" in result) {
            document.getElementsByName("levelon")[0].value = result.levelon;
          }
          if ("levellounge" in result) {
            document.getElementsByName("levellounge")[0].value = result.levellounge;
          }
        }
      };
      xhttp.open("GET", "dimmerload", true);
      xhttp.send();
    }
  </script>)literal";

const char webMqtt[] = R"literal(
  <form method='POST' action='mqttsave'>
    <div class="settings">
      <b>MQTT status</b><span></span><span></span>
      <label>Client ID</label>
      <span id="clientid">---</span>
      <span></span>
      <label>Connection status</label>
      <span id="status">---</span>
      <span></span>
      <b>MQTT settings</b><span></span><span></span>
      <label>Broker address</label>
      <input type="text" name="brokeraddress"></input>
      <span>Hostname or IP address</span>
      <label>Port</label>
      <input type="number" min=0 max=65535 step=1 onchange="checkLimits(this);" name="mqttport"></input>
      <span></span>
      <label>Username</label>
      <input type="text" name="mqttusername"></input>
      <span></span>
      <label>Password</label>
      <input type="password" autocomplete="off" name="mqttpassword"/>
      <span></span>
      <label>Main topic</label>
      <input type="text" name="maintopic"></input>
      <span></span>
      <label>QOS</label>
      <input type="number" min=0 max=1 step=1 onchange="checkLimits(this);" name="mqttqos"></input>
      <span>Subscribe</span>
      <label>Retain</label>
      <input type="checkbox" name="mqttretain"></input>
      <span>Publish</span>
      <label>MQTT enabled</label>
      <input type="checkbox" name="usemqtt"></input>
      <span></span>
      <span></span>
      <input type='submit' value='Store settings'/>
      <span>Reboot required</span>
      <b>MQTT supported topics</b><span></span><span></span>
    </div>
  </form>
  <script type="text/javascript">
    onload = function() {menuIndex(); mqttLoad();} 
    setInterval(function() { mqttUpdate(); }, 5000); 
    function checkLimits(t) {
      if (t.hasAttribute("min")) {
        if (Number(t.value) < Number(t.getAttribute("min"))) {
          t.value = t.getAttribute("min");
        }
      }
      if (t.hasAttribute("max")) {
        if (Number(t.value) > Number(t.getAttribute("max"))) {
          t.value = t.getAttribute("max");
        }
      }
      if (t.hasAttribute("step")) {
        if (Number.isInteger(Number(t.getAttribute("step")))) {
          t.value = Math.round(Number(t.value));
        }
      }
    }
    function mqttLoad() {
      var xhttp = new XMLHttpRequest();
      xhttp.onreadystatechange = function() {
        if (this.readyState == 4 && this.status == 200) {
          var result = JSON.parse(this.responseText);
          if ("clientid" in result) {
            document.getElementById("clientid").innerHTML = result.clientid;
          }
          if ("status" in result) {
            document.getElementById("status").innerHTML = result.status;
          }
          if ("brokeraddress" in result) {
            document.getElementsByName("brokeraddress")[0].value = result.brokeraddress;
          }
          if ("mqttport" in result) {
            document.getElementsByName("mqttport")[0].value = result.mqttport;
          }
          if ("mqttusername" in result) {
            document.getElementsByName("mqttusername")[0].value = result.mqttusername;
          }
          if ("mqttpassword" in result) {
            document.getElementsByName("mqttpassword")[0].value = result.mqttpassword;
          }
          if ("maintopic" in result) {
            document.getElementsByName("maintopic")[0].value = result.maintopic;
          }
          if ("mqttqos" in result) {
            document.getElementsByName("mqttqos")[0].value = result.mqttqos;
          }
          if ("mqttretain" in result) {
            document.getElementsByName("mqttretain")[0].checked = result.mqttretain;
          }
          if ("usemqtt" in result) {
            document.getElementsByName("usemqtt")[0].checked = result.usemqtt;
          }
          if ("topics" in result) {
            var settingsClass = document.getElementsByClassName("settings")[0];
            result.topics.forEach((top) => {
              var label = document.createElement("label");
              label.innerHTML = top.topic;
              var val = document.createElement("span");
              val.id = top.tag;
              val.innerHTML = top.value;
              var descr = document.createElement("span");
              descr.innerHTML = top.description;
              settingsClass.appendChild(label);
              settingsClass.appendChild(val);
              settingsClass.appendChild(descr);
            });
          }
        }
      };
      xhttp.open("GET", "mqttload", true);
      xhttp.send();
    }
    function mqttUpdate() {
      var xhttp = new XMLHttpRequest();
      xhttp.onreadystatechange = function() {
        if (this.readyState == 4 && this.status == 200) {
          var result = JSON.parse(this.responseText);
          if ("clientid" in result) {
            document.getElementById("clientid").innerHTML = result.clientid;
          }
          if ("status" in result) {
            document.getElementById("status").innerHTML = result.status;
          }
          if ("topics" in result) {
            result.topics.forEach((top) => {
              document.getElementById(top.tag).innerHTML = top.value;
            });
          }
        }
      };
      xhttp.open("GET", "mqttupdate", true);
      xhttp.send();
    }
  </script>)literal";

const char webLog[] = R"literal(
  <div class="logging">
    <textarea readonly name="logarea"></textarea>
    <label>
      <input type="checkbox" name="autoscroll" checked>
      Automatic scrolling
    </label>
  </div>
  <script type="text/javascript">
    onload = function() {menuIndex(); logLoad();}
    setInterval(function() { logUpdate(); }, 1000); 
    function logLoad() {
      var xhttp = new XMLHttpRequest();
      var logarea = document.getElementsByName("logarea")[0];
      xhttp.onreadystatechange = function() {
        if (this.readyState == 4 && this.status == 200) {
          var result = JSON.parse(this.responseText);
          AddtoLogarea(result);
        }
      }
      if (document.getElementsByName('autoscroll')[0].checked) {
        logarea.scrollTop = logarea.scrollHeight;  
      }
      xhttp.open("GET", "logload", true);
      xhttp.send();
    }
    function logUpdate() {
      var xhttp = new XMLHttpRequest();
      xhttp.onreadystatechange = function() {
        if (this.readyState == 4 && this.status == 200) {
          var result = JSON.parse(this.responseText);
          AddtoLogarea(result);
        }
      };
      xhttp.open("GET", "logupdate", true);
      xhttp.send();
    }
    function AddtoLogarea(values) {
      var i = 0;
      var added = false;
      var logarea = document.getElementsByName("logarea")[0];
      var lines = logarea.value.split("\n").length;
      for (i in values) {
        logline = values[i];
        if (lines>=1000) {
          var dlines = logarea.value.split("\n");
          dlines.splice(0, 1);
          logarea.value = dlines.join("\n");  
        } else {
          lines += 1;
          added = true;
        }
        logarea.value += logline + "\n";
      }
      if ((added) && (document.getElementsByName('autoscroll')[0].checked)) {
        logarea.scrollTop = logarea.scrollHeight;
      }
    }
  </script>)literal";

const char webReboot[] = R"literal(
  <form method='POST' action='doreboot'>
    <div class="settings">
      <b>Please confirm reboot</b><span></span><span></span>
      <input type='submit' value='Reboot'/><span></span><span></span>
    </div>
  </form>)literal";

const char webRebooting[] = R"literal(
<!DOCTYPE html>
<html>
  <head>
    <meta content="text/html; charset=UTF-8" http-equiv="content-type">
    <title>Dimmer Control</title>
    <meta content="helly" name="author">
    <meta http-equiv="refresh" content="5;url=/" />
  </head>
  <body>
    <center>
      <h1>Device is rebooting</h1>
      <h2>Reload may be required ...</h2>
    </center>
  </body>
</html>)literal";

#endif
