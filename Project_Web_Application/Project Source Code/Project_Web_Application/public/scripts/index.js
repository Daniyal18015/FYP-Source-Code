const loginElement = document.querySelector('#login-form');
const contentElement = document.querySelector("#content-sign-in");
const userDetailsElement = document.querySelector('#user-details');
const authBarElement = document.querySelector("#authentication-bar");

// Elements for sensor readings
const voltageElement = document.getElementById("voltage");
const currentElement = document.getElementById("current");
const realElement = document.getElementById("real");
const reactiveElement = document.getElementById("reactive");
const apparentElement = document.getElementById("apparent");
const pfElement = document.getElementById("factor");
const Bill_1_Element = document.getElementById("bill_1");
const Bill_2_Element = document.getElementById("bill_2");
const Unit_1_Element = document.getElementById("unit_1");
const Unit_2_Element = document.getElementById("unit_2");



// MANAGE LOGIN/LOGOUT UI
const setupUI = (user) => {
  if (user) {
    //toggle UI elements
    loginElement.style.display = 'none';
    contentElement.style.display = 'block';
    authBarElement.style.display ='block';
    userDetailsElement.style.display ='block';
    userDetailsElement.innerHTML = user.email;

    // get user UID to get data from database
    var uid = user.uid;
    console.log(uid);

    // Database paths (with user UID)
    var dbPathVoltage = 'UsersData/' + uid.toString() + '/Input_Voltage';
    var dbPathCurrent = 'UsersData/' + uid.toString() + '/Load_Current';
    var dbPathRealPower = 'UsersData/' + uid.toString() + '/Real_Power';
    var dbPathReactivePower = 'UsersData/' + uid.toString() + '/Reactive_Power';
    var dbPathApparentPower = 'UsersData/' + uid.toString() + '/Apparent_Power';
    var dbPathPowerFactor = 'UsersData/' + uid.toString() + '/Power_Factor';
    var dbPathUnits_1 = 'UsersData/' + uid.toString() + '/Units_Line1';
    var dbPathBill_1 = 'UsersData/' + uid.toString() + '/Bill_Line1';
    var dbPathUnits_2 = 'UsersData/' + uid.toString() + '/Units_Line2';
    var dbPathBill_2 = 'UsersData/' + uid.toString() + '/Bill_Line2';

    // Database references
    var dbRefVoltage = firebase.database().ref().child(dbPathVoltage);
    var dbRefCurrent = firebase.database().ref().child(dbPathCurrent);
    var dbRefRealPower = firebase.database().ref().child(dbPathRealPower);
    var dbRefReactivePower = firebase.database().ref().child(dbPathReactivePower);
    var dbRefApparentPower = firebase.database().ref().child(dbPathApparentPower);
    var dbRefPowerFactor = firebase.database().ref().child(dbPathPowerFactor);
    var dbRefUnits_1 = firebase.database().ref().child(dbPathUnits_1);
    var dbRefBill_1 = firebase.database().ref().child(dbPathBill_1);
    var dbRefUnits_2 = firebase.database().ref().child(dbPathUnits_2);
    var dbRefBill_2 = firebase.database().ref().child(dbPathBill_2);
    // Update page with new readings
    dbRefVoltage.on('value', snap => {
      voltageElement.innerText = snap.val().toFixed(2);
    });

    dbRefCurrent.on('value', snap => {
      currentElement.innerText = snap.val().toFixed(2);
    });

    dbRefRealPower.on('value', snap => {
      realElement.innerText = snap.val().toFixed(2);
    });

    dbRefReactivePower.on('value', snap => {
      reactiveElement.innerText = snap.val().toFixed(2);
    });

    dbRefApparentPower.on('value', snap => {
      apparentElement.innerText = snap.val().toFixed(2);
    });

    dbRefPowerFactor.on('value', snap => {
      pfElement.innerText = snap.val().toFixed(2);
    });

    dbRefUnits_1.on('value', snap => {
      Unit_1_Element.innerText = snap.val().toFixed(2);
    });

    dbRefBill_1.on('value', snap => {
      Bill_1_Element.innerText = snap.val().toFixed(2);
    });

    dbRefUnits_2.on('value', snap => {
      Unit_2_Element.innerText = snap.val().toFixed(2);
    });

    dbRefBill_2.on('value', snap => {
      Bill_2_Element.innerText = snap.val().toFixed(2);
    });

  // if user is logged out
  } else{
    // toggle UI elements
    loginElement.style.display = 'block';
    authBarElement.style.display ='none';
    userDetailsElement.style.display ='none';
    contentElement.style.display = 'none';
  }
}