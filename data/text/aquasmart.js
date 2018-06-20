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
  var dados = 't1=' + document.getElementById('t1').checked + "&t2=" + document.querySelector(.liga_rele1AMIN).value + "&t3=" + document.getElementById('t3').checked + "&t4=" + document.getElementById('t4').checked;
  xmlhttp.onreadystatechange = function(){}
  xmlhttp.open("GET", 'setTomadas?' + dados, true);
  xmlhttp.send();
}

function EnviaAgendamento(){
	var xmlhttp = newXmlHttpRequest();
	var dados =
		't1' + document.querySelector(.liga_rele1HORA).value +
		"&t2" + document.querySelector(.liga_rele1MIN).value +
		"&t3" + document.querySelector(.desliga_rele1HORA).value + 
		"&t4" + document.querySelector(.desliga_rele1MIN).value +
		
		"&t5" + document.querySelector(.liga_rele2HORA).value +
		"&t6" + document.querySelector(.liga_rele2MIN).value +
		"&t7" + document.querySelector(.desliga_rele2HORA).value +
		"&t8" + document.querySelector(.desliga_rele2MIN).value +
		
		"&t9" + document.querySelector(.liga_rele3HORA).value +
		"&t10" + document.querySelector(.liga_rele3MIN).value +
		"&t11" + document.querySelector(.liga_rele3HORA).value +
		"&t12" + document.querySelector(.liga_rele3MIN).value +
		
		"&t13" + document.querySelector(.liga_rele4HORA).value +
		"&t14" + document.querySelector(.liga_rele4MIN).value +
		"&t15" + document.querySelector(.liga_rele4HORA).value +
		"&t16" + document.querySelector(.liga_rele4MIN).value;
 		
	xmlhttp.onreadystatechange = function(){}
	xmlhttp.open("GET",'agendados?' + dados, true);
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
