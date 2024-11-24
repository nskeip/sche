__   __   _        _                _   _               
\ \ / /__| |_     / \   _ __   ___ | |_| |__   ___ _ __ 
 \ V / _ \ __|   / _ \ | '_ \ / _ \| __| '_ \ / _ \ '__|
  | |  __/ |_   / ___ \| | | | (_) | |_| | | |  __/ |   
  |_|\___|\__| /_/   \_\_| |_|\___/ \__|_| |_|\___|_|   
                                                        
          _           __            __  
 ___  ___| |__   ___ / / __ ___   __\ \ 
/ __|/ __| '_ \ / _ \ | '_ ` _ \ / _ \ |
\__ \ (__| | | |  __/ | | | | | |  __/ |
|___/\___|_| |_|\___| |_| |_| |_|\___| |
                     \_\            /_/ 
 ___       _                           _            
|_ _|_ __ | |_ ___ _ __ _ __  _ __ ___| |_ ___ _ __ 
 | || '_ \| __/ _ \ '__| '_ \| '__/ _ \ __/ _ \ '__|
 | || | | | ||  __/ |  | |_) | | |  __/ ||  __/ |   
|___|_| |_|\__\___|_|  | .__/|_|  \___|\__\___|_|   
                       |_|                          

Roadmap:

v0.1
[x] Write a simple parser (without nesting).
[x] Fix the bug that invalidates pointers in Arena. Introduce ChainedArena.
  [x] Switch from Arena to simple memory tracker (as GNU C Library already has Arenas under the hood).
[x] Make a usable calculator.
[x] Add 0xdeadbeef and 0777 :)
[x] Add nesting to the parser.
[x] Add n-variable functions.

v0.2
[x] ReWrite parser.
  [x] Make it more permissive to variable names - closer to racket.
  [x] Remove 0xdeadbeef and 0777.
  [x] Remove memory_tracker.

TODO:

[ ] Running code from a file.
[ ] User defined functions.
[ ] Useful data types:
  [ ] Strings support.
  [ ] Real numbers support.
  [ ] Big numbers support.
