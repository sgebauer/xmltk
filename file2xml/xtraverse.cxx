
/* 
   Name: xtraverse.cxx
   Description: Implementation of xtraverse functions, prototyped in 
                xtraverse.h.
   Author: Iliana Avila-Campillo
   Date created:  July 11,2001
   Last modified: August 28,2001

*/

#include "xtraverse.h"


/* 

TODO (not in order of importance).
-Make sure there are no memory leaks
-More options for the user
-XLinks: options for user
-Inline short functions and improve performance in general (print_char_array)
-Probably further modularize code into more files?? 
-DEBUG flag, use standard C/C++ style
-Take out performance print out

*/

/***************************************** main *****************************************************/


#define DEBUG 0
#define SYMB_LINK_DEBUG 0

// a counter to display the time elapsed
int counter = 0;
// temporarily this will be a global variable
SysTimer measure; 

int main(int argC, char * argV[]){
  if(argC == 1){
	usage(argV);
	return 1;
  }
  
  // performance monitoring
  // SysTimer measure;
#if defined(XMLTK_PERFORMANCE)
  if (measure.isRunning()) {
    cout << "Error: timer started out running" << endl;
  }
  measure.start();
#endif

  // arguments:
  // start point directoy  -- default root
  // TODO other arguments:
  // file size to use XLinks: all sizes, no sizes, file extensions  -- default all sizes
  // order of traversal for the directories: alphabetical order, file size, 
  //             creation time (latest first) -- default alphabetical order
 
  // look for options in the command line
  char * file;
  char * startdir;
  // flag that checks if we got the start directory 
  bool got_sd = false;
  ofstream filestr;
  streambuf * psbuf;
  for (int i = 1; i < argC; i++){
	if(strcmp(argV[i],"-s") == 0 && i+1 < argC){
      // get the start directory
      startdir = argV[i+1];
      got_sd = true;    
    }else if(strcmp(argV[i],"-f") == 0 && i+1 < argC){    
      // redirect cout to the file
      file = argV[i+1];
      errno = 0;
      filestr.open(file);
      if(errno != 0){
     	perror("ERROR");
	errno = 0;
      }
      if(filestr.is_open()){
	psbuf = filestr.rdbuf();
	cout.rdbuf(psbuf);
	if(errno != 0){
	  perror("ERROR");
	  errno = 0;
	}
      }else{
	cerr << "ERROR: Could not open file \"";
	print_char_array(file);
	cerr << "\"" << endl;
      }
	break;
    }
    
  }
  
  // we did not get the start directory
  // default to root
  if(!got_sd){
    startdir = "/";
  }

  traverse(startdir);

  // close the file
  filestr.close();

  // performance monitoring
#if defined(XMLTK_PERFORMANCE)
  measure.stop();
	
  cout << endl << "PERFORMANCE: ";
  cout << "Total traversal took " << measure.elapsedTime()
       << " seconds." << endl;
#endif
  return 0;
}

/********************************** function definitions ********************************************/

void traverse(char * start_dir){

  // change to the starting directory
  if(chdir(start_dir) != 0){
    cout << "Could not change to \"";
    print_char_array(start_dir);
    cout << "\"" << endl;
    return;
  }
 
  // initialize the offset spaces for XML format
  int spaces = 0;
  
  // initialize a PathStack so that it
  // contains the start directory
  PathStack * pstack = new PathStack();
  if(pstack == NULL){
    cout << "Failed to allocate a PathStack." << endl;  
  }  
  // push the start directory's absolut path
  char * start_apath = gnu_getcwd();
  if(start_apath == NULL){
    cout << endl << " Could not gnu_getwcd " << endl;
    return;
  }   
  // initialize a depth stack 
  // this stack is used so that the <\directory> tag can be
  // printed on the right spot for each directory
  DepthStack * dstack = new DepthStack();
  dstack->push(get_file_depth(start_dir));

  pstack->push(start_apath);
  // get the depth (relative to root) of the starting directory
  // useful for the offset spaces for XML format
  int sdir_depth = get_file_depth(start_apath);
  int num_dirs = 0;
  // non-recursive traverse
  while(!pstack->is_empty()){
    // performance
    counter++;
    // every 50 directories, print time
    if(counter % 50 == 0){
      cout << endl << "  *****  TIME " << measure.lapTime() << "   *****" << endl;
    }
    
    // pop directory and print it's info
    char * popped = pstack->pop();
    if(DEBUG){
      cout << " Popped \"";
      print_char_array(popped);
      cout << "\""<< endl;
    }
    // print the XML for the directory
    spaces = get_file_depth(popped) - sdir_depth;
    output_dir_xml(spaces, popped);
    
    // push subdirectories of popped
   
    struct dirent ** files;
    // TODO: here use a function for ordering, depending on user choice
    int num_files = scandir(popped,&files,NULL,sizesort);
    if(num_files < 0){
      // either the directory cannot be read or the 
      // internal malloc failed
      if(DEBUG){
	cout << "  scandir failed   " << endl;
      }
    }
    // fill stack (push subdirectories)
    if(num_files > 0){
      num_dirs = fill_stack_print_files(pstack,dstack,files,popped,num_files,spaces+1);
      if(num_dirs == 0){
	// print the closing tags
	print_dir_closing_tags(dstack,pstack->is_empty(),sdir_depth);
      }
    }
    // make sure that we don't have any mem. leaks
    delete popped;
  }
  
  delete pstack;
  delete dstack;
}

int fill_stack_print_files(PathStack * pstack, DepthStack * dstack, struct dirent ** files, char * parent_path,
			   int num_files,int spaces){
  // if the number of files in the directory is 0, return
  if(num_files < 0){
    return 0;
  }
  int num_dirs = 0;
  if(DEBUG){
    cout << " Pushing.  Num files = " << num_files << endl;
  }
  // structure for the stats of a file
  struct stat stats;
  // push the directories
  for(int i = 0; i < num_files; i++){
    // make sure this file is not ".",".." or a symb. link
    if(file_select(parent_path,files[i])){
      // check to see if this is a directory
      char * abs_fpath = get_abs_path(parent_path,files[i]->d_name);
      if(abs_fpath == NULL){
	cout << endl << "Could not get_abs_path" << endl;
	return num_dirs;
      }
      if(lstat(abs_fpath,&stats) != 0){
	if(DEBUG){
	  cout << "  fsp: Could not get lstats of file" << endl;
	}
	return num_dirs;
      }
      if(S_ISDIR(stats.st_mode)!= 0){
	// this is a directory!
	pstack->push(abs_fpath);
	dstack->push(get_file_depth(abs_fpath));

	if(DEBUG){
	  cout << "  Pushed \"";
	  print_char_array(abs_fpath);
	  cout << "\"" << endl;
	}
	num_dirs++;
      } else{
	// this is not a directory, output its info
	output_file_xml(spaces,abs_fpath,&stats);
      }
        
    }
    
  } // end of for loop

  if(DEBUG){
    cout << " Finished pushing. Num dirs = " << num_dirs << endl;
  }

  return num_dirs;
}

bool file_select(char * parent_path,struct dirent * entry){ 
  struct stat buf;
  // get stats so that we can make sure that this is not 
  // a symbolic link
  char * file = get_abs_path(parent_path,entry->d_name);
  if(lstat(file,&buf) != 0){  
    if(DEBUG){
      cout << " fs: Could not get stats of file" << endl;
    }
    return false;
  }
  // take out later
  if( SYMB_LINK_DEBUG && S_ISLNK(buf.st_mode) != 0){
    cout << "  SYMBOLIC LINK: ";
    print_char_array(entry->d_name);
    cout << " ";
  }
  // if the file is a symbolic link,
  // or "." or ".." return false
  if ( (S_ISLNK(buf.st_mode) != 0)      ||
       (strcmp(entry->d_name, ".") == 0) ||
       (strcmp(entry->d_name, "..") == 0) )
    return false;
  else 
    return true;
}

inline void output_dir_xml(int spaces, char * abs_dpath){
  output_spaces(spaces);
  char * name = get_file_name(abs_dpath);
  cout << "<directory>" << endl;
  output_spaces(spaces+1);
  cout << "<name>";
  print_char_array(name);    // ? print full path?
  cout << "</name>" << endl;
}

// outputs the XML for one file using "spaces" for left offset
inline void output_file_xml(int spaces, char * abs_fpath,struct stat * stats){
  
  output_spaces(spaces);
  cout << "<file>" << endl;
  
  // name of the file
  output_spaces(spaces+1);
  cout << "<name>";
  print_char_array(get_file_name(abs_fpath));
  cout << "</name>" << endl;
  
  // xlink
  output_spaces(spaces+1);
  cout << "<filelink xlink:type=\"simple\"" << endl;
  output_spaces(spaces+1);
  // I am not sure if I need the "/" after localhost
  // the absolute path starts at root, so we will have
  // something like: file://localhost//u11/iliana/.....etc
  // shoulf I do: file://localhost/u11/iliana/....etc???
  cout << "          xlink:href=\"file:/localhost";
  // depending on user's option, specify behavior attributes like show and actuate
  print_char_array(abs_fpath);
  cout << "\">" << endl;
  // here, depending on option, output file or not
  // need to understand Makoto's path tool
  output_spaces(spaces+1);
  cout << "</filelink>" << endl;
  
  // full path name
  output_spaces(spaces+1);
  cout <<"<path>";
  print_char_array(abs_fpath);
  cout <<"</path>"<< endl;
  
  // size
  output_spaces(spaces+1);
  cout <<"<size>";
  cout << stats->st_size;
  cout <<"</size>"<< endl;  
  
  // persmissions
  output_spaces(spaces+1);
  cout <<"<permissions>";
  print_char_array(fileperms(stats->st_mode));
  cout << "</permissions>"<<endl;
  
  // type of file
  output_spaces(spaces+1);
  cout <<"<type>";
  print_char_array(filetype(stats->st_mode));
  cout << "</type>"<<endl;
  
  // user id
  output_spaces(spaces+1);
  cout <<"<userid>";
  cout << stats->st_uid;
  cout << "</userid>"<<endl;
  
  // group id
  output_spaces(spaces+1);
  cout <<"<groupid>";
  cout <<stats->st_gid;
  cout << "</groupid>"<<endl;
  
  // last access
  output_spaces(spaces+1);
  cout <<"<lastAccess>";
  char * last_access = ctime(&stats->st_atime);
  print_date(last_access);
  cout << "</lastAccess>"<< endl;
  
  // last modifications
  output_spaces(spaces+1);
  cout <<"<lastModification>";
  char * last_mod = ctime(&stats->st_mtime);
  print_date(last_mod);
  cout << "</lastModification>"<<endl;
  
  output_spaces(spaces);
  cout << "</file>"<< endl;
 
  
}
// this method was taken from http://lamagra.digibel.org/ezine/core01/perm.html
char * filetype(mode_t mode)
{
   switch(mode & S_IFMT) {
             case S_IFREG:
                      return("regular file");
	     case S_IFDIR:
	 	      return("directory");
	     case S_IFCHR:
		      return("character device");
	     case S_IFBLK:
                      return("block device");
             case S_IFLNK:
                      return("symbolic link");
             case S_IFIFO:
		      return("fifo");
             case S_IFSOCK:
		      return("socket");
    }
        return(NULL);
}




// this method was taken from http://lamagra.digibel.org/ezine/core01/perm.html
char * fileperms(mode_t mode)
   {
      int i;
      char *p;
      static char perms[10];

      p = perms;
      strcpy(perms, "---------");

      for(i=0;i<3;i++) {
           if(mode &(S_IREAD>>i*3))
             *++p='r';

           if(mode &(S_IWRITE>>i*3))
                 *++p='w';

           if(mode &(S_IEXEC>>i*3))
                 *++p='x';
         }

        if((mode & S_ISUID))
           perms[2] = 's';

        if((mode & S_ISGID))
           perms[5] = 's';

        if((mode & S_ISVTX))
           perms[8] = 't';

      return(perms);
}



// how this works is not immediately obvious, but believe me, it does have a logic behind it
inline void print_dir_closing_tags(DepthStack* dstack,bool pstack_empty,int sdir_depth){
  
  int prev_depth = -1;
  int depth = 0;
  // print the closing tag(s) for the directories
  while(!dstack->is_empty()){
    depth = dstack->top();
    if(depth == prev_depth && !pstack_empty){
      break;
    }
    dstack->pop();
    output_spaces(depth - sdir_depth);
    cout << "</directory>" << endl;
    prev_depth = depth;
    
  }
}
// for now it does some very ugly error checking
// change later
// this method was causing the process to abort
void output_file(char *  file){
  int length;
  char * buffer;
  //open the file for reading
  ifstream stream;
  errno = 0;
  stream.open(file,ifstream::in);
  // do not take this out, bug without it!!!
  if(errno !=0){
    if(DEBUG)
      perror("ERROR0:");
    return;
  }
  errno = 0;
    
  if(!stream.is_open()){
    return;
  } 

  // get lenght of file
  errno = 0;
  stream.seekg(0, ios::end);
  if(errno !=0){
    if(DEBUG)
      perror("ERROR1:");
    stream.close();
    return;
  }
  errno = 0;
  length = stream.tellg();
  if(errno !=0){
    if(DEBUG)
      perror("ERROR2:");
    stream.close();
    return;
  }
  errno = 0;
  stream.seekg(0, ios::beg);
  if(errno !=0){
    if(DEBUG)
      perror("ERROR3:");
    stream.close();
    return;
  }
  errno = 0;
  
  // allocate memory for buffer
  buffer = new char [length];
  if(buffer == NULL){
    cout << " Cold not allocate memory " << endl;
    if(errno !=0)
      if(DEBUG)
	perror("ERROR4:");
    errno = 0; 
    stream.close();
    return;
  }

  // read data as a block
  stream.read(buffer,length);
  if(errno !=0){
    if(DEBUG)
      perror("ERROR5:");
    stream.close();
    delete [] buffer;
    return;
  }
  errno = 0;
  stream.close();
  if(errno !=0){
    if(DEBUG)
      perror("ERROR6:");
    delete [] buffer;
    return;
  }
  errno = 0;
  // write the file
  //cout.write(buffer,length);
  //perror("ERROR:");
  delete[] buffer;
  if(errno !=0){
    if(DEBUG)
      perror("ERROR7:");
  }
  errno = 0;
}

// not used
inline void output_file_info(struct dirent * entry, struct stat * stats){
  print_char_array(entry->d_name);
}

inline void print_date(char * date){
  int size = strlen(date);
  char new_date[size];
  strncpy(new_date,date,size-1);
  *(new_date+(size-1)) = '\0';
  print_char_array(new_date);
}

// this was taken from http://www.netppl.fi/~pp/glibc21/libc_14.html#SEC270
// and modified for C++
inline char * gnu_getcwd (){
  int size = 100;
  char * buffer = new char[100];
  // make sure we have a long enough array
  // to store the full path
  while (1){
    char *value = getcwd(buffer, size);
    if (value != 0){
      return buffer;
    }
    size *= 2;
    delete buffer;
    buffer = new char[size];
  }
}

inline char * get_abs_path(char * parent_file,char * child_file){
  int pf_size = strlen(parent_file) + 1;
  int cf_size = strlen(child_file) + 1;
  char * abs_path = new char[pf_size + cf_size + 1];
  if(abs_path == NULL){
    return NULL;
  }
  
  if(strcmp(parent_file,"/") == 0){
    // the parent is root, no need for an extra
    // "/" between parent and child
    if(strcpy(abs_path,parent_file) == NULL ||
	 strcat(abs_path,child_file) == NULL){
      cout<< endl << "String manipulation error" << endl;
      return NULL;
    }
  }else{
    // parent is not root, need "/"
    if(strcpy(abs_path,parent_file) == NULL ||
       strcat(abs_path,"/") == NULL         ||
       strcat(abs_path,child_file) == NULL){
      cout<< endl << "String manipulation error" << endl;
      return NULL;
    }
  }
 
  return abs_path;
}

inline char * get_file_name(char * full_path){
  int fp_size = strlen(full_path);
  // point to the last letter in
  // the full path
  char * cp = full_path + fp_size;
  // decrement the cp pointer until the 
  // first '/' is found
  int fn_size = 0;
  while(*cp != '/'){
    cp--;
    fn_size++;   
  }
  char * file_name = new char[fn_size + 1];
  strcpy(file_name,cp+1);
  return file_name;
}

inline int  print_char_array(char * array){
  int str_size=0;
  while(*array != '\0'){
    cout << *array;
    *array++;
    str_size++;
  }
  return str_size;
}

inline void output_spaces(int num){
  for(int i = 0; i < num; i++){
    cout << "  ";
  }
}

void usage(char * args []){
  // more to come here
  // change to cout
  cerr << endl;
  cerr << "NAME" << endl << "file2xml" << endl << endl;
  cerr << "SYNOPSIS" << endl << endl;
  cerr << "file2xml [-s start directory name] ";
  cerr << " [-f file to direct output] ";
  cerr << endl << endl;
  cerr << "DESCRIPTION" << endl << endl;
  cerr << "This command traverses the file system starting at" << endl;
  cerr << "the given start directory (or root if skipped), and" << endl;
  cerr << "directs the XML view if the hierarchy to the file given" << endl;
  cerr << "as an argument (or standard output if skipped)."<< endl;
}

inline int get_file_depth(char * file_full_path){
  // root is at depth zero
  if(strcmp(file_full_path,"/") == 0){
    // root is at depth 0
    return 0;
  }
  // count the number of '/' seen
  char * cp = file_full_path;
  int depth = 0;
  while(*cp != '\0'){
    if(*cp == '/'){
      depth++;
    }
    cp++;
  }
  return depth;
}


// does not work
inline int sizesort(const void * a, const void * b){
  struct dirent ** a_dirent = (struct dirent **)a;
  struct dirent ** b_dirent = (struct dirent **)b;
  struct stat a_stats;
  struct stat b_stats;
  char * a_path = (*a_dirent)->d_name;
  char * b_path = (*b_dirent)->d_name;

  /*  
  print_char_array(a_path);
  cout << " ";
  print_char_array(b_path);
  cout << " ";
  */

  // lstat does not work
  // needs the full path?????
  if(lstat(a_path,&a_stats) != 0 && lstat(b_path,&b_stats) != 0){
	/*    
    cout << "IN SIZESORT: returning" << a_stats.st_size - b_stats.st_size << endl;
    cout << "a size: " << a_stats.st_size << " b size: " << b_stats.st_size << endl;
	*/
    if(a_stats.st_size - b_stats.st_size < 0)
      return -1;
    if(a_stats.st_size - b_stats.st_size > 0)
      return 1;
  }  
  //  cout << "IN SIZESORT: returning 0" << endl;
  return 0;
  
}
