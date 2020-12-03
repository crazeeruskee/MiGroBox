var stored = 0;
var disp_hide = 0;

function print_disp(message) {
	document.getElementById("calc-display").innerText = message;
}

function number(num) {
	var disp = document.getElementById("calc-display").innerText;
	if (disp_hide == 0) { disp = ""; }
	disp = disp + num;
	document.getElementById("calc-display").innerText = disp;
	disp_hide = disp;
}

function evaluate() {
	var result = 0;
	var disp = document.getElementById("calc-display").innerText;
	op = document.getElementById("calc-display").value;
	if (op == 'times') { result = stored * parseInt(disp); }
	if (op == 'plus') { result = parseInt(stored) + parseInt(disp); }
	if (op == 'minus') { result = stored - parseInt(disp); }
	if (op == 'divide') { result = stored / parseInt(disp); }
	document.getElementById("calc-display").innerText = result;
	disp_hide = 0;
	stored = disp;
}