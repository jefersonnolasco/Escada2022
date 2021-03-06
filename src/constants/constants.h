#ifndef CONSTANTS_H
#define CONSTANTS_H

#include <Arduino.h>

const char index_html[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html>
<head>
<meta charset="UTF-8">
<title>Escada Inteligente</title>
<script>
  function change(value){
    document.getElementById("sensor").value=value;
  }
</script>
<style>
  html { 
    font-family: Helvetica; 
    display: inline-block; 
    margin: 0px auto; 
    text-align: center;
  }
  body{
    margin-top: 50px;
  } 
  .title{ 
    color: rgb(0,86,179); 
  }
  .text{
    font-size: 120%;
  }

  .button {
    border: none;
    color: white;
    padding: 10px 25px;
    font-size: 16px;
    margin: 3px 1px;
    transition-duration: 0.4s;
    cursor: pointer;
    border-radius: 5px;
    text-decoration: none;
  }

  .button1 {
    background-color: rgb(197, 224, 253);
    color: black; 
    border: 1px solid rgb(130, 188, 250);
  }

  .button1:hover {
    background-color: rgb(0,86,179);
    color: white;
  }

  .buttonx {
    background-color: rgb(255, 255, 255);
    color: black; 
    border: 1px solid rgb(54, 86, 121);
  }

  .buttonx:hover {
    background-color: rgb(35, 141, 255);
    color: white;
  }

  .lbl1 {
    font-size: 60%;
    position: relative;
    bottom: 5px;
    right: -10px;
  }

  .lbl2 {
    font-size: 60%;
    position: relative;
    bottom: 5px;
    left: 360px;
  }

  .ipt {
    width: 100%;
  }

  .div-main {
    background-color: rgb(250, 250, 250);
    text-align: left;
    max-width: 420px;
    border-top: solid rgb(160, 160, 160) 1px;
    border-bottom: solid rgb(160, 160, 160) 1px;
    padding: 30px;
    margin: auto;
    margin-bottom: 10px;
  }

  .output {
    font-size: 80%;
  }

</style>
</head>
<body>
  <h1 class="title">Escada Inteligente</h1>
  <form action="/">
    <div class="div-main">
      <label class="text" for="cor">Selecione a cor da iluminação:</label>
      <input name="color" id="color" type="color" value="@@color_hex@@"><br><br> 

      <label class="text" for="velocity">Velocidade dos passos:</label>
      <input class="ipt" name="velocity" id="velocity" type="range" min="1" max="2000" value="@@velocity@@">
      <span class="lbl1">rápido</span>
      <span class="lbl2">lento</span><br><br>

      <label class="text" for="pause">Pausa para desligar:</label>
      <input class="ipt" name="pause" id="pause" type="range" min="3000" max="20000" value="@@pause@@">
      <span class="lbl1">menor</span>
      <span class="lbl2">maior</span><br><br>

      <label class="text" for="bright">Intensidade do brilho:</label>
      <input class="ipt" name="bright" id="bright" type="range" min="10" max="220" value="@@bright@@">
      <span class="lbl1">menor</span>
      <span class="lbl2">maior</span><br><br>

      <label class="text" for="daylight">Sensibilidade da luz natural:</label>
      <input class="ipt" name="daylight" id="daylight" type="range" min="1" max="1500" oninput="this.nextElementSibling.value = this.value" value="@@daylight@@">
      <output class="output"></output>
      <span class="lbl1">menor</span>
      <span class="lbl2">maior</span><br><br>

      <span>Simular ativação de sensores</span><br>
      <input type="hidden" id="sensor" name="sensor" value="">
      <button class="button buttonx" onclick="change(1)">Sensor 1</button>
      <button class="button buttonx" onclick="change(2)">Sensor 2</button>
      <button class="button buttonx" onclick="change(3)">Sensor 3</button>
      <br><br>
      
      <span>Luz atual: @@luzatual@@</span>
    </div>
    <button type="submit" class="button button1">Salvar</button>
  </form> 

</body>
</html>
)rawliteral";

#endif