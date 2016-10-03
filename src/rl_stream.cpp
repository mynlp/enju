//----------------------------------------------------------------------------
//  Input stream through readline library
//
//                                         Programmed by Tossy-2 / T.Yoshino
//                                        mailto: tossy-2@is.s.u-tokyo.ac.jp
//----------------------------------------------------------------------------

#include "rl_stream.h"
#include "in.h"
#include <algorithm>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <string>
#if defined (HAVE_UNISTD_H)
# include <unistd.h>
#endif

namespace lilfes {

using std::cin;
using std::copy;
using std::cout;
using std::flush;
using std::string;
using std::streambuf;
using std::strlen;

rl_istream rl_istream::rl_stdin;
int rl_streambuf::ref_count = 0;
}

#ifdef WITH_READLINE

#include <readline/readline.h>
#include <readline/history.h>

namespace lilfes {


//----------------------------------------------------------------------------
// Readline Typedefs
//----------------------------------------------------------------------------

// #if HAVE_RL_COMMAND_FUNC_T
// typedef rl_command_func_t *CommandFunction;
// typedef rl_completion_func_t *CompFunction;
// typedef rl_compentry_func_t *EntryFunction;
// #else
// // BSD
// typedef Function *CommandFunction;
// typedef CPPFunction *CompFunction;
// typedef Function *EntryFunction;
// #endif

//----------------------------------------------------------------------------
// Input stream class definition
//----------------------------------------------------------------------------

// Constructor for rl_streambuf
rl_streambuf::rl_streambuf(const char *prompt) : streambuf(), pszPrompt( prompt ), buffer_size( DEFAULT_BUFFER_SIZE), isEof( false )
{
  buffer = new char[ buffer_size ];
  if ( ref_count == 0 ) {
    //rlin.tie( &cout );
    rl_initialize();
    using_history();
    //read_history(LILFES_HISTORY_NAME);
  
    // completers are removed by yusuke
    /*
      rl_bind_key('?', (CommandFunction) insert_query);
      rl_bind_key(',', (CommandFunction) insert_larrow);
      rl_bind_key('.', (CommandFunction) insert_rarrow);
      rl_bind_key(':', (CommandFunction) insert_goal);
      rl_attempted_completion_function =  (CompFunction) lilfes_completer;
      rl_completion_entry_function = (EntryFunction) lilfes_generator;
    */
  }
  ++ref_count;
}

rl_streambuf::~rl_streambuf()
{
  delete [] buffer;
  --ref_count;
  if ( ref_count == 0 ) {
    /*
#ifdef LILFES_HISTORY_NAME
#ifdef LILFES_HISTORY_LIMIT
    if(!write_history(LILFES_HISTORY_NAME))
      history_truncate_file(LILFES_HISTORY_NAME, LILFES_HISTORY_LIMIT);
#else
    write_history(LILFES_HISTORY_NAME);
#endif
#endif
    */
  }
}

// buffer underflow -- eating a line through readline
int rl_streambuf::underflow()
{
  if(gptr() >= egptr()){
    const char* prompt = isatty(0) ? pszPrompt : "";
    char* ptr = readline((char *) prompt);
    if(ptr){
      int l = strlen(ptr);
      if ( l > 0 ) add_history(ptr); // add to history list
      check_buffer( l + 1 );
      strncpy( buffer, ptr, l );
      buffer[ l ] = '\n';
      setg( buffer, buffer, buffer + l + 1);
      // new buffer is prepared
    } else {
      isEof = true;
      return EOF; // reached EOF
    }
  }
  
  return sbumpc();
}

//----------------------------------------------------------------------------
// Readline completer
//----------------------------------------------------------------------------

// Completer
/*
enum complete_domain {
  DOM_TOP = 0,
  DOM_MAX
} domain;
char *wordlist[DOM_MAX][4] =
{
  { "?-", NULL },
};

char **lilfes_completer(const char *text, int start, int end)
{
  // Determine domain
  domain = DOM_TOP;
  
  if(lexer->GetParsing())
    domain = DOM_MAX;
  else{
    if(rl_point == 0) goto set_dom;
    for(int i = 0; i < rl_point; i++)
      if(rl_line_buffer[i] != ' '){
        domain = DOM_MAX; break;
      }
  }

set_dom:
  return NULL;
}

char *lilfes_generator(const char *text, int state)
{
  static char **ptr;
  
  if(domain < DOM_MAX){
    if(!state) ptr = wordlist[domain];
    
    if(text[0])
      while(*ptr &&
	    (strlen(*ptr) >= strlen(text) &&
	     memcmp(*ptr, text, strlen(text)) == 0)) ptr++;
    if(*ptr == NULL) return NULL;
    char *newbuf = (char *) malloc(strlen(*ptr) + 1);
    return strcpy(newbuf, *ptr++);
  }else
    //return rl_filename_completion_function(text, state);
    return NULL;
}

//----------------------------------------------------------------------------
// Keysequence functions
//----------------------------------------------------------------------------

int insert_query(int, int)
{
  return rl_insert_text((char *) (rl_point == 0 ? "?- " : "?"));
}

int insert_larrow(int, int)
{
  if(rl_point > 0 && rl_line_buffer[rl_point - 1] == '.'){
    rl_delete_text(rl_point, rl_point - 1); rl_point--;
    if(rl_point > 0 && rl_line_buffer[rl_point - 1] != ' ')
      return rl_insert_text(" <- ");
    else
      return rl_insert_text("<- ");
  }else return rl_insert_text(",");
}

int insert_rarrow(int, int)
{
  if(rl_point > 0 && rl_line_buffer[rl_point - 1] == ','){
    rl_delete_text(rl_point, rl_point - 1); rl_point--;
    if(rl_point > 0 && rl_line_buffer[rl_point - 1] != ' ')
      return rl_insert_text(" -> ");
    else
      return rl_insert_text("-> ");
  }else return rl_insert_text(".");
}

int insert_goal(int, int)
{
  if(rl_point == 0 || rl_line_buffer[rl_point - 1] == ' ')
    return rl_insert_text(":- ");
  else
    return rl_insert_text(":");
}
*/

} // namespace lilfes

//////////////////////////////////////////////////////////////////////

#else // WITH_READLINE

// without readline support

namespace lilfes {


rl_streambuf::rl_streambuf( const char* prompt ) : pszPrompt( prompt ), buffer_size( DEFAULT_BUFFER_SIZE ), isEof( false ) {
  buffer = new char[ buffer_size ];
  memset(buffer, '\0', buffer_size);
  ++ref_count;
}

rl_streambuf::~rl_streambuf()
{
  delete [] buffer;
  --ref_count;
}

// buffer underflow -- eating a line through readline
int rl_streambuf::underflow()
{
  if(gptr() >= egptr()){
    cout << pszPrompt << flush;
    string line;
    getline( cin, line );
    if ( cin ) {
      int l = line.size();
      check_buffer( l + 1 );
      line.copy( buffer, l );
      buffer[ l ] = '\n';
      setg( buffer, buffer, buffer + l + 1 );
    } else {
      isEof = true;
      return EOF; // reached EOF
    }
  }
#ifndef LILFES_WIN_
  return sbumpc();
#else
  return sgetc();
#endif
}

}

#endif // WITH_READLINE
