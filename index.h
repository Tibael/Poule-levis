
const char MAIN_page[] PROGMEM = R"=====(
 <!DOCTYPE html>

<html>

<head>

<style>

.button {

background-color: #990033;
border: none;
color: white;
padding: 7px 15px;
text-align: center;
cursor: pointer;

}

</style>

</head>

<body>
<h1>Poule-Levis</h1>

<div>Heure d'ouverture : <span id="heureOuverture"></span><br></div>
<div>Heure de fermeture : <span id="heureFermeture"></span><br></div>
<div>Heure actuelle : <span id="heureActuelle"></span><br></div>

<form action="/set">
    <label for="TIME-UP">Temps de mont&eacute;e du Poule-Levis (sec) :</label>
    <input type="number" name="TIME-UP" id="TIME-UP">
    <input type="submit" value="Changer">
  </form><br>

<form action="/set">
    <label for="TIME-DOWN">Temps de descente du Poule-Levis (sec) :</label>
    <input type="number" name="TIME-DOWN" id="TIME-DOWN">
    <input type="submit" value="Changer">
  </form><br> 

<form action="/set">
    <label for="TIME-AFTER-SUNRISE">Ouverture, Nombre d'heure apr&egrave;s lever du soleil :</label>
    <select name="TIME-AFTER-SUNRISE" id="TIME-AFTER-SUNRISE">
        <option value="0">0</option>
        <option value="1">1</option>
        <option value="2">2</option>
        <option value="3">3</option>
        <option value="4">4</option>
    </select>
    <input type="submit" value="Appliquer">
  </form><br>

<form action="/set">
<label for="TIME-AFTER-SUNSET">Fermeture, Nombre d'heure apr&egrave;s coucher du soleil :</label>
    <select name="TIME-AFTER-SUNSET" id="TIME-AFTER-SUNSET">
        <option value="0">0</option>
        <option value="1">1</option>
        <option value="2">2</option>
        <option value="3">3</option>
        <option value="4">4</option>
    </select>
<input type="submit" value="Appliquer">
</form><br>

<a href=wifiWipe>
<input type=button class=button value="CHANGER DE WIFI"></a>

<a href=webserial target="_blank">
<input type=button class=button value="CONSOLE"></a>
<br> <br>

<a href="motor-down" >
<input type=button class=button value="DESCENDRE LE POULE-LEVIS"></a>

<a href="motor-up" >
<input type=button class=button value="REMONTER LE POULE-LEVIS"></a>
<br> <br>
<a href="motor-stop" >
<input type=button class=button value="STOPPER LE MOTEUR"></a>

<script>

if (!!window.EventSource) {
  var source = new EventSource('/events');
  source.addEventListener('open', function(e) {
    console.log("Events Connected");
  }, false);
  source.addEventListener('error', function(e) {
    if (e.target.readyState != EventSource.OPEN) {
      console.log("Events Disconnected");
    }
  }, false);

  source.addEventListener('message', function(e) {
    console.log("message", e.data);
  }, false);

  source.addEventListener('WIFI SETUP', function(e) {
    console.log("Wifi Setup address : ", e.data);
    document.getElementById("IPaddress").href = e.data;
  }, false);

  source.addEventListener('HEURE OUVERTURE', function(e) {
    console.log("Heure ouverture : ", e.data);
    document.getElementById("heureOuverture").innerHTML = e.data;
  }, false);

  source.addEventListener('HEURE FERMETURE', function(e) {
    console.log("heure fermeture : ", e.data);
    document.getElementById("heureFermeture").innerHTML = e.data;
  }, false);

  source.addEventListener('HEURE ACTUELLE', function(e) {
    console.log("heure actuelle :", e.data);
    document.getElementById("heureActuelle").innerHTML = e.data;
  }, false);

  source.addEventListener('TIME-AFTER-SUNRISE', function(e) {
    console.log("time after sunrise :", e.data);
    document.getElementById("TIME-AFTER-SUNRISE").selectedIndex = e.data;
  }, false);

  source.addEventListener('TIME-AFTER-SUNSET', function(e) {
    console.log("time after sunset :", e.data);
    document.getElementById("TIME-AFTER-SUNSET").selectedIndex = e.data;
  }, false);

  source.addEventListener('TIME-UP', function(e) {
    console.log("time up :", e.data);
    document.getElementById("TIME-UP").value = e.data;
  }, false);

  source.addEventListener('TIME-DOWN', function(e) {
    console.log("time down :", e.data);
    document.getElementById("TIME-DOWN").value = e.data;
  }, false);
}
</script>
</body>
</html>
)=====";
