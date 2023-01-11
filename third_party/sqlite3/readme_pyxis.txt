
SQLite 3.3.13
SQLite is a small C library that implements a self-contained, embeddable, zero-configuration SQL database engine.  SQLite is in the public domain and does not require a license (see http://www.sqlite.org/copyright.html for further information).

- From http://www.sqlite.org/download.html
  sqlite-source-3_3_13.zip  The pure C source code for the SQLite library.
  sqlitedll-3_3_13.zip      A DLL of the SQLite library without the TCL bindings.
  sqlite-3_3_13.zip         A command-line program for accessing and modifing SQLite databases.

- Generated .lib and .exp files using this command:
  lib /def:sqlite3.def

Marc Lepage
2007-02-23



Update:

SQLLite 3.7.13

generate in the same way:

 lib /def:sqlite3.def

Idan Shatz
2012-08-20