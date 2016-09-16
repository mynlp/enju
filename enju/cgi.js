//////////////////////////////////////////////////////////////////////
////
////  Copyright (c) 2007, Tsujii Laboratory, The University of Tokyo.
////  All rights reserved.
////
////  User interface for Enju CGI
////
//////////////////////////////////////////////////////////////////////

// XSLT for tree display
var fs_namespace = "http://www-tsujii.is.s.u-tokyo.ac.jp/lilfes";
var xhtml_namespace = "http://www.w3.org/1999/xhtml";
var xslt_file = "fs.xsl";
var xslt = null;

// HTML elements
var display_result = null;
var sentence_input_form = null;

// HTTP request
var cgi_name = "http://kmcs.nii.ac.jp/%7Eyusuke/enju/parse.cgi";
var parse_request = null;

// Parse tree info.
var parse_tree_xml = null;
var node_list = {};  // list of node information
var arg_labels = [ "mod", "arg1", "arg2", "arg3", "arg4" ];

// hack for IS
function dummyCreateElementNS( ns, tag ) {
  return document.createElement( tag );
}
if ( ! document.createElementNS ) {
  document.createElementNS = dummyCreateElementNS;
}

//////////////////////////////////////////////////////////////////////
//
//  library

// well-known hack for HTTP connection
function createXMLHttpRequest() {
  if ( window.XMLHttpRequest ) {
    return new XMLHttpRequest();
  } else {
    return new ActiveXObject( "Microsoft.XMLHTTP" );
  }
}

// load an XML document
function loadXML(url, callback) {
  var xhr = createXMLHttpRequest();

  xhr.onreadystatechange = function() {
    if (xhr.readyState == 4 && xhr.status == 200) {
      callback(xhr.responseXML);
    }
  }

  xhr.open("GET", url);
  xhr.send();
}

// delete all children of an element
function deleteChildren( elem ) {
  if ( ! elem ) { return; }
  if ( ! elem.childNodes ) { return; }
  while ( elem.childNodes.length > 0 ) {
    elem.removeChild( elem.firstChild );
  }
}

// show a specified element
function showElement( id ) {
  var elem = document.getElementById( id );
  if ( elem ) {
    elem.style.visibility = "visible";
  }
}

// hide a specified element
function hideElement( id ) {
  var elem = document.getElementById( id );
  if ( elem ) {
    elem.style.visibility = "hidden";
  }
}

// get absolute position of an element
function elementPosition( elem ) {
  var left = 0;
  var top = 0;
  var e = elem;
  while ( e ) {
    left += e.offsetLeft;
    top += e.offsetTop;
    e = e.offsetParent;
  }
  return { left: left, top: top };
}

// apply "fs.xsl" to a given XML
function applyXSLT( xml, target ) {
  var fragment = document.implementation.createDocument( "", "", null );
  target.appendChild( xslt.transformToFragment( xml, fragment ) );
}

// put an element to "display_result" div
function putResult( element ) {
  if ( display_result == null ) {
    display_result = document.getElementById( "display_result" );
  }
  deleteChildren( display_result );
  display_result.appendChild( element );
}

//////////////////////////////////////////////////////////////////////

function createPopup( target, label ) {
  var popup = document.getElementById( label );
  if ( ! popup ) {
    popup = document.createElement( "div" );
    popup.id = label;
    popup.appendChild( document.createTextNode( label ) );
    popup.style.position = "absolute";
    popup.style.borderStyle = "solid";
    popup.style.borderWidth = "1px";
    popup.style.borderColor = "red";
    popup.style.backgroundColor = "white";
    popup.style.fontSize = "x-small";
  }
  popup.style.visibility = "visible";
  display_result.appendChild( popup );
  var target_position = elementPosition( target );
  target_position.left += target.offsetWidth / 2;
  popup.style.left = target_position.left + "px";
  target_position.top -= popup.offsetHeight;
  popup.style.top = target_position.top + "px";
  return popup;
}

function highlightOn( id ) {
  var target = document.getElementById( id );
//  target.style.backgroundColor = "pink";
  target.style.color = "red";
  var node = node_list[ id ];
  if ( node.pred ) {
    // terminal
    createPopup( target, node.pred );  // predicate type
    for ( var i = 0; i < arg_labels.length; ++i ) {
      var arg = node[ arg_labels[ i ] ];
      if ( arg ) {
        var arg_node = document.getElementById( arg );
        if ( arg_node ) { createPopup( arg_node, arg_labels[ i ] ); }
      }
    }
  } else if ( node.head ) {
    var head_node = document.getElementById( node.head );
    createPopup( head_node, "head" );
    if ( node.head != node.sem_head ) {
      var sem_head_node = document.getElementById( node.sem_head );
      createPopup( sem_head_node, "sem_head" );
    }
  }
}

function highlightOff( id ) {
  var target = document.getElementById( id );
//  target.style.backgroundColor = null;
  target.style.color = null;
  var node = node_list[ id ];
  if ( node.pred ) {
    // terminal
    document.getElementById( node.pred ).style.visibility = "hidden";
    for ( var i = 0; i < arg_labels.length; ++i ) {
      hideElement( arg_labels[ i ] );
    }
  } else if ( node.head ) {
    // nonterminal
    hideElement( "head" );
    hideElement( "sem_head" );
  }
}

function createFS( id ) {
  var node = node_list[ id ];
  var fs = document.createElementNS( fs_namespace, "fs" );
  for ( var attr in node ) {
    var feat = document.createElementNS( fs_namespace, "feat" );
    feat.setAttribute( "edge", attr );
    feat.appendChild( document.createTextNode( node[ attr ] ) );
    fs.appendChild( feat );
  }
  var div = document.createElementNS( xhtml_namespace, "div" );
  div.appendChild( fs );
  return div;
}

//////////////////////////////////////////////////////////////////////
//
//  Main program

function parseSentence( s, m ) {
  if ( !xslt ) {
    // XSLT is not loaded yet; try again a bit later
    setTimeout(parseSentence, 100); // 100ms
    return;
  }

  if ( sentence_input_form == null ) {
    sentence_input_form = document.getElementById( "sentence_input_form" );
  }
  if ( s != null ) {
    sentence_input_form.sentence.value = s;
  }
  var sentence = sentence_input_form.sentence.value;
  var model = null;
  for ( var i = 0; i < sentence_input_form.model.length; ++i ) {
    if ( sentence_input_form.model[ i ].checked ) {
      model = sentence_input_form.model[ i ].value;
    }
  }
  if ( m ) {
    model = m;
  }
  var query_string = cgi_name + "?sentence=" + encodeURIComponent( sentence );
  if ( model ) {
    query_string += "&model=" + encodeURIComponent( model );
  }

  if ( document.all ) {
    // IE
    window.open( query_string, "parse", "", false );
    return;
  }

  putResult( document.createTextNode( "Wait..." ) );

  if ( parse_request == null ) {
    parse_request = createXMLHttpRequest();
  } else {
    parse_request.onreadystatechange = function () {}
    parse_request.abort();
    parse_request = createXMLHttpRequest();  // hack for mozilla's bug
  }

  parse_request.onreadystatechange = updateParseResult;
  parse_request.open( "GET", query_string, true );
  parse_request.setRequestHeader( "Content-Type", "application/x-www-form-urlencoded" );
  parse_request.send( "" );

  return false;
}

function updateParseResult() {
  if ( parse_request && parse_request.readyState == 4 ) {
    if ( ! ( parse_request.responseText && parse_request.status == 200 ) ) {
      putResult( document.createTextNode( "Disconnected from data server." ) );
    } else if ( ! ( parse_request.responseXML && parse_request.responseXML.documentElement ) ) {
      putResult( document.createTextNode( "Server returned Non-XML text." ) );
    } else {
      parse_tree_xml = parse_request.responseXML.documentElement;
      parse_request = null;
      displayParseTree( parse_tree_xml, false );
    }
  }
}

function displayParseTree( xml, is_full ) {

  function rememberNode( id, xml ) {
    var node = {};
//    var id = xml.getAttribute( "id" );
    for ( var i = 0; i < xml.attributes.length; ++i ) {
      node[ xml.attributes[ i ].nodeName ] = xml.attributes[ i ].nodeValue;
    }
    node_list[ id ] = node;
    return node;
  }

  var doc = null;
  doc = document.implementation.createDocument( "", "", null );

  function changeViewButton() {
    var message_div = document.createElement( "div" );
    var view_change_form = document.createElement( "form" );
    var view_change_button = document.createElement( "input" );
    view_change_button.type = "button";
    view_change_button.value = "Click to change the view.";
    view_change_button.onclick = function () { displayParseTree( parse_tree_xml, ! is_full ); }
    view_change_form.appendChild( view_change_button );
    message_div.appendChild( view_change_form );
    return message_div;
  }

  function convertParseTreeRecursive( xml ) {
    if ( xml.nodeType == 3 ) {         // TEXT_NODE
//      return document.createTextNode( xml.nodeValue );
      return null;
    } else if ( xml.nodeType == 1 ) {  // ELEMENT_NODE
      if ( xml.nodeName == "cons" ) {
        // nonterminal node
        var id = xml.getAttribute( "id" );
        rememberNode( id, xml );
        var tree = doc.createElementNS( fs_namespace, "tree" );
        // nonterminal symbol
        var nts = doc.createElementNS( xhtml_namespace, "div" );
        nts.id = id;
        if ( is_full ) {
          nts.appendChild( createFS( id ) );
        } else {
          var cat = xml.getAttribute( "cat" );
          var xcat = xml.getAttribute( "xcat" );
          if ( xcat ) { cat = cat + "-" + xcat; }
          nts.appendChild( doc.createTextNode( cat ) );
        }
        tree.appendChild( nts );
        // recursively apply to child nodes
        var child_nodes = xml.childNodes;
        for ( var i = 0; i < child_nodes.length; ++i ) {
          var dtr = convertParseTreeRecursive( child_nodes[ i ] );
          if ( dtr == null ) {
            // do nothing
          } else if ( dtr.nodeType == 3 ) {  // TEXT_NODE
            tree.appendChild( dtr );
          } else if ( dtr.nodeType == 1 ) {  // ELEMENT_NODE
            var dtr_elem = doc.createElementNS( fs_namespace, "dtr" );
            dtr_elem.appendChild( dtr );
            tree.appendChild( dtr_elem );
          }
        }
        return tree;
      } else if ( xml.nodeName == "tok" ) {
        // terminal node
        var id = xml.getAttribute( "id" );
        rememberNode( id, xml );
        var word_id = "word_" + id;
        rememberNode( word_id, xml );
        var tree = doc.createElementNS( fs_namespace, "tree" );
        // terminal symbol
        var term = doc.createElementNS( xhtml_namespace, "div" );
        term.id = id;
        if ( is_full ) {
          term.appendChild( createFS( id ) );
        } else {
          var pos = xml.getAttribute( "pos" );
          term.appendChild( doc.createTextNode( pos ) );
        }
        tree.appendChild( term );
        var dtr = doc.createElementNS( fs_namespace, "dtr" );
        var word = doc.createElementNS( fs_namespace, "tree" );
        var word_div = doc.createElementNS( xhtml_namespace, "div" );
        word_div.id = word_id;
        word_div.appendChild( doc.createTextNode( xml.firstChild.nodeValue ) );
        word.appendChild( word_div );
        dtr.appendChild( word );
        tree.appendChild( dtr );
        return tree;
      }
    }
    return null;
  }

  node_list = {};
  var top_element = document.createElement( "div" );
  if ( xml.nodeName == "sentence" ) {
    var formatting_div = document.createElement( "div" );
    formatting_div.appendChild( document.createTextNode( "Formatting.  Please wait..." ) );
    putResult( formatting_div );
    top_element.appendChild( document.createElement( "hr" ) );
    var status_div = document.createElement( "div" );
    status_div.appendChild( document.createTextNode( "Parse status: " + xml.getAttribute( "parse_status" ) ) );
    top_element.appendChild( status_div );
    var tree = document.createElement( "div" );
    var is_available = false;
    for ( var i = 0; i < xml.childNodes.length; ++i ) {
      if ( xml.childNodes[ i ].nodeName == "cons" ) {
        var dtr = convertParseTreeRecursive( xml.childNodes[ i ] );
        if ( dtr != null ) {
          tree.appendChild( dtr );
          is_available = true;
        }
      }
    }
    if ( is_available ) {
      // at least one tree was shown
      top_element.appendChild( changeViewButton() );
    }
    doc.appendChild( tree );
    applyXSLT( tree, top_element );
    //top_element.onclick = function () { displayParseTree( parse_tree_xml, ! is_full ); }
  } else if ( xml.nodeName == "error" ) {
    top_element.appendChild( document.createTextNode( "Sorry: " + xml.getAttribute( "message" ) ) );
  } else {
    top_element.appendChild( document.createTextNode( xml.nodeValue ) );
  }

  putResult( top_element );
  for ( var node in node_list ) {
    var elem = document.getElementById( node );
    elem.onmouseover = new Function( "highlightOn(\"" + node + "\");" );
    elem.onmouseout = new Function( "highlightOff(\"" + node + "\");" );
  }
}


// load the XSLT sheet at once
loadXML( xslt_file, function( xslt_doc ) {
  xslt = new XSLTProcessor();
  xslt.importStylesheet( xslt_doc );
});
