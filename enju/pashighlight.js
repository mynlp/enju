// Copyright (c) 2005, Tsujii Laboratory, The University of Tokyo.
// All rights reserved.
//
// highlight predicate and arguments

var predicate_color = "pink";
var argument_color = "yellow";
var label_color = "white";
var label_border_color = "red";

function predicate_on(id) {
  var target = document.getElementById(id);
  if (target) {
    target.style.backgroundColor=predicate_color;
  }
}
function argument_on(label,id) {
  var target = document.getElementById(id);
  var popup = document.getElementById(label);
  if (target && popup) {
    target.style.backgroundColor=argument_color;
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
  var target = document.getElementById(id);
  if (target) {
    target.style.backgroundColor=null;
  }
  var popup = document.getElementById(label);
  if (popup) {
    popup.style.visibility = "hidden";
  }
}

