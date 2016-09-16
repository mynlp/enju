//////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2005, Tsujii Laboratory, The University of Tokyo.
//  All rights reserved.
//
//  highlight covered words
//
//////////////////////////////////////////////////////////////////////

var highlight_color = "yellow";

function highlight_words_on(start_id, end_id) {
  for (id = start_id; id < end_id; ++id) {
    var target = document.getElementById(id);
    target.style.backgroundColor=highlight_color;
  }
}
function highlight_words_off(start_id,end_id) {
  for (id = start_id; id < end_id; ++id ) {
    document.getElementById(id).style.backgroundColor=null;
  }
}

