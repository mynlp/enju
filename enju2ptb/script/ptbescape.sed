#!/bin/sed -f
s,/,\\/,g
s/\*/\\*/g
s,\\\\,\\,g
s/{/-LCB-/g
s/}/-RCB-/g
s/(-LRB- {)/(-LRB- -LCB-)/g
s/(-RRB- })/(-RRB- -RCB-)/g
s/(-LRB- ()/(-LRB- -LRB-)/g
s/(-RRB- ))/(-RRB- -RRB-)/g
# s/(error \([.?!]\))/(. \1)/g
# s/(error ,)/(, ,)/g
# s/(error ``)/(`` ``)/g
# s/(error '')/('' '')/g
# s/(error \(;\|:\|--\))/(: \1)/g
