/* 
   Name: xtraverse.h
   Description:
   Set of functions that implement the xtraverse tool.
   Author: Iliana Avila-Campillo
   Date created:  July 11, 2001
   Last modified: August 28, 2001

*/

/***** libraries *****/

#include <iostream>
#include <fstream>
#include <stdio.h>
#include <string>
#include <assert.h>
#include <errno.h>
// class taken from the web
// for performance monitoring
#include "SysTimer.h"
using namespace std;

// directory and file  manipulation
#include <unistd.h>     
#include <sys/types.h>
#include <sys/dir.h>
#include <sys/param.h>
#include <dirent.h>
#include <sys/stat.h>

// my libraries
#include "pathstack.h"
#include "depthstack.h"

#ifndef XTRAVERSE_H_
#define XTRAVERSE_H_

// global variables



/***** function prototypes *****/

// Non-recursive version of the traversal algorithm
// Also uses the a DirentStack.
// It takes the name of the starting array as an arg.
void traverse(char * start_dir);

// Pushes to "stack" all the directories (dirent *) found in the structure "files"
// that can be opened (have read permissions enabled).
// If it finds files in "files" or directories that can't be opened it outputs 
// their information.
// parent_path: the full path of the directory form which "files" was obtained
// num_files: the number of files in "files" (including directories)
// spaces: the number of spaces that should be printed before the file names (XML format)
// Returns the number of directories pushed into the stack.
int fill_stack_print_files(PathStack * pstack, DepthStack * dstack,struct dirent ** files,char * parent_path,
			   int num_files,int spaces);

// Prints and array of characters as if it was a string
inline int  print_char_array(char * array);

// returns true if the entry's d_name field is not ".", ".."
// or a symbolic link.
// parent_file: the full path of entry's parent directory
// false otherwise
bool  file_select(char * parent_file,struct dirent * entry);

// ouputs "num" spaces
// useful for reflecting the directory
// hierarchy with left offsets (XML format)
inline void output_spaces(int num);

// outputs the XML text for a file
// spaces: offset from the left
// abs_fpath: the absolut path of the file
// stats: the stats of the file that contains size, date, etc;
inline void output_file_xml(int spaces, char * abs_fpath, struct stat * stats);

//outputs the XML text for the directory
// spaces: left offset
// abs_dpath: the absolute path of the directory

inline void output_dir_xml(int spaces, char * abs_dpath);

// these methods were taken from http://lamagra.digibel.org/ezine/core01/perm.html
char * filetype(mode_t mode);
char * fileperms(mode_t mode);

// prints a date without the the newline character that ctime inserts
// NOTE: char * ctime(mode_t) function inserts a newline at the end of the
// char array it returns. This is not correct for the XML format.
// date: date obtained by calling ctime(mode_t)
inline void print_date(char * date);

// prints the <\directory> tags correctly
// given a depth stack that stores all the 
// depths of the directories pushed to the 
// absoulute path stack
// pstack_empty tells us if the path stack is empty or not
// sdir_depth: the depth of the starting directory
inline void print_dir_closing_tags(DepthStack * dstack,bool pstack_empty, int sdir_depth);

// outputs the file information corresponding to "entry"
// and its "stats"
// assumes that file is in the current directory
inline void output_file_info(struct dirent * entry, struct stat * stats);

// gets the current absolut path
// immitates GNU's getwcd:
// it returns an array of the exact 
// full path size, no need to worry about size
// MOST delete THE RETURNED POINTER AFTER USAGE
inline char * gnu_getcwd();

// returns a pointer to the absolute path of 
// file "child" given the absolute path to parent_file
// MUST delete RETURNED ARRAY AFTER USING IT
inline char * get_abs_path(char * parent_file,char * child_file);

// output the usage for the xtraverse tool
void usage(char * args[]);

// depending on the size limit that the user specified
// it either prints the file named "file" or an XLink to this file
// TODO:If the file does not have permissions for reading?
void output_file(char * file);

// it returns the name of a file given its
// full path name terminated by a '\0' character
// MUST delete RETURNED CHAR * AFTER USAGE 
inline char * get_file_name(char * full_path);


// returns the depth of the file described
// by file_full_path.
// the depth of root is zero
inline int get_file_depth(char * file_full_path);


// given two pointers to struct dirent** 
// returns 0 if they are equal, -1 if the first
// is less than the second, and 1 if it is greater

// does not work
inline int sizesort(const void * a, const void * b);



#endif
