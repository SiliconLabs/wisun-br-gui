//##################get elements by ID##################
var message = document.getElementById('message');
const toggle_dp = document.getElementById("toggle_dp"); //toggle dropdown
const restartn_bt = document.getElementById("restartn_bt");
const startn_bt = document.getElementById("startn_bt");
const stopn_bt = document.getElementById("stopn_bt");
const status_icon = document.getElementById("status_icon");
const status_text = document.getElementById("status_text");
const network_name = document.getElementById("network-name");
const domain = document.getElementById("domain");
const Mode = document.getElementById("Mode");
const PhyPlan = document.getElementById("PhyPlan");
const PhyMode = document.getElementById("PhyMode");
const classe = document.getElementById("classe");
const PanID = document.getElementById("PanID");
const Size = document.getElementById("Size");
const GTK0 = document.getElementById("GTK0");
const GTK1 = document.getElementById("GTK1");
const GTK2 = document.getElementById("GTK2");
const GTK3 = document.getElementById("GTK3");
const GAK0 = document.getElementById("GAK0");
const GAK1 = document.getElementById("GAK1");
const GAK2 = document.getElementById("GAK2");
const GAK3 = document.getElementById("GAK3");
const test_jump = document.getElementById("test_jump");
//##################Onload actions##################
let wisun_dbus;
let properties;
window.onload = function exampleFunction() {
  state();
}
//##################wsbrd.conf file reading##################
promise = cockpit.file("/etc/wsbrd.conf", { superuser: "try" }).read().
  done(function (initial_conf, tag) {

    message.value += initial_conf;
  });

function save_config() {
  console.log("input input");
  cockpit.file("/etc/wsbrd.conf", { superuser: "try" }).replace(message.value);
}

//##################Dropdown##################

function dropdown_toggel() {
  document.getElementById("dropdown_content").classList.toggle("show");
}

// Close the dropdown if the user clicks outside of it
window.onclick = function (event) {
  if (!event.target.matches('.pf-c-dropdown__toggle')) {
    var dropdowns = document.getElementsByClassName('pf-c-dropdown__menu pf-m-align-right');
    var i;
    for (i = 0; i < dropdowns.length; i++) {
      var openDropdown = dropdowns[i];
      if (openDropdown.classList.contains('show')) {
        openDropdown.classList.remove('show');
      }
    }
  }
}
//##################Service start,restart and stop##################

function restart_network() {
  cleare_configuration();
  cockpit.spawn(["systemctl", "restart", "wisun-borderrouter.service"], { superuser: "require" })
    .stream(rn_success)
    .then(state)
    .catch(rn_fail);
}
function start_network() {
  cleare_configuration();
  cockpit.spawn(["systemctl", "start", "wisun-borderrouter.service"], { superuser: "require" })
    .stream(rn_success)
    .then(state)
    .catch(rn_fail);
}

function stop_network() {
  cleare_configuration();
  cockpit.spawn(["systemctl", "stop", "wisun-borderrouter.service"], { superuser: "require" })
    .stream(rn_success)
    .then(state)
    .catch(rn_fail);
}

function rn_success(data) {
  console.log(data);
}

function rn_fail(data) {
  console.log("fail")
  console.log(data);
}

function change_status(data) {
  if (data.trim() === "active") {
    status_icon.className = "fas fa-sync-alt pf-u-success-color-100"
    status_text.innerHTML = "Running"
    console.log(data);
  }
  else if (data.trim() === "inactive") {
    status_icon.className = "fas fa-pause --pf-global--disabled-color--100"
    status_text.innerHTML = "Inactive"
    console.log(data);
  }
  else {
    status_icon.className = "fas fa-exclamation-circle pf-u-danger-color-100"
    status_text.innerHTML = "Failed to start service"
    console.log(data);
  }
}

function state() {

  cockpit.spawn(["systemctl", "is-active", "wisun-borderrouter.service"], { superuser: "require" })
    .stream(change_status)
    .then(get_status);
}

function get_status() {
  wisun_dbus = cockpit.dbus("com.silabs.Wisun.BorderRouter", { "bus": "system" });
  properties = wisun_dbus.proxy("com.silabs.Wisun.BorderRouter", "/com/silabs/Wisun/BorderRouter");
  properties.wait().then(function () {
    if (properties.valid && typeof properties.WisunMode != 'undefined') {
      network_name.innerHTML = properties.WisunNetworkName;
      domain.innerHTML = properties.WisunDomain;
      PanID.innerHTML = "0x" + properties.WisunPanId.toString(16).toUpperCase();
      Size.innerHTML = properties.WisunSize.toString(16).toUpperCase();
      GAK0.innerHTML = base64_decode(properties.Gaks[0]);
      GAK1.innerHTML = base64_decode(properties.Gaks[1]);
      GAK2.innerHTML = base64_decode(properties.Gaks[2]);
      GAK3.innerHTML = base64_decode(properties.Gaks[3]);
      GTK0.innerHTML = base64_decode(properties.Gtks[0]);
      GTK1.innerHTML = base64_decode(properties.Gtks[1]);
      GTK2.innerHTML = base64_decode(properties.Gtks[2]);
      GTK3.innerHTML = base64_decode(properties.Gtks[3]);
      if(properties.WisunClass != 0){
        PhyPlan.innerHTML = "Class";
        PhyMode.innerHTML = "Mode"
        classe.innerHTML = properties.WisunClass;
        Mode.innerHTML = properties.WisunMode.toString(16).toUpperCase();
      }else if(properties.WisunClass == 0){
        PhyPlan.innerHTML = "Channel Plan ID";
        PhyMode.innerHTML = "PHY Mode ID"
        classe.innerHTML = properties.WisunChanPlanId;
        Mode.innerHTML =  properties.WisunPhyModeId.toString(16).toUpperCase();
      }
    } else if (properties.valid && typeof properties.WisunMode == 'undefined') {
      properties.onchange = get_status();
      a = false;
    } else if (properties.valid == false && typeof properties.WisunMode == 'undefined') {
      properties.onchange = get_status();
    } else {
      cleare_configuration();
    }
  });
}

function cleare_configuration() {
  network_name.innerHTML = "";
  classe.innerHTML = "";
  domain.innerHTML = "";
  Mode.innerHTML = "";
  PanID.innerHTML = "";
  Size.innerHTML = "";
  GAK0.innerHTML = "";
  GAK1.innerHTML = "";
  GAK2.innerHTML = "";
  GAK3.innerHTML = "";
  GTK0.innerHTML = "";
  GTK1.innerHTML = "";
  GTK2.innerHTML = "";
  GTK3.innerHTML = "";
}

function base64_decode(base64) {
  encoded_base64 = atob(base64);
  let string = "";
  for (let cnt = 0; cnt < encoded_base64.length; cnt++) {
    string += ("0" + encoded_base64.charCodeAt(cnt).toString(16)).slice(-2).toUpperCase();
    if (cnt < encoded_base64.length - 1) {
      string += ":";
    }
  }
  return string;
}
function jump(){

  cockpit.jump("/system/logs#/?prio=debug&service=wisun-borderrouter.service");

}
//Event listeners
message.addEventListener("input", save_config); //to track textbox inputs
toggle_dp.addEventListener("click", dropdown_toggel); //Dropdown
restartn_bt.addEventListener("click", restart_network);
startn_bt.addEventListener("click", start_network);
stopn_bt.addEventListener("click", stop_network);
test_jump.addEventListener("click",jump)
