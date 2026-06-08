

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>


// Tokens of parser
const char* TIMESCALE="$timescale";
const char* MODULE="$scope module";
const char* FUNCTION="$scope function";
const char* BEGIN="$scope begin";
const char* UP="$upscope";
const char* WIRE="$var wire";
const char* REG="$var reg";
const char* DUMPVARS="$dumpvars";

// svcd cant handle these currently
// TODO: check if gtkwave can :)
const char* INTEGER="$var integer";


// Gotten from file for these "wanted" items
char* items[257];

// This will store the position in the hierarcy
char pos[100][100];
int posi=0;

// This simple storer works well as long as there is not more than
// about 255*255 unique signals, it will store the signals
// for renaming
char* storer[256][256];

// TODO make some hash table or so structure if there is ever
// signal codes that are more than 2 bytes
void storer_init()
{
  int i,j;
  for (i=0;i<256;i++)
    {
      for (j=0;j<256;j++)
	{
	  storer[i][j] = NULL;
	}
    }
}

void storer_destruct()
{
  int i,j;
  for (i=0;i<256;i++)
    {
      for (j=0;j<256;j++)
	{
	  if (storer[i][j]!=NULL) free(storer[i][j]);
	}
    }
}

int has_spc(const char* str)
{
  const char* pek = str;
  while (1)
    {
      if (*pek==' ') return 1;
      if (*pek=='\0') return 0;
      pek++;
    }
}

// Counts chars up until before a space
int strlen_spc(const char* str)
{
  int retval=0;
  while (1)
    {
      if ( (str[retval] == ' ') || (str[retval] == '\0') )
	return retval;
      retval++;
    }
}

void storer_insert(const char* item, const char* value)
{
  assert(strlen_spc(item)<=2);

  // copy and malloc internally...
  char *tmp = (char*) malloc(strlen(value) + 1);
  strcpy(tmp, value);

  storer[item[0]][item[1]]=tmp;
}

const char* storer_find(const char* item)
{
  char ch;
  if (item[1]=='\n') ch=' '; else ch = item[1];
  return storer[item[0]][ch];
}

// New signal names from !,",# etc..
int ticker=33;


// TODO: Some clever way to pre-store the lenghts
int cmp(const char* s1, const char* s2)
{
  // Avoid overrunning to short strings
  if (strlen(s1) > strlen(s2)) return 0;
  return !strncmp(s1, s2, strlen(s1));
}

// Search for the first space and replace with a null
void termspc(char* line)
{
  int i;
  for (i=0;;i++)
    {
      if (line[i]=='\0') return;
      if (line[i]==' ')
	{
	  line[i]='\0';
	  return;
	}
    }
}

char* getkw(char* line, int pos)
{
  int i=0;

  // Simple state machine to pass two items that are space separated
  // in order to end up on the actual token:
  // "$var wire 8 0" Cntr_next [7:0] $end"
  // We have already passed the first two and now we skip two more
  int state = 0;

  while (1)
    {
      char ch=line[i];
      switch(state)
	{
	case 0:
	  {
	    if (ch!=' ') state = 1;
	    if (pos==1) return &line[i];
	    break;
	  }
	case 1:
	  {
	    if (ch==' ') state = 2;
	    break;
	  }
	case 2:
	  {
	    if (ch!=' ') state = 3;
	    if (pos==2) return &line[i];
	    break;
	  }
	case 3:
	  {
	    if (ch==' ') state = 4;
	    break;
	  }
	case 4:
	  {
	    if (ch!=' ')
	      {
		if (pos==3) return &line[i];

		// Far out 
		state = 5;
	      }
	  }
	default:
	  {
	    fprintf(stderr, "Internal error\n");
	    exit(1);
	  }
	}
      i++;
	  
    }
}

int check_hier(char* current)
{
  // Do an accumulation of our position in the hierarchy into a string for comparision
  
  int i,j;
  
  char* pek = current;
  
  // First we create this item as a dot hierarchy for easy compare
  
  for (i=0;i<=posi;i++)
    {
      j=0;
      while (1)
	{
	  if (pos[i][j] == '\0')
	    {
	      if (i<posi)
		*pek++ = '.';
	      break;
	    }
	  else
	    {
	      *pek++ = pos[i][j];
	    }
	  j++;
	}
    }

  *pek++='\0';

  // Then we loop through the list of desired items, if found return 1 else 0
  for (i=0;;i++)
    {
      if (items[i][0] == '\0') break;

      // TODO again here we might benefit from cashing string lengths
      int slc = strlen(current);
      
      if (slc != strlen(items[i])) continue;

      if (!strncmp(current, items[i], slc))
	{
	  return 1;
	}
    }
  return 0;
  
}

void echo(const char* buff)
{
  printf("%s", buff);
}

void check_var(char* buff, const char* type)
{
  // Extract the actual keyword
  char* pek=buff + strlen(type);
  const char *signame = getkw(pek, 2);
  const char *keyword = getkw(pek, 3);
  strcpy(pos[posi], keyword);
  termspc(pos[posi]);

  char current[4096];
  if (check_hier(current))
    {
      char tmps[100];

      // TODO if we need more than about 100 signals
      // we need to make two-byte signal codes
      sprintf(tmps, "%c", ticker++);
      assert(ticker < 126);
      
      storer_insert(signame, tmps);
      
      // First print all until token to substitute
      int i, spccnt;
      
      i=0;
      spccnt=0;
      while (1)
	{
	  printf("%c", buff[i]);
	  if (buff[i] == ' ') spccnt++;
	  i++;
	  if (spccnt == 3) break;
	}

      // !,",#...
      // Now the new signal token
      printf("%s", tmps);

      // Step past the original signal token
      while (1)
	{
	  if (buff[i] == ' ') break;
	  i++;
	}

      // Then just pring finale in verbatim
      printf("%s", &buff[i]);
    }
}

FILE* openfile(const char* name, const char* perm)
{
  FILE* retval=fopen(name, perm);

  if (NULL==retval)
    {
      fprintf(stderr, "Couldn't open file: %s\n", name);
      exit(1);
    }

  return retval;
}

int main(int argc, char *argv[])
{
  FILE* FILEIN=stdin;
  char buff[4096];

  char* timescale="";
  
  if (argc==3)
    timescale=argv[2];
  
  if (argc==2  || argc==3)
    {
      // Read in the list of desired signals
      FILE* desired=openfile(argv[1], "r");
      int i;

      char buff[4096];
      
      for (i=0;i<256;i++)
	{
	  buff[0]='\0';
	  fgets(buff, 4096, desired);
	  int s=strlen(buff);
	  items[i]=(char *) malloc(s);
	  strcpy(items[i], buff);
	  items[i][s-1]='\0';
	  
	  if (feof(desired))
	    {
	      items[i+1]="";
	      break;
	    }
	}
    }
  else
    {
      fprintf(stderr, "Usage: %s <desired signals> [<timescale>]\n", argv[0]);
      exit(1);
    }

  storer_init();

  int in_timescale=0;
  
  while (!feof(FILEIN))
    {

      fgets(buff, 4096, FILEIN);

      if (in_timescale)
	{
	  if (timescale[0]!='\0')
	    {
	      printf("    %s\n", timescale);
	    }
	  else
	    {
	      echo(buff);
	    }
	  in_timescale=0;
	}
      else if (cmp(TIMESCALE, buff))
	{
	  in_timescale=1;
	  echo(buff);
	}
      else if (cmp(MODULE, buff))
	{
	  strcpy(pos[posi], buff+strlen(MODULE)+1);
	  termspc(pos[posi]);
	  posi++;
	  echo(buff);
	}
      else if (cmp(BEGIN, buff))
	{
	  strcpy(pos[posi], buff+strlen(BEGIN)+1);
	  termspc(pos[posi]);
	  posi++;
	  echo(buff);
	}
      else if (cmp(FUNCTION, buff))
	{
	  strcpy(pos[posi], buff+strlen(FUNCTION)+1);
	  termspc(pos[posi]);
	  posi++;
	  echo(buff);
	}
      else if (cmp(UP, buff))
	{
	  posi--;
	  if (posi<0)
	    {
	      fprintf(stderr, "Parse error\n");
	      exit(1);
	    }
	  echo(buff);
	}
      else if (cmp(WIRE, buff))
	{
	  check_var(buff, WIRE);
	}
      else if (cmp(REG, buff))
	{
	  check_var(buff, REG);
	}
      else if (cmp(INTEGER, buff))
	{
	  check_var(buff, INTEGER);
	}
      else if (cmp(DUMPVARS, buff))
	{
	  echo(buff);
	  // Now begins the signal values as such
	  break;
	}
      else
	{
	  echo(buff);
	}
    }

  // With the filtering we only want one hash thingy in
  // consequence
  // TODO: should be the closest before signal change we
  // actually print
  
  int last_was_hash;
  
  while (!feof(FILEIN))
    {
      fgets(buff, 4096, FILEIN);

      if (buff[0] == '#')
	{
	  if (!last_was_hash)
	    printf("%s", buff);
	  last_was_hash=1;
	  continue;
	}
      
      // The file format has some strange simplifications
      // In the case of the signal value having only one
      // char, there is no space on the line
      if (has_spc(buff))
	{
	  const char* pek=buff;
	  while (1)
	    {
	      if (*pek==' ') break;
	      pek++;
	    }
	  pek++;
      
	  // So now pek is at the original signal name...

	  const char* new_signal_name = storer_find(pek);
	  if (new_signal_name!=NULL)
	    {
	      // Echo out the signal value and a space...
	      const char* pek2=buff;
	      while (1)
		{
		  printf("%c", *pek2);
		  if (*pek2==' ') break;
		  pek2++;
		}
	      printf("%s\n", new_signal_name);
	      last_was_hash = 0;
	    }
	}
      else // The two or three character shorthand
	{
	  const char* pek=buff;
	  
	  // Signal name starts on second char always
	  pek++;
      
	  // So now pek is at the original signal name...

	  const char* new_signal_name = storer_find(pek);
	  if (new_signal_name!=NULL)
	    {
	      printf("%c%s\n", buff[0], new_signal_name);
	      last_was_hash = 0;
	    }
	}
      
    }

  storer_destruct();

  // destruct items
  int i;
  for (i=0;i<256;i++)
    {
      if (items[i][0]=='\0') break;
      free(items[i]);
    }
}
