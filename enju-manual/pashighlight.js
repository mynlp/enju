// highlight predicate and arguments

var predicate_color = "pink";
var argument_color = "yellow";
var label_color = "white";
var label_border_color = "red";

function predicate_on(id) {
  var target = document.getElementById(id);
  target.style.backgroundColor=predicate_color;
}
function argument_on(label,id) {
  var target = document.getElementById(id);
  target.style.backgroundColor=argument_color;
  var popup = document.getElementById(label);
  if (popup) {
    var target_left = target.offsetLeft;
    var target_top = target.offsetTop;
    popup.style.borderStyle = "solid";
    popup.style.borderWidth = "1px";
    popup.style.borderColor = label_border_color;
    popup.style.backgroundColor = label_color;
    popup.style.left = target_left + "px";
    popup.style.top = target_top - parseInt( popup.offsetHeight ) + "px";
    popup.style.visibility = "visible";
  }
}
function highlight_off(label,id) {
  document.getElementById(id).style.backgroundColor=null;
  var popup = document.getElementById(label);
  if (popup) {
    popup.style.visibility = "hidden";
  }
}

