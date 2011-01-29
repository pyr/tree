# tree

tree is a utility that displays an ascii-art formatted tree of a directory
structure. you can use it to get a quick glance of your files. tree is the
equivalent functionnaly of GNU tree which can be found on many recent Linux
systems. tree is much simpler and doesn't provide as many features.

# features

Some interesting features of tree:

* no warnings in lint
* no compile-time warnings, even with aggressive options
* no expensive color support
* no useless HTML output support
* use of fts_ functions

# screenshot

    (pyr@phoenix) tree$ tree
    .
    |-- CVS
    |   |-- Entries
    |   |-- Repository
    |   `-- Root
    |-- Makefile
    |-- foo
    |   `-- bar
    |-- tree.1
    `-- tree.c

    2 directories, 7 files


# download

 The latest release is tree 0.61
 Tree is now in the OpenBSD ports tree.

# manpage

[tree(1)](http://spootnik.org/ipcalc/ipcalc.1.html)
