function newXmlHttpRequest() {
  if (window.XMLHttpRequest) {
    return new XMLHttpRequest();
  } else if (window.ActiveXObject) {
    return new ActiveXObject("Microsoft.XMLHTTP");
  }
}

$(function() {
  $('#t2').change(function() {
    getData();
  })
})

function EnviaDados(){
  var xmlhttp = newXmlHttpRequest();
  var dados = 't1=' + document.getElementById('t1').checked + "&t2=" + document.getElementById('t2').checked + "&t3=" + document.getElementById('t3').checked + "&t4=" + document.getElementById('t4').checked;
  xmlhttp.onreadystatechange = function(){}
  xmlhttp.open("GET", 'setTomadas?' + dados, true);
  xmlhttp.send();
}

function alloff(){
  var xmlhttp = newXmlHttpRequest();
  xmlhttp.onreadystatechange = function(){}
  xmlhttp.open("GET", 'alloff');
  xmlhttp.send();
}

setInterval(function() {
  // Call a function repetatively with 10 Second interval
  getData();
}, 30000); //10000mSeconds update rate

function getData() {
  var xhttp = newXmlHttpRequest();
  xhttp.onreadystatechange = function() {
    if (this.readyState == 4 && this.status == 200) {
      var resposta = xmlhttp.responseText;
      var Tid;
      var i;
      for(i = 1 ;i<=length(Resposta) ;i++ ){
        Tid = 't';
        Tid += i;
        $(Tid).bootstrapToggle(resposta[i-1]?'on':'off');
      }
    }
  };
  xhttp.open("GET", "callback", true);
  xhttp.send();
}

var start=0;
function editLabel(idLabel,idform) {
  // --[Mudança de estado por toggle]--
  if((new Date().getTime() - start) > 1000){
    $(idform).toggleClass('hidden');
    if(($(idform).hasClass('hidden')) & (idform.value != '')){
      idLabel.innerHTML = idform.value;
    }
  }
  start = new Date().getTime();

}

function hideforms() {
  // $('#formT1').addClass('hidden');
  // $('#formT2').addClass('hidden');
  // $('#formT3').addClass('hidden');
  // $('#formT4').addClass('hidden');
}