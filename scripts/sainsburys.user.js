// ==UserScript==
// @name					Sainsbury's Bank CSV Export
// @namespace			https://github.com/mmmsoup
// @description		Download CSV files of your transactions for Sainsbury's Bank accounts in Fine standard format (https://github.com/mmmsoup/fine). 
// @version				1
// @match					https://online.sainsburysbank.co.uk/*
// @grant					none
// ==/UserScript==

const months = {
	"Jan": "01",
  "Feb": "02",
  "Mar": "03",
  "Apr": "04",
  "May": "05",
  "Jun": "06",
  "Jul": "07",
  "Aug": "08",
  "Sep": "09",
  "Oct": "10",
  "Nov": "11",
  "Dec": "12"
}

function export_csv() {
  const form = document.getElementById("csvexport-form");
  const datebox = form.querySelector("#csvexport-datebox");
  const tocsv = form.querySelector("#csvexport-tocsv");
  
  if (tocsv.style.width != "0px") { // show date picker
  	tocsv.style.width = "0px";
    datebox.style.width = datebox.getAttribute("initial_width");
    datebox.children[1].select();
  } else { // actually download CSV
    const fromstr = datebox.children[1].value;
    const tostr = datebox.children[3].value;
    
    // day, month, year
    const fromparts = fromstr.split("/").map(function(x) { return parseInt(x) });
    const toparts = tostr.split("/").map(function(x) { return parseInt(x) });
    if (fromparts.length != 3 || toparts.length != 3) {
    	alert("date should be in d-m-y order, separated by forward slashes (/)");
      return;
    }
    for (var i = 0; i < 3; i++) {
      console.log(fromparts[i], toparts[i], NaN);
    	if (fromparts[i] !== fromparts[i]) {
        alert("invalid character in date '"+fromstr+"'");
        return;
      }
      if (toparts[i] !== toparts[i]) {
        alert("invalid character in date '"+tostr+"'");
        return;
      }
    }
    const fromdate = fromparts[2] * 10000 + fromparts[1] * 100 + fromparts[0];
    const todate = toparts[2] * 10000 + toparts[1] * 100 + toparts[0];
    if (fromdate > todate) {
    	alert("from date more recent than to date!");
      return;
    }
    
    
  	console.log("bonjour");
    datebox.style.width = "0px";
  	tocsv.style.width = tocsv.getAttribute("initial_width");
  
    const acct_sum_el = document.querySelector("div.account-summary");
    var output = "Sainsbury's Bank: " + acct_sum_el.querySelector("h2").innerText + " [" + acct_sum_el.querySelector("p.sort-code > strong").innerText + " " + acct_sum_el.querySelector("p.account-number > strong").innerText + "]\n";
    const rows = document.querySelectorAll(".account-transactions-table > tbody > tr.no-transaction-details");
    const reverse = datebox.getAttribute("reverse") == "true";
    for (var i = 0; i < rows.length; i++) {
    	var row = rows[reverse ? rows.length - i - 1 : i];
      var date = parseInt("20" + row.children[0].innerText.slice(7,9) + months[row.children[0].innerText.slice(3,6)] + row.children[0].innerText.slice(0,2));
      if (date >= fromdate && date <= todate) {
        var datestr = date.toString();
      	output += "\"" + datestr.slice(0, 4) + "-" + datestr.slice(4,6) + "-" + datestr.slice(6,8) + "\",\"" + row.children[2].innerText.replace(/[\s£,]+/g,"") + "\",\"" + row.children[1].innerText + "\"\n";
      }
    }
    
    const today = new Date();
		const date_today = today.getFullYear() + "-" + String(today.getMonth() + 1).padStart(2, '0') + "-" + String(today.getDate()).padStart(2, '0');
    
    const file = new Blob([output], {type: "text/csv"});
    const a = document.createElement("a");
    const url = URL.createObjectURL(file);
    a.href = url;
    a.download = "sainsburys-" + date_today + ".csv";
    document.body.appendChild(a);
    a.click();
    setTimeout(function() {
    	document.body.removeChild(a);
    	window.URL.revokeObjectURL(url);
    }, 0);
  }
}

const styles = " \
	form#csvexport-form { \
		display: ruby; \
	} \
	form#csvexport-form input[type=text] { \
		text-align: center; \
		height: 16px; \
 		display: table; \
  	width: 80px; \
	} \
	form#csvexport-form input[type=submit], form#csvexport-form label { \
		border: none; \
		color: #b9005c; \
		font-size: 12px; \
		font-weight: bold; \
		font-family: 'mary_ann_betabold'; \
		line-height: inherit; \
	} \
	form#csvexport-form input[type=submit]:hover { \
		cursor: pointer; \
	} \
	.csvexport-hideable { \
		transition: width 0.5s; \
	} \
	div#csvexport-datebox { \
		display: flex; \
		overflow: clip; \
	} \
	div#csvexport-datebox * { \
		margin: 0px 4px; \
	}	\
	div#csvexport-datebox label { \
		line-height: inherit; \
  	clear: none; \
	} \
"

var stylesheet = document.createElement("style")
stylesheet.innerText = styles
document.head.appendChild(stylesheet)

var form = document.createElement("form");
form.action = "javascript:document.getElementById('csvexport-dummy-button').click()";
form.id = "csvexport-form";
form.style.paddingRight = "8px";

var parent = document.querySelector("#LatestTransactionPanel > span");
parent.style.whiteSpace = "nowrap";
parent.style.display = "flex";
parent.style.height = "26px";
parent.style.lineHeight = parent.style.height;
parent.insertBefore(form, parent.children[0]);

var datebox = document.createElement("div");
datebox.id = "csvexport-datebox";
var dates = [document.createElement("input"), document.createElement("input")];
dates[0].type = "text";
dates[0].name = "from";
dates[1].type = "text";
dates[1].name = "to";
var labels = [document.createElement("label"), document.createElement("label")];
labels[0].for = dates[0].name;
labels[0].innerText = "From";
labels[1].for = dates[1].name;
labels[1].innerText = "To";
const transactions = document.querySelectorAll(".account-transactions-table > tbody > tr.no-transaction-details");
var extrema = [transactions[0], transactions[transactions.length-1]].map(function(el) { return parseInt("20" + el.children[0].innerText.slice(7,9) + months[el.children[0].innerText.slice(3,6)] + el.children[0].innerText.slice(0,2)) });
if (extrema[1] < extrema[0]) {
	extrema.sort();
	datebox.setAttribute("reverse", "true");
}
extrema.forEach((date, i) => {
  const str = date.toString();
  dates[i].value = str.slice(6,8) + "/" + str.slice(4,6) + "/" + str.slice(0, 4);
});
datebox.appendChild(labels[0]);
datebox.appendChild(dates[0]);
datebox.appendChild(labels[1]);
datebox.appendChild(dates[1]);
form.appendChild(datebox);
datebox.setAttribute("initial_width", getComputedStyle(datebox).width);
datebox.style.width = "0px";
datebox.classList.add("csvexport-hideable");

var submit = [document.createElement("input"), document.createElement("input")];
submit[0].type = "submit";
submit[1].type = "submit";
submit[0].value = "Export";
submit[1].value = " to CSV";
submit[0].style.paddingRight = "0px";
submit[1].style.paddingLeft = "0px";
submit[1].style.paddingRight = "0px";
submit[1].id = "csvexport-tocsv";
submit[1].classList.add("csvexport-hideable");
form.appendChild(submit[0]);
form.appendChild(submit[1]);
submit[1].setAttribute("initial_width", getComputedStyle(submit[1]).width);
submit[1].style.width = submit[1].getAttribute("initial_width");

var dummy = document.createElement("button");
dummy.id = "csvexport-dummy-button";
dummy.style.display = "none";
dummy.onclick = export_csv;
document.body.appendChild(dummy);
